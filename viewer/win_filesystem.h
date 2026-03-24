#ifndef WIN_FILESYSTEM_H_
#define WIN_FILESYSTEM_H_

#include <string>
#include <vector>

namespace win_filesystem
{
	bool CreateFilePathList(const wchar_t* pwzFolderPath, const wchar_t* pwzFileSpec, std::vector<std::wstring>& paths);
	bool GetFilePathListAndIndex(const std::wstring& wstrPath, const wchar_t* pwzFileSpec, std::vector<std::wstring>& paths, size_t* nIndex);
	std::string LoadFileAsString(const wchar_t* pwzFilePath);
	std::wstring GetCurrentProcessPath();
	std::wstring CreateWorkFolder(const std::wstring &wstrRelativePath);
}
#endif // WIN_FILESYSTEM_H_
