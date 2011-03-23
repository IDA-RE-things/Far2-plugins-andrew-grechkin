#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "23";
	static const char MONTH[] = "03";
	static const char YEAR[] = "2011";
	static const char UBUNTU_VERSION_STYLE[] = "11.03";
	
	//Software Status
	static const char STATUS[] = "Release";
	static const char STATUS_SHORT[] = "r";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 6;
	static const long BUILD = 15;
	static const long REVISION = 54;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 1;
	#define RC_FILEVERSION 1,6,15,54
	#define RC_FILEVERSION_STRING "1, 6, 15, 54\0"
	static const char FULLVERSION_STRING[] = "1.6.15.54";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 10;
	

}
#endif //VERSION_H
