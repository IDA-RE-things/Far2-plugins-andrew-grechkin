#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "05";
	static const char MONTH[] = "03";
	static const char YEAR[] = "2010";
	static const char UBUNTU_VERSION_STYLE[] = "10.03";
	
	//Software Status
	static const char STATUS[] = "Beta";
	static const char STATUS_SHORT[] = "b";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 0;
	static const long BUILD = 3;
	static const long REVISION = 14;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 14;
	#define RC_FILEVERSION 1,0,3,14
	#define RC_FILEVERSION_STRING "1, 0, 3, 14\0"
	static const char FULLVERSION_STRING[] = "1.0.3.14";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 3;
	

}
#endif //VERSION_H
