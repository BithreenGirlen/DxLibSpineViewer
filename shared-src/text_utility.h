#ifndef TEXT_UTILITY_H_
#define TEXT_UTILITY_H_

#include <string>
#include <vector>

namespace text_utility
{
	template <typename CharType>
	void TextToLines(const std::basic_string<CharType>& text, std::vector<std::basic_string<CharType>>& lines)
	{
		std::basic_string<CharType> temp{};
		for (auto& c : text)
		{
			if (c == CharType('\r') || c == CharType('\n'))
			{
				if (!temp.empty())
				{
					lines.push_back(temp);
					temp.clear();
				}
				continue;
			}
			temp.push_back(c);
		}

		if (!temp.empty())
		{
			lines.push_back(temp);
		}
	}

	template <typename CharType>
	void SplitTextBySeparator(const std::basic_string<CharType>& text, const CharType separator, std::vector<std::basic_string<CharType>>& splits)
	{
		for (size_t nRead = 0; nRead < text.size();)
		{
			size_t nPos = text.find(separator, nRead);
			if (nPos == std::basic_string<CharType>::npos)
			{
				size_t nLen = text.size() - nRead;
				splits.emplace_back(text.substr(nRead, nLen));
				break;
			}

			size_t nLen = nPos - nRead;
			splits.emplace_back(text.substr(nRead, nLen));
			nRead += nLen + 1;
		}
	}

	template <typename CharType>
	void ReplaceAll(std::basic_string<CharType>& src, const std::basic_string<CharType>& strOld, const std::basic_string<CharType>& strNew)
	{
		if (strOld.empty() || strOld == strNew) return;

		for (size_t nRead = 0;;)
		{
			size_t nPos = src.find(strOld, nRead);
			if (nPos == std::basic_string<CharType>::npos) break;
			src.replace(nPos, strOld.size(), strNew);
			nRead = nPos + strNew.size();
		}
	}
	template <typename CharType>
	void ReplaceAll(std::basic_string<CharType>& src, const CharType* strOld, const CharType* strNew)
	{
		ReplaceAll(src, std::basic_string<CharType>(strOld), std::basic_string<CharType>(strNew));
	}

	template <typename CharType>
	void EliminateTag(std::basic_string<CharType>& src)
	{
		std::basic_string<CharType> result{};
		result.reserve(src.size());
		int iCount = 0;
		for (const auto& c : src)
		{
			if (c == CharType('<'))
			{
				++iCount;
				continue;
			}
			else if (c == CharType('>'))
			{
				--iCount;
				continue;
			}

			if (iCount == 0)
			{
				result.push_back(c);
			}
		}
		src = result;
	}

	template <typename CharType>
	void EliminateRuby(std::basic_string<CharType>& src)
	{
		const CharType strRuby[] = { '<', 'r', 'u', 'b', 'y', '>', '\0' };

		for (size_t nRead = 0;;)
		{
			size_t nPos = src.find(strRuby, nRead);
			if (nPos == std::basic_string<CharType>::npos)break;

			size_t nPos1 = src.find(CharType('|'), nPos);
			if (nPos1 == std::basic_string<CharType>::npos)break;

			size_t nPos2 = src.find(CharType('<'), nPos1);
			if (nPos2 == std::basic_string<CharType>::npos)break;

			size_t nLen = nPos2 - nPos1;
			src.erase(nPos1, nLen);
			nRead = nPos1;
		}
	}

	template <typename CharType>
	void ToXmlTags(const std::basic_string<CharType>& strText, const CharType* tagName, std::vector<std::basic_string<CharType>>& tags)
	{
		std::basic_string<CharType> strStart{ CharType('<') };
		if (tagName != nullptr)strStart += tagName;

		for (size_t nRead = 0;;)
		{
			size_t nPos = strText.find(strStart, nRead);
			if (nPos == std::basic_string<CharType>::npos)break;
			nRead = nPos + strStart.size() - 1;

			size_t nEnd = strText.find(CharType('>'), nRead);
			if (nEnd == std::basic_string<CharType>::npos)break;
			++nEnd;

			tags.push_back(strText.substr(nPos, nEnd - nPos));

			nRead = nEnd;
		}
	}

	template <typename CharType>
	void GetXmlAttributes(const std::basic_string<CharType>& strTag, std::vector<std::pair<std::basic_string<CharType>, std::basic_string<CharType>>>& attributes, bool bSingleQuote = false)
	{
		const CharType cQuote = bSingleQuote ? CharType('\'') : CharType('"');

		size_t nPos = strTag.find(CharType('<'));
		if (nPos == std::basic_string<CharType>::npos)return;
		++nPos;

		size_t nEnd = strTag.find(CharType('>'), nPos);
		if (nEnd == std::basic_string<CharType>::npos)return;

		attributes.clear();

		for (; nPos < nEnd && strTag[nPos] != CharType(' '); ++nPos);

		size_t nRead = ++nPos;
		for (; nPos < nEnd; ++nPos)
		{
			const char& c = strTag[nPos];

			if (c == '=')
			{
				std::basic_string<CharType> strName = strTag.substr(nRead, nPos - nRead);

				size_t nValueStart = strTag.find(cQuote, nPos);
				if (nValueStart == std::basic_string<CharType>::npos)break;
				++nValueStart;

				nPos = strTag.find(cQuote, nValueStart);
				if (nPos == std::basic_string<CharType>::npos)break;

				std::basic_string<CharType> strValue = strTag.substr(nValueStart, nPos - nValueStart);
				attributes.push_back({ strName, strValue });

				for (; nPos < nEnd && strTag[nPos] != CharType(' '); ++nPos);
				nRead = ++nPos;
			}
		}
	}
} /* namespace text_utility */

#endif // !TEXT_UTILITY_H_
