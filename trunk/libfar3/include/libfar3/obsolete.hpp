#ifndef OBSOLETE_HPP_
#define OBSOLETE_HPP_

#include <libfar3/helper.hpp>


namespace Far {

	///=============================================================================================
	struct InitDialogItemF {
		FARDIALOGITEMTYPES Type;
		int X1, Y1, X2, Y2;
		DWORD Flags;
		PCWSTR Data;
	};

	inline void InitDialogItemsF(const InitDialogItemF * Init, FarDialogItem * Item, int ItemsNumber) {
		for (int i = 0; i < ItemsNumber; ++i) {
			Base::Memory::zero(&Item[i], sizeof(Item[i]));
			Item[i].Type = Init[i].Type;
			Item[i].X1 = Init[i].X1;
			Item[i].Y1 = Init[i].Y1;
			Item[i].X2 = Init[i].X2;
			Item[i].Y2 = Init[i].Y2;
			Item[i].Flags = Init[i].Flags;
			if ((DWORD_PTR)Init[i].Data < 2048) {
				Item[i].Data = Far::get_msg((size_t)Init[i].Data);
			} else {
				Item[i].Data = Init[i].Data;
			}
		}
	}

}


#endif
