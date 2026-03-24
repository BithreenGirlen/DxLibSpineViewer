
#include <Windows.h>
#include <shlwapi.h>

#include "win_filesystem.h"

#pragma comment(lib, "Shlwapi.lib")

namespace win_filesystem
{
	/*指定階層のファイル・フォルダ名一覧取得*/
	static bool CreateFilaNameList(const wchar_t* pwzFolderPath, const wchar_t* pwzFileNamePattern, std::vector<std::wstring>& wstrNames)
	{
		if (pwzFolderPath == nullptr)return false;

		std::wstring wstrPath = pwzFolderPath;
		if (pwzFileNamePattern != nullptr)
		{
			if (wcschr(pwzFileNamePattern, L'*') == nullptr)
			{
				wstrPath += L'*';
			}
			wstrPath += pwzFileNamePattern;
		}
		else
		{
			wstrPath += L'*';
		}

		WIN32_FIND_DATAW sFindData;

		HANDLE hFind = ::FindFirstFileW(wstrPath.c_str(), &sFindData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			if (pwzFileNamePattern != nullptr)
			{
				do
				{
					/*ファイル一覧*/
					if (!(sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						wstrNames.push_back(sFindData.cFileName);
					}
				} while (::FindNextFileW(hFind, &sFindData));
			}
			else
			{
				do
				{
					/*フォルダ一覧*/
					if ((sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						if (wcscmp(sFindData.cFileName, L".") != 0 && wcscmp(sFindData.cFileName, L"..") != 0)
						{
							wstrNames.push_back(sFindData.cFileName);
						}
					}
				} while (::FindNextFileW(hFind, &sFindData));
			}

			::FindClose(hFind);
		}
		return wstrNames.size() > 0;
	}
}

/*指定階層のファイル・フォルダ一覧作成*/
bool win_filesystem::CreateFilePathList(const wchar_t* pwzFolderPath, const wchar_t* pwzFileSpec, std::vector<std::wstring>& paths)
{
	if (pwzFolderPath == nullptr || *pwzFolderPath == L'\0')return false;

	std::wstring wstrParent = pwzFolderPath;
	if (wstrParent.back() != L'\\')
	{
		wstrParent += L"\\";
	}
	std::vector<std::wstring> wstrNames;

	if (pwzFileSpec != nullptr)
	{
		const auto SplitSpecs = [](const wchar_t* pwzFileSpec, std::vector<std::wstring>& specs)
			-> void
			{
				std::wstring wstrTemp;
				for (const wchar_t* p = pwzFileSpec; *p != L'\0' && p != nullptr; ++p)
				{
					if (*p == L';')
					{
						if (!wstrTemp.empty())
						{
							specs.push_back(wstrTemp);
							wstrTemp.clear();
						}
						continue;
					}

					wstrTemp.push_back(*p);
				}

				if (!wstrTemp.empty())
				{
					specs.push_back(wstrTemp);
				}
			};
		std::vector<std::wstring> specs;
		SplitSpecs(pwzFileSpec, specs);

		for (const auto& spec : specs)
		{
			CreateFilaNameList(wstrParent.c_str(), spec.c_str(), wstrNames);
		}
	}
	else
	{
		CreateFilaNameList(wstrParent.c_str(), pwzFileSpec, wstrNames);
	}

	/*名前順に整頓*/
	for (size_t i = 0; i < wstrNames.size(); ++i)
	{
		size_t nIndex = i;
		for (size_t j = i; j < wstrNames.size(); ++j)
		{
			if (::StrCmpLogicalW(wstrNames[nIndex].c_str(), wstrNames[j].c_str()) > 0)
			{
				nIndex = j;
			}
		}
		std::swap(wstrNames[i], wstrNames[nIndex]);
	}

	for (const std::wstring& wstr : wstrNames)
	{
		paths.push_back(wstrParent + wstr);
	}

	return paths.size() > 0;
}
/*指定経路と同階層のファイル・フォルダ一覧作成・相対位置取得*/
bool win_filesystem::GetFilePathListAndIndex(const std::wstring& wstrPath, const wchar_t* pwzFileSpec, std::vector<std::wstring>& paths, size_t* nIndex)
{
	std::wstring wstrParent;

	size_t nPos = wstrPath.find_last_of(L"\\/");
	if (nPos != std::wstring::npos)
	{
		wstrParent = wstrPath.substr(0, nPos);
	}

	win_filesystem::CreateFilePathList(wstrParent.c_str(), pwzFileSpec, paths);

	const auto& iter = std::find(paths.begin(), paths.end(), wstrPath);
	if (iter != paths.end())
	{
		*nIndex = std::distance(paths.begin(), iter);
	}

	return iter != paths.end();
}
/*文字列としてファイル読み込み*/
std::string win_filesystem::LoadFileAsString(const wchar_t* pwzFilePath)
{
	std::string strResult;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD ulSize = INVALID_FILE_SIZE;
	DWORD ulRead = 0;
	BOOL iRet = FALSE;

	hFile = ::CreateFileW(pwzFilePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)goto end;

	ulSize = ::GetFileSize(hFile, nullptr);
	if (ulSize == INVALID_FILE_SIZE)goto end;

	strResult.resize(ulSize);
	iRet = ::ReadFile(hFile, &strResult[0], ulSize, &ulRead, nullptr);
	/* To suppress warning C28193 */
	if (iRet == FALSE)goto end;

end:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(hFile);
	}

	return strResult;
}

std::wstring win_filesystem::GetCurrentProcessPath()
{
	wchar_t sBuffer[MAX_PATH]{};
	DWORD ulLength = ::GetModuleFileNameW(nullptr, sBuffer, MAX_PATH);
	if (ulLength == 0)return {};

	const wchar_t* p = sBuffer + ulLength;
	for (; p != sBuffer; --p)
	{
		if (*p == L'\\' || *p == L'/')break;
	}
	return std::wstring(sBuffer, p - sBuffer);
}

std::wstring win_filesystem::CreateWorkFolder(const std::wstring& wstrRelativePath)
{
	if (wstrRelativePath.empty())return std::wstring();

	std::wstring wstrPath = GetCurrentProcessPath();
	if (wstrPath.empty())return std::wstring{};

	wstrPath.push_back(L'\\');
	size_t nRead = 0;
	if (wstrRelativePath[0] == L'\\' || wstrRelativePath[0] == L'/')++nRead;

	for (const wchar_t* pStart = wstrRelativePath.data();;)
	{
		size_t nPos = wstrRelativePath.find_first_of(L"\\/", nRead);
		if (nPos == std::wstring::npos)
		{
			wstrPath.append(pStart + nRead, wstrRelativePath.size() - nRead).push_back(L'\\');
			::CreateDirectoryW(wstrPath.c_str(), nullptr);

			break;
		}
		wstrPath.append(pStart + nRead, nPos - nRead).push_back(L'\\');
		::CreateDirectoryW(wstrPath.c_str(), nullptr);

		nRead = nPos + 1;
	}

	return wstrPath;
}
