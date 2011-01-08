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
int		SZip::Compress(const CStrA &arc, const CStrA &path, const CStrA &type, const CStrA &/*lvl*/, bool recur) {
	CStrA	cmd = "a";
	if (recur)
		cmd += " -r";
	cmd += " -t";
	cmd += type;
	cmd += " -hide -y -- ";
	cmd += arc;
	cmd += " ";
	cmd += path;

	WinBuf<CHAR>	buf(65535);
	SevenZipSetUnicodeMode(true);
	int Result = SevenZip(NULL, cmd.c_str(), buf.data(), buf.capacity());
//	cout << buf.c_str();
//	ofstream file("c:\\Temp\\eeee");
//	file << cmd.c_str();
//	file << buf.c_str();
	return	Result;
}

int		SZip::Extract(const CStrA &arc, const CStrA &path, const CStrA &fl, bool recur, bool fullpath) {
	CStrA	cmd = fullpath ? "x" : "e";
	cmd += " -o";
	cmd += path;
	if (recur)
		cmd += " -r";
	cmd += " -hide -y -- ";
	cmd += arc;
	cmd += " ";
	cmd += fl;

	WinBuf<CHAR>	buf(65535);
	SevenZipSetUnicodeMode(true);
	int Result = SevenZip(NULL, cmd.c_str(), buf.data(), buf.capacity());
//	cout << buf.c_str();
//	ofstream file("c:\\Temp\\eeee");
//	file << buf.c_str();
	return	Result;
}

int		SZip::Delete(const CStrA &arc, const CStrA &path) {
	CStrA	cmd = "d -- ";
	cmd += arc;
	cmd += " ";
	cmd += path;
	WinBuf<CHAR>	buf(65535);
	int Result = SevenZip(NULL, cmd.c_str(), buf.data(), buf.capacity());
//	cout << buf.c_str();
//	ofstream file("c:\\eeee");
//	file << buf.c_str();
	return	Result;
}
