#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "28";
	static const char MONTH[] = "07";
	static const char YEAR[] = "2010";
	static const char UBUNTU_VERSION_STYLE[] = "10.07";
	
	//Software Status
	static const char STATUS[] = "Release Candidate";
	static const char STATUS_SHORT[] = "rc";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 1;
	static const long BUILD = 5;
	static const long REVISION = 17;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 109;
	#define RC_FILEVERSION 1,1,5,17
	#define RC_FILEVERSION_STRING "1, 1, 5, 17\0"
	static const char FULLVERSION_STRING[] = "1.1.5.17";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 3;
	

}
#endif //VERSION_H
