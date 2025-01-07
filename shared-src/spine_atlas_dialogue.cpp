

#include <unordered_map>

#include "spine_atlas_dialogue.h"

#include "dialogue_template.h"
#include "win_text.h"


CSpineAtlasDialogue::CSpineAtlasDialogue()
{
	int iFontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);
	m_hFont = ::CreateFont(iFontHeight, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"yumin");
}

CSpineAtlasDialogue::~CSpineAtlasDialogue()
{
	if (m_hFont != nullptr)
	{
		::DeleteObject(m_hFont);
	}
}

HWND CSpineAtlasDialogue::Create(HINSTANCE hInstance, HWND hWndParent, const wchar_t* pwzWindowName, CDxLibSpinePlayer* pPlayer)
{
	CDialogueTemplate sWinDialogueTemplate;
	sWinDialogueTemplate.SetWindowSize(256, 120);
	std::vector<unsigned char> dialogueTemplate = sWinDialogueTemplate.Generate(pwzWindowName);

	m_pDxLibSpinePlayer = pPlayer;

	return ::CreateDialogIndirectParamA(hInstance, (LPCDLGTEMPLATE)dialogueTemplate.data(), hWndParent, (DLGPROC)DialogProc, (LPARAM)this);
}
/*C CALLBACK*/
LRESULT CSpineAtlasDialogue::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		::SetWindowLongPtr(hWnd, DWLP_USER, lParam);
	}

	auto pThis = reinterpret_cast<CSpineAtlasDialogue*>(::GetWindowLongPtr(hWnd, DWLP_USER));
	if (pThis != nullptr)
	{
		return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	}
	return FALSE;
}
/*メッセージ処理*/
LRESULT CSpineAtlasDialogue::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return OnInit(hWnd);
	case WM_SIZE:
		return OnSize();
	case WM_CLOSE:
		return OnClose();
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	default:
		break;
	}
	return FALSE;
}
/*WM_INITDIALOG*/
LRESULT CSpineAtlasDialogue::OnInit(HWND hWnd)
{
	m_hWnd = hWnd;

	m_hSlotStatic = ::CreateWindowEx(0, WC_STATIC, L"Slots", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, nullptr, ::GetModuleHandle(NULL), nullptr);
	m_hAttachmentStatic = ::CreateWindowEx(0, WC_STATIC, L"Attachments", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, nullptr, ::GetModuleHandle(NULL), nullptr);

	m_slotComboBox.Create(m_hWnd);
	m_attachmentComboBox.Create(m_hWnd);

	m_hReattachButton = ::CreateWindowExW(0, WC_BUTTONW, L"Re-attach", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, m_hWnd, reinterpret_cast<HMENU>(Controls::kReattachButton), ::GetModuleHandle(NULL), nullptr);

	if (m_pDxLibSpinePlayer != nullptr)
	{
		auto slotAttachmentMap = m_pDxLibSpinePlayer->GetSlotNamesWithTheirAttachments();

		std::vector<std::wstring> wstrSlotNames;
		wstrSlotNames.reserve(slotAttachmentMap.size());
		for (const auto& slot : slotAttachmentMap)
		{
			wstrSlotNames.push_back(win_text::WidenUtf8(slot.first));
		}
		m_slotComboBox.Setup(wstrSlotNames);

		std::vector<std::wstring> wstrAttachmentNames;
		wstrAttachmentNames.reserve(slotAttachmentMap.size() * 2);
		for (const auto& slot : slotAttachmentMap)
		{
			const auto &attachments = slot.second;
			for (const auto& attachment : attachments)
			{
				std::wstring wstr = win_text::WidenUtf8(attachment);
				const auto& iter = std::find(wstrAttachmentNames.begin(), wstrAttachmentNames.end(), wstr);
				if (iter == wstrAttachmentNames.cend())wstrAttachmentNames.push_back(wstr);
			}
		}
		m_attachmentComboBox.Setup(wstrAttachmentNames);
	}

	ResizeControls();

	::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));

	return TRUE;
}
/*WM_CLOSE*/
LRESULT CSpineAtlasDialogue::OnClose()
{
	::DestroyWindow(m_hWnd);
	m_hWnd = nullptr;
	return 0;
}
/*WM_SIZE*/
LRESULT CSpineAtlasDialogue::OnSize()
{
	ResizeControls();

	return 0;
}
/*WM_COMMAND*/
LRESULT CSpineAtlasDialogue::OnCommand(WPARAM wParam, LPARAM lParam)
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
		switch (wmId)
		{
		case Controls::kReattachButton:
			OnReattachButton();
			break;
		}
	}

	return 0;
}
/*EnumChildWindows CALLBACK*/
BOOL CSpineAtlasDialogue::SetFontCallback(HWND hWnd, LPARAM lParam)
{
	::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
	/*TRUE: 続行, FALSE: 終了*/
	return TRUE;
}
/*再装着ボタン*/
void CSpineAtlasDialogue::OnReattachButton()
{
	if (m_pDxLibSpinePlayer != nullptr)
	{
		std::wstring wstrSlotName = m_slotComboBox.GetSelectedItemText();
		std::wstring wstrAtlasRegionName = m_attachmentComboBox.GetSelectedItemText();
		if (!wstrSlotName.empty() && !wstrAtlasRegionName.empty())
		{
			std::string strSlotName = win_text::NarrowUtf8(wstrSlotName);
			std::string strAtlasRegionName = win_text::NarrowUtf8(wstrAtlasRegionName);
			m_pDxLibSpinePlayer->ReplaceAttachment(strSlotName.c_str(), strAtlasRegionName.c_str());
		}
	}

}
/*再配置*/
void CSpineAtlasDialogue::ResizeControls()
{
	RECT clientRect;
	::GetClientRect(m_hWnd, &clientRect);

	long clientWidth = clientRect.right - clientRect.left;
	long clientHeight = clientRect.bottom - clientRect.top;

	long spaceX = clientWidth / 96;
	long spaceY = clientHeight / 96;

	long x = spaceX;
	long y = spaceY;
	long w = clientWidth / 2 - spaceX * 2;
	long h = clientHeight * 8 / 10;

	int iFontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);

	if (m_hSlotStatic != nullptr)
	{
		::MoveWindow(m_hSlotStatic, x, y, w, h, TRUE);
	}
	y += iFontHeight;

	HWND hSlotComboBox = m_slotComboBox.GetHwnd();
	if (hSlotComboBox != nullptr)
	{
		::MoveWindow(hSlotComboBox, x, y, w, h, TRUE);
	}

	x += clientWidth / 2;
	y = spaceY;
	if (m_hAttachmentStatic != nullptr)
	{
		::MoveWindow(m_hAttachmentStatic, x, y, w, h, TRUE);
	}
	y += iFontHeight;

	HWND hAttachmentComboBox = m_attachmentComboBox.GetHwnd();
	if (hAttachmentComboBox != nullptr)
	{
		::MoveWindow(hAttachmentComboBox, x, y, w, h, TRUE);
	}

	w = clientWidth / 4;
	h = iFontHeight * 2;
	x = spaceX;
	y = clientHeight - h -spaceY;
	if (m_hReattachButton != nullptr)
	{
		::MoveWindow(m_hReattachButton, x, y, w, h, TRUE);
	}
}
