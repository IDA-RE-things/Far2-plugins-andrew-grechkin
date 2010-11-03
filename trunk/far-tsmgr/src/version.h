#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "13";
	static const char MONTH[] = "10";
	static const char YEAR[] = "2010";
	static const char UBUNTU_VERSION_STYLE[] = "10.10";
	
	//Software Status
	static const char STATUS[] = "Beta";
	static const char STATUS_SHORT[] = "b";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 0;
	static const long BUILD = 6;
	static const long REVISION = 21;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 148;
	#define RC_FILEVERSION 1,0,6,21
	#define RC_FILEVERSION_STRING "1, 0, 6, 21\0"
	static const char FULLVERSION_STRING[] = "1.0.6.21";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 4;
	

}
#endif //VERSION_H
