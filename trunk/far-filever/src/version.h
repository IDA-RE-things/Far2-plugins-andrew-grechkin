#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{

	//Date Version Types
	static const char DATE[] = "15";
	static const char MONTH[] = "09";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.09";

	//Software Status
	static const char STATUS[] = "Release";
	static const char STATUS_SHORT[] = "r";

	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 0;
	static const long BUILD = 12;
	static const long REVISION = 17;

	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 12;
	#define RC_FILEVERSION 1,0,12,17
	#define RC_FILEVERSION_STRING "1, 0, 12, 17\0"
	static const char FULLVERSION_STRING[] = "1.0.12.17";

	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 2;


}
#endif //VERSION_H
