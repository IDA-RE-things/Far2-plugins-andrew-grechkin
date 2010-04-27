/**
	fblock: format block
	Allow to format selected block of text in FAR`s internal editor

	© 2010 Andrew Grechkin

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
#include "win_def.h"

#include "../../far/far_helper.hpp"

#define defPluginKeyName  L"\\FBlock2"
#define CenterMenu(title,bottom,help,keys,retcode,items,num) \
    psi.Menu(psi.ModuleNumber,-1,-1,0, FMENU_WRAPMODE | FMENU_AUTOHIGHLIGHT, title,bottom,help,keys,retcode,items,num)

#define ShowWarning(msgs,num,help) psi.Message(psi.ModuleNumber, FMSG_WARNING,help,msgs,num,1)
#define ShowQuestion(msgs,num,help) psi.Message(psi.ModuleNumber, 0,help,msgs,num,2)

enum {
	msgTemplate = 5,
	msgMargin, msgLeftMargin, msgRightMargin, msgParagraph,
	msgAlignment, msgLeft, msgRight, msgCenter, msgFullJustify, msgForceJustify,
	msgParagraphPerLine, msgSeparateParagraphs, msgKeepEmpty, msgCatchParagraphs,
	msgError1, msgError2, msgError3,
	msgTemplate2,
	msgDelete1, msgDelete2
};

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions 	fsf;

struct		formattingParams {
	WCHAR		templateName[32];
	WCHAR		startParagraphs[32];
	size_t		leftMargin;
	size_t		rightMargin;
	size_t		paragraphIndent;
	size_t		formattingType;
	bool		paragraphPerLine;
	bool		separateParagraphs;
	bool		keepEmpty;
	bool		catchParagraphs;

	formattingParams() {
		reset();
	}

	void		reset() {
		Copy(templateName, L"Default", sizeofa(templateName));
		Copy(startParagraphs, L"   ", sizeofa(startParagraphs));
		leftMargin = 1;
		paragraphIndent = 4;
		rightMargin = 78;
		formattingType = msgFullJustify;
		paragraphPerLine = false;
		separateParagraphs = false;
		keepEmpty = false;
		catchParagraphs = false;
	}
	bool		check() {
		if (leftMargin && paragraphIndent
				&& rightMargin > paragraphIndent
				&& rightMargin > leftMargin
				&& formattingType >= msgLeft
				&& formattingType <= msgForceJustify
		   )
			return	true;
		reset();
		return	false;
	}

	bool		load(int templateNumber) {
		HKEY	hKey;
		WCHAR	strBuf[512];

		Copy(strBuf, psi.RootKey, sizeofa(strBuf));
		Cat(strBuf, defPluginKeyName, sizeofa(strBuf));
		reset();
		if (RegOpenKeyExW(HKEY_CURRENT_USER, strBuf, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
			return	false;

		DWORD size = sizeof(*this);
		int retCode = RegQueryValueExW(hKey, fsf.itoa(templateNumber, strBuf, 10), 0, NULL, (BYTE*)this, &size);
		RegCloseKey(hKey);
		templateName[sizeofa(templateName) - 1] = L'\0';
		return ((retCode == ERROR_SUCCESS) && check());
	}
	bool		save(int templateNumber) {
		if (!check())
			return	false;

		HKEY	hKey;
		WCHAR	strBuf[512];
		DWORD	ignore;

		Copy(strBuf, psi.RootKey, sizeofa(strBuf));
		Cat(strBuf, defPluginKeyName, sizeofa(strBuf));

		if (::RegCreateKeyExW(HKEY_CURRENT_USER, strBuf, 0, NULL, 0, KEY_WRITE, NULL, &hKey, &ignore) != ERROR_SUCCESS)
			return	false;
		::RegSetValueExW(hKey, fsf.itoa(templateNumber, strBuf, 10), 0, REG_BINARY, (BYTE*)this, sizeof(*this));
		::RegCloseKey(hKey);
		return	true;
	}

	void		ToDialog(const InitDialogItem *dialog, FarDialogItem * farDialog, size_t i) {
		while (--i) {
			if ((farDialog[i].Type == DI_RADIOBUTTON) && ((PCWSTR)formattingType == dialog[i].Data))
				farDialog[i].Selected = true;
			switch ((size_t)dialog[i].Data) {
				case msgLeftMargin:
//					FARitoa(leftMargin,	farDialog[i+1].Data, 10);
					break;
				case msgRightMargin:
//					FARitoa(rightMargin,    farDialog[i+1].Data, 10);
					break;
				case msgParagraph:
//					FARitoa(paragraphIndent, farDialog[i+1].Data, 10);
					break;
				case msgParagraphPerLine:
					farDialog[i].Selected = paragraphPerLine;
					break;
				case msgSeparateParagraphs:
					farDialog[i].Selected = separateParagraphs;
					break;
				case msgKeepEmpty:
					farDialog[i].Selected = keepEmpty;
					break;
				case msgCatchParagraphs:
					farDialog[i].Selected = catchParagraphs;
//					Copy((PWSTR)farDialog[i+1].PtrData, startParagraphs, sizeofa(startParagraphs));
					break;
				case -2:
//					Copy((PWSTR)farDialog[i].PtrData, templateName, sizeofa(templateName));
					break;
			}
		}
	}
	void		FromDialog(InitDialogItem * dialog, HANDLE hDlg, size_t i) {
		while (--i) {
			if ((dialog[i].Type == DI_RADIOBUTTON) && GetCheck(hDlg, i)) {
				formattingType = (size_t)dialog[i].Data;
			}

			switch ((size_t)dialog[i].Data) {
				case msgLeftMargin:
					leftMargin         = fsf.atoi(GetDataPtr(hDlg, i + 1));
					break;
				case msgRightMargin:
					rightMargin        = fsf.atoi(GetDataPtr(hDlg, i + 1));
					break;
				case msgParagraph:
					paragraphIndent    = fsf.atoi(GetDataPtr(hDlg, i + 1));
					break;
				case msgParagraphPerLine:
					paragraphPerLine   = GetCheck(hDlg, i);
					break;
				case msgSeparateParagraphs:
					separateParagraphs = GetCheck(hDlg, i);
					break;
				case msgKeepEmpty:
					keepEmpty          = GetCheck(hDlg, i);
					break;
				case msgCatchParagraphs:
					catchParagraphs    = GetCheck(hDlg, i);
					Copy(startParagraphs, GetDataPtr(hDlg, i + 1), sizeofa(startParagraphs));
					break;
				case -2:
					Copy(templateName, GetDataPtr(hDlg, i), sizeofa(templateName));
					break;
			}
		}
	}

	static bool	del(int templateNumber) {
		HKEY	hKey;
		WCHAR	strBuf[512];

		Copy(strBuf, psi.RootKey, sizeof(strBuf));
		Cat(strBuf, defPluginKeyName, sizeof(strBuf));
		if (RegOpenKeyExW(HKEY_CURRENT_USER, strBuf, 0,
						  KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) return false;
		int retCode = RegDeleteValueW(hKey, fsf.itoa(templateNumber, strBuf, 10));
		RegCloseKey(hKey);
		return (retCode == ERROR_SUCCESS);
	}
};

///======================================================================================= Templates
/*
bool			CallTemplateDialog(formattingParams * params) {

#define TEMPLATEDLGWIDTH	(51)
#define TEMPLATEDLGHEIGHT	(24)

	struct InitDialogItem Items[] = {
		{DI_DOUBLEBOX,  3, 1, TEMPLATEDLGWIDTH - 4, TEMPLATEDLGHEIGHT - 2, 0, 0, 0, 0, GetMsg(msgTemplate)},
		{DI_EDIT,       5, 2, 45, 0, 1, 0, 0,        0, L"" },
		{DI_TEXT,       0, 3, 0, 0, 0, 0, DIF_SEPARATOR | DIF_BOXCOLOR, 0, L""},
		{DI_TEXT,       5, 4, 0, 0, 0, 0, 0,        0, GetMsg(msgLeftMargin) },
		{DI_EDIT,      20, 4, 23, 0, 0, 0, 0,        0, L"" },
		{DI_TEXT,       5, 5, 0, 0, 0, 0, 0,        0, GetMsg(msgParagraph) },
		{DI_EDIT,      20, 5, 23, 0, 0, 0, 0,        0, L"" },
		{DI_TEXT,       5, 6, 0, 0, 0, 0, 0,        0, GetMsg(msgRightMargin) },
		{DI_EDIT,      20, 6, 23, 0, 0, 0, 0,        0, L"" },
		{DI_TEXT,       0, 7, 0, 0, 0, 0, DIF_SEPARATOR | DIF_BOXCOLOR, 0, L"" },
		{DI_TEXT,       5, 8, 0, 0, 0, 0, 0,        0, GetMsg(msgAlignment) },
		{DI_RADIOBUTTON, 7, 9, 0, 0, 0, 0, DIF_GROUP, 0, GetMsg(msgLeft) },
		{DI_RADIOBUTTON, 7, 10, 0, 0, 0, 0, 0,        0, GetMsg(msgRight) },
		{DI_RADIOBUTTON, 7, 11, 0, 0, 0, 0, 0,        0, GetMsg(msgCenter) },
		{DI_RADIOBUTTON, 7, 12, 0, 0, 0, 0, 0,        0, GetMsg(msgFullJustify) },
		{DI_RADIOBUTTON, 7, 13, 0, 0, 0, 0, 0,        0, GetMsg(msgForceJustify) },
		{DI_TEXT,       0, 14, 0, 0, 0, 0, DIF_SEPARATOR | DIF_BOXCOLOR, 0, L""},
		{DI_CHECKBOX,   7, 15, 0, 0, 0, 0, 0,        0, GetMsg(msgParagraphPerLine) },
		{DI_CHECKBOX,   7, 16, 0, 0, 0, 0, 0,        0, GetMsg(msgSeparateParagraphs) },
		{DI_CHECKBOX,   7, 17, 0, 0, 0, 0, 0,        0, GetMsg(msgKeepEmpty) },
		{DI_CHECKBOX,   7, 18, 0, 0, 0, 0, 0,        0, GetMsg(msgCatchParagraphs) },
		{DI_EDIT,      11, 19, 20, 0, 0, 0, 0,        0, L"" },
		{DI_TEXT,       0, 20, 0, 0, 0, 0, DIF_SEPARATOR | DIF_BOXCOLOR, 0, L""},
		{DI_BUTTON,     0, 21, 0, 0, 0, 0, DIF_CENTERGROUP, 1, GetMsg(txtBtnOk) },
		{DI_BUTTON,     0, 21, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(txtBtnCancel) }
	};

	FarDialogItem	FarItems[sizeofa(Items)];

//	ConvertDialog(templateDialog, farTemplateDialog, sizeofa(Items));
	InitDialogItems(Items, FarItems, sizeofa(Items));
//	ParamsToDialog(templateDialog, FarItems, sizeofa(Items), params);
	HANDLE hDlg = psi.DialogInit(psi.ModuleNumber, -1, -1, TEMPLATEDLGWIDTH, TEMPLATEDLGHEIGHT, L"Contents", FarItems, sizeofa(Items), 0, 0, NULL, 0);
	if (hDlg) {
		if (psi.DialogRun(hDlg) == (int)(sizeofa(Items) - 2)) {
//			DialogToParams(templateDialog, FarItems, sizeofa(Items), params);
			psi.DialogFree(hDlg);
			return	true;
		}
	}
	return	false;
}
void			CallTemplateMenu(formattingParams& dstParams) {
	int		selectedItem = 0;

	while (true) {
		FarMenuItem * menuItems = NULL;
		int		i = 0;

		formattingParams tmpParams;
		while (tmpParams.load(i + 1)) {
			menuItems = (FarMenuItem*) WinMem::Realloc(menuItems, sizeof(struct FarMenuItem) * (i + 1));
			menuItems[i].Checked = menuItems[i].Separator = 0;
			menuItems[i].Selected = (i == selectedItem);
			Copy((PWSTR)menuItems[i].Text, tmpParams.templateName, sizeof(menuItems[0].Text));
			i++;
		}
		int usedKeys[] = { VK_INSERT, VK_F4, VK_DELETE, 0 };

		bool validParams;
		int retKey, retCode = CenterMenu(GetMsg(msgTemplate2), L"Ins, F4, Del", L"Templates", usedKeys,
										 &retKey, menuItems, i);
//		psi.Menu(psi.ModuleNumber,-1,-1,0, FMENU_AUTOHIGHLIGHT | FMENU_WRAPMODE,
//				GetMsg(msgTemplate2), NULL, L"Templates", NULL, NULL, MenuItems, i);


		WinMem::Free(menuItems);
		if (retCode == -1)
			return;

		validParams = tmpParams.load(retCode + 1);
		switch (retKey) {
			case -1: // Enter
				if (validParams) {
					dstParams = tmpParams;
					return;
				}
				break;
			case 0: // Insert
				tmpParams.reset();
				retCode = i;
			case 1: // F4
				if (CallTemplateDialog(&tmpParams))
					tmpParams.save(retCode + 1);
				selectedItem = retCode;
				break;
			case 2: // Delete
				selectedItem = retCode - 1;
				if (validParams) {
					const WCHAR* question[] = { GetMsg(msgDelete1), GetMsg(msgDelete2), NULL,
												GetMsg(txtBtnOk), GetMsg(txtBtnCancel)
											  };
					question[2] = tmpParams.templateName;
					if (!ShowQuestion(question, sizeof(question) / sizeof(question[0]), NULL))
						while (++retCode < i) {
							tmpParams.load(retCode + 1);
							tmpParams.save(retCode);
						}
					tmpParams.del(retCode);
				}
				break;
		}
	}
}
*/

///=================================================================================== Format string
int				UpdateBuffer(PWSTR buf, int truelen, int fulllen) {
	int		j = 0;

	if (IsSpace(buf[truelen]))
		truelen++;
	while (truelen < fulllen)
		buf[j++] = buf[truelen++];
	return j;
}
PWSTR			AddSpaces(PWSTR ptr, int n) {
	while (n-- > 0)
		*ptr++ = L' ';
	return	ptr;
}
void InsertFormattedString(PCWSTR srcString, int srcLength, int leftMargin, int rightMargin,
						   int formattingType, int y, PCWSTR eol) {
	PWSTR	buf = (PWSTR)WinMem::Alloc(rightMargin);
	int		leftIndent = leftMargin;

	switch (formattingType) {
		case msgRight:
			leftIndent = rightMargin - srcLength + 1;
			break;
		case msgCenter:
			leftIndent = ((rightMargin + leftMargin - srcLength) / 2) + 1;
	}

	PWSTR	ptr = AddSpaces(buf, leftIndent - 1);

	if ((formattingType == msgFullJustify) ||
			(formattingType == msgForceJustify)) {
		int		i;
		size_t	currentSpaces = 0;
		for (i = 0; i < srcLength; i++) {
			if (srcString[i] == L' ')
				currentSpaces++;
		}
		size_t	additionalSpaces = rightMargin - leftMargin - srcLength + 1;
		size_t	additionalSpaces1 = currentSpaces ? additionalSpaces / currentSpaces : 0;
		int		additionalSpaces2 = currentSpaces - (currentSpaces ? additionalSpaces % currentSpaces : 0);

		for (i = 0; i < srcLength; i++) {
			if (srcString[i] == L' ') {
				ptr = AddSpaces(ptr, (((additionalSpaces2--) > 0) ? 1 : 2) + additionalSpaces1);
			} else {
				*ptr++ = srcString[i];
			}
		}
		InsertString(y, buf, ptr - buf, eol);
	} else {
		Copy(ptr, srcString, srcLength);
		InsertString(y, buf, leftIndent - 1 + srcLength , eol);
	}
	WinMem::Free(buf);
}
void PerformFormatting(const formattingParams &params) {
	EditorInfo		ei;
	psi.EditorControl(ECTL_GETINFO, &ei);

	EditorGetString	str;
	int		currentInputLine = (ei.BlockType == BTYPE_NONE) ? ei.CurLine : ei.BlockStartLine;
	int		currentOutputLine = currentInputLine;
	PWSTR	pureString = (PWSTR)WinMem::Alloc(sizeof(WCHAR) * (2 + params.rightMargin - Min(params.paragraphIndent, params.leftMargin)));

	size_t	requiredLength = 0, leftIndent = 0, pureStringLength = 0, fullStringLength = 0;
	bool	processingWord = false, startNewParagraph = true, endThisParagraph = false, workDone = false, emptyString = false;

	while (true) {
		if (endThisParagraph || emptyString) {
			if (fullStringLength) {
				if (IsSpace(pureString[fullStringLength-1]))
					fullStringLength--;
			}
			if (fullStringLength) {
				InsertFormattedString(pureString, fullStringLength, leftIndent, params.rightMargin,
									  ((params.formattingType == msgFullJustify) ? msgLeft : params.formattingType),
									  currentOutputLine, str.StringEOL);
				currentInputLine++;
				currentOutputLine++;
				fullStringLength = 0;
			}
			if (params.separateParagraphs || emptyString) {
				InsertString(currentOutputLine, L"", 0, str.StringEOL);
				currentInputLine++;
				currentOutputLine++;
				emptyString = false;
			}
			startNewParagraph = true;
			endThisParagraph = false;
			ReleaseString(&str);
		}
		if (workDone)
			break;

//    emptyString = false;
		GetString(currentInputLine, &str);
		PCWSTR	curStr = str.StringText;

		if (!startNewParagraph && params.keepEmpty && curStr[0] == 0) {
			// не убивать пустые строки
//     if ((ei.BlockStartLine + ei.BlockHeight) > currentInputLine) { //последнюю пустую
			endThisParagraph  = true;
			emptyString       = true;
			continue;
		}
		if (!startNewParagraph && params.catchParagraphs && params.startParagraphs[0] && !Cmp(curStr, params.startParagraphs, Len(params.startParagraphs)))  {
			// отловить параграф
			endThisParagraph  = true;
			continue;
		}

		if (startNewParagraph) {
			leftIndent = (params.formattingType == msgCenter) ? params.leftMargin : params.paragraphIndent;
			requiredLength = params.rightMargin - leftIndent + 2;
			processingWord = false;
			pureStringLength = fullStringLength = 0;
			endThisParagraph = startNewParagraph = params.paragraphPerLine;
		}
		if (ei.BlockType == BTYPE_NONE) {
			endThisParagraph = true;
			workDone = true;
		}

		if ((ei.BlockType != BTYPE_NONE) && ((str.SelStart == -1) || (str.SelEnd == 0))) {
			endThisParagraph = !params.paragraphPerLine;
			workDone = true;
		} else {
			for (int i = -1;i < str.StringLength;i++) { // character loop
				if (IsSpace(curStr[i]) || i < 0) { // line always starts with whitespace
					if (processingWord) { // whitespace ends the word
						pureStringLength = fullStringLength;
						pureString[fullStringLength++] = ' ';
						processingWord = false;
					}
					// else: consequent whitespaces are ignored
				} else { // non-whitespace is a part of word
					pureString[fullStringLength++] = curStr[i];
					processingWord = true;
				}
				if (fullStringLength >= requiredLength) {
					if (!pureStringLength) {
						PCWSTR errorMessage[] = { GetMsg(msgError1), GetMsg(msgError2),
												  GetMsg(msgError3), GetMsg(txtBtnOk)
												};
						ShowWarning(errorMessage, sizeof(errorMessage) / sizeof(errorMessage[0]), L"WordTooLong");
						ReleaseString(&str);
						WinMem::Free(pureString);
						return;
					}
					InsertFormattedString(pureString, pureStringLength,
										  leftIndent, params.rightMargin, params.formattingType,
										  currentOutputLine, str.StringEOL);
					currentOutputLine++;
					currentInputLine++; // cause text got shifted down
					fullStringLength = UpdateBuffer(pureString, pureStringLength, fullStringLength);
					pureStringLength = 0;
					leftIndent = params.leftMargin;
					requiredLength = params.rightMargin - leftIndent + 2;
				}
			}	// character loop
			DeleteString(currentInputLine);
			if (!endThisParagraph) ReleaseString(&str);
		}
	} // string loop
	WinMem::Free(pureString);
	UnselectBlock();
}

///========================================================================================== Export
void	WINAPI	EXP_NAME(SetStartupInfo)(const PluginStartupInfo *psi) {
	InitFSF(psi);
}
void	WINAPI	EXP_NAME(GetPluginInfo)(PluginInfo *pi) {
	pi->StructSize = sizeof(PluginInfo);
	pi->Flags = PF_EDITOR | PF_DISABLEPANELS;
	pi->DiskMenuStringsNumber = 0;

	static PCWSTR	MenuStrings[1];
	MenuStrings[0] = GetMsg(MenuTitle);
	pi->PluginMenuStrings = MenuStrings;
	pi->PluginMenuStringsNumber = sizeofa(MenuStrings);

	pi->PluginConfigStringsNumber = 0;
}
HANDLE	WINAPI	EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
	formattingParams	params;
	params.load(0);
	static WCHAR	mleft[20];
	static WCHAR	mright[20];
	static WCHAR	mpara[20];
	static WCHAR	mcatch[20];
	Num2Str(mleft, params.leftMargin);
	Num2Str(mright, params.rightMargin);
	Num2Str(mpara, params.paragraphIndent);
	Copy(mcatch, params.startParagraphs, sizeofa(mcatch));
	const size_t	WIDTH = 54;
	const size_t	HEIGHT = 17;
	for (;;) {
		InitDialogItem Items[] = {
			{DI_DOUBLEBOX,   3, 1, WIDTH - 4, HEIGHT - 2, 0, 0, 0, 0, (PCWSTR)DlgTitle},
			{DI_SINGLEBOX,   5, 2, 26 , 8, 0, 0, DIF_LEFTTEXT, 0, (PCWSTR)msgMargin},
			{DI_TEXT,        7,  3, 0,  0, 0, 0, 0,            0, (PCWSTR)msgLeftMargin},
			{DI_EDIT,        20, 3, 24, 0, 1, 0, 0,            0, mleft},
			{DI_TEXT,         7, 4, 0,  0, 0, 0, 0,            0, (PCWSTR)msgRightMargin},
			{DI_EDIT,        20, 4, 24, 0, 0, 0, 0,            0, mright},
			{DI_TEXT,         7, 5, 0,  0, 0, 0, 0,            0, (PCWSTR)msgParagraph},
			{DI_EDIT,        20, 5, 24, 0, 0, 0, 0,            0, mpara},

			{DI_SINGLEBOX,   28, 2, WIDTH - 6 , 8, 0, 0, DIF_LEFTTEXT, 0, (PCWSTR)msgAlignment},
			{DI_RADIOBUTTON, 30, 3, 0, 0, 0, 0, DIF_GROUP,     0, (PCWSTR)msgLeft},
			{DI_RADIOBUTTON, 30, 4, 0, 0, 0, 0, 0,             0, (PCWSTR)msgRight},
			{DI_RADIOBUTTON, 30, 5, 0, 0, 0, 0, 0,             0, (PCWSTR)msgCenter},
			{DI_RADIOBUTTON, 30, 6, 0, 0, 0, 0, 0,             0, (PCWSTR)msgFullJustify},
			{DI_RADIOBUTTON, 30, 7, 0, 0, 0, 0, 0,             0, (PCWSTR)msgForceJustify},

			{DI_CHECKBOX,     5,  9, 0, 0, 0, 0, 0,            0, (PCWSTR)msgParagraphPerLine},
			{DI_CHECKBOX,     5, 10, 0, 0, 0, 0, 0,            0, (PCWSTR)msgSeparateParagraphs},
			{DI_CHECKBOX,     5, 11, 0, 0, 0, 0, 0,            0, (PCWSTR)msgKeepEmpty},
			{DI_CHECKBOX,     5, 12, 0, 0, 0, 0, 0,            0, (PCWSTR)msgCatchParagraphs},
			{DI_EDIT,        30, 12, WIDTH - 8, 0, 0, 0, 0,        0, mcatch},
			{DI_TEXT,         0, 13, 0, 0, 0, 0, DIF_SEPARATOR | DIF_BOXCOLOR, 0, L""},
//			{DI_BUTTON,       0, 14, 0, 0, 0, 0, DIF_CENTERGROUP, 0, (PCWSTR)msgTemplate},
			{DI_BUTTON,       0, 14, 0, 0, 0, 0, DIF_CENTERGROUP, 1, (PCWSTR)txtBtnOk},
			{DI_BUTTON,       0, 14, 0, 0, 0, 0, DIF_CENTERGROUP, 0, (PCWSTR)txtBtnCancel}
		};

		FarDialogItem FarItems[sizeofa(Items)];
		InitDialogItems(Items, FarItems, sizeofa(Items));
		params.ToDialog(Items, FarItems, sizeofa(Items));
		HANDLE hDlg = psi.DialogInit(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, L"Contents", FarItems, sizeofa(FarItems), 0, 0, NULL, 0);
		if (hDlg) {
			int ret = psi.DialogRun(hDlg);
			if (ret < 0 || (Items[ret].Data == (PCWSTR)txtBtnCancel)) {
				psi.DialogFree(hDlg);
				return	INVALID_HANDLE_VALUE;
			}
			if (Items[ret].Data == (PCWSTR)txtBtnOk) {
				params.FromDialog(Items, hDlg, sizeofa(Items));
				params.save(0);
				PerformFormatting(params);
				psi.DialogFree(hDlg);
				return	INVALID_HANDLE_VALUE;
			} else if (Items[ret].Data == (PCWSTR)msgTemplate) {
//				CallTemplateMenu(params);
			}
			psi.DialogFree(hDlg);
		}
	}
	return	INVALID_HANDLE_VALUE;
}

///===================================================================================== Entry point
extern "C"
	BOOL WINAPI DllMainCRTStartup(HANDLE hDll, DWORD dwReason, LPVOID lpReserved) {
	return	true;
}
