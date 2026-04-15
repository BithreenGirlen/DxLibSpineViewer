#ifndef DIALOGUE_TEMPLATE_H_
#define DIALOGUE_TEMPLATE_H_

#include <Windows.h>

/// @brief 子ウィンドウを有さないDLGTEMPLATEEX生成器
class CDialogueTemplate
{
public:
	CDialogueTemplate();
	~CDialogueTemplate();

	/// @brief ウィンドウの大きさを設定
	void setWindowSize(unsigned short usWidth, unsigned short usHeight);
	/// @brief 抓みによる大きさ変更をできるようにするか
	void makeWindowResizable(bool isResizable);
	/// @brief 自身を子ウィンドウとするか。
	/// @remark タブウィンドウ等に用いる
	void makeWindowChild(bool isChild);

	/// @brief 生成
	const unsigned char* generate(const wchar_t* windowTitle = nullptr);
private:
	enum Constants {kBaseWidth = 200, kBaseHeight = 240};

	WORD m_usWidth = Constants::kBaseWidth;
	WORD m_usHeight = Constants::kBaseHeight;

	unsigned char* m_pData = nullptr;

	bool m_isResizable = false;
	bool m_isChild = false;

	void release();
};

#endif // !DIALOGUE_TEMPLATE_H_
