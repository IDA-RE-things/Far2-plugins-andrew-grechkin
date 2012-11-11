#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{

	//Date Version Types
	static const char DATE[] = "19";
	static const char MONTH[] = "10";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.10";

	//Software Status
	static const char STATUS[] = "Release Candidate";
	static const char STATUS_SHORT[] = "rc";

	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 0;
	static const long BUILD = 13;
	static const long REVISION = 0;

	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 13;
	#define RC_FILEVERSION 1,0,13,0
	#define RC_FILEVERSION_STRING "1, 0, 13, 0\0"
	static const char FULLVERSION_STRING[] = "1.0.13.0";

}
#endif //VERSION_H
