#ifndef _FAR_PANELACTIONS_HPP_
#define _FAR_PANELACTIONS_HPP_

#include <libfar3/helper.hpp>


struct PanelController;


struct PanelActions {
	typedef bool (PanelController::* ptrToFunc)();

	PanelActions();

	void add(WORD Key, DWORD Control, PCWSTR text, ptrToFunc func = nullptr, PCWSTR long_text = nullptr);

	bool exec_func(PanelController * panel, WORD Key, DWORD Control) const;

	const KeyBarTitles * get_titles() const;

private:
	struct KeyAction {
		FarKey Key;
		ptrToFunc Action;
	};

	std::vector<KeyBarLabel> m_labels;
	std::vector<KeyAction> m_actions;
	KeyBarTitles m_titles;
};




#endif
