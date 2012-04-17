#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{

	//Date Version Types
	static const char DATE[] = "10";
	static const char MONTH[] = "04";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.04";

	//Software Status
	static const char STATUS[] = "Release";
	static const char STATUS_SHORT[] = "r";

	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 0;
	static const long BUILD = 10;
	static const long REVISION = 15;

	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 10;
	#define RC_FILEVERSION 1,0,10,15
	#define RC_FILEVERSION_STRING "1, 0, 10, 15\0"
	static const char FULLVERSION_STRING[] = "1.0.10.15";

	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 2;


}
#endif //VERSION_H
