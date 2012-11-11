#include <libfar3/globalinfo_i.hpp>
#include <libfar3/plugin_i.hpp>

#include <libbase/logger.hpp>

namespace Far {

	///========================================================================================= nvi
	void GlobalInfo_i::GetGlobalInfoW(GlobalInfo * Info) const
	{
		LogTrace();
		Info->StructSize = sizeof(*Info);
		Info->MinFarVersion = FARMANAGERVERSION;
		Info->Author = get_author();
		Info->Description = get_description();
		Info->Guid = *get_guid();
		Info->Title = get_title();
		Info->Version = get_version();
	}

	int GlobalInfo_i::ConfigureW(const ConfigureInfo * Info)
	{
		LogTrace();
		if (Info->StructSize < sizeof(*Info))
		return 0;
		return Configure(Info);
	}

	void GlobalInfo_i::SetStartupInfoW(const PluginStartupInfo * Info)
	{
		LogTrace();
		if (Info->StructSize < sizeof(*Info))
		return;
		m_plugin = CreatePlugin(Info);
	}

	Plugin_i * GlobalInfo_i::get_plugin() const
	{
		return m_plugin;
	}

	///=============================================================================================
	GlobalInfo_i::GlobalInfo_i() :
		m_plugin(nullptr)
	{
		LogTrace();
	}

	GlobalInfo_i::~GlobalInfo_i()
	{
		delete m_plugin;
		LogTrace();
	}

	int GlobalInfo_i::Configure(const ConfigureInfo * /*Info*/)
	{
		LogTrace();
		return 0;
	}

}
