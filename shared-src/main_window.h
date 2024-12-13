#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "spine_player_shared.h"
#include "spine_setting_dialogue.h"
#include "spine_manipulator_dialogue.h"
#include "win_image.h"

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
	std::wstring m_wstrWindowName = L"DxLib spine";
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
	LRESULT OnRButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonUp(WPARAM wParam, LPARAM lParam);

	enum Menu
	{
		kOpenFolder = 1, kFileSetting, kSelectFiles,
		kSeeThroughImage, kSkeletonSetting,
		kSnapAsPNG, kStartRecording, kSaveAsGIF, kSaveAsPNGs
	};
	enum MenuBar{kFile, kImage};

	POINT m_CursorPos{};
	bool m_bSpeedHavingChanged = false;
	bool m_bLeftDowned = false;
	bool m_bRightCombinated = false;

	HMENU m_hMenuBar = nullptr;
	bool m_bBarHidden = false;
	bool m_bTransparent = false;
	bool m_bPlayReady = false;
	bool m_bUnderRecording = false;

	std::vector<std::wstring> m_folders;
	size_t m_nFolderIndex = 0;

	std::vector<SImageFrame> m_imageFrames;
	std::vector<std::wstring> m_imageFrameNames;

	float m_fDelta = 1 / 60.f;

	void InitialiseMenuBar();

	void MenuOnOpenFolder();
	void MenuOnFileSetting();
	void MenuOnSelectFiles();

	void MenuOnSeeThroughImage();
	void MenuOnSkeletonSetting();
	void MenuOnStartRecording();
	void MenuOnEndRecording(bool bAsGif = true);

	void KeyUpOnNextFolder();
	void KeyUpOnForeFolder();
	void MenuOnSaveAsPng();

	void ChangeWindowTitle(const wchar_t* pwzTitle);
	std::wstring GetWindowTitle();
	void SwitchWindowMode();

	bool SetupResources(const wchar_t* pwzFolderPath);
	void ClearFolderInfo();

	void UpdateDrawingInterval();

	CDxLibSpinePlayer m_DxLibSpinePlayer;
	CSpineSettingDialogue m_SpineSettingDialogue;
	CSpineManipulatorDialogue m_SpineManipulatorDialogue;
};

#endif //MAIN_WINDOW_H_