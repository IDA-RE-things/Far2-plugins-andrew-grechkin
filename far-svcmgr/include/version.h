#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{

	//Date Version Types
	static const char DATE[] = "10";
	static const char MONTH[] = "09";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.09";

	//Software Status
	static const char STATUS[] = "Release Candidate";
	static const char STATUS_SHORT[] = "rc";

	//Standard Version Type
	static const long MAJOR = 2;
	static const long MINOR = 0;
	static const long BUILD = 10;
	static const long REVISION = 27;

	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 10;
	#define RC_FILEVERSION 2,0,10,27
	#define RC_FILEVERSION_STRING "2, 0, 10, 27\0"
	static const char FULLVERSION_STRING[] = "2.0.10.27";

	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 4;


}
#endif //VERSION_H
