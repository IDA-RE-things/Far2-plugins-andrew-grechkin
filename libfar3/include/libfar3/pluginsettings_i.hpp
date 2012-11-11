#ifndef _FAR_PLUGINSETTINGS_I_HPP_
#define _FAR_PLUGINSETTINGS_I_HPP_

#include <libfar3/fwd.hpp>

namespace Far {

	///============================================================================ PluginSettings_i
	struct PluginSettings_i {
		virtual ~PluginSettings_i();

	public:
		// nvi
		bool load_settings()
		{
			return load_settings_();
		}

		bool save_settings() const
		{
			return save_settings_();
		}

	private:
		virtual bool load_settings_();

		virtual bool save_settings_() const;
	};

}

#endif
