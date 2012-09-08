#ifndef _FAR_GLOBALINFO_HPP_
#define _FAR_GLOBALINFO_HPP_

#include <libfar3/helper.hpp>

#include <libbase/shared_ptr.hpp>


///====================================================================================== GlobalInfo
struct FarGlobalInfo: public Far::GlobalInfo_i, private Base::Uncopyable {
	FarGlobalInfo();

	~FarGlobalInfo();

	virtual GUID get_guid() const;

	virtual PCWSTR get_name() const;

	virtual PCWSTR get_description() const;

	virtual PCWSTR get_author() const;

	virtual void GetGlobalInfo(GlobalInfo * info) const;

	virtual int Configure(const ConfigureInfo * Info);

	void load_settings();

	ssize_t AddToPluginsMenu;
	ssize_t AddToDisksMenu;
	ssize_t TimeOut;
	WCHAR Prefix[32];

private:
	void save_settings() const;

	Far::Settings_t * m_settings;
};


extern Base::shared_ptr<FarGlobalInfo> globalInfo;


FarGlobalInfo * create_GlobalInfo();

void destroy(FarGlobalInfo * info);


#endif
