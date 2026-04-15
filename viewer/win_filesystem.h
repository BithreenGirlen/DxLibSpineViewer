#ifndef WIN_FILESYSTEM_H_
#define WIN_FILESYSTEM_H_

#include <string>
#include <vector>

namespace win_filesystem
{
	/* This project uses C++14, so std::wstring_view is not available. */

	bool CreateFilePathList(const wchar_t* folderPath, size_t folderPathLength, const wchar_t* fileSpec, std::vector<std::wstring>& paths);
	bool CreateFilePathList(const std::wstring& folderPath, const wchar_t* fileSpec, std::vector<std::wstring>& paths);
	bool GetFilePathListAndIndex(const std::wstring& path, const wchar_t* fileSpec, std::vector<std::wstring>& paths, size_t* nIndex);

	/// @brief Get the directory where the executable exists.
	/// @param length A pointer to receive length
	/// @return A pointer to static buffer
	const wchar_t* GetCurrentProcessPath(size_t* length);

	bool CreateSubDirectoryToBuffer(const wchar_t* directoryName, size_t directoryNameLength, wchar_t* dst, size_t dstSize, size_t& nWritten);
	std::wstring CreateSubDirectory(const wchar_t* relativePath, size_t relativePathLength);
	std::wstring CreateSubDirectory(const std::wstring& relativePath);

	std::string LoadFileAsString(const wchar_t* filePath);
	bool SaveDataToFile(const wchar_t* filePath, const void* pData, unsigned long dataLength, bool toOverwrite = true);
	
	bool DoesFileExist(const wchar_t* filePath);
	bool RenameFile(const wchar_t* filePathOld, const wchar_t* filePathNew);
}
#endif // WIN_FILESYSTEM_H_
