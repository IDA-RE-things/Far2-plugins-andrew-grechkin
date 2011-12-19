#include <libwin_com/7zip.h>
#include <libwin_net/exception.h>

int main() try {
	SevenZip::Lib lib(L"7z.dll");
	printf(L"7-zip library version: %s\n", lib.get_version().c_str());

	const SevenZip::Codecs & cd = lib.codecs();
	printf(L"Number of codecs: %Id\n", cd.size());
	for (SevenZip::Codecs::iterator it = cd.begin(); it != cd.end(); ++it) {
		printf(L"%s: %s\n", it->first.c_str(), it->second->ext.c_str());
	}

	return 0;
} catch (WinError & e) {
	printf(L"Error: %s\n", e.what().c_str());
	printf(L"Where: %s\n", e.where().c_str());

	return e.code();
}
