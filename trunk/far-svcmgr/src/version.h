#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "09";
	static const char MONTH[] = "09";
	static const char YEAR[] = "2010";
	static const char UBUNTU_VERSION_STYLE[] = "10.09";
	
	//Software Status
	static const char STATUS[] = "Release Candidate";
	static const char STATUS_SHORT[] = "rc";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 1;
	static const long BUILD = 4;
	static const long REVISION = 11;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 3;
	#define RC_FILEVERSION 1,1,4,11
	#define RC_FILEVERSION_STRING "1, 1, 4, 11\0"
	static const char FULLVERSION_STRING[] = "1.1.4.11";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 2;
	

}
#endif //VERSION_H
