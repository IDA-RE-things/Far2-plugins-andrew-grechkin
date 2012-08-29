#ifndef _FAR_GLOBALINFO_HPP_
#define _FAR_GLOBALINFO_HPP_

#include <libfar3/helper.hpp>

#include <libbase/shared_ptr.hpp>


extern Base::shared_ptr<Far::GlobalInfo_i> globalInfo;


///====================================================================================== GlobalInfo
struct FarGlobalInfo: public Far::GlobalInfo_i {
	static FarGlobalInfo * create();

	virtual void destroy();

	virtual GUID get_guid() const;

	virtual PCWSTR get_name() const;

	virtual PCWSTR get_description() const;

	virtual PCWSTR get_author() const;

	virtual PCWSTR get_prefix() const;

	virtual void GetGlobalInfo(GlobalInfo * info) const;

private:
	~FarGlobalInfo();

	FarGlobalInfo();
};


#endif
