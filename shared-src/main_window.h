#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "spine_player_shared.h"
#include "spine_setting_dialogue.h"
#include "spine_manipulator_dialogue.h"
#include "spine_atlas_dialogue.h"

#include "win_image.h"
#include "mf_video_encoder.h"

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

	enum Menu
	{
		kOpenFolder = 1, kFileSetting, kSelectFiles,
		kSkeletonSetting, kAtlasSetting,
		kSeeThroughImage,
		kSnapAsPNG, kSnapAsJPG,
		kStartStoringImages, kStartVideoRecording,
		kSaveAsGIF, kSaveAsPNGs,
		kEndVideoRecording
	};
	enum MenuBar
	{
		kFile, kImage, kWindow
	};

	POINT m_cursorPos{};
	bool m_bLeftCombinated = false;
	bool m_bLeftDowned = false;
	bool m_bRightCombinated = false;

	HMENU m_hMenuBar = nullptr;
	bool m_bBarHidden = false;
	bool m_bTransparent = false;
	bool m_bPlayReady = false;

	enum class RecorderState
	{
		Idle,
		StoringImages,
		InitialisingVideoStream,
		RecordingVideo
	};
	RecorderState m_recoderState = RecorderState::Idle;

	std::vector<std::wstring> m_folders;
	size_t m_nFolderIndex = 0;

	std::vector<SImageFrame> m_imageFrames;
	std::vector<std::wstring> m_imageFrameNames;
	int m_iFrameCount = 0;

	float m_fDelta = 1 / 60.f;

	void InitialiseMenuBar();

	void MenuOnOpenFolder();
	void MenuOnFileSetting();
	void MenuOnSelectFiles();

	void MenuOnSkeletonSetting();
	void MenuOnAtlasSetting();

	void MenuOnSeeThroughImage();

	void KeyUpOnNextFolder();
	void KeyUpOnForeFolder();

	void MenuOnSaveAsJpg();
	void MenuOnSaveAsPng();

	void MenuOnStartRecording(bool bAsVideo);
	void MenuOnEndRecording(bool bAsGif = true);

	void ChangeWindowTitle(const wchar_t* pwzTitle);
	std::wstring GetWindowTitle();
	void SwitchWindowMode();

	bool SetupResources(const wchar_t* pwzFolderPath);
	void ClearFolderInfo();

	void UpdateDrawingInterval();
	void StepOnRecording();

	CDxLibSpinePlayer m_DxLibSpinePlayer;
	CSpineSettingDialogue m_SpineSettingDialogue;
	CSpineManipulatorDialogue m_SpineManipulatorDialogue;
	CSpineAtlasDialogue m_SpineAtlasDialogue;
	CMfVideoEncoder m_MfVideoEncoder;

	void UpdateWindowResizableAttribute();
	void ResizeWindow();
};

#endif //MAIN_WINDOW_H_