#ifndef SVC_MGR_OPT_HPP
#define SVC_MGR_OPT_HPP

class PluginOptions {
	int AddToDisksMenu;
	int DisksMenuDigit;
	int AddToPluginsMenu;
public:
	void		Read();
	void		Write();
};


#endif // SVC_MGR_OPT
