#ifndef FONT_SETTING_DIALOGUE_H_
#define FONT_SETTING_DIALOGUE_H_

#include <Windows.h>

#include "dialogue_controls.h"

class CFontSettingDialogue
{
public:
	CFontSettingDialogue();
	~CFontSettingDialogue();

	HWND open(HINSTANCE hInstance, HWND hWndParent, const wchar_t* windowName);

	HWND getHwnd()const { return m_hWnd; }
private:
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT onInit(HWND hWnd);
	LRESULT onClose();
	LRESULT onSize();
	LRESULT onCommand(WPARAM wParam, LPARAM lParam);

	enum Constants { kFontSize = 16 };
	enum Controls
	{
		kApplyButton = 1,
		kFontSizeSlider
	};

	HFONT m_hFont = nullptr;

	/// @brief EnumChildWindows callback 
	static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

	CStatic m_fontNameStatic;
	CComboBox m_fontNameComboBox;
	int m_lastFontNameIndex = -1;

	CStatic m_fontSizeStatic;
	CSlider m_fontSizeSlider;
	CButton m_boldButton;

	CButton m_applyButton;

	void resizeControls();

	void onApplyButton();

	void setSliderPosition();
};
#endif // !FONT_SETTING_DIALOGUE_H_
