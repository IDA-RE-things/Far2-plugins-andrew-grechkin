#ifndef _FAR_OPTIONS_HPP_
#define _FAR_OPTIONS_HPP_

#include <libfar3/helper.hpp>

#include <libbase/shared_ptr.hpp>


typedef Far::Settings_t Settings_type;


///========================================================================================= Options
struct Options {
	ssize_t AddToPluginsMenu;
	ssize_t AddToDisksMenu;
	ssize_t TimeOut;
	WCHAR Prefix[32];

	Options();

	void load();

	void save() const;

	int configure();

private:
	Base::shared_ptr<Settings_type> m_settings;
	WCHAR Timeout[3];
};


extern Base::shared_ptr<Options> options;


#endif
