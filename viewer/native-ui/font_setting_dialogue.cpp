
#include "font_setting_dialogue.h"

#include "dialogue_template.h"
#include "../win_font.h"
#include "../win_text.h"

#include <imgui.h>

CFontSettingDialogue::CFontSettingDialogue()
{
	int fontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);
	m_hFont = ::CreateFont(fontHeight, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Yu mincho");
}

CFontSettingDialogue::~CFontSettingDialogue()
{
	if (m_hFont != nullptr)
	{
		::DeleteObject(m_hFont);
	}
}

HWND CFontSettingDialogue::open(HINSTANCE hInstance, HWND hWndParent, const wchar_t* windowName)
{
	CDialogueTemplate dialogueTemplate;
	dialogueTemplate.setWindowSize(160, 100);

	return ::CreateDialogIndirectParam(hInstance, (LPCDLGTEMPLATE)dialogueTemplate.generate(windowName), hWndParent, (DLGPROC)DialogProc, (LPARAM)this);
}
/* C CALLBACK */
LRESULT CFontSettingDialogue::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		::SetWindowLongPtr(hWnd, DWLP_USER, lParam);
	}

	auto pThis = reinterpret_cast<CFontSettingDialogue*>(::GetWindowLongPtr(hWnd, DWLP_USER));
	if (pThis != nullptr)
	{
		return pThis->handleMessage(hWnd, uMsg, wParam, lParam);
	}
	return FALSE;
}
/* メッセージ処理 */
LRESULT CFontSettingDialogue::handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return onInit(hWnd);
	case WM_SIZE:
		return onSize();
	case WM_CLOSE:
		return onClose();
	case WM_COMMAND:
		return onCommand(wParam, lParam);
	default:
		break;
	}
	return FALSE;
}
/* WM_INITDIALOG */
LRESULT CFontSettingDialogue::onInit(HWND hWnd)
{
	m_hWnd = hWnd;

	m_fontNameStatic.create(L"Font name", m_hWnd);
	m_fontNameComboBox.create(m_hWnd);

	m_fontSizeStatic.create(L"Size", m_hWnd);
	m_fontSizeSlider.create(L"", m_hWnd, reinterpret_cast<HMENU>(Controls::kFontSizeSlider), 8, 64, 1);

	m_boldButton.create(L"Bold", m_hWnd, nullptr, true);

	m_applyButton.create(L"Apply", m_hWnd, reinterpret_cast<HMENU>(Controls::kApplyButton));

	CWinFont winFont;

	std::vector<std::wstring> fontNames = winFont.getSystemFontFamilyNames();
	m_fontNameComboBox.setup(fontNames);

	if (m_lastFontNameIndex == -1)
	{
		std::wstring localeFontName = winFont.findLocaleFontName(L"Yu mincho");
		if (!localeFontName.empty())
		{
			int index = m_fontNameComboBox.findIndex(localeFontName.c_str());
			if (index != -1)
			{
				m_fontNameComboBox.setSelectedItem(index);
				m_lastFontNameIndex = index;
			}
		}
	}
	else
	{
		m_fontNameComboBox.setSelectedItem(m_lastFontNameIndex);
	}

	resizeControls();

	setSliderPosition();

	::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));

	return TRUE;
}
/* WM_CLOSE */
LRESULT CFontSettingDialogue::onClose()
{
	::DestroyWindow(m_hWnd);
	m_hWnd = nullptr;

	return 0;
}
/* WM_SIZE */
LRESULT CFontSettingDialogue::onSize()
{
	resizeControls();

	return 0;
}
/* WM_COMMAND */
LRESULT CFontSettingDialogue::onCommand(WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	int msgSource = LOWORD(lParam);
	if (msgSource == 0)
	{
		/* Menus */
	}
	else
	{
		/* Controls */

		WORD notificationCode = HIWORD(wParam);
		if (notificationCode == CBN_SELCHANGE)
		{
			/* Notification code */
		}
		else
		{
			switch (id)
			{
			case Controls::kApplyButton:
				onApplyButton();
				break;
			default:
				break;
			}
		}
	}

	return 0;
}

BOOL CFontSettingDialogue::SetFontCallback(HWND hWnd, LPARAM lParam)
{
	::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
	/*TRUE: 続行, FALSE: 終了*/
	return TRUE;
}

void CFontSettingDialogue::resizeControls()
{
	RECT clientRect;
	::GetClientRect(m_hWnd, &clientRect);

	long clientWidth = clientRect.right - clientRect.left;
	long clientHeight = clientRect.bottom - clientRect.top;

	long spaceX = clientWidth / 24;
	long spaceY = clientHeight / 96;

	int fontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);

	long x = spaceX;
	long y = spaceY * 2;
	long w = clientWidth - spaceX * 2;
	long h = clientHeight * 8 / 10;
	::MoveWindow(m_fontNameStatic.getHwnd(), x, y, w, h, TRUE);

	y += fontHeight;
	::MoveWindow(m_fontNameComboBox.getHwnd(), x, y, w, h, TRUE);

	y += clientHeight * 1 / 6;
	h = clientHeight * 1 / 6;
	::MoveWindow(m_fontSizeStatic.getHwnd(), x, y, w, h, TRUE);

	y += fontHeight;
	::MoveWindow(m_fontSizeSlider.getHwnd(), x, y, w, h, TRUE);

	y += h + spaceY;
	::MoveWindow(m_boldButton.getHwnd(), x, y, w, h, TRUE);

	w = clientWidth / 4;
	h = static_cast<int>(fontHeight * 1.5);
	x = clientWidth - w - spaceX * 2;
	y = clientHeight - h - spaceY * 2;
	::MoveWindow(m_applyButton.getHwnd(), x, y, w, h, TRUE);
}
/*適用ボタン*/
void CFontSettingDialogue::onApplyButton()
{
	std::wstring fontName = m_fontNameComboBox.getSelectedItemText();
	if (!fontName.empty())
	{
		CWinFont winFont;
		bool isBold = m_boldButton.isChecked();

		std::vector<std::wstring> fontFilePaths = winFont.findFontFilePaths(fontName.c_str(), isBold, false);
		if (!fontFilePaths.empty())
		{
			float fontSize = static_cast<float>(m_fontSizeSlider.getPosition());

			ImGuiIO& io = ImGui::GetIO();
			const auto& fontAtlas = io.Fonts;
			fontAtlas->Clear();

			const ImWchar* glyph = fontAtlas->GetGlyphRangesChineseFull();
			std::string strFontFilePath = win_text::NarrowUtf8(fontFilePaths[0]);
			fontAtlas->AddFontFromFileTTF(strFontFilePath.c_str(), fontSize, nullptr, glyph);

			ImGuiStyle& style = ImGui::GetStyle();
			style._NextFrameFontSizeBase = fontSize;

			m_lastFontNameIndex = m_fontNameComboBox.getSelectedItemIndex();
		}
	}
}

void CFontSettingDialogue::setSliderPosition()
{
	ImGuiStyle& style = ImGui::GetStyle();
	long long fontSize = static_cast<long long>(style.FontSizeBase);
	/* Before initial rendering, font size remains zero. */
	m_fontSizeSlider.setPosition(fontSize == 0 ? 20 : fontSize);
}
