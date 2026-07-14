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

		for (size_t nPos = 0;;)
		{
			nPos = src.find(strOld, nPos);
			if (nPos == std::basic_string<CharType>::npos)break;
			src.replace(nPos, strOld.size(), strNew);
			nPos += strNew.size();
		}
	}
	template <typename CharType, size_t sizeOld, size_t sizeNew>
	void ReplaceAll(std::basic_string<CharType>& src, const CharType(&strOld)[sizeOld], const CharType(&strNew)[sizeNew])
	{
		const size_t lenOld = sizeOld - 1;
		const size_t lenNew = sizeNew - 1;

		if (lenOld == 0 || (lenOld == lenNew && std::char_traits<CharType>::compare(strOld, strNew, lenOld) == 0)) return;

		for (size_t nPos = 0;;)
		{
			nPos = src.find(strOld, nPos, lenOld);
			if (nPos == std::basic_string<CharType>::npos)break;
			src.replace(nPos, lenOld, strNew, lenNew);
			nPos += lenNew;
		}
	}

	template <typename CharType>
	void EliminateSingleCharacterInPlace(std::basic_string<CharType>& str, const CharType characterToBeEliminated)
	{
		size_t nWritten = 0;
		for (auto& c : str)
		{
			if (c == characterToBeEliminated)
			{
				continue;
			}

			str[nWritten++] = c;
		}

		str.resize(nWritten);
	}

	template <typename CharType>
	void EliminateTagInPlace(std::basic_string<CharType>& str)
	{
		size_t nWritten = 0;
		int iCount = 0;
		for (const auto& c : str)
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
				str[nWritten++] = c;
			}
		}

		str.resize(nWritten);
	}

	template <typename CharType>
	size_t UnescapeInPlace(CharType* pSrc)
	{
		CharType* pStart = pSrc;
		CharType* pDst = pSrc;
		while (*pSrc)
		{
			if (*pSrc == CharType('\\'))
			{
				pSrc++;
				if (!*pSrc) break;

				switch (*pSrc)
				{
				case CharType('b'):  *pDst++ = CharType('\b'); break;
				case CharType('f'):  *pDst++ = CharType('\f'); break;
				case CharType('n'):  *pDst++ = CharType('\n'); break;
				case CharType('r'):  *pDst++ = CharType('\r'); break;
				case CharType('t'):  *pDst++ = CharType('\t'); break;
				default: *pDst++ = *pSrc; break;
				}
			}
			else
			{
				*pDst++ = *pSrc;
			}
			pSrc++;
		}

		*pDst = CharType('\0');

		return pDst - pStart;
	}
	template <typename CharType>
	void UnescapeInPlace(std::basic_string<CharType>& src)
	{
		size_t unescapedLength = UnescapeInPlace(&src[0]);
		src.resize(unescapedLength);
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
