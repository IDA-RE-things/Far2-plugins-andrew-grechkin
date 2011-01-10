/** win_arc.hpp
 *	@functions to manipulate archives
 *	@author GrAnD, 2009
 *	@link (7-zip32)
**/

#include "arc.h"

//#include <iostream>
//#include <fstream>
//using namespace std;

///============================================================================================ SZip
int		SZip::Compress(const astring &arc, const astring &path, const astring &type, const astring &/*lvl*/, bool recur) {
	astring	cmd = "a";
	if (recur)
		cmd += " -r";
	cmd += " -t";
	cmd += type;
	cmd += " -hide -y -- ";
	cmd += arc;
	cmd += " ";
	cmd += path;

	auto_array<CHAR> buf(65535);
	SevenZipSetUnicodeMode(true);
	int Result = SevenZip(NULL, cmd.c_str(), buf.data(), buf.size());
//	cout << buf.c_str();
//	ofstream file("c:\\Temp\\eeee");
//	file << cmd.c_str();
//	file << buf.c_str();
	return	Result;
}

int		SZip::Extract(const astring &arc, const astring &path, const astring &fl, bool recur, bool fullpath) {
	astring	cmd = fullpath ? "x" : "e";
	cmd += " -o";
	cmd += path;
	if (recur)
		cmd += " -r";
	cmd += " -hide -y -- ";
	cmd += arc;
	cmd += " ";
	cmd += fl;

	auto_array<CHAR> buf(65535);
	SevenZipSetUnicodeMode(true);
	int Result = SevenZip(NULL, cmd.c_str(), buf.data(), buf.size());
//	cout << buf.c_str();
//	ofstream file("c:\\Temp\\eeee");
//	file << buf.c_str();
	return	Result;
}

int		SZip::Delete(const astring &arc, const astring &path) {
	astring	cmd = "d -- ";
	cmd += arc;
	cmd += " ";
	cmd += path;
	auto_array<CHAR> buf(65535);
	int Result = SevenZip(NULL, cmd.c_str(), buf.data(), buf.size());
//	cout << buf.c_str();
//	ofstream file("c:\\eeee");
//	file << buf.c_str();
	return	Result;
}
