#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "spine_player_shared.h"
#include "dxlib_recorder.h"

#include "native-ui/spine_setting_dialogue.h"
#include "native-ui/spine_manipulator_dialogue.h"
#include "native-ui/spine_atlas_dialogue.h"
#include "native-ui/export_setting_dialogue.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();

	bool Create(HINSTANCE hInstance, const wchar_t* pwzWindowName);
	int MessageLoop();

	HWND GetHwnd()const { return m_hWnd; }
private:
	const wchar_t* m_swzClassName = L"Dxlib-spine window";
	const wchar_t* m_swzDefaultWindowName = L"DxLib spine";

	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonUp(WPARAM wParam, LPARAM lParam);

	struct Menu abstract final
	{
		enum
		{
			kOpenFiles = 1, kOpenFolder,  kExtensionSetting, kImportCocos,
			kSkeletonSetting, kAtlasSetting, kAddEffectFile, kExportSetting,
			kSeeThroughImage, kAllowManualSizing, kReverseZoomDirection,
			kSnapAsPNG, kSnapAsJPG,
			kStartStoringImages, kStartVideoRecording,
			kSaveAsGIF, kSaveAsPNGs,
			kEndVideoRecording
		};
	};
	struct MenuBar abstract final
	{
		enum
		{
			kFile, kTool, kWindow
		};
	};

	POINT m_cursorPos{};
	bool m_bLeftCombinated = false;
	bool m_bLeftDowned = false;
	bool m_bLeftDragged = false;
	bool m_bRightCombinated = false;

	HMENU m_hMenuBar = nullptr;
	bool m_bBarHidden = false;

	bool m_bTransparent = false;
	bool m_bManuallyResizable = false;
	bool m_bZoomReversed = false;

	std::vector<std::wstring> m_folders;
	size_t m_nFolderIndex = 0;

	float m_fDelta = 1 / 60.f;

	void InitialiseMenuBar();

	void MenuOnOpenFiles();
	void MenuOnOpenFolder();
	void MenuOnExtensionSetting();
	void MenuOnImportCocos();

	void MenuOnSkeletonSetting();
	void MenuOnAtlasSetting();
	void MenuOnAddFile();
	void MenuOnExportSetting();

	void MenuOnSeeThroughImage();
	void MenuOnAllowManualSizing();
	void MenuOnReverseZoomDirection();

	void KeyUpOnNextFolder();
	void KeyUpOnForeFolder();

	void MenuOnSaveAsJpg();
	void MenuOnSaveAsPng();

	void MenuOnStartRecording(bool bAsVideo);
	void MenuOnEndRecording(bool bAsGif = true);

	void ChangeWindowTitle(const wchar_t* pwzTitle);
	std::wstring GetWindowTitle();
	void ToggleWindowBorderStyle();
	bool SetMenuCheckState(unsigned int uiMenuIndex, unsigned int uiItemIndex, bool checked) const;
	void UpdateMenuItemState();

	bool SetupResources(const wchar_t* pwzFolderPath);
	void ClearFolderInfo();

	void UpdateDrawingInterval();
	void StepOnRecording();

	CDxLibSpinePlayer m_dxLibSpinePlayer;
	CSpineSettingDialogue m_spineSettingDialogue;
	CSpineManipulatorDialogue m_spineManipulatorDialogue;
	CSpineAtlasDialogue m_spineAtlasDialogue;

	CDxLibRecorder m_dxLibRecorder;
	CExportSettingDialogue m_exportSettingDialogue;

	void UpdateWindowResizableAttribute();
	void ResizeWindow();
};

#endif //MAIN_WINDOW_H_