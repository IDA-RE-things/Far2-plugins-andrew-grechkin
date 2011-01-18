#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "18";
	static const char MONTH[] = "01";
	static const char YEAR[] = "2011";
	static const char UBUNTU_VERSION_STYLE[] = "11.01";
	
	//Software Status
	static const char STATUS[] = "Beta";
	static const char STATUS_SHORT[] = "b";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 0;
	static const long BUILD = 2;
	static const long REVISION = 2;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 2;
	#define RC_FILEVERSION 1,0,2,2
	#define RC_FILEVERSION_STRING "1, 0, 2, 2\0"
	static const char FULLVERSION_STRING[] = "1.0.2.2";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 1;
	

}
#endif //VERSION_H
