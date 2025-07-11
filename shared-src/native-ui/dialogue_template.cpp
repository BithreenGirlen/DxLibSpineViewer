
#include <string>

#include "dialogue_template.h"


CDialogueTemplate::CDialogueTemplate()
{

}

CDialogueTemplate::~CDialogueTemplate()
{

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

std::vector<unsigned char> CDialogueTemplate::Generate(const wchar_t* wszWindowTitle)
{
	/*
	* Dialogue box template without any controls.
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

	struct SDialogueTemplateEx
	{
		SDialogueTemplateHeader header;
		std::wstring wstrTitle = L"Dialogue";
		SDialogueTemplateFont font;
		std::wstring wstrTypeface = L"MS Shell Dlg";
	};
#pragma pack (pop)

	SDialogueTemplateEx sDialogueTemplateEx;

	sDialogueTemplateEx.header.cx = m_usWidth;
	sDialogueTemplateEx.header.cy = m_usHeight;

	if (m_bResizable)
	{
		sDialogueTemplateEx.header.style |= WS_THICKFRAME & ~DS_MODALFRAME;
	}
	if (wszWindowTitle != nullptr)
	{
		sDialogueTemplateEx.wstrTitle = wszWindowTitle;
	}

	std::vector<unsigned char> v;
	v.resize(
		sizeof(SDialogueTemplateHeader) +
		(sDialogueTemplateEx.wstrTitle.size() + 1LL) * sizeof(wchar_t) +
		sizeof(SDialogueTemplateFont) +
		(sDialogueTemplateEx.wstrTypeface.size() + 1LL) * sizeof(wchar_t)
	);

	size_t nWritten = 0;
	size_t nLen = sizeof(SDialogueTemplateHeader);
	memcpy(&v[nWritten], &sDialogueTemplateEx.header, nLen);
	nWritten += nLen;

	nLen = (sDialogueTemplateEx.wstrTitle.size() + 1LL) * sizeof(wchar_t);
	memcpy(&v[nWritten], sDialogueTemplateEx.wstrTitle.c_str(), nLen);
	nWritten += nLen;

	nLen = sizeof(SDialogueTemplateFont);
	memcpy(&v[nWritten], &sDialogueTemplateEx.font, nLen);
	nWritten += nLen;

	nLen = (sDialogueTemplateEx.wstrTypeface.size() + 1LL) * sizeof(wchar_t);
	memcpy(&v[nWritten], sDialogueTemplateEx.wstrTypeface.c_str(), nLen);
	nWritten += nLen;

	return v;
}
