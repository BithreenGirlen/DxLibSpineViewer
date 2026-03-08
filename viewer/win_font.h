#ifndef WIN_FONT_H_
#define WIN_FONT_H_

#include <string>
#include <vector>

class CWinFont
{
public:
	CWinFont();
	~CWinFont();

	/// @brief 実行環境の言語・地域名取得
	/// @return 言語・地域名への内部保有ポインタ
	const wchar_t* const GetLocaleName();

	/// @brief 或る地域に於ける書体名を基に実行環境の書体名探索
	/// @param pwzFontFamilyName 何処かしらかの地域の書体名
	/// @return 実行環境の言語・文字で表される書体名
	std::wstring FindLocaleFontName(const wchar_t* pwzFontFamilyName);

	/// @brief 搭載書体名一覧取得
	/// @return 実行環境の言語・文字で表される書体名一覧
	std::vector<std::wstring> GetSystemFontFamilyNames();


	/// @brief 搭載書体名からファイル経路探索
	/// @param pwzFontFamilyName 書体名
	/// @param bBold 太字是否
	/// @param bItalic 斜体是否
	/// @return 候補となる書体ファイル経路
	std::vector<std::wstring> FindFontFilePaths(const wchar_t* pwzFontFamilyName, bool bBold, bool bItalic);
private:
	class Impl;
	Impl* m_pImpl = nullptr;
};

#endif // !WIN_FONT_H_
