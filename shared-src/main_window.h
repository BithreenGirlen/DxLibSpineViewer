#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "spine_player_shared.h"
#include "dxlib_recorder.h"
#include "win_clock.h"

#include "native-ui/spine_setting_dialogue.h"
#include "native-ui/spine_manipulator_dialogue.h"
#include "native-ui/spine_atlas_dialogue.h"
#include "native-ui/export_setting_dialogue.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();

	bool Create(HINSTANCE hInstance, const wchar_t* pwzWindowName, HICON hIcon = nullptr);
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
			kFitToManualSize, kFitToDefaultSize,
			kSnapAsPNG, kSnapAsJPG,
			kExportAsGif, kExportAsVideo, kExportAsPngs, kExportAsJpgs,
			kEndRecording
		};
	};
	struct MenuBar abstract final
	{
		enum
		{
			kFile, kTool, kWindow
		};
	};

	POINT m_lastCursorPos{};

	bool m_wasLeftCombinated = false;
	bool m_wasLeftPressed = false;
	bool m_hasLeftBeenDragged = false;
	bool m_wasRightCombinated = false;

	HMENU m_hMenuBar = nullptr;

	bool m_isFramelessWindow = false;
	bool m_isTransparentWindow = false;
	bool m_isManuallyResizable = false;

	bool m_isZoomDirectionReversed = false;

	std::vector<std::wstring> m_folders;
	size_t m_nFolderIndex = 0;

	bool m_hasProcessedWmPaint = false;
	CWinClock m_winclock;

	void Tick();

	void InitialiseMenuBar();

	void MenuOnOpenFiles();
	void MenuOnOpenFolder();
	void MenuOnExtensionSetting();
	void MenuOnImportCocos();

	void MenuOnSkeletonSetting();
	void MenuOnAtlasSetting();
	void MenuOnAddFile();
	void MenuOnExportSetting();

	void MenuOnMakeWindowTransparent();
	void MenuOnAllowManualSizing();
	void MenuOnReverseZoomDirection();
	void MenuOnFiToManualSize();
	void MenuOnFitToDefaultSize();

	void KeyUpOnNextFolder();
	void KeyUpOnForeFolder();

	void MenuOnSaveAsJpg();
	void MenuOnSaveAsPng();

	void MenuOnStartRecording(int menuKind);
	void MenuOnEndRecording();

	void ChangeWindowTitle(const wchar_t* pwzTitle);
	std::wstring GetWindowTitle() const;

	void ToggleWindowFrameStyle();
	void UpdateMenuItemState();

	bool LoadSpineFilesInFolder(const wchar_t* folderPath);
	bool LoadSpineFiles(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel, const wchar_t* windowName);
	void ClearFolderPathList();

	std::wstring BuildExportFilePath();
	std::wstring FormatAnimationTime(float fAnimationTime);
	void StepRecording();

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