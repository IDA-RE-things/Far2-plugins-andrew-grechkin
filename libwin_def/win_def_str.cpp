#include "win_def.h"

UINT				CheckUnicode(const PVOID buf, size_t size) {
	int test = 0xFFFF;
	if (::IsTextUnicode(buf, (int)size, &test)) {
		if (test & IS_TEXT_UNICODE_UNICODE_MASK) {
			return	CP_UTF16le;
		} else if (test & IS_TEXT_UNICODE_REVERSE_MASK) {
			return	CP_UTF16be;
		}
		/*
						if (!(test&IS_TEXT_UNICODE_ODD_LENGTH) && !(test&IS_TEXT_UNICODE_ILLEGAL_CHARS)) {
							if ((test&IS_TEXT_UNICODE_NULL_BYTES) ||
									(test&IS_TEXT_UNICODE_CONTROLS) ||
									(test&IS_TEXT_UNICODE_REVERSE_CONTROLS)) {
								if ((test&IS_TEXT_UNICODE_CONTROLS) || (test&IS_TEXT_UNICODE_STATISTICS)) {
									nCodePage = CP_UTF16le;
									bDetect = true;
								} else if ((test&IS_TEXT_UNICODE_REVERSE_CONTROLS) || (test&IS_TEXT_UNICODE_REVERSE_STATISTICS)) {
									nCodePage = CP_UTF16be;
									bDetect = true;
								}
							}
						}
		*/
	}
	return	0;
}
UINT				IsUTF8(const PVOID buf, size_t size) {
	bool	Ascii = true;
	UINT	Octets = 0;
	for (size_t i = 0; i < size; ++i) {
		BYTE c = ((PBYTE)buf)[i];
		if (c & 0x80)
			Ascii = 0;
		if (Octets) {
			if ((c & 0xC0) != 0x80)
				return	0;
			Octets--;
		} else {
			if (c & 0x80) {
				while (c & 0x80) {
					c <<= 1;
					Octets++;
				}
				Octets--;
				if (!Octets)
					return	0;
			}
		}
	}
	return	(Octets > 0 || Ascii) ? 0 : CP_UTF8;
}
UINT				GetCP(HANDLE hFile, bool bUseHeuristics, bool &bSignatureFound) {
	DWORD	dwTemp = 0;
	DWORD	size = 0;

	::SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	if (::ReadFile(hFile, &dwTemp, sizeof(dwTemp), &size, NULL)) {
		if (LOWORD(dwTemp) == BOM_UTF16le) {
			::SetFilePointer(hFile, 2, NULL, FILE_BEGIN);
			bSignatureFound = true;
			return	CP_UTF16le;
		} else if (LOWORD(dwTemp) == BOM_UTF16be) {
			::SetFilePointer(hFile, 2, NULL, FILE_BEGIN);
			bSignatureFound = true;
			return	CP_UTF16be;
		} else if ((dwTemp & 0x00FFFFFF) == BOM_UTF8) {
			::SetFilePointer(hFile, 3, NULL, FILE_BEGIN);
			bSignatureFound = true;
			return	CP_UTF8;
		} else if (dwTemp == BOM_UTF32le) {
			bSignatureFound = true;
			return	CP_UTF32le;
		} else if (dwTemp == BOM_UTF32be) {
			bSignatureFound = true;
			return	CP_UTF32be;
		} else
			::SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	}

	UINT	nCodePage = 0;
	if (!bSignatureFound && bUseHeuristics) {
		size = 0x8000; // BUGBUG. TODO: configurable
		PBYTE 	buf;
		if (WinMem::Alloc(buf, size) && FileRead(hFile, buf, size)) {
			UINT	cp = CheckUnicode(buf, size);
			if (cp) {
				nCodePage = cp;
			} else if (IsUTF8(buf, size)) {
				nCodePage = CP_UTF8;
			} else {
				/*
				nsUniversalDetectorEx *ns = new nsUniversalDetectorEx();

				ns->HandleData((const char*)Buffer, (PRUint32)sz);
				ns->DataEnd();

				int cp = ns->getCodePage();

				if (cp != -1) {
					nCodePage = cp;
					bDetect = true;
				}

				delete ns;
				*/
			}
			WinMem::Free(buf);
		}
	}
	return	nCodePage;
}

CStrMW::MzsData::MzsData(PCWSTR in): m_capa(0), m_size(0) {
	PCWSTR	ptr = in;
	while (*ptr) {
		ptr = ptr + Len(ptr) + 1;
		++m_size;
	}
	m_capa = ptr - in + 1;
	m_data = new WCHAR[m_capa];
	WinMem::Copy(m_data, in, m_capa * sizeof(WCHAR));
}
PCWSTR				CStrMW::operator[](int index) const {
	PCWSTR	ptr = c_str();
	int		cnt = 0;
	while (*ptr && (cnt++ < index)) {
		ptr = ptr + Len(ptr) + 1;
	}
	return	ptr;
}

///====================================================================== Функции работы со строками
/*
PWSTR				CharFirstOf(PCWSTR in, PCWSTR mask) {
	size_t	lin = Len(in);
	size_t	len = Len(mask);
	for (size_t i = 0; i < lin; ++i) {
		for (size_t j = 0; j < len; ++j) {
			if (in[i] == mask[j])
				return	(PWSTR)&in[i];
		}
	}
	return	NULL;
}
PWSTR				CharFirstNotOf(PCWSTR in, PCWSTR mask) {
	size_t	lin = Len(in);
	size_t	len = Len(mask);
	for (size_t i = 0; i < lin; ++i) {
		for (size_t j = 0; j < len; ++j) {
			if (in[i] == mask[j])
				break;
			if (j == len - 1)
				return	(PWSTR)&in[i];
		}
	}
	return	NULL;
}
*/

CStrA				Hash2Str(PBYTE hash, size_t size) {
	CHAR	buf[(size + 1) * 2];
	PSTR	tmp = buf;
	for (size_t i = 0; i < size; ++i) {
		snprintf(tmp, sizeofa(buf) - i * 2, "%02x", hash[i]);
		tmp += 2;
	}
	return	buf;
}
CStrA				Hash2StrNum(PBYTE hash, size_t size) {
	CHAR	buf[(size + 1) * 4];
	PSTR	tmp = buf;
	for (size_t i = 0; i < size; ++i) {
		tmp += snprintf(tmp, sizeofa(buf) - i * 2, "%02i ", hash[i]);
	}
	return	buf;
}
bool				Str2Hash(const CStrA &str, PVOID &hash, ULONG &size) {
	size_t strsize = str.size();
	if (strsize % 2 == 0) {
		size = strsize / 2;
		if (WinMem::Alloc(hash, size)) {
			for (size_t i = 0; i < size; ++i) {
				CStrA	tmp = str.substr(i * 2, 2);
				((PBYTE)hash)[i] = (BYTE)AsLong(tmp.c_str(), 16);
			}
			return	true;
		}
	}
	return	false;
}
