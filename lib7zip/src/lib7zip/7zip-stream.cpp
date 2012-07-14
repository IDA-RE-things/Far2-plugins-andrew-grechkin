#include <lib7zip/7zip.hpp>
#include <libext/exception.hpp>

#define UNKNOWN_IMPL_ITF(iid) \
	if (riid == IID_##iid) { *object = this; AddRef(); return S_OK; }

namespace SevenZip {
	///============================================================================== FileReadStream
	ULONG WINAPI FileReadStream::AddRef() {
		return UnknownImp::AddRef();
	}

	ULONG WINAPI FileReadStream::Release() {
		return UnknownImp::Release();
	}

	HRESULT WINAPI FileReadStream::QueryInterface(REFIID riid, void** object) {
		//	printf(L"FileReadStream::QueryInterface()\n");
		UNKNOWN_IMPL_ITF(IInStream)
		UNKNOWN_IMPL_ITF(ISequentialInStream)
		return UnknownImp::QueryInterface(riid, object);
	}

	FileReadStream::~FileReadStream() {
		//	printf(L"FileReadStream::~FileReadStream()\n");
	}

	FileReadStream::FileReadStream(const ustring & path):
		WinFile(path, FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN) {
		//	printf(L"FileReadStream::FileReadStream(%s)\n", path);
	}

	HRESULT WINAPI FileReadStream::Read(void * data, UInt32 size, UInt32 * processedSize) {
		//	printf(L"FileReadStream::Read(%d)\n", size);
		try {
			DWORD read = WinFile::read(data, size);
			if (processedSize)
				*processedSize = read;
		} catch (Ext::AbstractError & e) {
			return e.code();
		} catch (...) {
			return E_FAIL;
		}
		return S_OK;
	}

	HRESULT WINAPI FileReadStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) {
		//	printf(L"FileReadStream::Seek(%Id, %d)\n", offset, seekOrigin);
		try {
			uint64_t new_position;
			WinFile::set_position(offset, seekOrigin);
			new_position = WinFile::get_position();
			if (newPosition)
				*newPosition = new_position;
		} catch (Ext::AbstractError & e) {
			return e.code();
		} catch (...) {
			return E_FAIL;
		}
		return S_OK;
	}

	///============================================================================= FileWriteStream
	ULONG WINAPI FileWriteStream::AddRef() {
		return UnknownImp::AddRef();
	}

	ULONG WINAPI FileWriteStream::Release() {
		return UnknownImp::Release();
	}

	HRESULT WINAPI FileWriteStream::QueryInterface(REFIID riid, void ** object) {
		UNKNOWN_IMPL_ITF(IOutStream)
		UNKNOWN_IMPL_ITF(ISequentialOutStream)
		return UnknownImp::QueryInterface(riid, object);
	}

	FileWriteStream::~FileWriteStream() {
		//	printf(L"FileWriteStream::~FileWriteStream()\n");
	}

	FileWriteStream::FileWriteStream(const ustring & path, DWORD creat):
		WinFile(path, GENERIC_READ | GENERIC_WRITE, 0, nullptr, creat, 0) {
	}

	HRESULT WINAPI FileWriteStream::Write(PCVOID data, UInt32 size, UInt32 * processedSize) {
		DWORD written;
		bool result = write_nt(data, size, written);
		if (processedSize != NULL)
			*processedSize = written;
		return ConvertBoolToHRESULT(result);
	}

	HRESULT WINAPI FileWriteStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) {
		HRESULT result = ConvertBoolToHRESULT(set_position_nt(offset, seekOrigin));
		if (newPosition != NULL)
			*newPosition = get_position();
		return result;
	}

	HRESULT WINAPI FileWriteStream::SetSize(UInt64 newSize) {
		uint64_t currentPos = get_position();
		set_position(newSize);
		HRESULT result = ConvertBoolToHRESULT(set_eof());
		set_position(currentPos);
		return result;
	}
}
