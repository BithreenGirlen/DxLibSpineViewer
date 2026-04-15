#ifndef WIN_DIALOGUE_H_
#define WIN_DIALOGUE_H_

#include <string>
#include <vector>

namespace win_dialogue
{
	std::wstring SelectFolder(const wchar_t* title, void* hParentWnd);
	std::wstring SelectOpenFile(const wchar_t* fileType, const wchar_t* fileSpec, const wchar_t* title, void* hParentWnd, bool bAny = false);
	std::vector<std::wstring> SelectOpenFiles(const wchar_t* fileType, const wchar_t* fileSpec, const wchar_t* title, void* hParentWnd, bool bAny = false);
	std::wstring SelectSaveFile(const wchar_t* fileType, const wchar_t* fileSpec, const wchar_t* defaultFileName, void* hParentWnd);

	void ShowMessageBox(const char* title, const char* message);
	/// @brief Show message box in PeekMessage loop by validating owner window temporarily
	void ShowErrorMessageValidatingOwnerWindow(const wchar_t* message, void* pOwnerWindow);
}
#endif // WIN_DIALOGUE_H_