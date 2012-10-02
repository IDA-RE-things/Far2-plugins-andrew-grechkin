#include <libext/exception.hpp>

#include <libext/dacl.hpp>
#include <libext/sd.hpp>

#include <libext/rc.hpp>
#include <libext/service.hpp>
#include <libext/services.hpp>

#include <libbase/logger.hpp>
#include <libbase/observer_p.hpp>

Ext::WinSD init_sd(bool allow_network) {
	LogTrace();
	Ext::Sid accessSid(allow_network ? WinWorldSid : WinLocalSid);
	Ext::Sid systemSid(WinLocalSystemSid);

	Ext::ExpAccessArray eaa(2);
	new (&eaa[0]) Ext::ExpAccess(accessSid, FILE_ALL_ACCESS, SET_ACCESS);
	new (&eaa[1]) Ext::ExpAccess(systemSid, FILE_ALL_ACCESS, SET_ACCESS);

	return Ext::WinSD((PSID)accessSid, (PSID)accessSid, &eaa, nullptr);
}

void test_sd() {
	LogTrace();
	Ext::Sid accessSid(true ? WinWorldSid : WinLocalSid);
	Ext::Sid systemSid(WinLocalSystemSid);

	LogTrace();
	Ext::ExpAccessArray eaa(2);
	new (&eaa[0]) Ext::ExpAccess(accessSid, FILE_ALL_ACCESS, SET_ACCESS);
	new (&eaa[1]) Ext::ExpAccess(systemSid, FILE_ALL_ACCESS, SET_ACCESS);

	LogTrace();
	Ext::WinSD sd(init_sd(true));

	LogTrace();
	Ext::WinSD::parse(sd);
	Ext::WinDacl::parse(sd.get_dacl());
	Ext::WinDacl::parse(sd.get_sacl());
	LogTrace();
}


struct ServicesView: public Base::Observer_p {
	ServicesView(Ext::Services * svcs):
		Base::Observer_p(svcs),
		m_svcs(svcs)
	{
		LogTrace();
	}

	void notify(void * /*data*/) {
		LogReport(L"Services changed. size: %Iu\n", m_svcs->size());
	}

private:
	Ext::Services * m_svcs;
};

void test_service() {
	LogTrace();

//	Ext::RemoteConnection rc(L"localhost");
//	Ext::RemoteConnection * prc = nullptr;
//
//	LogDebug(L"host1: '%s'\n", rc.get_host().c_str());
//	LogDebug(L"host2: '%s'\n", prc->get_host().c_str());
//
//	LogTrace();
//	Ext::Service::Manager scm(&rc);
//
//	LogDebug(L"svc: %d\n", scm.is_exist(L"FARBCopy"));

	LogTrace();
	Ext::Services::Filter filter;
	LogTrace();
	Ext::Services svcs(&filter);
	LogTrace();
	ServicesView obs(&svcs);

	svcs.start_batch();
	LogTrace();
	svcs.get_filter()->set_type(Ext::Service::TYPE_SVC);
	LogTrace();
	svcs.get_filter()->set_type(Ext::Service::TYPE_DRV);
	svcs.stop_batch();

//	LogDebug(L"size: %Iu\n", svcs.size());


	LogTrace();
}


int main() try {
	Base::Logger::set_target(Base::Logger::get_TargetToConsole());
	Base::Logger::set_color_mode(true);
	Base::Logger::set_level(Base::Logger::LVL_TRACE);
	LogTrace();

	test_service();

	return 0;
} catch (Ext::AbstractError & e) {
	LogError(L"exception cought: %s, %s", e.what().c_str(), e.where());
	return e.code();
} catch (std::exception & e) {
	LogError(L"std::exception [%S]:\n", typeid(e).name());
	LogError(L"What: %S\n", e.what());
	return 1;
}
