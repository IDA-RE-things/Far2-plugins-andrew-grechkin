#ifndef HEADER_INCLUDED
#define HEADER_INCLUDED

#define MBox(rt) (int)MessageBox(NULL, rt, Title, MB_SYSTEMMODAL | MB_ICONINFORMATION)
#define MBoxA(rt) (int)MessageBoxA(NULL, rt, "Information", MB_SYSTEMMODAL | MB_ICONINFORMATION)
#define isMeForeground() (GetForegroundWindow() == hWnd || GetForegroundWindow() == hOpWnd)
#define isPressed(inp) HIBYTE(GetKeyState(inp))

#define GET_X_LPARAM(inPx) ((int)(short)LOWORD(inPx))
#define GET_Y_LPARAM(inPy) ((int)(short)HIWORD(inPy))

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif

#ifdef KL_MEM
#include "c:\\lang\\kl.h"
#else
#include "kl_parts.h"
#endif

#include "resource.h"

//------------------------------------------------------------------------
///| Code optimizing |////////////////////////////////////////////////////
//------------------------------------------------------------------------

//#include <intrin.h>
inline void DisplayLastError() {
	TCHAR out[200];
	DWORD dw = GetLastError();
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
	wsprintf(out, _T("Error code ''%d'':\n%s"), dw, lpMsgBuf);
	MessageBox(0, out, _T("Error occurred"), MB_SYSTEMMODAL | MB_ICONERROR);
}

//------------------------------------------------------------------------
///| Registry |///////////////////////////////////////////////////////////
//------------------------------------------------------------------------

class Registry {

public:
	HKEY regMy;
	bool OpenKey(HKEY inHKEY, const TCHAR *regPath, uint access) {
		bool res = false;
		if (access == KEY_READ)
			res = RegOpenKeyEx(inHKEY, regPath, 0, KEY_READ, &regMy) == ERROR_SUCCESS;
		else
			res = RegCreateKeyEx(inHKEY, regPath, 0, NULL, 0, access, 0, &regMy, 0) == ERROR_SUCCESS;
		return res;
	}
	bool OpenKey(const TCHAR *regPath, uint access) {
		return OpenKey(HKEY_CURRENT_USER, regPath, access);
	}
	void CloseKey() {
		RegCloseKey(regMy);
	}

	template <class T> void Save(const TCHAR *regKey, T value) {
		RegSetValueEx(regMy, regKey, NULL, REG_BINARY, (LPBYTE)(&value), sizeof(T));
	}
	void Save(const TCHAR *regKey, const TCHAR *value) {
		RegSetValueEx(regMy, regKey, NULL, REG_SZ, (LPBYTE)(value), _tcslen(value) * sizeof(TCHAR));
	}
	void Save(const TCHAR *regKey, TCHAR *value) {
		Save(regKey, (const TCHAR *)value);
	}

	template <class T> bool Load(const TCHAR *regKey, T *value) {
		DWORD len = sizeof(T);
		if (RegQueryValueEx(regMy, regKey, NULL, NULL, (LPBYTE)(value), &len) == ERROR_SUCCESS)
			return true;
		return false;
	}
	bool Load(const TCHAR *regKey, TCHAR *value) {
		DWORD len = MAX_PATH * sizeof(TCHAR);
		if (RegQueryValueEx(regMy, regKey, NULL, NULL, (LPBYTE)(value), &len) == ERROR_SUCCESS)
			return true;
		return false;
	}
};

//------------------------------------------------------------------------
///| Global variables |///////////////////////////////////////////////////
//------------------------------------------------------------------------

struct VirtualConsole {
	bool IsForceUpdate;
	uint TextWidth, TextHeight;
	uint Width, Height;
	LOGFONT LogFont, LogFont2;
	COLORREF Colors[0x10];

	struct {
		bool isVisible;
		bool isVisiblePrev;
		short x;
		short y;
		COLORREF foreColor;
		COLORREF bgColor;
		BYTE foreColorNum;
		TCHAR ch[2];
	} Cursor;

	HANDLE  hConOut_;
	HANDLE  hConOut();
	HDC     hDC, hBgDc;
	HBITMAP hBitmap, hBgBitmap;
	SIZE	bgBmp;
	HFONT   hFont, hFont2, hSelectedFont;

	TCHAR *ConChar;
	WORD  *ConAttr;

	VirtualConsole(HANDLE hConsoleOutput = NULL);
	~VirtualConsole();

	bool InitFont(void);
	bool InitDC(void);
	void Free(bool bFreeFont = true);
	bool Update(bool isForce = false);
	void SelectFont(HFONT hNew);
};

const char pHelp[] = "\
Console emulation program.\n\
By default this program launches \"Far.exe\" from the same directory it is in.\n\
\n\
Command line switches:\n\
/? - This help screen.\n\
/ct - Clear Type anti-aliasing.\n\
/fs - Full screen mode.\n\
/font <fontname> - Specify the font name.\n\
/size <fontsize> - Specify the font size.\n\
/cmd <commandline> - Command line to start. This must be the last used switch.\n\
\n\
Command line examples:\n\
ConEmu.exe /ct /font \"Lucida Console\" /size 16 /cmd far.exe \"c:\\1 2\\\"\n\
\n\
(c) 2006-2008, Zoin.\n\
Thanks to NightRoman for drawing process optimization and other fixes.\n\
(based on console emulator by SEt)";

extern VirtualConsole *pVCon;

struct gSettings {
	TCHAR Cmd[MAX_PATH], pBgImage[MAX_PATH];

	DWORD FontSizeX;
	DWORD FontSizeX2;
	bool isShowBgImage, isBackgroundImageValid;
	bool isFullScreen;
	bool isFixFarBorders;
	bool isCursorV;
	bool isCursorColor;
	bool isRClickSendKey;
	bool isForceMonospace;
	bool isConMan;
	DWORD wndWidth, wndHeight;
	int wndX, wndY;
	u8 bgImageDarker;
};

extern gSettings gSet;

extern HWND hWnd, hOpWnd, hConWnd;
extern TCHAR temp[MAX_PATH];
extern uint cBlinkNext;
extern DWORD WindowMode;

bool LoadImageFrom(TCHAR *inPath);
BOOL CALLBACK wndOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);

bool SetWindowMode(uint inMode);
HFONT CreateFontIndirectMy(LOGFONT *inFont);
void SyncWindowToConsole();
void SyncConsoleToWindow();
void LoadSettings();

void SetConsoleSizeTo(HWND inConWnd, int inSizeX, int inSizeY);

#endif
