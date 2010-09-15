#include "win_def.h"
#include "win_net.h"

#include "Header.h"
#include <windows.h>

//externs
VirtualConsole *pVCon;
HWND hWnd, hOpWnd, hConWnd;
WCHAR temp[MAX_PATH];
uint cBlinkNext;
DWORD WindowMode;
gSettings gSet;

//local variables
HANDLE hChildProcess;

bool isWndNotFSMaximized;
bool isShowConsole;
bool isNotFullDrag;

WCHAR Title[MAX_PATH];

POINT cwShift, cwConsoleShift;
COORD srctWindowLast;
uint cBlinkShift;

#define WM_TRAYNOTIFY WM_USER+1
#define ID_SETTINGS 0xABCD
#define ID_HELP 0xABCE
#define ID_TOTRAY 0xABCF

#define SC_RESTORE_SECRET 0x0000f122
#define SC_MAXIMIZE_SECRET 0x0000f032

class		WinConsole {
	HWND	m_hwnd;
public:
	~WinConsole() {
		::FreeConsole();
	}
	WinConsole() {
		CheckAPI(::AllocConsole());
		CheckAPI((m_hwnd = ::GetConsoleWindow()));

	}
	UINT		cp() const {
		return	::GetConsoleCP();
	}
	void		cp(UINT cp = CP_UTF16le) const {
		CheckAPI(::SetConsoleOutputCP(cp));
	}
	RECT		rect() const {
		RECT	Result;
		CheckAPI(::GetWindowRect(m_hwnd, &Result));
		return	Result;
	}
	void		enable(bool in = true) const {
		::EnableWindow(m_hwnd, in);
	}
	void		show(int in) const {
		CheckAPI(::ShowWindow(m_hwnd, in));
	}
	operator	HWND() const {
		return	m_hwnd;
	}
};


class		TrayIcon {
	NOTIFYICONDATA IconData;

public:
	bool isWindowInTray;

	void HideWindowToTray() {
		GetWindowText(hWnd, IconData.szTip, 127);
		Shell_NotifyIcon(NIM_ADD, &IconData);
		ShowWindow(hWnd, SW_HIDE);
		isWindowInTray = true;
	}

	void RestoreWindowFromTray() {
		ShowWindow(hWnd, SW_SHOW);
		SetForegroundWindow(hWnd);
		Shell_NotifyIcon(NIM_DELETE, &IconData);
		EnableMenuItem(GetSystemMenu(hWnd, false), ID_TOTRAY, MF_BYCOMMAND | MF_ENABLED);
		isWindowInTray = false;
	}

	void LoadIcon(HWND inWnd, int inIconResource) {
		IconData.cbSize = sizeof(NOTIFYICONDATA);
		IconData.uID = 1;
		IconData.hWnd = inWnd;
		IconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		IconData.uCallbackMessage = WM_TRAYNOTIFY;
		IconData.hIcon = (HICON)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(inIconResource), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	}

	void Delete() {
		Shell_NotifyIcon(NIM_DELETE, &IconData);
	}

} Icon;



void GetCWShift(HWND inWnd, POINT *outShift) {
	RECT cRect, wRect;
	GetClientRect(inWnd, &cRect);
	GetWindowRect(inWnd, &wRect);
	outShift->x = wRect.right  - wRect.left - cRect.right;
	outShift->y = wRect.bottom - wRect.top  - cRect.bottom;
}

void ShowSysmenu(HWND Wnd, HWND Owner, int x, int y) {
	bool iconic = IsIconic(Wnd);
	bool zoomed = IsZoomed(Wnd);
	bool visible = IsWindowVisible(Wnd);
	int style = GetWindowLong(Wnd, GWL_STYLE);

	HMENU systemMenu = GetSystemMenu(Wnd, false);
	if (!systemMenu)
		return;

	EnableMenuItem(systemMenu, SC_RESTORE, MF_BYCOMMAND | (iconic || zoomed ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(systemMenu, SC_MOVE, MF_BYCOMMAND | (!(iconic || zoomed) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(systemMenu, SC_SIZE, MF_BYCOMMAND | (!(iconic || zoomed) && (style & WS_SIZEBOX) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(systemMenu, SC_MINIMIZE, MF_BYCOMMAND | (!iconic && (style & WS_MINIMIZEBOX) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(systemMenu, SC_MAXIMIZE, MF_BYCOMMAND | (!zoomed && (style & WS_MAXIMIZEBOX) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(systemMenu, ID_TOTRAY, MF_BYCOMMAND | (visible ? MF_ENABLED : MF_GRAYED));

	SendMessage(Wnd, WM_INITMENU, (WPARAM)systemMenu, 0);
	SendMessage(Wnd, WM_INITMENUPOPUP, (WPARAM)systemMenu, MAKELPARAM(0, true));
	SetActiveWindow(Owner);

	int command = TrackPopupMenu(systemMenu, TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, x, y, 0, Owner, null_ptr);

	if (Icon.isWindowInTray)
		switch (command) {
			case SC_RESTORE:
			case SC_MOVE:
			case SC_SIZE:
			case SC_MINIMIZE:
			case SC_MAXIMIZE:
				SendMessage(Wnd, WM_TRAYNOTIFY, 0, WM_LBUTTONDOWN);
				break;
		}

	if (command)
		PostMessage(Wnd, WM_SYSCOMMAND, (WPARAM)command, 0);
}

HFONT CreateFontIndirectMy(LOGFONT *inFont) {
	DeleteObject(pVCon->hFont2);

	int width = gSet.FontSizeX2 ? gSet.FontSizeX2 : inFont->lfWidth;
	pVCon->hFont2 = CreateFont(abs(inFont->lfHeight), abs(width), 0, 0, FW_NORMAL,
							   0, 0, 0, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, 0, pVCon->LogFont2.lfFaceName);

	return CreateFontIndirect(inFont);
}

void SyncWindowToConsole() {
	RECT clR, wndR;
	GetClientRect(hWnd, &clR);
	GetWindowRect(hWnd, &wndR);
	MoveWindow(hWnd, wndR.left, wndR.top, pVCon->Width + (wndR.right - wndR.left - clR.right), pVCon->Height + (wndR.bottom - wndR.top - clR.bottom), 1);
}

void SyncConsoleToWindow() {
	COORD srctWindow;
	RECT pRect;
	GetClientRect(hWnd, &pRect);

	srctWindow.X = (pRect.right - pRect.left)  / pVCon->LogFont.lfWidth;
	srctWindow.Y = (pRect.bottom - pRect.top) / pVCon->LogFont.lfHeight;

	if (!gSet.isFullScreen && !IsZoomed(hWnd)) {
		gSet.wndWidth = srctWindow.X;
		wsprintf(temp, _T("%i"), gSet.wndWidth);
		SetDlgItemText(hOpWnd, tWndWidth, temp);

		gSet.wndHeight = srctWindow.Y;
		wsprintf(temp, _T("%i"), gSet.wndHeight);
		SetDlgItemText(hOpWnd, tWndHeight, temp);
	}

//	MoveWindow(hConWnd, 0, 0, 1, 1, 0);
	SetConsoleScreenBufferSize(pVCon->hConOut(), srctWindow);
//	MoveWindow(hConWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 0);
}

bool SetWindowMode(uint inMode) {
	static RECT wndNotFS;
	switch (inMode) {
		case rNormal:
		case rMaximized:
			if (gSet.isFullScreen) {
				SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
				SetWindowPos(hWnd, HWND_TOP, wndNotFS.left, wndNotFS.top, wndNotFS.right - wndNotFS.left, wndNotFS.bottom - wndNotFS.top, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
			}
			gSet.isFullScreen = false;
			SendMessage(hWnd, WM_SYSCOMMAND, inMode == rNormal ? SC_RESTORE : SC_MAXIMIZE, 0);
			break;

		case rFullScreen:
			if (!gSet.isFullScreen) {
				gSet.isFullScreen = true;
				isWndNotFSMaximized = IsZoomed(hWnd);
				if (isWndNotFSMaximized)
					SendMessage(hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);

				GetWindowRect(hWnd, &wndNotFS);

				SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_VISIBLE);
				SetWindowPos(hWnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED | SWP_SHOWWINDOW);

				CheckRadioButton(hOpWnd, rNormal, rFullScreen, rFullScreen);
			}
			break;
	}

	if (inMode == rNormal) {
		EnableWindow(GetDlgItem(hOpWnd, tWndWidth), true);
		EnableWindow(GetDlgItem(hOpWnd, tWndHeight), true);

	} else {
		EnableWindow(GetDlgItem(hOpWnd, tWndWidth), false);
		EnableWindow(GetDlgItem(hOpWnd, tWndHeight), false);

	}
	SyncConsoleToWindow();
	return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	switch (messg) {
		case WM_PAINT: {
			if (*Title != '{') {
				wchar_t *a = wcsstr(Title, L"PictureView by ");
				if (a && wcschr(a, ':')) {
					if (messg) result = DefWindowProc(hWnd, messg, wParam, lParam);
					break;
				}
			}

			PAINTSTRUCT ps;
			HDC hDc = BeginPaint(hWnd, &ps);

			RECT rect;
			HBRUSH hBrush = CreateSolidBrush(pVCon->Colors[0]);
			SelectObject(hDc, hBrush);
			GetClientRect(hWnd, &rect);

			rect.left = pVCon->Width;
			rect.top = 0;
			FillRect(hDc, &rect, hBrush);

			rect.left = 0;
			rect.top = pVCon->Height;
			rect.right = pVCon->Width;
			FillRect(hDc, &rect, hBrush);

			DeleteObject(hBrush);

			BitBlt(hDc, 0, 0, pVCon->Width, pVCon->Height, pVCon->hDC, 0, 0, SRCCOPY);
			EndPaint(hWnd, &ps);
			break;
		}
		case WM_TIMER: {
			HWND foreWnd = GetForegroundWindow();
			if (!isShowConsole) {
				if (foreWnd == hConWnd)
					SetForegroundWindow(hWnd);
				if (IsWindowVisible(hConWnd))
					ShowWindow(hConWnd, SW_HIDE);
			}

			if (cBlinkNext++ >= cBlinkShift) {
				cBlinkNext = 0;
				if (foreWnd == hWnd || foreWnd == hOpWnd)
					// switch cursor
					pVCon->Cursor.isVisible = !pVCon->Cursor.isVisible;
				else
					// turn cursor off
					pVCon->Cursor.isVisible = false;
			}

			DWORD ExitCode;
			if (GetExitCodeProcess(hChildProcess, &ExitCode))
				if (ExitCode != STILL_ACTIVE) {
					DestroyWindow(hWnd);
					break;
				}

			WCHAR nCapt[0x200];
			GetWindowText(hConWnd, nCapt, 0x200);

			if (wcscmp(Title, nCapt)) {
				wcscpy(Title, nCapt);
				SetWindowText(hWnd, Title);
			}

			static bool isPiewUpdate = true;
			if (Title[0] != '{') {
				wchar_t *a = wcsstr(Title, _T("PictureView by "));
				if (a && wcschr(a, ':') || Title[0] == 'T' || Title[0] == 1060) {
					isPiewUpdate = true;
					if (pVCon->Update(false))
						InvalidateRect(hWnd, 0, 0);
					break;
				}
			}

			if (pVCon->Update(false)) {
				RECT wndR;
				GetClientRect(hWnd, &wndR);
				if ((wndR.right - wndR.left) / pVCon->LogFont.lfWidth  != pVCon->TextWidth ||
						(wndR.bottom - wndR.top) / pVCon->LogFont.lfHeight != pVCon->TextHeight) {
					if (!gSet.isFullScreen && !IsZoomed(hWnd))
						SyncWindowToConsole();
					else
						SyncConsoleToWindow();
				}

				InvalidateRect(hWnd, null_ptr, FALSE);
			}

			if (isPiewUpdate) {
				isPiewUpdate = false;
				SyncConsoleToWindow();
				InvalidateRect(hWnd, 0, 0);
			}
			break;
		}

		case WM_SIZING: {
			COORD srctWindow;
			RECT *pRect = (RECT*)lParam;

			srctWindow.X = (pRect->right - pRect->left - cwShift.x)  / pVCon->LogFont.lfWidth;
			srctWindow.Y = (pRect->bottom - pRect->top - cwShift.y) / pVCon->LogFont.lfHeight;

			if ((srctWindowLast.X != srctWindow.X || srctWindowLast.Y != srctWindow.Y) && !isNotFullDrag) {
				if (!gSet.isFullScreen && !IsZoomed(hWnd)) {
					gSet.wndWidth = srctWindow.X;
					wsprintf(temp, _T("%i"), gSet.wndWidth);
					SetDlgItemText(hOpWnd, tWndWidth, temp);

					gSet.wndHeight = srctWindow.Y;
					wsprintf(temp, _T("%i"), gSet.wndHeight);
					SetDlgItemText(hOpWnd, tWndHeight, temp);
				}
				//MoveWindow(hConWnd, 0, 0, 4+ srctWindow.X * pVCon->LogFont.lfWidth + cwConsoleShift.x, srctWindow.Y * pVCon->LogFont.lfHeight + cwConsoleShift.y, 0);
				MoveWindow(hConWnd, 0, 0, 100, 100, 0);
				SetConsoleScreenBufferSize(pVCon->hConOut(), srctWindow);
				MoveWindow(hConWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 0);
				//MoveWindow(hConWnd, 0, 0, srctWindow.X * pVCon->LogFont.lfWidth + cwConsoleShift.x + 20, srctWindow.Y * pVCon->LogFont.lfHeight + cwConsoleShift.y + 20, 0);
				srctWindowLast = srctWindow;
			}

			switch (wParam) {
				case WMSZ_RIGHT:
				case WMSZ_BOTTOM:
				case WMSZ_BOTTOMRIGHT:
					pRect->right =  srctWindow.X * pVCon->LogFont.lfWidth  + cwShift.x + pRect->left;
					pRect->bottom = srctWindow.Y * pVCon->LogFont.lfHeight + cwShift.y + pRect->top;
					break;
				case WMSZ_LEFT:
				case WMSZ_TOP:
				case WMSZ_TOPLEFT:
					pRect->left =  pRect->right - srctWindow.X * pVCon->LogFont.lfWidth  - cwShift.x;
					pRect->top  = pRect->bottom - srctWindow.Y * pVCon->LogFont.lfHeight - cwShift.y;
					break;
				case WMSZ_TOPRIGHT:
					pRect->right =  srctWindow.X * pVCon->LogFont.lfWidth  + cwShift.x + pRect->left;
					pRect->top  = pRect->bottom - srctWindow.Y * pVCon->LogFont.lfHeight - cwShift.y;
					break;
				case WMSZ_BOTTOMLEFT:
					pRect->left =  pRect->right - srctWindow.X * pVCon->LogFont.lfWidth  - cwShift.x;
					pRect->bottom = srctWindow.Y * pVCon->LogFont.lfHeight + cwShift.y + pRect->top;
					break;
			}
			result = true;
			break;
		}
		case WM_SIZE:
			if (isNotFullDrag) {
				COORD srctWindow;
				RECT pRect = {0, 0, LOWORD(lParam), HIWORD(lParam)};

				srctWindow.X = (pRect.right - pRect.left)  / pVCon->LogFont.lfWidth;
				srctWindow.Y = (pRect.bottom - pRect.top) / pVCon->LogFont.lfHeight;

				if ((srctWindowLast.X != srctWindow.X || srctWindowLast.Y != srctWindow.Y)) {
					if (!gSet.isFullScreen && !IsZoomed(hWnd)) {
						gSet.wndWidth = srctWindow.X;
						wsprintf(temp, _T("%i"), gSet.wndWidth);
						SetDlgItemText(hOpWnd, tWndWidth, temp);

						gSet.wndHeight = srctWindow.Y;
						wsprintf(temp, _T("%i"), gSet.wndHeight);
						SetDlgItemText(hOpWnd, tWndHeight, temp);
					}
					//MoveWindow(hConWnd, 0, 0, 4+ srctWindow.X * pVCon->LogFont.lfWidth + cwConsoleShift.x, srctWindow.Y * pVCon->LogFont.lfHeight + cwConsoleShift.y, 0);
					MoveWindow(hConWnd, 0, 0, 1, 1, 0);
					SetConsoleScreenBufferSize(pVCon->hConOut(), srctWindow);
					MoveWindow(hConWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 0);
					//MoveWindow(hConWnd, 0, 0, srctWindow.X * pVCon->LogFont.lfWidth + cwConsoleShift.x + 20, srctWindow.Y * pVCon->LogFont.lfHeight + cwConsoleShift.y + 20, 0);
					srctWindowLast = srctWindow;
				}
			}

			{
				static bool wPrevSizeMax = false;
				if ((wParam == SIZE_MAXIMIZED || (wParam == SIZE_RESTORED && wPrevSizeMax)) && hConWnd) {
					if (wParam == SIZE_MAXIMIZED) wPrevSizeMax = true;
					else wPrevSizeMax = false;
					RECT pRect;
					GetWindowRect(hWnd, &pRect);
					pRect.right = LOWORD(lParam) + pRect.left + cwShift.x;
					pRect.bottom = HIWORD(lParam) + pRect.top + cwShift.y;

					SendMessage(hWnd, WM_SIZING, WMSZ_TOP, (LPARAM)&pRect);
					SendMessage(hWnd, WM_SIZING, WMSZ_RIGHT, (LPARAM)&pRect);
				}
			}
			break;

		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:

		case WM_MOUSEWHEEL:

		case WM_ACTIVATE:
		case WM_ACTIVATEAPP:
		case WM_KILLFOCUS:
		case WM_SETFOCUS:
			if (messg == WM_KEYDOWN && wParam == VK_SPACE && isPressed(VK_CONTROL) && isPressed(VK_LWIN) && isPressed(VK_MENU)) {
				if (!IsWindowVisible(hConWnd)) {
					isShowConsole = true;
					ShowWindow(hConWnd, SW_SHOWNORMAL);
					SetParent(hConWnd, 0);
					EnableWindow(hConWnd, true);
				} else {
					isShowConsole = false;
					ShowWindow(hConWnd, SW_HIDE);
					SetParent(hConWnd, hWnd);
					EnableWindow(hConWnd, false);
				}
				break;
			}

			{
				static bool isSkipNextAltUp = false;
				if (messg == WM_SYSKEYDOWN && wParam == VK_RETURN && lParam & 29) {
					if (isPressed(VK_SHIFT))
						break;

					if (!gSet.isFullScreen)
						SetWindowMode(rFullScreen);
					else
						SetWindowMode(isWndNotFSMaximized ? rMaximized : rNormal);

					isSkipNextAltUp = true;
				} else if (messg == WM_SYSKEYDOWN && wParam == VK_SPACE && lParam & 29 && !isPressed(VK_SHIFT)) {
					RECT rect, cRect;
					GetWindowRect(hWnd, &rect);
					GetClientRect(hWnd, &cRect);
					WINDOWINFO wInfo;
					GetWindowInfo(hWnd, &wInfo);
					ShowSysmenu(hWnd, hWnd, rect.right - cRect.right - wInfo.cxWindowBorders, rect.bottom - cRect.bottom - wInfo.cyWindowBorders);
				} else if (messg == WM_KEYUP && wParam == VK_MENU && isSkipNextAltUp) isSkipNextAltUp = false;
				else if (messg == WM_SYSKEYDOWN && wParam == VK_F9 && lParam & 29 && !isPressed(VK_SHIFT))
					SetWindowMode(IsZoomed(hWnd) ? rNormal : rMaximized);
				else
					PostMessage(hConWnd, messg, wParam, lParam);
			}
			break;

		case WM_MOUSEMOVE:

		case WM_RBUTTONDOWN:

		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:

		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
			if (messg == WM_LBUTTONDOWN)
				SetCapture(hWnd);
			else
				if (messg == WM_LBUTTONUP)
					ReleaseCapture();

			{
				RECT conRect;
				GetClientRect(hConWnd, &conRect);
				WORD newX = MulDiv(GET_X_LPARAM(lParam), conRect.right, klMax<uint>(1, pVCon->Width)),
							newY = MulDiv(GET_Y_LPARAM(lParam), conRect.bottom, klMax<uint>(1, pVCon->Height));

				if (gSet.isRClickSendKey && messg == WM_RBUTTONDOWN) {
					WCHAR *BrF = _tcschr(Title, '{'), *BrS = _tcschr(Title, '}'), *Slash = _tcschr(Title, '\\');
					if (BrF && BrS && Slash && BrF == Title && (Slash == Title + 1 || Slash == Title + 3)) {
						PostMessage(hConWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(newX, newY));
						PostMessage(hConWnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(newX + 1, newY));
						PostMessage(hConWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(newX, newY));

						//PostMessage(hConWnd, messg, wParam, MAKELPARAM( newX, newY ));
						//PostMessage(hConWnd, messg, wParam, MAKELPARAM( newX, newY ));
						pVCon->Update(true);
						InvalidateRect(hWnd, null_ptr, FALSE);
						//Sleep(100);
						/*POINT cPos;			GetCursorPos(&cPos);
						RECT conRect;		GetWindowRect(hConWnd, &conRect);
						WINDOWINFO wInfo;	GetWindowInfo(hConWnd, &wInfo);

						SetCursorPos(newX + conRect.left + cwConsoleShift.x - wInfo.cxWindowBorders,
									 newY + conRect.top  + cwConsoleShift.y - wInfo.cyWindowBorders);
						Sleep(100);
						SetCursorPos(cPos.x, cPos.y);*/
						PostMessage(hConWnd, WM_KEYDOWN, VK_APPS, 0);
						break;
					}
				}
				PostMessage(hConWnd, messg == WM_RBUTTONDBLCLK ? WM_RBUTTONDOWN : messg, wParam, MAKELPARAM(newX, newY));
			}
			break;

		case WM_CLOSE:
			Icon.Delete();
			SendMessage(hConWnd, WM_CLOSE, 0, 0);
			break;

		case WM_CREATE:
			Icon.LoadIcon(hWnd, IDI_ICON1);
			break;

		case WM_SYSCOMMAND:
			switch (LOWORD(wParam)) {
				case ID_SETTINGS:
					DialogBox((HINSTANCE)GetModuleHandle(null_ptr), MAKEINTRESOURCE(IDD_DIALOG1), 0, wndOpProc);
					break;
				case ID_HELP:
					MessageBoxA(hOpWnd, pHelp, "About ConEmu...", MB_ICONQUESTION);
					break;
				case ID_TOTRAY:
					Icon.HideWindowToTray();
					break;
			}

			switch (wParam) {
				case SC_MAXIMIZE_SECRET:
					SetWindowMode(rMaximized);
					break;
				case SC_RESTORE_SECRET:
					SetWindowMode(rNormal);
					break;
				case SC_CLOSE:
					Icon.Delete();
					SendMessage(hConWnd, WM_CLOSE, 0, 0);
					break;

				case SC_MAXIMIZE:
					if (wParam == SC_MAXIMIZE)
						CheckRadioButton(hOpWnd, rNormal, rFullScreen, rMaximized);
				case SC_RESTORE:
					if (wParam == SC_RESTORE)
						CheckRadioButton(hOpWnd, rNormal, rFullScreen, rNormal);

				default:
					if (wParam != 0xF100) {
						PostMessage(hConWnd, messg, wParam, lParam);
						result = DefWindowProc(hWnd, messg, wParam, lParam);
					}
			}
			break;

		case WM_NCRBUTTONUP:
			Icon.HideWindowToTray();
			break;

		case WM_TRAYNOTIFY:
			switch (lParam) {
				case WM_LBUTTONUP:
					Icon.RestoreWindowFromTray();
					break;
				case WM_RBUTTONUP: {
					POINT mPos;
					GetCursorPos(&mPos);
					SetForegroundWindow(hWnd);
					ShowSysmenu(hWnd, hWnd, mPos.x, mPos.y);
					PostMessage(hWnd, WM_NULL, 0, 0);
				}
				break;
			}
			break;


		case WM_DESTROY:
			Icon.Delete();
			PostQuitMessage(0);
			break;

		case WM_INPUTLANGCHANGE:
		case WM_INPUTLANGCHANGEREQUEST:
		case WM_IME_NOTIFY:
			PostMessage(hConWnd, messg, wParam, lParam);
		default:
			if (messg) result = DefWindowProc(hWnd, messg, wParam, lParam);
	}
	return result;
}

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType) {
	return (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT ? true : false);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int /*nCmdShow*/) {
#ifndef _DEBUG
	klInit();
#endif

	pVCon = null_ptr;
	PCWSTR	szClassName = _T("VirtualConsoleClass");
	WCHAR	cmdLine[MAX_PATH], *curCommand;

	cBlinkShift = GetCaretBlinkTime() / 15;

	// Allocating console
	WinConsole	con;
	hConWnd = con;
	con.show(SW_HIDE);
	SetConsoleSizeTo(con, 4, 6);

	// Creating window and VirtualConsole
	RECT cRect = con.rect();

	pVCon = new VirtualConsole(0);
	LoadSettings();

	const WNDCLASS wc = {CS_DBLCLKS, WndProc, 0, 0, hInstance, LoadIcon(GetModuleHandle(null_ptr), (LPCTSTR)IDI_ICON1), LoadCursor(null_ptr, IDC_ARROW), null_ptr /*(HBRUSH)COLOR_BACKGROUND*/, null_ptr, szClassName};// | CS_DROPSHADOW
	if (!RegisterClass(&wc))
		return -1;

	hWnd = CreateWindow(szClassName, 0, WS_OVERLAPPEDWINDOW, gSet.wndX, gSet.wndY, cRect.right - cRect.left - 4, cRect.bottom - cRect.top - 4, null_ptr, null_ptr, (HINSTANCE)hInstance, null_ptr);
	if (!hWnd)
		return -1;

	HMENU hwndMain = GetSystemMenu(hWnd, FALSE);
	InsertMenu(hwndMain, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_TOTRAY, _T("Hide to &tray"));
	InsertMenu(hwndMain, 0, MF_BYPOSITION, MF_SEPARATOR, 0);
	InsertMenu(hwndMain, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_HELP, _T("&Help"));
	InsertMenu(hwndMain, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_SETTINGS, _T("S&ettings..."));

	::SetParent(con, hWnd);
	con.enable(false);

//------------------------------------------------------------------------
///| Processing command line |////////////////////////////////////////////
//------------------------------------------------------------------------

	STARTUPINFO stInf;
	GetStartupInfo(&stInf);

	_tcscpy(cmdLine, GetCommandLine());

	{
		WCHAR *cmdNew = _tcsstr(cmdLine, _T("/cmd"));
		if (cmdNew) {
			*cmdNew = 0;
			cmdNew += 5;
			_tcscpy(gSet.Cmd, cmdNew);
		}
	}

	curCommand = cmdLine;

	{
#ifdef KL_MEM
		uint params = klSplitCommandLine(curCommand);
#else
		uint params;
		klSplitCommandLine(curCommand, &params);
#endif

		if (params < 2)
			curCommand = null_ptr;

		for (uint i = 1; i < params; i++) {
			curCommand += _tcslen(curCommand) + 1;
			if (!klstricmp(curCommand, _T("/ct")) || !klstricmp(curCommand, _T("/cleartype")))
				pVCon->LogFont.lfQuality = CLEARTYPE_NATURAL_QUALITY;
			else if (!klstricmp(curCommand, _T("/font")) && i + 1 < params)
				_tcscpy(pVCon->LogFont.lfFaceName, curCommand += _tcslen(curCommand) + 1);
			else if (!klstricmp(curCommand, _T("/size")) && i + 1 < params)
				pVCon->LogFont.lfHeight = klatoi(curCommand += _tcslen(curCommand) + 1);
			else if (!klstricmp(curCommand, _T("/fs")))
				WindowMode = rFullScreen;
			else if (!klstricmp(curCommand, _T("/?"))) {
				MessageBoxA(null_ptr, pHelp, "About ConEmu...", MB_ICONQUESTION);
				return false;
			}
		}
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	SetHandleInformation(GetStdHandle(STD_INPUT_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	SetHandleInformation(GetStdHandle(STD_OUTPUT_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	SetHandleInformation(GetStdHandle(STD_ERROR_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

	if (*gSet.Cmd) {
		if (!CreateProcess(null_ptr, gSet.Cmd, null_ptr, null_ptr, FALSE, 0, null_ptr, null_ptr, &si, &pi)) {
			MBoxA("Cannot execute the command.");
			return -1;
		}
	} else {
		_tcscpy(temp, _T("far"));
		if (!CreateProcess(null_ptr, temp, null_ptr, null_ptr, FALSE, 0, null_ptr, null_ptr, &si, &pi)) {
			_tcscpy(temp, _T("cmd"));
			if (!CreateProcess(null_ptr, temp, null_ptr, null_ptr, FALSE, 0, null_ptr, null_ptr, &si, &pi)) {
				MBoxA("Cannot start Far or Cmd.");
				return -1;
			}
		}
	}

	CloseHandle(pi.hThread);
	hChildProcess = pi.hProcess;

//------------------------------------------------------------------------
///| Misc |///////////////////////////////////////////////////////////////
//------------------------------------------------------------------------

	SetConsoleCtrlHandler((PHANDLER_ROUTINE)HandlerRoutine, true);
	SetTimer(hWnd, 0, 10, null_ptr);

	Registry reg;
	if (reg.OpenKey(_T("Control Panel\\Desktop"), KEY_READ)) {
		reg.Load(_T("DragFullWindows"), &isNotFullDrag);
		reg.CloseKey();
	}

	SetForegroundWindow(hWnd);

	SetParent(hWnd, GetParent(GetShellWindow()));
	GetCWShift(hWnd, &cwShift);
	GetCWShift(hConWnd, &cwConsoleShift);
	pVCon->InitDC();
	SyncWindowToConsole();

	SetWindowMode(WindowMode);

	MSG lpMsg;
	while (GetMessage(&lpMsg, null_ptr, 0, 0)) {
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}
	KillTimer(hWnd, 0);
	delete pVCon;
	CloseHandle(hChildProcess);
	return 0;
}
