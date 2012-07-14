#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{

	//Date Version Types
	static const char DATE[] = "10";
	static const char MONTH[] = "07";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.07";

	//Software Status
	static const char STATUS[] = "Release";
	static const char STATUS_SHORT[] = "r";

	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 0;
	static const long BUILD = 11;
	static const long REVISION = 16;

	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 11;
	#define RC_FILEVERSION 1,0,11,16
	#define RC_FILEVERSION_STRING "1, 0, 11, 16\0"
	static const char FULLVERSION_STRING[] = "1.0.11.16";

	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 2;


}
#endif //VERSION_H
