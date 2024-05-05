#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "dxlib_spine_player.h"
#include "spine_setting_dialogue.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();
	bool Create(HINSTANCE hInstance, const wchar_t* pwzWindowName);
	int MessageLoop();
	HWND GetHwnd()const { return m_hWnd;}
private:
	const wchar_t* m_swzClassName = L"Dxlib-spine window";
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonUp(WPARAM wParam, LPARAM lParam);

	enum Menu
	{
		kOpenFolder = 1, kFileSetting, kSelectFiles,
		kSeeThroughImage
	};
	enum MenuBar{kFolder, kImage};

	POINT m_CursorPos{};
	bool m_bSpeedHavingChanged = false;
	bool m_bLeftDowned = false;

	HMENU m_hMenuBar = nullptr;
	bool m_bBarHidden = false;
	bool m_bTransparent = false;

	std::vector<std::wstring> m_folders;
	size_t m_nFolderIndex = 0;

	void InitialiseMenuBar();

	void MenuOnOpenFolder();
	void MenuOnFileSetting();
	void MenuOnSelectFiles();

	void MenuOnSeeThroughImage();

	void KeyUpOnNextFolder();
	void KeyUpOnForeFolder();

	void ChangeWindowTitle(const wchar_t* pwzTitle);
	void SwitchWindowMode();

	bool SetupResources(const wchar_t* pwzFolderPath);
	void ClearFolderInfo();

	CDxLibSpinePlayer m_DxLibSpinePlayer;
	CSpineSettingDialogue m_SpineSettingDialogue;
};

#endif //MAIN_WINDOW_H_