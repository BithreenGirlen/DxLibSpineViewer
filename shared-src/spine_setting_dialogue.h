#ifndef SPINE_SETTING_DIALOGUE_H_
#define SPINE_SETTING_DIALOGUE_H_

#include <Windows.h>

#include <string>

class CSpineSettingDialogue
{
public:
	CSpineSettingDialogue();
	~CSpineSettingDialogue();

	bool Open(HINSTANCE hInstance, HWND hWnd, const wchar_t* pwzWindowName);
	HWND GetHwnd()const { return m_hWnd; }

	const std::wstring& GetAtlasExtension() const { return m_wstrAtlasExtension; }
	const std::wstring& GetSkelExtension() const { return m_wstrSkelExtension; }
	bool IsSkelBinary() const { return m_bBinarySkel; }
private:
	const wchar_t* m_swzClassName = L"Spine setting dialogue";
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;
	HWND m_hParentWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	int MessageLoop();
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);

	enum Constants { kFontSize = 16 };
	enum Controls {kCheckButton = 1};
	HFONT m_hFont = nullptr;
	HWND m_hAtlasStatic = nullptr;
	HWND m_hAtlasEdit = nullptr;
	HWND m_hSkelStatic = nullptr;
	HWND m_hSkelEdit = nullptr;
	HWND m_hSkelBinCheckButton = nullptr;

	std::wstring m_wstrAtlasExtension = L".atlas";
	std::wstring m_wstrSkelExtension = L".skel";
	bool m_bBinarySkel = true;

	void GetClientAreaSize(long* width, long* height);
	static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

	void OnCheckButton();

	std::wstring GetEditBoxText(HWND hWnd);
	bool SetEditBoxText(HWND hWnd, const std::wstring& wstr);
	bool GetCheckState(HWND hWnd);
	void SetCheckBox(HWND hWnd, bool bToBeChecked);

	void GetInputs();
};
#endif // !SPINE_SETTING_DIALOGUE_H_
