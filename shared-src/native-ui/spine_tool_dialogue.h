#ifndef SPINE_TOOL_DIALOGUE_H_
#define SPINE_TOOL_DIALOGUE_H_

#include <Windows.h>

#include "../spine_player_shared.h"
#include "dialogue_controls.h"
#include "spine_tool_tabs.h"

class CSpineToolDialogue
{
public:
	CSpineToolDialogue();
	~CSpineToolDialogue();

	HWND Create(HINSTANCE hInstance, HWND hWndParent, const wchar_t* pwzWindowName, CDxLibSpinePlayer* pPlayer);
	HWND GetHwnd()const { return m_hWnd; }
private:
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnInit(HWND hWnd);
	LRESULT OnClose();
	LRESULT OnSize();
	LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);

	struct Constants { enum { kFontSize = 16 }; };
	struct BaseSize { enum { kWidth = 320, kHeight = 320 }; };
	struct Tab
	{
		enum
		{
			Animation = 0, Skin, Slot, Rendering,
		};
	};
	static constexpr const wchar_t* const m_tabNames[] = { L"Animation", L"Skin", L"Slot", L"Rendering" };

	HFONT m_hFont = nullptr;

	CTab m_tab;

	static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

	void ResizeControls();
	void OnTabSelect();

	CDxLibSpinePlayer* m_pDxLibSpinePlayer = nullptr;

	CSpineAnimationTab m_spineAnimationTab;
	CSpineSkinTab m_spineSkinTab;
	CSpineSlotTab m_spineSlotTab;
	CSpineRenderingTab m_spineRenderingTab;

	HWND m_hLastTab = nullptr;
};
#endif // !SPINE_TOOL_DIALOGUE_H_
