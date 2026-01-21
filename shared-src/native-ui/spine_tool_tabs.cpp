
#include <algorithm>
/* Not inclied to use this heavy STL */
//#include <regex>

#include "spine_tool_tabs.h"
#include "dialogue_layout.h"
#include "../win_text.h"

/// @brief global data for slot exclusion.
namespace slot_exclusion
{
	static std::string s_filter;
	static bool IsSlotToBeExcluded(const char* szSlotName, size_t nSlotNameLength)
	{
		if (s_filter.empty())return false;

		const char* pEnd = szSlotName + nSlotNameLength;
		return std::search(szSlotName, pEnd, s_filter.begin(), s_filter.end()) != pEnd;
		//return std::regex_search(szSlotName, pEnd, std::regex(s_filter));
	}
};

/// @brief Conversion between UTF-16LE and UTF-8 for a list
namespace controls_util
{
	static void WidenList(const std::vector<std::string>& strList, std::vector<std::wstring>& wstrList)
	{
		wstrList.resize(strList.size());
		for (size_t i = 0; i < strList.size(); ++i)
		{
			wstrList[i] = win_text::WidenUtf8(strList[i]);
		}
		std::sort(wstrList.begin(), wstrList.end());
	}

	static void NarrowList(const std::vector<std::wstring>& wstrList, std::vector<std::string>& strList)
	{
		strList.resize(wstrList.size());
		for (size_t i = 0; i < strList.size(); ++i)
		{
			strList[i] = win_text::NarrowUtf8(wstrList[i]);
		}
	}
}

/* ==================== Base tab class ==================== */

HWND CTabBase::Create(HINSTANCE hInstance, HWND hWndParent, const unsigned char* pDialogueTemplate, CDxLibSpinePlayer* pPlayer)
{
	m_pDxLibSpinePlayer = pPlayer;

	return ::CreateDialogIndirectParamA(hInstance, (LPCDLGTEMPLATE)pDialogueTemplate, hWndParent, (DLGPROC)DialogProc, (LPARAM)this);
}

void CTabBase::OnRefresh()
{
	RefreshControls();
}

/*C CALLBACK*/
LRESULT CTabBase::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		::SetWindowLongPtr(hWnd, DWLP_USER, lParam);
	}

	CTabBase* pThis = reinterpret_cast<CTabBase*>(::GetWindowLongPtr(hWnd, DWLP_USER));
	if (pThis != nullptr)
	{
		return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	}
	return FALSE;
}
/*メッセージ処理*/
LRESULT CTabBase::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
LRESULT CTabBase::OnInit(HWND hWnd)
{
	m_hWnd = hWnd;

	CreateControls();
	RefreshControls();
	ResizeControls();

	return TRUE;
}
/*WM_CLOSE*/
LRESULT CTabBase::OnClose()
{
	::DestroyWindow(m_hWnd);
	m_hWnd = nullptr;
	return 0;
}
/*WM_SIZE*/
LRESULT CTabBase::OnSize()
{
	ResizeControls();

	return 0;
}

/* ==================== Slot tab ==================== */

bool CSpineSlotTab::HasSlotExclusionFilter()
{
	return !slot_exclusion::s_filter.empty();
}

bool(*CSpineSlotTab::GetSlotExcludeCallback())(const char*, size_t)
{
	return &slot_exclusion::IsSlotToBeExcluded;
}

/*WM_COMMAND*/
LRESULT CSpineSlotTab::OnCommand(WPARAM wParam, LPARAM lParam)
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
			if (reinterpret_cast<HWND>(lParam) == m_replaceableSlotComboBox.GetHwnd())
			{
				OnSlotSelect();
			}
		}
		else
		{
			switch (wmId)
			{
			case Controls::kExcludeButton:
				OnExcludeButton();
				break;
			case Controls::kReplaceButton:
				OnReplaceButton();
				break;
			case Controls::kBoundButton:
				OnBoundButton();
				break;
			default:
				break;
			}
		}
	}

	return 0;
}

void CSpineSlotTab::CreateControls()
{
	m_slotFilterEdit.Create(win_text::WidenUtf8(slot_exclusion::s_filter).c_str(), m_hWnd);
	m_slotFilterEdit.SetHint(L"Partial slot name to exclude");

	m_slotListView.Create(m_hWnd, { L"Slots to exclude" }, true);

	m_slotExcludeButton.Create(L"Exclude slot", m_hWnd, reinterpret_cast<HMENU>(Controls::kExcludeButton));

	m_slotReplacementSeparator.Create(L"", m_hWnd, true);
	m_slotStatic.Create(L"Slot", m_hWnd);
	m_attachmentStatic.Create(L"Attachment", m_hWnd);

	m_replaceableSlotComboBox.Create(m_hWnd);
	m_attachmentComboBox.Create(m_hWnd);

	m_replaceButton.Create(L"Replace slot", m_hWnd, reinterpret_cast<HMENU>(Controls::kReplaceButton));

	m_slotBoundSeparator.Create(L"", m_hWnd, true);;
	m_slotBoundComboBox.Create(m_hWnd);
	m_slotBoundButton.Create(L"Bound", m_hWnd, reinterpret_cast<HMENU>(Controls::kBoundButton));
	m_slotBoundStatic.Create(L"", m_hWnd);
}
/*寸法・位置調整*/
void CSpineSlotTab::ResizeControls()
{
	using namespace dialogue_layout;
	LayoutControls(m_hWnd, Constants::kFontSize,
		{
			{m_slotFilterEdit.GetHwnd()},
			{m_slotListView.GetHwnd(), WidthOption::Auto, HeightOption::List},
			{m_slotExcludeButton.GetHwnd(), WidthOption::Quarter},

			{m_slotReplacementSeparator.GetHwnd(), WidthOption::Auto, HeightOption::Fixed, 0, 1},

			{m_slotStatic.GetHwnd()},
			{m_replaceableSlotComboBox.GetHwnd(), WidthOption::Auto, HeightOption::Combo},
			{m_attachmentStatic.GetHwnd()},
			{m_attachmentComboBox.GetHwnd(), WidthOption::Auto, HeightOption::Combo},
			{m_replaceButton.GetHwnd(), WidthOption::Quarter},

			{m_slotBoundSeparator.GetHwnd(), WidthOption::Auto, HeightOption::Fixed, 0, 1},

			{m_slotBoundComboBox.GetHwnd(), WidthOption::Auto, HeightOption::Combo},
			{m_slotBoundButton.GetHwnd(), WidthOption::Quarter},
			{m_slotBoundStatic.GetHwnd()},
		});
	m_slotListView.AdjustWidth();
}

void CSpineSlotTab::RefreshControls()
{
	std::vector<std::wstring> temp;
	controls_util::WidenList(m_pDxLibSpinePlayer->getSlotNames(), temp);
	m_slotListView.CreateSingleList(temp);

	m_slotAttachmentMap = m_pDxLibSpinePlayer->getSlotNamesWithTheirAttachments();

	std::vector<std::wstring> wstrSlotNames;
	wstrSlotNames.reserve(m_slotAttachmentMap.size());
	for (const auto& slot : m_slotAttachmentMap)
	{
		wstrSlotNames.push_back(win_text::WidenUtf8(slot.first));
	}
	m_replaceableSlotComboBox.Setup(wstrSlotNames);
	OnSlotSelect();

	m_slotBoundComboBox.Setup(temp);
}
/*適用ボタン押下*/
void CSpineSlotTab::OnExcludeButton()
{
	if (m_pDxLibSpinePlayer != nullptr)
	{
		const std::wstring wstrSlotExclusionRegex = m_slotFilterEdit.GetText();
		slot_exclusion::s_filter = win_text::NarrowUtf8(wstrSlotExclusionRegex);
		if (!slot_exclusion::s_filter.empty())
		{
			m_pDxLibSpinePlayer->setSlotExcludeCallback(&slot_exclusion::IsSlotToBeExcluded);
		}
		else
		{
			m_pDxLibSpinePlayer->setSlotExcludeCallback(nullptr);

			const std::vector<std::wstring> wstrCheckedItems = m_slotListView.PickupCheckedItems();
			std::vector<std::string> strCheckedItems;
			controls_util::NarrowList(wstrCheckedItems, strCheckedItems);
			m_pDxLibSpinePlayer->setSlotsToExclude(strCheckedItems);
		}
	}
}
/*選択項目変更*/
void CSpineSlotTab::OnSlotSelect()
{
	const std::string strSlotName = win_text::NarrowUtf8(m_replaceableSlotComboBox.GetSelectedItemText());
	if (strSlotName.empty())return;

	const auto& iter = m_slotAttachmentMap.find(strSlotName);
	if (iter != m_slotAttachmentMap.cend())
	{
		const std::vector<std::string>& attachmentNames = iter->second;

		std::vector<std::wstring> wstrAttachmentNames;
		controls_util::WidenList(attachmentNames, wstrAttachmentNames);
		m_attachmentComboBox.Setup(wstrAttachmentNames);
		::InvalidateRect(m_hWnd, nullptr, FALSE);
	}
}
/*再装着ボタン*/
void CSpineSlotTab::OnReplaceButton()
{
	const std::wstring wstrSlotName = m_replaceableSlotComboBox.GetSelectedItemText();
	const std::wstring wstrAtlasRegionName = m_attachmentComboBox.GetSelectedItemText();
	if (!wstrSlotName.empty() && !wstrAtlasRegionName.empty())
	{
		const std::string strSlotName = win_text::NarrowUtf8(wstrSlotName);
		const std::string strAtlasRegionName = win_text::NarrowUtf8(wstrAtlasRegionName);
		m_pDxLibSpinePlayer->replaceAttachment(strSlotName.c_str(), strAtlasRegionName.c_str());
	}
}

void CSpineSlotTab::OnBoundButton()
{
	const std::wstring wstrSlotName = m_slotBoundComboBox.GetSelectedItemText();
	const std::string strSlotName = win_text::NarrowUtf8(wstrSlotName);
	DxLib::FLOAT4 bound = m_pDxLibSpinePlayer->getCurrentBoundingOfSlot(strSlotName);
	if (bound.z == 0.f)
	{
		::SetWindowTextW(m_slotBoundStatic.GetHwnd(), L"The slot not found in this animation");
	}
	else
	{
		wchar_t sBuffer[128]{};
		swprintf_s(sBuffer, L"Slot bound: (%.2f, %.2f, %.2f, %.2f)", bound.x, bound.y, bound.x + bound.z, bound.y + bound.w);
		::SetWindowTextW(m_slotBoundStatic.GetHwnd(), sBuffer);
	}
}

/* ==================== Animation tab ==================== */

LRESULT CSpineAnimationTab::OnCommand(WPARAM wParam, LPARAM lParam)
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
		case Controls::kSetAnimationButton:
			OnSetAnimationButton();
			break;
		case Controls::kAddAnimationTracksButton:
			OnAddAnimationTracksButton();
			break;
		default:
			break;
		}
	}

	return 0;
}

void CSpineAnimationTab::CreateControls()
{
	m_animationComboBox.Create(m_hWnd);
	m_setAnimationButton.Create(L"Set animation", m_hWnd, reinterpret_cast<HMENU>(Controls::kSetAnimationButton));

	m_addAnimationTracksSeparator.Create(L"", m_hWnd, true);

	m_animationListView.Create(m_hWnd, { L"Animation tracks to add" }, true);
	m_addAnimationTracksButton.Create(L"Add", m_hWnd, reinterpret_cast<HMENU>(Controls::kAddAnimationTracksButton));
}

void CSpineAnimationTab::ResizeControls()
{
	using namespace dialogue_layout;
	LayoutControls(m_hWnd, Constants::kFontSize,
		{
			{m_animationComboBox.GetHwnd(), WidthOption::Auto, HeightOption::Combo},
			{m_setAnimationButton.GetHwnd(), WidthOption::Quarter},
			{m_addAnimationTracksSeparator.GetHwnd(), WidthOption::Auto, HeightOption::Fixed, 0, 1},

			{m_animationListView.GetHwnd(), WidthOption::Auto, HeightOption::List},
			{m_addAnimationTracksButton.GetHwnd(), WidthOption::Quarter},
		});
	m_animationListView.AdjustWidth();
}

void CSpineAnimationTab::RefreshControls()
{
	const auto& animationNames = m_pDxLibSpinePlayer->getAnimationNames();
	std::vector<std::wstring> temp;
	controls_util::WidenList(animationNames, temp);

	m_animationComboBox.Setup(temp);
	m_animationListView.CreateSingleList(temp);
}

void CSpineAnimationTab::OnSetAnimationButton()
{
	const std::wstring wstrAnimationName = m_animationComboBox.GetSelectedItemText();
	const std::string strAnimationName = win_text::NarrowUtf8(wstrAnimationName);
	m_pDxLibSpinePlayer->setAnimationByName(strAnimationName.c_str());
}

void CSpineAnimationTab::OnAddAnimationTracksButton()
{
	const std::vector<std::wstring> wstrCheckedItems = m_animationListView.PickupCheckedItems();
	std::vector<std::string> strCheckedItems;
	controls_util::NarrowList(wstrCheckedItems, strCheckedItems);
	m_pDxLibSpinePlayer->addAnimationTracks(strCheckedItems);
}

/* ==================== Skin tab ==================== */

LRESULT CSpineSkinTab::OnCommand(WPARAM wParam, LPARAM lParam)
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
		case Controls::kSetSkinButton:
			OnSetSkinButton();
			break;
		case Controls::kMixSkinButton:
			OnMixSkinButton();
			break;
		default:
			break;
		}
	}

	return 0;
}

void CSpineSkinTab::CreateControls()
{
	m_skinComboBox.Create(m_hWnd);
	m_setSkinButton.Create(L"Set skin", m_hWnd, reinterpret_cast<HMENU>(Controls::kSetSkinButton));

	m_mixSkinSeparator.Create(L"", m_hWnd, true);

	m_skinListView.Create(m_hWnd, { L"Skins to mix" }, true);
	m_mixSkinButton.Create(L"Mix skins", m_hWnd, reinterpret_cast<HMENU>(Controls::kMixSkinButton));
}

void CSpineSkinTab::ResizeControls()
{
	using namespace dialogue_layout;
	LayoutControls(m_hWnd, Constants::kFontSize,
		{
			{m_skinComboBox.GetHwnd(), WidthOption::Auto, HeightOption::Combo},
			{m_setSkinButton.GetHwnd(), WidthOption::Quarter},
			{m_mixSkinSeparator.GetHwnd(), WidthOption::Auto, HeightOption::Fixed, 0, 1},

			{m_skinListView.GetHwnd(), WidthOption::Auto, HeightOption::List},
			{m_mixSkinButton.GetHwnd(), WidthOption::Quarter},
		});
	m_skinListView.AdjustWidth();
}

void CSpineSkinTab::RefreshControls()
{
	const auto& skinNames = m_pDxLibSpinePlayer->getSkinNames();
	std::vector<std::wstring> temp;
	controls_util::WidenList(skinNames, temp);

	m_skinComboBox.Setup(temp);
	m_skinListView.CreateSingleList(temp);
}

void CSpineSkinTab::OnSetSkinButton()
{
	const std::wstring wstrSkinnName = m_skinComboBox.GetSelectedItemText();
	const std::string strSkinName = win_text::NarrowUtf8(wstrSkinnName);
	m_pDxLibSpinePlayer->setSkinByName(strSkinName.c_str());
}

void CSpineSkinTab::OnMixSkinButton()
{
	const std::vector<std::wstring> wstrCheckedItems = m_skinListView.PickupCheckedItems();
	std::vector<std::string> strCheckedItems;
	controls_util::NarrowList(wstrCheckedItems, strCheckedItems);
	m_pDxLibSpinePlayer->mixSkins(strCheckedItems);
}

/* ==================== Rendering tab ==================== */

LRESULT CSpineRenderingTab::OnCommand(WPARAM wParam, LPARAM lParam)
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
		case Controls::kPmaButton:
			OnPmaButton();
			break;
		case Controls::kBlendModeButton:
			OnBlendModeButton();
			break;
		case Controls::kDrawOrderButton:
			OnDrawOrderButton();
			break;
		default:
			break;
		}
	}

	return 0;
}

void CSpineRenderingTab::CreateControls()
{
	m_pmaButton.Create(L"Alpha premultiplied", m_hWnd, reinterpret_cast<HMENU>(Controls::kPmaButton), true);
	m_blendModeSeparator.Create(L"", m_hWnd, true);
	m_blemdModeButton.Create(L"Force blend-mode-normal", m_hWnd, reinterpret_cast<HMENU>(Controls::kBlendModeButton), true);
	m_drawOrderSeparator.Create(L"", m_hWnd, true);
	m_drawOrderButton.Create(L"Reverse draw order", m_hWnd, reinterpret_cast<HMENU>(Controls::kDrawOrderButton), true);

#if defined(SPINE_4_0) || defined(SPINE_4_1_OR_LATER) || defined(SPINE_4_2_OR_LATER)
	::EnableWindow(m_pmaButton.GetHwnd(), FALSE);
#endif
	::EnableWindow(m_drawOrderButton.GetHwnd(), m_pDxLibSpinePlayer->getNumberOfSpines() > 1 ? TRUE : FALSE);
}

void CSpineRenderingTab::ResizeControls()
{
	using namespace dialogue_layout;
	LayoutControls(m_hWnd, Constants::kFontSize,
		{
			{m_pmaButton.GetHwnd(), WidthOption::Half},
			{m_blendModeSeparator.GetHwnd(), WidthOption::Auto, HeightOption::Fixed, 0, 1},

			{m_blemdModeButton.GetHwnd(), WidthOption::Half},
			{m_drawOrderSeparator.GetHwnd(), WidthOption::Auto, HeightOption::Fixed, 0, 1},
			{m_drawOrderButton.GetHwnd(), WidthOption::Half},
		});
}

void CSpineRenderingTab::RefreshControls()
{
	m_pmaButton.SetCheckBox(m_pDxLibSpinePlayer->isAlphaPremultiplied());
	m_blemdModeButton.SetCheckBox(m_pDxLibSpinePlayer->isBlendModeNormalForced());
	m_drawOrderButton.SetCheckBox(m_pDxLibSpinePlayer->isDrawOrderReversed());
}

void CSpineRenderingTab::OnPmaButton()
{
	m_pDxLibSpinePlayer->togglePma();
	RefreshControls();
}

void CSpineRenderingTab::OnBlendModeButton()
{
	m_pDxLibSpinePlayer->toggleBlendModeAdoption();
	RefreshControls();
}

void CSpineRenderingTab::OnDrawOrderButton()
{
	m_pDxLibSpinePlayer->setDrawOrder(!m_pDxLibSpinePlayer->isDrawOrderReversed());
	RefreshControls();
}
