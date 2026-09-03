#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "spine_player_dynamic.h"
#include "dxlib_recorder.h"
#include "dxlib_handle.h"
#include "win_clock.h"

#include "native-ui/spine_setting_dialogue.h"
#include "native-ui/font_setting_dialogue.h"

#include "dxlib-imgui/spine_tool_dialogue.h"

class CMainWindow
{
public:
	CMainWindow() = default;
	~CMainWindow() = default;

	bool create(HINSTANCE hInstance, const wchar_t* pwzWindowName);
	int messageLoop();

	HWND getHwnd()const { return m_hWnd; }
private:
	const wchar_t* m_className = L"Dxlib-spine window";
	const wchar_t* m_defaultWindowName = L"DxLib spine";

	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT onCreate(HWND hWnd);
	LRESULT onDestroy();
	LRESULT onClose();
	LRESULT onPaint();
	LRESULT onSize(WPARAM wParam, LPARAM lParam);
	LRESULT onKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT onCommand(WPARAM wParam, LPARAM lParam);
	LRESULT onMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT onMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT onLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT onLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT onRButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT onMButtonUp(WPARAM wParam, LPARAM lParam);

	using DxLibImageHandle = DxLibHandle<&DxLib::DeleteGraph>;

	struct Menu
	{
		enum
		{
			kOpenFiles = 1, kOpenFolder, kLoadOption, kImportCocos,
			kShowToolDialogue, kAddEffectFile, kFontSetting,
			kMakeWindowTransparent, kAllowDraggedResizing, kReverseZoomDirection,
			kFitToCurrentFrame, kFitToDefaultSize,
		};
	};
	struct MenuBar { enum { kFile, kTool, kWindow }; };
	struct PopupMenu
	{
		enum
		{
			kSnapAsPNG = 1, kSnapAsJPG,
			kExportAsGif, kExportAsVideo, kExportAsPngs, kExportAsJpgs,
			kEndRecording
		};
	};

	struct MouseState
	{
		/// @brief 左釦押下を要する操作を行ったか。
		bool wasLeftCombined = false;
		bool wasLeftPressed = false;
		bool wasLeftDragged = false;
		/// @brief 右釦押下を要する操作を行ったか。
		bool wasRightCombined = false;
		bool wasRightDragged = false;

		POINT lastCursorPos{};
	};

	struct WindowStyle
	{
		bool isBorderless = false;
		bool isTransparent = false;
		bool isResizable = false;
		/// @brief ホイール回転方向に対する拡縮を逆にするか
		bool isZoomReversed = false;
	};

	struct WindowState
	{
		bool hasProcessedWmPaint = false;
	};
	MouseState m_mouseState;
	WindowStyle m_windowStyle;
	WindowState m_windowState;
	bool m_toShowSpineTool = false;

	HMENU m_hMenuBar = nullptr;

	std::vector<std::wstring> m_folderPaths;
	size_t m_nFolderPathIndex = 0;

	CWinClock m_winclock;

	DxLibImageHandle m_spineRenderTexture = { DxLibImageHandle(-1) };
	CSpinePlayerDynamic m_dxLibSpinePlayer;
	CSpineSettingDialogue m_spineSettingDialogue;

	CDxLibRecorder m_dxLibRecorder;
	spine_tool_dialogue::SSpineToolDatum m_spineToolDatum;

	CFontSettingDialogue m_fontSettingDialogue;

	void tick();

	void initialiseMenuBar();

	void menuOnOpenFiles();
	void menuOnOpenFolder();
	void menuOnLoadOption();
	void menuOnImportCocos();

	void menuOnShowToolDialogue();
	void menuOnAddFile();
	void menuOnFont();

	/// @brief 背景色透過・不透明切り替え
	void menuOnMakeWindowTransparent();
	/// @brief 抓んでの寸法変更を許可するか。動画録画中は切り替え不可
	void menuOnAllowDraggedResizing();
	/// @brief ホイール回転に対する拡縮方向を反転
	void menuOnReverseZoomDirection();
	/// @brief 手動変更された現在の表示範囲に合わせる
	void menuOnFitToCurrentFrame();
	/// @brief 既定の表示範囲に戻す
	void menuOnFitToDefaultSize();

	void clearFolderPaths();
	void openForeFolder();
	void openNextFolder();

	/// @brief 現在の表示フレームをJPGとして保存
	void saveRenderTextureAsJpg();
	/// @brief 現在の表示フレームをPNGとして保存
	void saveRenderTextureAsPng();

	/// @brief 録画開始 
	void startRecording(int menuKind);
	/// @brief 録画更新処理
	void stepRecording();
	/// @brief 録画終了
	void endRecording();

	/// @brief ウィンドウ名称変更
	void changeWindowTitle(const wchar_t* windowTitle);
	/// @brief ウィンドウ名称取得
	std::wstring getWindowTitle() const;
	/// @brief バッファ上にウィンドウ名称書き込み
	int getWindowTitleToBuffer(wchar_t* dst, size_t dstSize) const;

	/// @brief ウィンドウ枠表示・非表示切り替え
	void toggleWindowBorderStyle();
	/// @brief メニュー欄項目の有効・無効状態更新
	void updateMenuItemState();
	void updateWindowResizableStyle();
	void resizeWindow();

	bool loadSpineFilesInFolder(const std::wstring& folderPath);
	bool loadSpineFiles(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel, const wchar_t* windowName);
	bool loadSpinesFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& textureDirectories, const std::vector<std::string>& skelData, const wchar_t* windowName);
	void postSpineLoading(bool hadLoaded, bool hasLoaded, const wchar_t* windowName);
	static void SpineTextureLoadCallback(void* pUserDatum, const char* textureFilePath, size_t filePathLength, void* pOutImage);

	std::wstring buildExportFilePath();
	wchar_t* formatAnimationTime(float fAnimationTime, int* length = nullptr);

	void imGuiSpineToolDialogue();
};

#endif //MAIN_WINDOW_H_