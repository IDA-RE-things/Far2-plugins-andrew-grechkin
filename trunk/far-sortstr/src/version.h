#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "05";
	static const char MONTH[] = "01";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.01";
	
	//Software Status
	static const char STATUS[] = "Release";
	static const char STATUS_SHORT[] = "r";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 7;
	static const long BUILD = 17;
	static const long REVISION = 56;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 1;
	#define RC_FILEVERSION 1,7,17,56
	#define RC_FILEVERSION_STRING "1, 7, 17, 56\0"
	static const char FULLVERSION_STRING[] = "1.7.17.56";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 10;
	

}
#endif //VERSION_H
