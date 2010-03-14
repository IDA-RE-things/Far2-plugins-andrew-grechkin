#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "13";
	static const char MONTH[] = "03";
	static const char YEAR[] = "2010";
	static const char UBUNTU_VERSION_STYLE[] = "10.03";
	
	//Software Status
	static const char STATUS[] = "Release";
	static const char STATUS_SHORT[] = "r";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 0;
	static const long BUILD = 4;
	static const long REVISION = 10;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 1;
	#define RC_FILEVERSION 1,0,4,10
	#define RC_FILEVERSION_STRING "1, 0, 4, 10\0"
	static const char FULLVERSION_STRING[] = "1.0.4.10";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 1;
	

}
#endif //VERSION_H
