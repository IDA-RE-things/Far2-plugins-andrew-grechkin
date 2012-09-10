#ifndef GLOBALINFO_I_HPP_
#define GLOBALINFO_I_HPP_

#include <libfar3/plugin.hpp>


namespace Far {

	///================================================================================ GlobalInfo_i
	struct GlobalInfo_i {
		virtual ~GlobalInfo_i();

		virtual GUID get_guid() const = 0;

		virtual PCWSTR get_name() const = 0;

		virtual PCWSTR get_description() const = 0;

		virtual PCWSTR get_author() const = 0;

		virtual VersionInfo get_version() const = 0;

		virtual void GetGlobalInfo(GlobalInfo * info) const;

		virtual int Configure(const ConfigureInfo * Info);
	};

}


#endif
