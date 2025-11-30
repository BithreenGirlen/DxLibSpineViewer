
#include "font_setting_dialogue.h"

#include "dialogue_template.h"
#include "../win_font.h"
#include "../win_text.h"

#include <imgui.h>

CFontSettingDialogue::CFontSettingDialogue()
{
	int iFontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);
	m_hFont = ::CreateFont(iFontHeight, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"yumin");
}

CFontSettingDialogue::~CFontSettingDialogue()
{
	if (m_hFont != nullptr)
	{
		::DeleteObject(m_hFont);
	}
}

HWND CFontSettingDialogue::Open(HINSTANCE hInstance, HWND hWndParent, const wchar_t* pwzWindowName)
{
	CDialogueTemplate winDialogueTemplate;
	winDialogueTemplate.SetWindowSize(160, 100);

	return ::CreateDialogIndirectParam(hInstance, (LPCDLGTEMPLATE)winDialogueTemplate.Generate(pwzWindowName), hWndParent, (DLGPROC)DialogProc, (LPARAM)this);
}
/*C CALLBACK*/
LRESULT CFontSettingDialogue::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		::SetWindowLongPtr(hWnd, DWLP_USER, lParam);
	}

	auto pThis = reinterpret_cast<CFontSettingDialogue*>(::GetWindowLongPtr(hWnd, DWLP_USER));
	if (pThis != nullptr)
	{
		return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	}
	return FALSE;
}
/*メッセージ処理*/
LRESULT CFontSettingDialogue::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return OnInit(hWnd);
	case WM_SIZE:
		return OnSize();
	case WM_CLOSE:
		return OnClose();
	case WM_NOTIFY:
		return OnNotify(wParam, lParam);
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	case WM_VSCROLL:
		return OnVScroll(wParam, lParam);
	default:
		break;
	}
	return FALSE;
}
/*WM_INITDIALOG*/
LRESULT CFontSettingDialogue::OnInit(HWND hWnd)
{
	m_hWnd = hWnd;

	m_fontNameStatic.Create(L"Font name", m_hWnd);
	m_fontNameComboBox.Create(m_hWnd);

	m_fontSizeStatic.Create(L"Size", m_hWnd);
	m_fontSizeSlider.Create(L"", m_hWnd, reinterpret_cast<HMENU>(Controls::kFontSizeSlider), 8, 64, 1);

	m_applyButton.Create(L"Apply", m_hWnd, reinterpret_cast<HMENU>(Controls::kApplyButton));

	CWinFont winFont;

	std::vector<std::wstring> fontNames = winFont.GetSystemFontFamilyNames();
	m_fontNameComboBox.Setup(fontNames);

	if (m_lastFontNameIndex == -1)
	{
		std::wstring wstrLocaleFontName = winFont.FindLocaleFontName(L"游明朝");
		if (!wstrLocaleFontName.empty())
		{
			int iIndex = m_fontNameComboBox.FindIndex(wstrLocaleFontName.c_str());
			if (iIndex != -1)
			{
				m_fontNameComboBox.SetSelectedItem(iIndex);
				m_lastFontNameIndex = iIndex;
			}
		}
	}
	else
	{
		m_fontNameComboBox.SetSelectedItem(m_lastFontNameIndex);
	}

	ResizeControls();

	SetSliderPosition();

	::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));

	return TRUE;
}
/*WM_CLOSE*/
LRESULT CFontSettingDialogue::OnClose()
{
	::DestroyWindow(m_hWnd);
	m_hWnd = nullptr;

	return 0;
}
/*WM_SIZE*/
LRESULT CFontSettingDialogue::OnSize()
{
	ResizeControls();

	return 0;
}
/*WM_NOTIFY*/
LRESULT CFontSettingDialogue::OnNotify(WPARAM wParam, LPARAM lParam)
{

	return 0;
}
/*WM_COMMAND*/
LRESULT CFontSettingDialogue::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int wmKind = LOWORD(lParam);
	if (wmKind == 0)
	{
		/*Menus*/
	}
	else
	{
		/*Controls*/

		WORD usCode = HIWORD(wParam);
		if (usCode == CBN_SELCHANGE)
		{
			/*Notification code*/
		}
		else
		{
			switch (wmId)
			{
			case Controls::kApplyButton:
				OnApplyButton();
				break;
			default:
				break;
			}
		}
	}

	return 0;
}
/*WM_VSCROLL*/
LRESULT CFontSettingDialogue::OnVScroll(WPARAM wParam, LPARAM lParam)
{
	return 0;
}
/*EnumChildWindows CALLBACK*/
BOOL CFontSettingDialogue::SetFontCallback(HWND hWnd, LPARAM lParam)
{
	::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
	/*TRUE: 続行, FALSE: 終了*/
	return TRUE;
}
/*再配置*/
void CFontSettingDialogue::ResizeControls()
{
	RECT clientRect;
	::GetClientRect(m_hWnd, &clientRect);

	long clientWidth = clientRect.right - clientRect.left;
	long clientHeight = clientRect.bottom - clientRect.top;

	long spaceX = clientWidth / 24;
	long spaceY = clientHeight / 96;

	long x = spaceX;
	long y = spaceY * 2;
	long w = clientWidth - spaceX * 2;
	long h = clientHeight * 8 / 10;

	int fontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);

	if (m_fontNameStatic.GetHwnd() != nullptr)
	{
		::MoveWindow(m_fontNameStatic.GetHwnd(), x, y, w, h, TRUE);
	}

	y += fontHeight;
	if (m_fontNameComboBox.GetHwnd() != nullptr)
	{
		::MoveWindow(m_fontNameComboBox.GetHwnd(), x, y, w, h, TRUE);
	}

	y += clientHeight * 1 / 6;
	h = clientHeight * 1 / 6;
	::MoveWindow(m_fontSizeStatic.GetHwnd(), x, y, w, h, TRUE);

	y += fontHeight;
	::MoveWindow(m_fontSizeSlider.GetHwnd(), x, y, w, h, TRUE);

	w = clientWidth / 4;
	h = static_cast<int>(fontHeight * 1.5);
	x = clientWidth - w - spaceX * 2;
	y = clientHeight - h - spaceY * 2;
	if (m_applyButton.GetHwnd() != nullptr)
	{
		::MoveWindow(m_applyButton.GetHwnd(), x, y, w, h, TRUE);
	}
}
/*適用ボタン*/
void CFontSettingDialogue::OnApplyButton()
{
	std::wstring wstrFontName = m_fontNameComboBox.GetSelectedItemText();
	if (!wstrFontName.empty())
	{
		CWinFont sWinFont;

		auto filePaths = sWinFont.FindFontFilePaths(wstrFontName.c_str(), false, false);
		if (!filePaths.empty())
		{
			float fontSize = static_cast<float>(m_fontSizeSlider.GetPosition());

			ImGuiIO& io = ImGui::GetIO();
			const auto& fontAtlas = io.Fonts;
			fontAtlas->Clear();

			const ImWchar* glyph = fontAtlas->GetGlyphRangesChineseFull();
			std::string strFontFilePath = win_text::NarrowUtf8(filePaths[0]);
			fontAtlas->AddFontFromFileTTF(strFontFilePath.c_str(), fontSize, nullptr, glyph);

			ImGuiStyle& style = ImGui::GetStyle();
			style._NextFrameFontSizeBase = fontSize;

			m_lastFontNameIndex = m_fontNameComboBox.GetSelectedItemIndex();
		}
	}
}

void CFontSettingDialogue::SetSliderPosition()
{
	ImGuiStyle& style = ImGui::GetStyle();
	long long llFontSize = static_cast<long long>(style.FontSizeBase);
	/* Before initial rendering, font size remains zero. */
	m_fontSizeSlider.SetPosition(llFontSize == 0 ? 20 : llFontSize);
}
