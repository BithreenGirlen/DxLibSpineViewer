#ifndef DIALOGUE_TEMPLATE_H_
#define DIALOGUE_TEMPLATE_H_

#include <Windows.h>

#include <vector>

class CDialogueTemplate
{
public:
	CDialogueTemplate();
	~CDialogueTemplate();

	void SetWindowSize(unsigned short usWidth, unsigned short usHeight);
	void MakeWindowResizable(bool bResizable);

	std::vector<unsigned char> Generate(const wchar_t* wszWindowTitle = nullptr);

private:
	enum Constants {kBaseWidth = 200, kBaseHeight = 240};

	WORD m_usWidth = Constants::kBaseWidth;
	WORD m_usHeight = Constants::kBaseHeight;

	bool m_bResizable = false;
};

#endif // !DIALOGUE_TEMPLATE_H_
