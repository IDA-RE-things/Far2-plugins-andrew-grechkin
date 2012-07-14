#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{

	//Date Version Types
	static const char DATE[] = "10";
	static const char MONTH[] = "07";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.07";

	//Software Status
	static const char STATUS[] = "Release Candidate";
	static const char STATUS_SHORT[] = "rc";

	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 2;
	static const long BUILD = 8;
	static const long REVISION = 25;

	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 6;
	#define RC_FILEVERSION 1,2,8,25
	#define RC_FILEVERSION_STRING "1, 2, 8, 25\0"
	static const char FULLVERSION_STRING[] = "1.2.8.25";

	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 4;


}
#endif //VERSION_H
