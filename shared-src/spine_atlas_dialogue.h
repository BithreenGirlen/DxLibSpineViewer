#ifndef SPINE_ATLAS_DIALOGUE_H_
#define SPINE_ATLAS_DIALOGUE_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "spine_player_shared.h"
#include "dialogue_controls.h"


class CSpineAtlasDialogue
{
public:
	CSpineAtlasDialogue();
	~CSpineAtlasDialogue();
	HWND Create(HINSTANCE hInstance, HWND hWndParent, const wchar_t* pwzWindowName, CDxLibSpinePlayer* pPlayer);
	HWND GetHwnd()const { return m_hWnd; }
private:
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnInit(HWND hWnd);
	LRESULT OnClose();
	LRESULT OnSize();
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);

	enum Constants { kFontSize = 16 };
	enum Controls
	{
		kReattachButton = 1,
	};

	HFONT m_hFont = nullptr;

	HWND m_hSlotStatic = nullptr;
	CComboBox m_slotComboBox;
	HWND m_hAttachmentStatic = nullptr;
	CComboBox m_attachmentComboBox;

	HWND m_hReattachButton = nullptr;

	static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

	void OnReattachButton();

	void ResizeControls();

	CDxLibSpinePlayer* m_pDxLibSpinePlayer = nullptr;
};
#endif // !SPINE_ATLAS_DIALOGUE_H_
