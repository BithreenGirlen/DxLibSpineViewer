
#include "dialogue_template.h"


CDialogueTemplate::CDialogueTemplate()
{

}

CDialogueTemplate::~CDialogueTemplate()
{
	Release();
}

void CDialogueTemplate::SetWindowSize(unsigned short usWidth, unsigned short usHeight)
{
	m_usWidth = usWidth;
	m_usHeight = usHeight;
}

void CDialogueTemplate::MakeWindowResizable(bool bResizable)
{
	m_bResizable = bResizable;
}

void CDialogueTemplate::MakeWindowChild(bool bChild)
{
	m_bChild = bChild;
}

const unsigned char* CDialogueTemplate::Generate(const wchar_t* wszWindowTitle)
{
	/*
	* Dialogue template without child controls.
	* https://learn.microsoft.com/en-us/windows/win32/dlgbox/dlgtemplateex
	*/
#pragma pack(push, 1)
	struct SDialogueTemplateHeader
	{
		WORD dlgVer = 0x01;
		WORD signature = 0xffff;
		DWORD helpID = 0x00;
		DWORD exstyle = 0x00;
		DWORD style = DS_MODALFRAME | DS_SETFONT | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU;
		WORD cDlgItems = 0x00;
		short x = 0x00;
		short y = 0x00;
		short cx = 0x80;
		short cy = 0x60;
		WORD menu = 0x00;
		WORD windowClass = 0x00;
	};

	struct SDialogueTemplateFont
	{
		WORD pointsize = 0x08;
		WORD weight = FW_REGULAR;
		BYTE italic = TRUE;
		BYTE characterset = ANSI_CHARSET;
	};
#pragma pack (pop)

	static constexpr const wchar_t defaultTitle[] = L"Dialogue";
	static constexpr const size_t defaultTitleSize = sizeof(defaultTitle);
	static constexpr const wchar_t defaultTypeFace[] = L"MS Shell Dlg";
	static constexpr const size_t defaultTypeFaceSize = sizeof(defaultTypeFace);
	struct SDialogueTemplateEx
	{
		SDialogueTemplateHeader header;
		const wchar_t* title = defaultTitle;
		SDialogueTemplateFont font;
		const wchar_t* typeFace = defaultTypeFace;
	};

	SDialogueTemplateEx sDialogueTemplateEx;

	sDialogueTemplateEx.header.cx = m_usWidth;
	sDialogueTemplateEx.header.cy = m_usHeight;

	if (m_bChild)
	{
		sDialogueTemplateEx.header.style &= ~(WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_BORDER);
		sDialogueTemplateEx.header.style |= WS_CHILD;
	}
	else if (m_bResizable)
	{
		sDialogueTemplateEx.header.style &= ~DS_MODALFRAME;
		sDialogueTemplateEx.header.style |= WS_THICKFRAME;
	}

	size_t titleSize = defaultTitleSize;
	if (wszWindowTitle != nullptr)
	{
		sDialogueTemplateEx.title = wszWindowTitle;
		titleSize = (wcslen(wszWindowTitle) + 1) * sizeof(wchar_t);
	}

	Release();
	size_t dataSize = sizeof(SDialogueTemplateHeader) + titleSize + sizeof(SDialogueTemplateFont) + defaultTypeFaceSize;
	m_pData = static_cast<unsigned char*>(malloc(dataSize));

	size_t nWritten = 0;
	size_t nLen = sizeof(SDialogueTemplateHeader);
	memcpy(&m_pData[nWritten], &sDialogueTemplateEx.header, nLen);
	nWritten += nLen;

	nLen = titleSize;
	memcpy(&m_pData[nWritten], sDialogueTemplateEx.title, nLen);
	nWritten += nLen;

	nLen = sizeof(SDialogueTemplateFont);
	memcpy(&m_pData[nWritten], &sDialogueTemplateEx.font, nLen);
	nWritten += nLen;

	nLen = defaultTypeFaceSize;
	memcpy(&m_pData[nWritten], sDialogueTemplateEx.typeFace, nLen);
	nWritten += nLen;

	return m_pData;
}

void CDialogueTemplate::Release()
{
	if (m_pData != nullptr)
	{
		free(m_pData);
		m_pData = nullptr;
	}
}
