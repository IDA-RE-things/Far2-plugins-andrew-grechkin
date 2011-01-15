#ifndef _WIN_NET_VOLUME_H_
#define _WIN_NET_VOLUME_H_

#include <libwin_def/std.h>

const size_t MAX_VOL_LEN = 64;
const size_t MAX_MOUNT_POINT_LEN = 512;

/**
 * Callback-функция для перечисления томов/точек монтирования
 * @param[in] name - имя тома
 * @param[in,out] data - дополнительные данные, переданные при вызове перечислителя
 * return \c true - если нужно продолжить перечисление
 * @sa EnumVolumes
*/
typedef bool (*ENUMVOLUMECALLBACK)(PCWSTR name, PVOID data);

void EnumVolumes(ENUMVOLUMECALLBACK callback, PVOID callbackData);

void EnumVolumeMountPoints(ENUMVOLUMECALLBACK callback, PCWSTR volName, PVOID callbackData);

bool GetVolumePathNamesList(PCWSTR volName, TCHAR *&mntPointsList, DWORD &listSize);

bool CheckDriveRemovable(PCWSTR name, bool &isRemovable);

AutoUTF GetVolumeByFileName(PCWSTR fileName);

void GetDriveGeometry(PCWSTR name, DISK_GEOMETRY &g);

void GetVolumeDiskExtents(PCWSTR name, auto_buf<PVOLUME_DISK_EXTENTS> &v);

#endif
