#include <libcom/std.hpp>
//#include <libcom/msxml.hpp>
#include <libcom/variant.hpp>
#include <libcom/bstr.hpp>
#include <libext/exception.hpp>
#include <libbase/logger.hpp>
#include <libbase/str.hpp>


void test_msxml() {
	using Com::ComObject;
	using Com::BStr;
	using Com::Variant;

	ComObject<IXMLDOMDocument> spXMLDOM;
	CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument,(PVOID*)&spXMLDOM);

	VARIANT_BOOL ret;
	CheckApiError(spXMLDOM->load(Variant(L"C:\\1.xml"), &ret));
	LogDebug(L"XML loaded: %d\n", (int)ret);

	BStr bstrSS(L"xmldata/xmlnode");
	ComObject<IXMLDOMNode> spXMLNode;
	CheckApiError(spXMLDOM->selectSingleNode(bstrSS, &spXMLNode));

	ComObject<IXMLDOMNode> spXMLChildNode;
	CheckApiError(spXMLDOM->createNode(Variant((DWORD)NODE_ELEMENT), BStr(L"xmlchildnode"), NULL, &spXMLChildNode));

	ComObject<IXMLDOMNode> spInsertedNode;
	CheckApiError(spXMLNode->appendChild(spXMLChildNode,&spInsertedNode));

	CheckApiError(spXMLDOM->save(Variant(L"c:\\updatedxml.xml")));
}


int main() try {
	Base::Logger::set_target(Base::Logger::get_TargetToConsole());
	Base::Logger::set_color_mode(true);
	Base::Logger::set_level(Base::Logger::LVL_TRACE);
	LogTrace();

	Com::init();
	test_msxml();

	return 0;
} catch (Ext::AbstractError & e) {
	LogError(L"exception cought: %s, %s", e.what().c_str(), e.where());
	return e.code();
} catch (std::exception & e) {
	LogError(L"std::exception [%S]:\n", typeid(e).name());
	LogError(L"What: %S\n", e.what());
	return 1;
}


