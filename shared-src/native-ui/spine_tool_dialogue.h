#ifndef SPINE_TOOL_DIALOGUE_H_
#define SPINE_TOOL_DIALOGUE_H_

#include <Windows.h>

#include "../spine_player_shared.h"
#include "dialogue_controls.h"
#include "spine_manipulator_dialogue.h"
#include "spine_atlas_dialogue.h"

class CSpineToolDialogue
{
public:
	CSpineToolDialogue();
	~CSpineToolDialogue();

	HWND Create(HINSTANCE hInstance, HWND hWndParent, const wchar_t* pwzWindowName, CDxLibSpinePlayer* pPlayer);
	HWND GetHwnd()const { return m_hWnd; }

	const CSpineManipulatorDialogue* GetManipulatorDialogue() const { return &m_spineManipulatorDialogue; }
	const CSpineAtlasDialogue* GetAtlasDialogue() const { return &m_spineAtlasDialogue; }
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
			Size = 0, Animation, Skin, Slot, Rendering, Export,
		};
	};
	static constexpr const wchar_t* const m_tabNames[] = { L"Size/Scale", L"Animation", L"Skin", L"Slot", L"Rendering", L"Export" };

	HFONT m_hFont = nullptr;

	CTab m_tab;

	static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

	LRESULT ResizeControls();

	std::vector<unsigned char> GenerateTabPageDialogueTemplate(const wchar_t* windowName);

	CDxLibSpinePlayer* m_pDxLibSpinePlayer = nullptr;

	CSpineManipulatorDialogue m_spineManipulatorDialogue;
	CSpineAtlasDialogue m_spineAtlasDialogue;

	HWND m_hLastTab = nullptr;
};
#endif // !SPINE_TOOL_DIALOGUE_H_
