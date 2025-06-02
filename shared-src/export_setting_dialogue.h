#ifndef EXPORT_SETTING_DIALOGUE_H_
#define EXPORT_SETTING_DIALOGUE_H_

#include <Windows.h>

#include "dialogue_controls.h"

class CExportSettingDialogue
{
public:
	CExportSettingDialogue();
	~CExportSettingDialogue();

	bool Open(HINSTANCE hInstance, HWND hOwnerWnd, const wchar_t* windowName);
	HWND GetHwnd()const { return m_hWnd; }

	unsigned short GetImageFps()const { return m_imageFps; }
	unsigned short GetVideoFps()const { return m_videoFps; }
	bool IsToExportPerAnimation() const { return m_isToExportPerAnimation; }
private:
	static constexpr unsigned int kFontSize = 16;

	const wchar_t* m_className = L"Export setting dialogue";
	HWND m_hWnd = nullptr;

	int MessageLoop();

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);

	struct Controls abstract final
	{
		enum
		{
			kExportMethod = 1
		};
	};

	HFONT m_hFont = nullptr;

	static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

	CStatic m_imageFpsStatic;
	CSpin m_imageFpsSpin;
	CStatic m_videoFpsStatic;
	CSpin m_videoFpsSpin;

	CButton m_exportMethodButton;

	void ResizeControls();

	unsigned short m_imageFps = 30;
	unsigned short m_videoFps = 60;
	bool m_isToExportPerAnimation = true;

	void GetInputs();
};
#endif // !EXPORT_SETTING_DIALOGUE_H_
