#include "Header.h"

HANDLE VirtualConsole::hConOut() {
	if (gSet.isConMan) {
		if (hConOut_)
			CloseHandle(hConOut_);

		hConOut_ = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
							  0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	}
	return hConOut_;
}

VirtualConsole::VirtualConsole(HANDLE hConsoleOutput) {
	if (gSet.isConMan)
		hConOut_ = 0;
	else
		hConOut_ = hConsoleOutput ? hConsoleOutput : GetStdHandle(STD_OUTPUT_HANDLE);

	TextWidth = TextHeight = Width = Height = 0;
	hDC = null_ptr;
	hBitmap = null_ptr;
	hFont = null_ptr;
	ConChar = null_ptr;
	ConAttr = null_ptr;

	LogFont.lfHeight = 16;
	LogFont.lfWidth = 0;
	LogFont.lfEscapement = LogFont.lfOrientation = 0;
	LogFont.lfWeight = FW_NORMAL;
	LogFont.lfItalic = LogFont.lfUnderline = LogFont.lfStrikeOut = FALSE;
	LogFont.lfCharSet = DEFAULT_CHARSET;
	LogFont.lfOutPrecision = OUT_TT_PRECIS;
	LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	LogFont.lfQuality = ANTIALIASED_QUALITY;
	LogFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
	_tcscpy(LogFont.lfFaceName, _T("Lucida Console"));

	Registry RegConColors, RegConDef;
	if (RegConColors.OpenKey(_T("Console"), KEY_READ)) {
		RegConDef.OpenKey(HKEY_USERS, _T(".DEFAULT\\Console"), KEY_READ);

		TCHAR ColorName[] = _T("ColorTable00");
		for (uint i = 0x10; i--;) {
			ColorName[10] = i / 10 + '0';
			ColorName[11] = i % 10 + '0';
			if (!RegConColors.Load(ColorName, (DWORD *)&Colors[i]))
				RegConDef.Load(ColorName, (DWORD *)&Colors[i]);
		}

		RegConDef.CloseKey();
		RegConColors.CloseKey();
	}
}

VirtualConsole::~VirtualConsole() {
	Free(true);
}

void VirtualConsole::Free(bool bFreeFont) {
	if (hDC) {
		DeleteDC(hDC);
		hDC = null_ptr;
	}
	if (hBitmap) {
		DeleteObject(hBitmap);
		hBitmap = null_ptr;
	}
	if (bFreeFont && hFont) {
		DeleteObject(hFont);
		DeleteObject(hFont2);
		hFont2 = null_ptr;
		hFont = null_ptr;
	}
	if (ConChar) {
		delete[] ConChar;
		ConChar = null_ptr;
	}
	if (ConAttr) {
		delete[] ConAttr;
		ConAttr = null_ptr;
	}
}

bool VirtualConsole::InitFont(void) {
	Free(true);
	hFont = CreateFontIndirectMy(&LogFont);
	return hFont != null_ptr;
}

bool VirtualConsole::InitDC(void) {
	if (hFont)
		Free(false);
	else
		if (!InitFont())
			return false;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(hConOut(), &csbi))
		return false;

	IsForceUpdate = true;
	TextWidth = csbi.dwSize.X;
	TextHeight = csbi.dwSize.Y;

	ConChar = new TCHAR[TextWidth * TextHeight * 2];
	ConAttr = new WORD [TextWidth * TextHeight * 2];
	if (!ConChar || !ConAttr)
		return false;

	hSelectedFont = null_ptr;
	const HDC hScreenDC = GetDC(0);
	if ((hDC = CreateCompatibleDC(hScreenDC))) {
		SelectObject(hDC, hFont);
		TEXTMETRIC tm;
		GetTextMetrics(hDC, &tm);
		if (gSet.isForceMonospace)
			LogFont.lfWidth = tm.tmMaxCharWidth;
		else
			LogFont.lfWidth = tm.tmAveCharWidth;
		LogFont.lfHeight = tm.tmHeight;

		Width = TextWidth * LogFont.lfWidth;
		Height = TextHeight * LogFont.lfHeight;

		hBitmap = CreateCompatibleBitmap(hScreenDC, Width, Height);
		SelectObject(hDC, hBitmap);
	}
	ReleaseDC(0, hScreenDC);

	return hBitmap != null_ptr;
}

//#define isCharUnicode(inChar) (inChar <= 0x2668 ? 0 : 1)
bool isCharUnicode(WCHAR inChar) {
	//if (inChar <= 0x2668)
	if (gSet.isFixFarBorders) {
		//if (! (inChar > 0x2500 && inChar < 0x251F))
		if (!(inChar > 0x2013 && inChar < 0x266B))
			/*if (inChar != 0x2550 && inChar != 0x2502 && inChar != 0x2551 && inChar != 0x007D &&
			inChar != 0x25BC && inChar != 0x2593 && inChar != 0x2591 && inChar != 0x25B2 &&
			inChar != 0x2562 && inChar != 0x255F && inChar != 0x255A && inChar != 0x255D &&
			inChar != 0x2554 && inChar != 0x2557 && inChar != 0x2500 && inChar != 0x2534 && inChar != 0x2564) // 0x2520*/
			return false;
		else
			return true;
	} else {
		if (inChar < 0x01F1 || inChar > 0x0400 && inChar < 0x045F || inChar > 0x2012 && inChar < 0x203D || /*? - not sure that optimal*/ inChar > 0x2019 && inChar < 0x2303 || inChar > 0x24FF && inChar < 0x266C)
			return false;
		else
			return true;
	}
}

void BlitPictureTo(VirtualConsole *vc, int inX, int inY, int inWidth, int inHeight) {
	BitBlt(vc->hDC, inX, inY, inWidth, inHeight, vc->hBgDc, inX, inY, SRCCOPY);
	if (vc->bgBmp.cx < (int)inWidth || vc->bgBmp.cy < (int)inHeight) {
		HBRUSH hBrush = CreateSolidBrush(vc->Colors[0]);
		SelectObject(vc->hDC, hBrush);
		RECT rect = {vc->bgBmp.cx, inY, inWidth, inHeight};
		FillRect(vc->hDC, &rect, hBrush);

		rect.left = inX;
		rect.top = vc->bgBmp.cy;
		rect.right = vc->bgBmp.cx;
		FillRect(vc->hDC, &rect, hBrush);

		DeleteObject(hBrush);
	}
}

void VirtualConsole::SelectFont(HFONT hNew) {
	if (hSelectedFont != hNew) {
		hSelectedFont = hNew;
		SelectObject(hDC, hNew);
	}
}

bool VirtualConsole::Update(bool isForce) {
	bool lRes = false;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(hConOut(), &csbi))
		return false;

	if (isForce || !hDC || TextWidth != csbi.dwSize.X || TextHeight != csbi.dwSize.Y)
		InitDC();

	// use and reset additional force flag
	if (IsForceUpdate) {
		isForce = IsForceUpdate;
		IsForceUpdate = false;
	}

	const bool drawImage = gSet.isShowBgImage && gSet.isBackgroundImageValid;
	const uint TextLen = TextWidth * TextHeight;
	COORD coord = {0, 0};
	DWORD iCount;
	ReadConsoleOutputCharacter(hConOut(), ConChar, TextLen, coord, &iCount);
	ReadConsoleOutputAttribute(hConOut(), ConAttr, TextLen, coord, &iCount);

	CONSOLE_CURSOR_INFO	cinf;
	GetConsoleCursorInfo(hConOut(), &cinf);

	//------------------------------------------------------------------------
	///| Drawing text (if there were changes in console) |////////////////////
	//------------------------------------------------------------------------
	bool updateText, updateCursor;
	if (isForce) {
		updateText = updateCursor = true;
	} else {
		// About cursor: check both 'cinf.bVisible' and 'Cursor.isVisible',
		// because console may have cursor hidden and its position changed -
		// in this case last visible cursor remains shown at its old place.
		// Also, don't check foreground window here for the same reasons.
		updateText = (memcmp(ConChar + TextLen, ConChar, TextLen * sizeof(TCHAR)) || memcmp(ConAttr + TextLen, ConAttr, TextLen * 2));
		updateCursor = (Cursor.x != csbi.dwCursorPosition.X || Cursor.y != csbi.dwCursorPosition.Y) && (cinf.bVisible || Cursor.isVisible);
	}
	if (updateText || updateCursor) {
		lRes = true;

		// counters
		TCHAR* ConCharLine;
		WORD* ConAttrLine;
		int i, pos, row;
		if (updateText) {
			i = TextLen - TextWidth;
			pos = Height - LogFont.lfHeight;
			row = TextHeight - 1;
		} else {
			i = TextWidth * Cursor.y;
			pos = LogFont.lfHeight * Cursor.y;
			row = Cursor.y;
		}
		ConCharLine = ConChar + i;
		ConAttrLine = ConAttr + i;

		i64 tick, tick2;
		if (hOpWnd)
			QueryPerformanceCounter((LARGE_INTEGER *)&tick);

		if (gSet.isForceMonospace || !drawImage)
			SetBkMode(hDC, OPAQUE);
		else
			SetBkMode(hDC, TRANSPARENT);

		// rows
		const bool skipNotChanged = !isForce && !gSet.isForceMonospace;
		for (; pos >= 0; ConCharLine -= TextWidth, ConAttrLine -= TextWidth, pos -= LogFont.lfHeight, --row) {
			// the line
			const WORD* const ConAttrLine2 = ConAttrLine + TextLen;
			const TCHAR* const ConCharLine2 = ConCharLine + TextLen;

			// skip not changed symbols except the old cursor
			int j = 0, end = TextWidth;
			if (skipNotChanged) {
				// *) Skip not changed tail symbols.
				while (--end >= 0 && ConCharLine[end] == ConCharLine2[end] && ConAttrLine[end] == ConAttrLine2[end]) {
					if (updateCursor && row == Cursor.y && end == Cursor.x)
						break;
				}
				if (end < j)
					continue;
				++end;

				// *) Skip not changed head symbols.
				while (j < end && ConCharLine[j] == ConCharLine2[j] && ConAttrLine[j] == ConAttrLine2[j]) {
					if (updateCursor && row == Cursor.y && j == Cursor.x)
						break;
					++j;
				}
				if (j >= end)
					continue;
			}

			// *) Now draw as much as possible in a row even if some symbols are not changed.
			// More calls for the sake of fewer symbols is slower, e.g. in panel status lines.
			for (int j2; j < end; j = j2) {
				const WORD attr = ConAttrLine[j];
				const bool isUnicode = isCharUnicode(ConCharLine[j]);

				SetTextColor(hDC, Colors[attr & 0x0F]);

				if (gSet.isForceMonospace) {
					SetTextColor(hDC, Colors[attr & 0x0F]);
					SetBkColor(hDC, Colors[attr >> 4 & 0x0F]);

					//for (j2 = j + 1; j2 < end && ConAttrLine[j2] == ConAttrLine[j2 - 1]; j2++);
					//TextOut(hDC, j * LogFont.lfWidth, pos, ConCharLine + j, j2 - j);
					j2 = j + 1;
					/**/
					WCHAR c = ConCharLine[j];

					if (c == 0x20 || !c)
						c = 0x3000;//0x2003;
					else if (c <= 0x7E && c >= 0x21)
						c += 0xFF01 - 0x21;

					RECT rect = {j * LogFont.lfWidth, pos, j2 * LogFont.lfWidth, pos + LogFont.lfHeight};
					if (!(drawImage && (attr >> 4 & 0x0F) < 2))
						SetBkColor(hDC, Colors[attr >> 4 & 0x0F]);
					else if (drawImage)
						BlitPictureTo(this, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
					ExtTextOut(hDC, rect.left, rect.top, ETO_CLIPPED | ETO_OPAQUE, &rect, &c, 1, 0);

					//TextOut(hDC, j * LogFont.lfWidth, pos, &c, 1);
					ABC abc;
					GetCharABCWidths(hDC, c, c, &abc);
					if (abc.abcA < 0 || abc.abcC < 0)
						c = c;
					OUTLINETEXTMETRIC otm[2];
					ZeroMemory(otm, sizeof(otm));
					otm->otmSize = sizeof(*otm);
					GetOutlineTextMetrics(hDC, sizeof(otm), otm);

					WCHAR xchar = 0;

					switch (ConCharLine[j]) {
						case 0x255F:
							xchar = 0x2500;
							break;
						case 0x2554:
						case 0x255A:
						case 0x2564:
						case 0x2566:
						case 0x2567:
						case 0x2569:
						case 0x2550:
							xchar = 0x2550;
							break;
						case 0x2557:
						case 0x255D:
						case 0x2562:
						case 0x2551:
							xchar = 0x0020;
							break;
						case 0x2591:
							xchar = 0x2591;
							break;
						case 0x2592:
							xchar = 0x2592;
							break;
						case 0x2593:
							xchar = 0x2593;
							break;
					}
					if (xchar) {
						RECT rect = {j * LogFont.lfWidth + LogFont.lfWidth / 2, pos, j2 * LogFont.lfWidth, pos + LogFont.lfHeight};
						ExtTextOut(hDC, rect.left, rect.top, ETO_CLIPPED | ETO_OPAQUE, &rect, &xchar, 1, 0);

						//TextOut(hDC, j * LogFont.lfWidth + LogFont.lfWidth/2, pos, &xchar, 1);/**/
					}
				} else if (!isUnicode) {
					for (j2 = j + 1; j2 < end && ConAttrLine[j2] == attr && !isCharUnicode(*(ConCharLine + j2)); j2++) {}
					if (gSet.isFixFarBorders)
						SelectFont(hFont);
				} else { //UNICODE
					if (!gSet.isFixFarBorders) {
						for (j2 = j + 1; j2 < end && ConAttrLine[j2] == attr && isCharUnicode(*(ConCharLine + j2)); j2++) {}
					} else {
						for (j2 = j + 1; j2 < end && ConAttrLine[j2] == attr && isCharUnicode(*(ConCharLine + j2)) && *(ConCharLine + j2) == *(ConCharLine + j2 + 1); j2++) {}
					}
					if (gSet.isFixFarBorders)
						SelectFont(hFont2);
				}

				if (!gSet.isForceMonospace) {
					RECT rect = {j * LogFont.lfWidth, pos, j2 * LogFont.lfWidth, pos + LogFont.lfHeight};
					if (!(drawImage && (attr >> 4 & 0x0F) < 2))
						SetBkColor(hDC, Colors[attr >> 4 & 0x0F]);
					else if (drawImage)
						BlitPictureTo(this, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);

					if (LogFont.lfCharSet == OEM_CHARSET && !isUnicode) {
						char *tmp = new char[end+5];
						WideCharToMultiByte(CP_OEMCP, 0, ConCharLine + j, j2 - j, tmp, end + 4, 0, 0);
						ExtTextOutA(hDC, rect.left, rect.top, ETO_CLIPPED | ((drawImage && (attr >> 4 & 0x0F) < 2) ? 0 : ETO_OPAQUE),
									&rect, tmp, j2 - j, 0);
						delete[] tmp;
					} else {
						ExtTextOut(hDC, rect.left, rect.top, ETO_CLIPPED | ((drawImage && (attr >> 4 & 0x0F) < 2) ? 0 : ETO_OPAQUE), &rect, ConCharLine + j, j2 - j, 0);
					}
				}

				// stop if all is done
				if (!updateText)
					goto done;

				// skip next not changed symbols again
				if (skipNotChanged) {
					// skip the same except the old cursor
					while (j2 < end && ConCharLine[j2] == ConCharLine2[j2] && ConAttrLine[j2] == ConAttrLine2[j2]) {
						if (updateCursor && row == Cursor.y && j2 == Cursor.x)
							break;
						++j2;
					}
				}
			}
		}
done:

		// now copy the data for future comparison
		if (updateText) {
			memcpy(ConChar + TextLen, ConChar, TextLen * sizeof(TCHAR));
			memcpy(ConAttr + TextLen, ConAttr, TextLen * 2);
		}

		if (hOpWnd) {
			QueryPerformanceCounter((LARGE_INTEGER *)&tick2);
			wsprintf(temp, _T("%i"), (tick2 - tick) / 100);
			SetDlgItemText(hOpWnd, tRender, temp);
		}
	}

	//------------------------------------------------------------------------
	///| Drawing cursor |/////////////////////////////////////////////////////
	//------------------------------------------------------------------------
	if ((lRes || Cursor.isVisible != Cursor.isVisiblePrev) && cinf.bVisible) {
		lRes = true;
		if ((Cursor.x != csbi.dwCursorPosition.X || Cursor.y != csbi.dwCursorPosition.Y)) {
			Cursor.isVisible = isMeForeground();
			cBlinkNext = 0;
		}

		int CurChar = csbi.dwCursorPosition.Y * TextWidth + csbi.dwCursorPosition.X;
		Cursor.ch[0] = ConChar[CurChar];
		Cursor.ch[1] = 0;
		Cursor.foreColor = Colors[ConAttr[CurChar] >> 4 & 0x0F];
		Cursor.foreColorNum = ConAttr[CurChar] >> 4 & 0x0F;
		Cursor.bgColor = Colors[ConAttr[CurChar] & 0x0F];
		Cursor.isVisiblePrev = Cursor.isVisible;
		Cursor.x = csbi.dwCursorPosition.X;
		Cursor.y = csbi.dwCursorPosition.Y;

		if (Cursor.isVisible) {
			if (gSet.isCursorColor) {
				SetTextColor(hDC, Cursor.foreColor);
				SetBkColor(hDC, Cursor.bgColor);
			} else {
				SetTextColor(hDC, Cursor.foreColor);
				SetBkColor(hDC, Cursor.foreColorNum < 5 ? Colors[15] : Colors[0]);
			}
		} else {
			if (drawImage)
				BlitPictureTo(this, Cursor.x * LogFont.lfWidth, Cursor.y * LogFont.lfHeight, LogFont.lfWidth, LogFont.lfHeight);

			SetTextColor(hDC, Cursor.bgColor);
			SetBkColor(hDC, Cursor.foreColor);
			cinf.dwSize = 99;
		}

		RECT rect;
		if (!gSet.isCursorV) {
			rect.left = Cursor.x * LogFont.lfWidth;
			rect.top = (Cursor.y + 1) * LogFont.lfHeight - MulDiv(LogFont.lfHeight, cinf.dwSize, 100);
			rect.right = (Cursor.x + 1) * LogFont.lfWidth;
			rect.bottom = (Cursor.y + 1) * LogFont.lfHeight;
		} else {
			rect.left = Cursor.x * LogFont.lfWidth;
			rect.top = Cursor.y * LogFont.lfHeight;
			rect.right = Cursor.x * LogFont.lfWidth + klMax(1, MulDiv(LogFont.lfWidth, cinf.dwSize, 100) + (cinf.dwSize > 10 ? 1 : 0));
			rect.bottom = (Cursor.y + 1) * LogFont.lfHeight;
		}

		if (LogFont.lfCharSet == OEM_CHARSET && !isCharUnicode(Cursor.ch[0])) {
			if (gSet.isFixFarBorders)
				SelectFont(hFont);

			char tmp[2];
			WideCharToMultiByte(CP_OEMCP, 0, Cursor.ch, 1, tmp, 1, 0, 0);
			ExtTextOutA(hDC, Cursor.x * LogFont.lfWidth, Cursor.y * LogFont.lfHeight,
						ETO_CLIPPED | ((drawImage && (Cursor.foreColorNum < 2) &&
										!Cursor.isVisible) ? 0 : ETO_OPAQUE), &rect, tmp, 1, 0);
		} else {
			if (gSet.isFixFarBorders && isCharUnicode(Cursor.ch[0]))
				SelectFont(hFont2);
			else
				SelectFont(hFont);

			ExtTextOut(hDC, Cursor.x * LogFont.lfWidth, Cursor.y * LogFont.lfHeight,
					   ETO_CLIPPED | ((drawImage && (Cursor.foreColorNum < 2) &&
									   !Cursor.isVisible) ? 0 : ETO_OPAQUE), &rect, Cursor.ch, 1, 0);
		}
	} else {
		// update cursor anyway to avoid redundant updates
		Cursor.x = csbi.dwCursorPosition.X;
		Cursor.y = csbi.dwCursorPosition.Y;
	}
	return lRes;
}
