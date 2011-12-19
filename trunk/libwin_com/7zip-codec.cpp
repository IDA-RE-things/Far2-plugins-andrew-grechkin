#include "7zip.h"
#include <libwin_net/exception.h>

namespace SevenZip {
	///======================================================================================= Codec
	Codec::Codec(const Lib & arc_lib, size_t idx):
		updatable(false) {
		CheckApiError(arc_lib.get_prop(idx, NArchive::kClassID, guid));
		CheckApiError(arc_lib.get_prop(idx, NArchive::kName, name));
		arc_lib.get_prop(idx, NArchive::kExtension, ext);
		arc_lib.get_prop(idx, NArchive::kAddExtension, add_ext);
		arc_lib.get_prop(idx, NArchive::kAssociate, kAssociate);
		arc_lib.get_prop(idx, NArchive::kStartSignature, start_sign);
		arc_lib.get_prop(idx, NArchive::kFinishSignature, finish_sign);
		arc_lib.get_prop(idx, NArchive::kUpdate, updatable);
		arc_lib.get_prop(idx, NArchive::kKeepName, kKeepName);
	}

	bool Codec::operator<(const Codec & rhs) const {
		return name < rhs.name;
	}

	bool Codec::operator==(const Codec & rhs) const {
		return name == rhs.name;
	}

	bool Codec::operator!=(const Codec & rhs) const {
		return name != rhs.name;
	}

	///====================================================================================== Codecs
	Codecs::Codecs() {
	}

	Codecs::Codecs(const Lib & lib) {
		cache(lib);
	}

	void Codecs::cache(const Lib & lib) {
		UInt32 num_formats = 0;
		CheckCom(lib.GetNumberOfFormats(&num_formats));
		clear();
		for (UInt32 idx = 0; idx < num_formats; ++idx) {
			std::tr1::shared_ptr<Codec> tmp(new Codec(lib, idx));
			insert(value_type(tmp->name, tmp));
		}
	}
}
