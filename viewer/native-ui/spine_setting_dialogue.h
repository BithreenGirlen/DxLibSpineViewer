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
	HWND getHwnd()const noexcept{ return m_hWnd; }

	const std::wstring& getAtlasExtension() const noexcept { return m_atlasExtension; }
	const std::wstring& getSkelExtension() const noexcept { return m_skelExtension; }

	void multiplyAlphaOnLoading(bool toMultiply) noexcept;
	bool isToMultiplyAlphaOnLoading() const noexcept;

	void findWebpOnLoading(bool toFindWebp) noexcept;
	bool isToFindWebpOnLoading() const noexcept;

	void ignoreMaskImageOnLoading(bool toIgnoreMaskImage) noexcept;
	bool isToIgnoreMaskImageOnLoading() const noexcept;

	int getMaskImageWidth() const noexcept;
	int getMaskImageHeight() const noexcept;
private:
	static constexpr int kDefaultFontSize = 16;
	static constexpr int kDefaultMaskImageDemention = 256;

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

	struct Controls
	{
		enum
		{
			kPmaButton = 1, kFindWebp, kIgnoreSmallImage
		};
	};
	HFONT m_hFont = nullptr;

	CStatic m_extensionSeparator;
	CStatic m_atlasStatic;
	CEdit m_atlasEdit;
	CStatic m_skelStatic;
	CEdit m_skelEdit;

	CButton m_pmaButton;
	CButton m_findWebpButton;

	CButton m_ignoreMaskImageButton;
	CStatic m_maskWidthStatic;
	CSpin m_maskWidthSpin;
	CStatic m_maskHeightStatic;
	CSpin m_maskHeightSpin;

	std::wstring m_atlasExtension = L".atlas";
	std::wstring m_skelExtension = L".skel";
	bool m_toMultiplyAlphaOnLoading = false;
	bool m_toFindWebp = false;
	bool m_toIgnoreMaskImage = false;

	int m_maskImageWidth = kDefaultMaskImageDemention;
	int m_maskImageHeight = kDefaultMaskImageDemention;

	void storeInputs() noexcept;
};
#endif // !SPINE_SETTING_DIALOGUE_H_
