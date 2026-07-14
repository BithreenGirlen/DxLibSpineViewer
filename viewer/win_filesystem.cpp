
#include <Windows.h>
#include <shlwapi.h>

#include "win_filesystem.h"

#pragma comment(lib, "Shlwapi.lib")

namespace win_filesystem
{
	/* 最大経路長 */
	static constexpr size_t kMaxPathLength = 1024;

	/// @brief 動的割り当てを行わない文字列操作
	template<size_t N>
	class StaticWString
	{
	public:
		const wchar_t* data() const noexcept { return m_data; }
		size_t size() const noexcept { return m_nWritten; }
		bool empty() const noexcept { return m_nWritten == 0; }
		const wchar_t front() const noexcept { return m_data[0]; }
		const wchar_t back() const noexcept { return m_nWritten == 0 ? m_data[0] : m_data[m_nWritten - 1]; }

		/// @brief 文字列連結
		StaticWString& append(const std::wstring& s) noexcept
		{
			if (m_nWritten + s.size() < MaxSize)
			{
				wmemcpy(m_data + m_nWritten, s.data(), s.size());
				m_nWritten += s.size();
				m_data[m_nWritten] = L'\0';
			}

			return *this;
		}
		StaticWString& append(const wchar_t* s, size_t length) noexcept
		{
			if (m_nWritten + length < MaxSize)
			{
				wmemcpy(m_data + m_nWritten, s, length);
				m_nWritten += length;
				m_data[m_nWritten] = L'\0';
			}

			return *this;
		}
		/// @brief 文字連結
		void pushBack(const wchar_t c) noexcept
		{
			m_data[m_nWritten] = c;
			++m_nWritten;
			m_data[m_nWritten] = L'\0';
		}
		/// @brief 文字挿入
		void insert(const wchar_t c, size_t nPos = 0) noexcept
		{
			if (m_nWritten + 1 > MaxSize)return;

			wmemmove(&m_data[nPos + 1], &m_data[nPos], m_nWritten - nPos);
			m_data[nPos] = c;
			++m_nWritten;
		}
		/// @brief 消去
		void clear() noexcept
		{
			wmemset(m_data, L'\0', MaxSize);
			m_nWritten = 0;
		}
		/// @brief 縮め
		void shrink(size_t nLength) noexcept
		{
			if (nLength >= m_nWritten)return;

			wmemset(m_data + nLength, L'\0', MaxSize - nLength);
			m_nWritten = nLength;
		}
	private:
		wchar_t m_data[N]{};
		size_t m_nWritten = 0;
		static constexpr size_t MaxSize = sizeof(m_data) / sizeof(wchar_t) - 1;
	};

	using StaticWStringMaxPath = StaticWString<kMaxPathLength>;

	/* 指定階層のファイル・フォルダ名一覧取得 */
	static bool CreateFilaNameList(const wchar_t* folderPath, size_t folderPathLength, const wchar_t* fileNamePattern, std::vector<std::wstring>& names)
	{
		StaticWStringMaxPath findDataPath;
		findDataPath.append(folderPath, folderPathLength);

		bool toFindDirectory = fileNamePattern == nullptr;
		if (!toFindDirectory)
		{
			if (wcschr(fileNamePattern, L'*') == nullptr)
			{
				findDataPath.pushBack(L'*');
			}
			findDataPath.append(fileNamePattern, wcslen(fileNamePattern));
		}
		else
		{
			findDataPath.pushBack(L'*');
		}

		WIN32_FIND_DATAW win32FindData;

		HANDLE hFind = ::FindFirstFileW(findDataPath.data(), &win32FindData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			if (!toFindDirectory)
			{
				do
				{
					/* ファイル一覧 */
					if (!(win32FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						names.emplace_back(win32FindData.cFileName);
					}
				} while (::FindNextFileW(hFind, &win32FindData));
			}
			else
			{
				do
				{
					/* フォルダ一覧 */
					if ((win32FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						if (wcscmp(win32FindData.cFileName, L".") != 0 && wcscmp(win32FindData.cFileName, L"..") != 0)
						{
							names.emplace_back(win32FindData.cFileName);
						}
					}
				} while (::FindNextFileW(hFind, &win32FindData));
			}

			::FindClose(hFind);
		}

		return names.size() > 0;
	}
}

bool win_filesystem::CreateFilePathList(const wchar_t* folderPath, size_t folderPathLength, const wchar_t* fileSpec, std::vector<std::wstring>& paths)
{
	if (folderPath == nullptr)return false;

	StaticWStringMaxPath parentFolderPath;
	parentFolderPath.append(folderPath, folderPathLength);
	if (parentFolderPath.back() != L'\\')
	{
		parentFolderPath.pushBack(L'\\');
	}

	std::vector<std::wstring> fileNames;
	if (fileSpec == nullptr)
	{
		CreateFilaNameList(parentFolderPath.data(), parentFolderPath.size(), fileSpec, fileNames);
	}
	else
	{
		using StaticWstring64 = StaticWString<64>;
		size_t fileSpecLength = wcslen(fileSpec);

		for (size_t nRead = 0;;)
		{
			const wchar_t* pPos = wcschr(&fileSpec[nRead], ';');
			if (pPos == nullptr)
			{
				StaticWstring64 s;
				s.append(&fileSpec[nRead], fileSpecLength - nRead);
				CreateFilaNameList(parentFolderPath.data(), parentFolderPath.size(), s.data(), fileNames);

				break;
			}

			size_t nLength = pPos - &fileSpec[nRead];
			StaticWstring64 s;
			s.append(&fileSpec[nRead], nLength);
			CreateFilaNameList(parentFolderPath.data(), parentFolderPath.size(), s.data(), fileNames);

			nRead += nLength + 1;
		}
	}

	/*名前順に整頓*/
	for (size_t i = 0; i < fileNames.size(); ++i)
	{
		size_t nIndex = i;
		for (size_t j = i; j < fileNames.size(); ++j)
		{
			if (::StrCmpLogicalW(fileNames[nIndex].c_str(), fileNames[j].c_str()) > 0)
			{
				nIndex = j;
			}
		}
		std::swap(fileNames[i], fileNames[nIndex]);
	}

	for (const std::wstring& fileName : fileNames)
	{
		paths.emplace_back(parentFolderPath.data() + fileName);
	}

	return !paths.empty();
}

bool win_filesystem::CreateFilePathList(const std::wstring& folderPath, const wchar_t* fileSpec, std::vector<std::wstring>& paths)
{
	return CreateFilePathList(folderPath.c_str(), folderPath.length(), fileSpec, paths);
}

/* 指定経路と同階層のファイル・フォルダ一覧作成・相対位置取得 */
bool win_filesystem::GetFilePathListAndIndex(const std::wstring& path, const wchar_t* pwzFileSpec, std::vector<std::wstring>& paths, size_t* nIndex)
{
	size_t nPos = path.find_last_of(L"\\/");
	if (nPos == std::wstring::npos)return false;

	StaticWStringMaxPath parentPath;
	parentPath.append(path.data(), nPos);

	win_filesystem::CreateFilePathList(parentPath.data(), parentPath.size(), pwzFileSpec, paths);

	const auto& iter = std::find(paths.begin(), paths.end(), path);
	if (iter != paths.end())
	{
		*nIndex = std::distance(paths.begin(), iter);
	}

	return iter != paths.end();
}

const wchar_t* win_filesystem::GetCurrentProcessPath(size_t* length)
{
	static wchar_t s_basePath[kMaxPathLength]{};
	static size_t s_basePathLength = 0;
	if (s_basePath[0] == L'\0')
	{
		static constexpr size_t basePathSize = sizeof(s_basePath) / sizeof(wchar_t);
		DWORD length = ::GetModuleFileNameW(nullptr, s_basePath, basePathSize);
		wchar_t* pEnd = s_basePath + length;
		for (; pEnd != s_basePath; --pEnd)
		{
			if (*pEnd == L'\\' || *pEnd == L'/')break;
		}

		wchar_t* pFileName = pEnd + 1;
		size_t fileNameLength = s_basePath + length - pFileName;
		wmemset(pFileName, L'\0', fileNameLength);

		s_basePathLength = pFileName - s_basePath;
	}

	if (length != nullptr)*length = s_basePathLength;

	return s_basePath;
}

bool win_filesystem::CreateSubDirectoryToBuffer(const wchar_t* directoryName, size_t directoryNameLength, wchar_t* dst, size_t dstSize, size_t& nWritten)
{
	if (directoryName == nullptr)return false;

	size_t nBasePathLength = 0;
	const wchar_t* pBasePath = GetCurrentProcessPath(&nBasePathLength);
	if (dstSize < nBasePathLength)return false;

	wmemcpy(dst, pBasePath, nBasePathLength);
	nWritten = nBasePathLength;

	size_t nRead = 0;
	if (dst[nWritten - 1ULL] != L'\\' && dst[nWritten - 1ULL] != L'/')
	{
		dst[nWritten++] = L'\\';
		dst[nWritten] = L'\0';

	}
	if (directoryName[0] == L'\\' || directoryName[0] == L'/')
	{
		++nRead;
	}

	for (;;)
	{
		const wchar_t* pRead = &directoryName[nRead];
		const wchar_t* pPos = wcspbrk(pRead, L"\\/");
		if (pPos == nullptr)
		{
			size_t nLength = directoryNameLength - nRead;
			if (dstSize < nWritten + nLength + 1)return false;

			wmemcpy(dst + nWritten, pRead, nLength);
			nWritten += nLength;
			dst[nWritten++] = L'\\';
			dst[nWritten] = L'\0';

			::CreateDirectoryW(dst, nullptr);

			break;
		}

		size_t nLength = pPos - pRead;
		if (dstSize < nWritten + nLength + 1)return false;

		wmemcpy(dst + nWritten, pRead, nLength);
		nWritten += nLength;
		dst[nWritten++] = L'\\';
		dst[nWritten] = L'\0';

		::CreateDirectoryW(dst, nullptr);

		nRead += nLength + 1;
	}

	return true;
}

std::wstring win_filesystem::CreateSubDirectory(const wchar_t* relativePath, size_t relativePathLength)
{
	wchar_t buffer[kMaxPathLength]{};
	size_t nWritten = 0;
	CreateSubDirectoryToBuffer(relativePath, relativePathLength, buffer, sizeof(buffer) / sizeof(wchar_t) - 1, nWritten);

	return buffer;
}

std::wstring win_filesystem::CreateSubDirectory(const std::wstring& relativePath)
{
	return CreateSubDirectory(relativePath.data(), relativePath.length());
}

std::string win_filesystem::LoadFileAsString(const wchar_t* filePath)
{
	std::string fileData;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD fileSize = INVALID_FILE_SIZE;
	DWORD nReadBytes = 0;
	BOOL iRet = FALSE;

	hFile = ::CreateFileW(filePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)goto end;

	fileSize = ::GetFileSize(hFile, nullptr);
	if (fileSize == INVALID_FILE_SIZE)goto end;

	fileData.resize(fileSize);
	iRet = ::ReadFile(hFile, &fileData[0], fileSize, &nReadBytes, nullptr);
	/* To suppress warning C28193 */
	if (iRet == FALSE)goto end;

end:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(hFile);
	}

	return fileData;
}

bool win_filesystem::SaveDataToFile(const wchar_t* filePath, const void* pData, unsigned long dataLength, bool toOverwrite)
{
	if (filePath != nullptr)
	{
		HANDLE hFile = ::CreateFileW(filePath, GENERIC_WRITE, 0, nullptr, toOverwrite ? CREATE_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::SetFilePointer(hFile, NULL, nullptr, FILE_END);
			DWORD nWritten = 0;
			BOOL iRet = ::WriteFile(hFile, pData, dataLength, &nWritten, nullptr);
			::CloseHandle(hFile);

			return iRet == TRUE;
		}
	}
	return false;
}

bool win_filesystem::DoesFileExist(const wchar_t* filePath)
{
	WIN32_FILE_ATTRIBUTE_DATA win32FileAttributeData{};
	BOOL iRet = ::GetFileAttributesExW(filePath, GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &win32FileAttributeData);

	return iRet != 0;
}

bool win_filesystem::RenameFile(const wchar_t* filePathOld, const wchar_t* filePathNew)
{
	return ::MoveFileW(filePathOld, filePathNew) != 0;
}

