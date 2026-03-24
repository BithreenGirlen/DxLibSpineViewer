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

	bool Open(HINSTANCE hInstance, HWND hWnd, const wchar_t* pwzWindowName);
	HWND GetHwnd()const { return m_hWnd; }

	const std::wstring& GetAtlasExtension() const { return m_wstrAtlasExtension; }
	const std::wstring& GetSkelExtension() const { return m_wstrSkelExtension; }

private:
	const wchar_t* m_swzClassName = L"Spine setting dialogue";
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	int MessageLoop();
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);

	enum Constants { kFontSize = 16 };

	HFONT m_hFont = nullptr;

	CStatic m_atlasStatic;
	CEdit m_atlasEdit;
	CStatic m_skelStatic;
	CEdit m_skelEdit;

	std::wstring m_wstrAtlasExtension = L".atlas";
	std::wstring m_wstrSkelExtension = L".skel";

	static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

	void GetInputs();
};
#endif // !SPINE_SETTING_DIALOGUE_H_
