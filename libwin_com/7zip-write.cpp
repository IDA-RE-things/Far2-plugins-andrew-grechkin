#include "7zip.h"
#include <libwin_net/exception.h>

#define UNKNOWN_IMPL_ITF(iid) \
	if (riid == IID_##iid) { *object = static_cast<iid*>(this); AddRef(); return S_OK; }

namespace SevenZip {
	DirItem::DirItem(const ustring & file_path, const ustring & file_name):
		WinFileInfo(MakePath(file_path, file_name).c_str()),
		path(file_path),
		name(file_name) {
	}

	///================================================================================ DirStructure
	DirStructure::DirStructure() {
	}

	DirStructure::DirStructure(const ustring & path) {
		add(path);
	}

	void DirStructure::add(const ustring & add_path) {
		ustring path(PathNice(add_path));
		path = get_fullpath(ensure_no_end_path_separator(path));
		path = ensure_path_prefix(path);
		if (FS::is_exist(path)) {
			size_t pos = path.find_last_of(PATH_SEPARATORS);
			if (pos != ustring::npos) {
				base_add(path.substr(0, pos), path.substr(pos + 1));
			}
		}
	}

	void DirStructure::base_add(const ustring & base_path, const ustring & name) {
		//	printf(L"DirStructure::base_add(%s, %s)\n", base_path.c_str(), name.c_str());
		push_back(DirItem(base_path, name));
		ustring path(MakePath(base_path, name));
		if (FS::is_dir(path)) {
			WinDir dir(path);
			for (WinDir::iterator it = dir.begin(); it != dir.end(); ++it) {
				if (it.is_dir() || it.is_link_dir()) {
					base_add(base_path, MakePath(name, it.name()));
				} else {
					base_add(base_path, MakePath(name, it.name()));
					//				push_back(DirItem(base_path, MakePath(name, it.name())));
				}
			}
		}
	}

	///============================================================================== UpdateCallback
	UpdateCallback::~UpdateCallback() {
		//	printf(L"ArchiveUpdateCallback::~ArchiveUpdateCallback()\n");
	}

	UpdateCallback::UpdateCallback(const DirStructure & items, const ustring & pass):
		Password(pass),
		AskPassword(false),
		IgnoreErrors(true),
		DirItems(items) {
		//	printf(L"ArchiveUpdateCallback::ArchiveUpdateCallback()\n");
	};

	ULONG WINAPI UpdateCallback::AddRef() {
		return UnknownImp::AddRef();
	}

	ULONG WINAPI UpdateCallback::Release() {
		return UnknownImp::Release();
	}

	HRESULT WINAPI UpdateCallback::QueryInterface(REFIID riid, void** object) {
		//	printf(L"ArchiveUpdateCallback::QueryInterface()\n");
		UNKNOWN_IMPL_ITF(IArchiveUpdateCallback2)
		UNKNOWN_IMPL_ITF(ICryptoGetTextPassword2)
		return UnknownImp::QueryInterface(riid, object);
	}

	HRESULT WINAPI UpdateCallback::SetTotal(UInt64 /*size*/) {
		//	printf(L"ArchiveUpdateCallback::SetTotal(%Id)\n", size);
		return S_OK;
	}

	HRESULT WINAPI UpdateCallback::SetCompleted(const UInt64 * /* completeValue */) {
		//	printf(L"ArchiveUpdateCallback::SetCompleted()\n");
		return S_OK;
	}

	HRESULT WINAPI UpdateCallback::GetUpdateItemInfo(UInt32 /*index*/, Int32 * newData, Int32 * newProperties, UInt32 * indexInArchive) {
		//	printf(L"ArchiveUpdateCallback::GetUpdateItemInfo(%d)\n", index);
		if (newData)
			*newData = Int32(true);
		if (newProperties)
			*newProperties = Int32(true);
		if (indexInArchive)
			*indexInArchive = (UInt32)-1;
		return S_OK;
	}

	HRESULT WINAPI UpdateCallback::GetProperty(UInt32 index, PROPID propID, PROPVARIANT * value) {
		PropVariant prop;

		if (propID == kpidIsAnti) {
			prop = false;
			prop.detach(value);
			return S_OK;
		}

		const DirItem & dirItem = DirItems[index];
		switch (propID) {
			case kpidIsDir:
				prop = dirItem.is_dir_or_link();
				break;

			case kpidPosixAttrib:
				//			prop = (uint32_t)0777;
				break;

			case kpidMTime:
				prop = dirItem.mtime_ft();
				break;

			case kpidPath:
				prop = dirItem.name.c_str();
				break;

			case kpidUser:
				//			prop = L"user";
				break;

			case kpidGroup:
				//			prop = L"group";
				break;

			case kpidSize:
				prop = dirItem.is_dir_or_link() ? 0ull : dirItem.size();
				break;

			case kpidAttrib:
				prop = (uint32_t)dirItem.attr();
				break;

			case kpidCTime:
				prop = dirItem.ctime_ft();
				break;

			case kpidATime:
				prop = dirItem.atime_ft();
				break;

				//		case kpidIsAnti:
				//			prop = false;
				//			break;

			case kpidTimeType:
				prop = (uint32_t)NFileTimeType::kWindows;
				break;

			default:
				printf(L"ArchiveUpdateCallback::GetProperty(%d, %d [%s])\n", index, propID, NamedValues<int>::GetName(ArcItemPropsNames, sizeofa(ArcItemPropsNames), (int) propID));
				break;
		}
		prop.detach(value);
		return S_OK;
	}

	HRESULT WINAPI UpdateCallback::GetStream(UInt32 index, ISequentialInStream ** inStream) {
		//	printf(L"ArchiveUpdateCallback::GetStream(%d)\n", index);

		const DirItem &dirItem = DirItems[index];
		//	PrintString("Compressing  ");
		PrintString(dirItem.name);

		if (dirItem.is_dir_or_link())
			return S_OK;

		try {
			FileReadStream * stream(new FileReadStream(MakePath(dirItem.path, dirItem.name).c_str()));
			ComObject<ISequentialInStream>(stream).detach(*inStream);
		} catch (WinError &e) {
			failed_files.push_back(FailedFile(dirItem.name, e.code()));
			// if (systemError == ERROR_SHARING_VIOLATION)
			{
				PrintString(L"\nWARNING: can't open file");
				PrintString(e.what());
				return S_FALSE;
			}
		}
		return S_OK;
	}

	HRESULT WINAPI UpdateCallback::SetOperationResult(Int32 /*operationResult*/) {
		//	printf(L"ArchiveUpdateCallback::SetOperationResult(%d)\n", operationResult);
		return S_OK;
	}

	HRESULT WINAPI UpdateCallback::GetVolumeSize(UInt32 index, UInt64 *size) {
		printf(L"ArchiveUpdateCallback::GetVolumeSize(%d)\n", index);
		if (VolumesSizes.size() == 0)
			return S_FALSE;
		if (index >= (UInt32)VolumesSizes.size())
			index = VolumesSizes.size() - 1;
		*size = VolumesSizes[index];
		return S_OK;
	}

	HRESULT WINAPI UpdateCallback::GetVolumeStream(UInt32 index, ISequentialOutStream **volumeStream) {
		printf(L"ArchiveUpdateCallback::GetVolumeStream(%d)\n", index);
		ustring res = Num2Str(index + 1);
		while (res.size() < 2)
			res = ustring(L"0") + res;
		ustring fileName = VolName;
		fileName += L'.';
		fileName += res;
		fileName += VolExt;
		try {
			FileWriteStream * stream(new FileWriteStream(fileName.c_str(), false));
			ComObject<ISequentialOutStream>(stream).detach(*volumeStream);
		} catch (WinError &e) {
			return S_FALSE;
		}
		return S_OK;
	}

	HRESULT WINAPI UpdateCallback::CryptoGetTextPassword2(Int32 * passwordIsDefined, BSTR * password) {
//		printf(L"%S\n", __PRETTY_FUNCTION__);
		if (Password.empty() && AskPassword) {
			// You can ask real password here from user
			// Password = GetPassword(OutStream);
			// PasswordIsDefined = true;
			PrintString("Password is not defined");
			return E_ABORT;
		}
		try {
			*passwordIsDefined = !Password.empty();
			BStr(Password).detach(*password);
		} catch (WinError &e) {
			return E_ABORT;
		}
		return S_OK;
	}

	///=============================================================================== CreateArchive
	CompressProperties::CompressProperties():
		level(5),
		solid(false),
		encrypt_header(false) {
	}

	CreateArchive::CreateArchive(const Lib & lib, const ustring & path, const ustring & codec):
		m_lib(lib),
		m_path(path),
		m_codec(codec) {
		CheckCom(m_lib.CreateObject(&m_lib.codecs().at(codec)->guid, &IID_IOutArchive, (PVOID*)&m_arc));
	}

	void CreateArchive::compress() {
		set_properties();

		ComObject<IOutStream> outStream(new FileWriteStream(m_path + L"." + m_codec, CREATE_NEW));
		ComObject<IArchiveUpdateCallback2> updateCallback(new UpdateCallback(*this, password));

		CheckCom(m_arc->UpdateItems(outStream, DirStructure::size(), updateCallback));
	}

	ComObject<IOutArchive> CreateArchive::operator->() const {
		return m_arc;
	}

	void CreateArchive::set_properties() {
		ComObject<ISetProperties> setProperties;
		m_arc->QueryInterface(IID_ISetProperties, (PVOID*)&setProperties);
		if (setProperties) {
			std::vector<PCWSTR> prop_names;
			std::vector<PropVariant> prop_vals;

			prop_names.push_back(L"x"); prop_vals.push_back(PropVariant((UInt32)level));
			if (m_codec == L"7z") {
//				prop_names.push_back(L"0"); prop_vals.push_back(PropVariant(m_lib.methods().at(method)->name));
				prop_names.push_back(L"s"); prop_vals.push_back(PropVariant(solid));
				prop_names.push_back(L"he"); prop_vals.push_back(PropVariant(encrypt_header));
			} else if (m_codec == L"zip") {
				if (!password.empty()) {
//					prop_names.push_back(L"p"); prop_vals.push_back(PropVariant(password));
				}
			}
			CheckCom(setProperties->SetProperties(&prop_names[0], &prop_vals[0], prop_names.size()));
		}
	}
}
