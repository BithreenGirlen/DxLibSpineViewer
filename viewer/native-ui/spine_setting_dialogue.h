#ifndef SPINE_SETTING_DIALOGUE_H_
#define SPINE_SETTING_DIALOGUE_H_

#include <Windows.h>

#include <string>

#include "dialogue_controls.h"

class CSpineSettingDialogue
{
public:
	CSpineSettingDialogue();
	~CSpineSettingDialogue();

	bool open(HINSTANCE hInstance, HWND hWnd, const wchar_t* windowName);
	HWND getHwnd()const { return m_hWnd; }

	const std::wstring& getAtlasExtension() const { return m_atlasExtension; }
	const std::wstring& getSkelExtension() const { return m_skelExtension; }

private:
	const wchar_t* m_className = L"Spine setting dialogue";
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	int messageLoop();
	LRESULT handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT onCreate(HWND hWnd);
	LRESULT onDestroy();
	LRESULT onClose();
	LRESULT onPaint();
	LRESULT onSize();
	LRESULT onCommand(WPARAM wParam, LPARAM lParam);

	enum Constants { kFontSize = 16 };

	HFONT m_hFont = nullptr;

	CStatic m_atlasStatic;
	CEdit m_atlasEdit;
	CStatic m_skelStatic;
	CEdit m_skelEdit;

	std::wstring m_atlasExtension = L".atlas";
	std::wstring m_skelExtension = L".skel";

	/// @brief EnumChildWindows callback 
	static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

	void storeEditBoxInputs();
};
#endif // !SPINE_SETTING_DIALOGUE_H_
