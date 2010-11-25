#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "25";
	static const char MONTH[] = "11";
	static const char YEAR[] = "2010";
	static const char UBUNTU_VERSION_STYLE[] = "10.11";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 3;
	static const long BUILD = 11;
	static const long REVISION = 51;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 337;
	#define RC_FILEVERSION 1,3,11,51
	#define RC_FILEVERSION_STRING "1, 3, 11, 51\0"
	static const char FULLVERSION_STRING[] = "1.3.11.51";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 9;
	

}
#endif //VERSION_H
