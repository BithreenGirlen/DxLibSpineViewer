

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
	sWinDialogueTemplate.SetWindowSize(120, 80);
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

	m_slotStatic.Create(L"Slot", m_hWnd);
	m_attachmentStatic.Create(L"Attachment", m_hWnd);

	m_slotComboBox.Create(m_hWnd);
	m_attachmentComboBox.Create(m_hWnd);

	m_reattachButton.Create(L"Re-attach", m_hWnd, reinterpret_cast<HMENU>(Controls::kReattachButton));

	if (m_pDxLibSpinePlayer != nullptr)
	{
		m_slotAttachmentMap = m_pDxLibSpinePlayer->GetSlotNamesWithTheirAttachments();

		std::vector<std::wstring> wstrSlotNames;
		wstrSlotNames.reserve(m_slotAttachmentMap.size());
		for (const auto& slot : m_slotAttachmentMap)
		{
			wstrSlotNames.push_back(win_text::WidenUtf8(slot.first));
		}
		m_slotComboBox.Setup(wstrSlotNames);
		OnSlotSelect();
	}

	ResizeControls();

	::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));

	return TRUE;
}
/*WM_CLOSE*/
LRESULT CSpineAtlasDialogue::OnClose()
{
	m_slotAttachmentMap.clear();

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

		WORD usCode = HIWORD(wParam);
		if (usCode == CBN_SELCHANGE)
		{
			/*Notification code*/
			if (reinterpret_cast<HWND>(lParam) == m_slotComboBox.GetHwnd())
			{
				OnSlotSelect();
			}
		}
		else
		{
			switch (wmId)
			{
			case Controls::kReattachButton:
				OnReattachButton();
				break;
			default:
				break;
			}
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

	long spaceX = clientWidth / 24;
	long spaceY = clientHeight / 96;

	long x = spaceX;
	long y = spaceY * 2;
	long w = clientWidth - spaceX * 2;
	long h = clientHeight * 8 / 10;

	long fontHeight = static_cast<long>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);

	::MoveWindow(m_slotStatic.GetHwnd(), x, y, w, h, TRUE);

	y += fontHeight;
	::MoveWindow(m_slotComboBox.GetHwnd(), x, y, w, h, TRUE);

	y += fontHeight * 2 + spaceY;
	::MoveWindow(m_attachmentStatic.GetHwnd(), x, y, w, h, TRUE);

	y += fontHeight;
	::MoveWindow(m_attachmentComboBox.GetHwnd(), x, y, w, h, TRUE);

	w = clientWidth / 2;
	h = static_cast<int>(fontHeight * 1.5);
	x = spaceX;
	y = clientHeight - h -spaceY * 2;
	::MoveWindow(m_reattachButton.GetHwnd(), x, y, w, h, TRUE);
}
/*選択項目変更*/
void CSpineAtlasDialogue::OnSlotSelect()
{
	std::string strSlotName = win_text::NarrowUtf8(m_slotComboBox.GetSelectedItemText());
	if (strSlotName.empty())return;

	const auto& iter = m_slotAttachmentMap.find(strSlotName);
	if (iter != m_slotAttachmentMap.cend())
	{
		std::vector<std::string> &attachmentNames = iter->second;

		std::vector<std::wstring> wstrAttachmentNames;
		wstrAttachmentNames.reserve(attachmentNames.size());
		for (const auto& attachmentName : attachmentNames)
		{
			wstrAttachmentNames.push_back(win_text::WidenUtf8(attachmentName));
		}
		m_attachmentComboBox.Setup(wstrAttachmentNames);
		::InvalidateRect(m_hWnd, nullptr, FALSE);
	}
}
