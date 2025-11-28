#ifndef SPINE_TOOL_TABS
#define SPINE_TOOL_TABS

#include <Windows.h>

#include "dialogue_controls.h"
#include "../spine_player_shared.h"

class CTabBase
{
public:
	virtual ~CTabBase() = default;

	HWND Create(HINSTANCE hInstance, HWND hWndParent, unsigned char* pDialogueTemplate, HFONT hFont, CDxLibSpinePlayer* pPlayer);
	HWND GetHwnd()const { return m_hWnd; }
protected:
	HWND m_hWnd;

	static LRESULT CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnInit(HWND hWnd);
	LRESULT OnClose();
	LRESULT OnSize();
	virtual LRESULT OnCommand(WPARAM wParam, LPARAM lParam) = 0;

	struct Constants { enum { kFontSize = 16 }; };

	HFONT m_hFont = nullptr;

	static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

	virtual void CreateControls() = 0;
	virtual void ResizeControls() = 0;
	virtual void RefreshControls() = 0;

	CDxLibSpinePlayer* m_pDxLibSpinePlayer = nullptr;
};

/// @brief Exclude/Replace slot
class CSpineSlotTab : public CTabBase
{
public:
	static bool HasSlotExclusionFilter();
	static bool (*GetSlotExcludeCallback())(const char*, size_t);
private:
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam) override;

	enum Controls
	{
		kExcludeButton = 1, kReplaceButton
	};

	CEdit m_slotFilterEdit;
	CListView m_slotListView;
	CButton m_slotExcludeButton;

	CStatic m_slotReplacementSeparator;

	CStatic m_slotStatic;
	CComboBox m_slotComboBox;
	CStatic m_attachmentStatic;
	CComboBox m_attachmentComboBox;
	CButton m_replaceButton;

	void CreateControls() override;
	void ResizeControls() override;
	void RefreshControls() override;

	void OnExcludeButton();
	void OnReplaceButton();

	void OnSlotSelect();

	std::unordered_map<std::string, std::vector<std::string>> m_slotAttachmentMap;
};

/// @brief Set/Mix animation
class CSpineAnimationTab : public CTabBase
{
public:

private:
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam) override;

	struct Controls
	{
		enum { kSetAnimationButton = 1, kMixAnimationButton };
	};

	CComboBox m_animationComboBox;
	CButton m_setAnimationButton;

	CStatic m_mixAnimationSeparator;

	CListView m_animationListView;
	CButton m_mixAnimationButton;

	void CreateControls() override;
	void ResizeControls() override;
	void RefreshControls() override;

	void OnSetAnimationButton();
	void OnMixAnimationButton();
};

/// @brief Set/Mix skin
class CSpineSkinTab : public CTabBase
{
public:

private:
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam) override;

	struct Controls
	{
		enum { kSetSkinButton = 1, kMixSkinButton };
	};


	void CreateControls() override;
	void ResizeControls() override;
	void RefreshControls() override;

	CComboBox m_skinComboBox;
	CButton m_setSkinButton;

	CStatic m_mixSkinSeparator;

	CListView m_skinListView;
	CButton m_mixSkinButton;

	void OnSetSkinButton();
	void OnMixSkinButton();
};

#endif // !SPINE_TOOL_TABS
