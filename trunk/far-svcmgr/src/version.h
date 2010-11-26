#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "08";
	static const char MONTH[] = "11";
	static const char YEAR[] = "2010";
	static const char UBUNTU_VERSION_STYLE[] = "10.11";
	
	//Software Status
	static const char STATUS[] = "Release Candidate";
	static const char STATUS_SHORT[] = "rc";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 1;
	static const long BUILD = 6;
	static const long REVISION = 23;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 4;
	#define RC_FILEVERSION 1,1,6,23
	#define RC_FILEVERSION_STRING "1, 1, 6, 23\0"
	static const char FULLVERSION_STRING[] = "1.1.6.23";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 4;
	

}
#endif //VERSION_H
