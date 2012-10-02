#ifndef _FAR_GLOBALINFO_I_HPP_
#define _FAR_GLOBALINFO_I_HPP_

#include <libfar3/plugin.hpp>


namespace Far {


	struct GlobalInfo_i;
	struct PanelController_i;
	struct Plugin_i;


	///================================================================================ GlobalInfo_i
	struct GlobalInfo_i {
	public: // nvi
		void GetGlobalInfoW(GlobalInfo * Info) const;

		int ConfigureW(const ConfigureInfo * Info);

		void SetStartupInfoW(const PluginStartupInfo * Info);

		const GUID * guid() const;

		Plugin_i * get_plugin() const;

	public:
		GlobalInfo_i();

		virtual ~GlobalInfo_i();

	public:
		virtual PCWSTR get_author() const = 0;

		virtual PCWSTR get_description() const = 0;

		virtual const GUID * get_guid() const = 0;

		virtual PCWSTR get_title() const = 0;

		virtual VersionInfo get_version() const = 0;

		virtual void GetInfo(GlobalInfo * Info) const final;

		virtual int Configure(const ConfigureInfo * Info);

		virtual Plugin_i * CreatePlugin(const PluginStartupInfo * Info) = 0;

	private:
		Plugin_i * m_plugin;
	};

}


#endif
