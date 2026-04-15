
#include "dialogue_template.h"


CDialogueTemplate::CDialogueTemplate()
{

}

CDialogueTemplate::~CDialogueTemplate()
{
	release();
}

void CDialogueTemplate::setWindowSize(unsigned short usWidth, unsigned short usHeight)
{
	m_usWidth = usWidth;
	m_usHeight = usHeight;
}

void CDialogueTemplate::makeWindowResizable(bool isResizable)
{
	m_isResizable = isResizable;
}

void CDialogueTemplate::makeWindowChild(bool isChild)
{
	m_isChild = isChild;
}

const unsigned char* CDialogueTemplate::generate(const wchar_t* windowTitle)
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

	SDialogueTemplateEx dialogueTemplateEx;

	dialogueTemplateEx.header.cx = m_usWidth;
	dialogueTemplateEx.header.cy = m_usHeight;

	if (m_isChild)
	{
		dialogueTemplateEx.header.style &= ~(WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_BORDER);
		dialogueTemplateEx.header.style |= WS_CHILD;
	}
	else if (m_isResizable)
	{
		dialogueTemplateEx.header.style &= ~DS_MODALFRAME;
		dialogueTemplateEx.header.style |= WS_THICKFRAME;
	}

	size_t titleSize = defaultTitleSize;
	if (windowTitle != nullptr)
	{
		dialogueTemplateEx.title = windowTitle;
		titleSize = (wcslen(windowTitle) + 1) * sizeof(wchar_t);
	}

	release();
	size_t dataSize = sizeof(SDialogueTemplateHeader) + titleSize + sizeof(SDialogueTemplateFont) + defaultTypeFaceSize;
	m_pData = static_cast<unsigned char*>(malloc(dataSize));
	if (m_pData == nullptr)return nullptr;

	size_t nWritten = 0;
	size_t nLen = sizeof(SDialogueTemplateHeader);
	memcpy(&m_pData[nWritten], &dialogueTemplateEx.header, nLen);
	nWritten += nLen;

	nLen = titleSize;
	memcpy(&m_pData[nWritten], dialogueTemplateEx.title, nLen);
	nWritten += nLen;

	nLen = sizeof(SDialogueTemplateFont);
	memcpy(&m_pData[nWritten], &dialogueTemplateEx.font, nLen);
	nWritten += nLen;

	nLen = defaultTypeFaceSize;
	memcpy(&m_pData[nWritten], dialogueTemplateEx.typeFace, nLen);
	nWritten += nLen;

	return m_pData;
}

void CDialogueTemplate::release()
{
	if (m_pData != nullptr)
	{
		free(m_pData);
		m_pData = nullptr;
	}
}
