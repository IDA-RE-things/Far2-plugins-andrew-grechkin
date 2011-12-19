#include <libwin_com/7zip.h>
#include <libwin_net/exception.h>

int main() try {
	SevenZip::Lib lib(L"7z.dl1l");
	printf(L"7-zip library version: %s\n", lib.get_version().c_str());
	return 0;
} catch (WinError & e) {
	printf(L"Error: %s\n", e.what().c_str());
	printf(L"Where: %s\n", e.where().c_str());
	return e.code();
}
