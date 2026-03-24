

#include "spine_file_verifier.h"

#if defined (ALLOW_CPP20_STL)
	#include <algorithm>
	#include <cctype>
#else 
	#include <ctype.h>
	#include <memory.h>
	#include <stdint.h>
#endif

namespace spine_file_verifier
{
   /*
	* バイナリ形式格納データ型式仕様
	* |  型式  |  長さ(Bytes)  |
	* | variant | 1- 5 |
	* | int | 4 |
	* | float | 4 |
	* | bool | 1 | 1がtrue, 0がfalse |
	* | string | variant + n | [Variant - 1]の値がバイナリ長を示し、UTF-8で表現された文字列が続く。空文字はVarinat = 1で表現。
	*
	* バイナリ形式ヘッダ仕様
	* |  型式  |  内容  |
	* | string | ハッシュ | // Spine 4.0, 4.1ではstringではなくint型2つから成る16進数表記
	* | string | 版 |
	* | float | 開始位置x座標 | // Spine 3.7以前には存在せず、この位置から幅データが始まる。
	* | float | 開始位置y座標 | // Spine 3.7以前には存在しない。
	* | float | 幅 |
	* | float | 高さ |
	* | bool | 任意データ有無 |
	* | string | 存在する場合、各種任意データ。 |
	*
	* ヘッダ直後にBoneの記述が続く。この並びはSpine 2.1より4.2に至るまで変更なし。
	*
	*/

#ifdef ALLOW_CPP20_STL
	/// @brief ハッシュ文字列か
	static bool IsHashString(std::string_view s)
	{
		return std::ranges::all_of(s, [](const char& c) {return std::isalnum(static_cast<const unsigned char>(c)) != 0 || c == '+' || c == '/'; });
	}

	/// @brief 版文字列か
	static bool IsVersionString(std::string_view s)
	{
		return std::ranges::all_of(s, [](const char& c) {return std::isdigit(static_cast<const unsigned char>(c)) != 0 || c == '.'; });
	}
#else
	/* To remove dependency on STL, this function plays the role of "std::search" */
	static const unsigned char* MemSearch(const unsigned char* first1, const unsigned char* end1, const unsigned char* first2, const unsigned char* end2)
	{
		const size_t length = end2 - first2;
		const unsigned char* end = end1 - length;
		for (const unsigned char* pos = first1; pos < end; ++pos)
		{
			if (::memcmp(pos, first2, length) == 0)
			{
				return pos;
			}
		}

		return end1;
	}

	static bool IsHashString(const unsigned char* s, size_t length)
	{
		for (size_t i = 0; i < length; ++i)
		{
			const auto& c = s[i];
			if (!(::isalnum(c) != 0 || c == '+' || c == '/'))
			{
				return false;
			}
		}

		return true;
	}

	static bool IsVersionString(const unsigned char* s, size_t length)
	{
		for (size_t i = 0; i < length; ++i)
		{
			const auto& c = s[i];
			if (!(::isdigit(c) != 0 || c == '.'))
			{
				return false;
			}
		}

		return true;
	}
#endif

	/// @brief JSON形式と思われるか
	static bool IsLikelyJsonSkeleton(const unsigned char* pData, size_t dataSize, SkeletonMetadata& skeletonMetaData)
	{
		/*
		* 空白文字なしで考えると、
		* ヘッダ開始 : '{' + "skeleton" + ':' + '{' = 12
		* ハッシュ組 : "hash" + ':' + '"' + 高々32バイト + '"' + ',' = 42
		* 版組 : "spine" + ':' + '"' + 高々6バイト + '"' + ',' = 17
		* 12 + 42 + 17 = 81を空白文字" \r\n\t"を考慮して凡そ2倍にしておく。
		* 実際にはBoneの記述が続くのでこれよりは長い筈。
		*/
		static constexpr size_t MinJsonSkeletonFileSize = 160;
		if (dataSize < MinJsonSkeletonFileSize)return false;

		static constexpr const unsigned char skeleton[] = R"("skeleton")";
		static constexpr const unsigned char versionKey[] = R"("spine")";

		const unsigned char* begin = pData;
		const unsigned char* end = begin + MinJsonSkeletonFileSize;
#ifdef ALLOW_CPP20_STL
		const unsigned char* iter = std::search(begin, end, skeleton, skeleton + sizeof(skeleton) - 1);
#else
		const unsigned char* iter = MemSearch(begin, end, skeleton, skeleton + sizeof(skeleton) - 1);
#endif
		if (iter == end)return false;
#ifdef ALLOW_CPP20_STL
		iter = std::search(begin, end, versionKey, versionKey + sizeof(versionKey) - 1);
#else
		iter = MemSearch(begin, end, versionKey, versionKey + sizeof(versionKey) - 1);
#endif
		if (iter == end)return false;

		iter += sizeof(versionKey) - 1;
		const unsigned char* versionStart = static_cast<const unsigned char*>(::memchr(iter, '"', end - iter));
		if (versionStart == nullptr)return false;
		++versionStart;
		const unsigned char* versionEnd = static_cast<const unsigned char*>(::memchr(versionStart, '"', end - versionStart));
		if (versionEnd == nullptr)return false;

#ifdef ALLOW_CPP20_STL
		skeletonMetaData.version = std::string_view(reinterpret_cast<const std::string_view::value_type*>(versionStart), versionEnd - versionStart);

		return IsVersionString(skeletonMetaData.version);
#else
		skeletonMetaData.version = versionStart;
		skeletonMetaData.versionLength = versionEnd - versionStart;

		return IsVersionString(skeletonMetaData.version, skeletonMetaData.versionLength);
#endif
	}

	/// @brief 16進数表記ハッシュと版文字列で始まるか
	static bool StartsWithHexHashPrecedingVersion(const unsigned char* pData, size_t dataSize, SkeletonMetadata& skeletonMetaData)
	{
		/* ハッシュ8バイト + 版文字列長さ1バイト + 版文字列6バイト(X.X.XX) */
		static constexpr const size_t HashLength = 8;
		static constexpr const size_t VersionEndPos = HashLength + 7;
		if (dataSize < VersionEndPos)return false;

		size_t pos = HashLength;
		uint8_t versionLength = pData[HashLength];
		if (versionLength < 1)return false;
		pos += sizeof(uint8_t);
		--versionLength;
		if (dataSize < pos + versionLength)return false;

#ifdef ALLOW_CPP20_STL
		skeletonMetaData.version = std::string_view(reinterpret_cast<const std::string_view::value_type*>(&pData[pos]), versionLength);

		return IsVersionString(skeletonMetaData.version);
#else
		skeletonMetaData.version = &pData[pos];
		skeletonMetaData.versionLength = versionLength;

		return IsVersionString(skeletonMetaData.version, skeletonMetaData.versionLength);
#endif
	}

	/// @brief 文字列表記のハッシュと版文字列で始まるか
	static bool StartsWithStringHashPrecedingVersion(const unsigned char* pData, size_t dataSize, SkeletonMetadata& skeletonMetaData)
	{
		/*
		* ハッシュは高々32バイト(上位最大16 + 下位最大16)
		* 版は高々6バイト(X.X.XX)
		* 従って 1 + 16 + 16 + 1 + 6 = 40が版までの最大長。
		* 実際にはBoneの記述が続くのでこれよりは長い筈。
		*/
		static constexpr const size_t MinBinarySkeletonFileSize = 40;
		if (dataSize < MinBinarySkeletonFileSize) return false;

		size_t pos = 0;
		{
			uint8_t hashLength = pData[pos];
			if (hashLength < 1)return false;
			pos += sizeof(uint8_t);
			--hashLength;
			if (dataSize < pos + hashLength)return false;

#ifdef ALLOW_CPP20_STL
			std::string_view hash(reinterpret_cast<const std::string_view::value_type*>(&pData[pos]), hashLength);

			if (!IsHashString(hash)) return false;
#else
			if (!IsHashString(&pData[pos], hashLength))return false;
#endif

			pos += hashLength;
		}

		uint8_t versionLength = pData[pos];
		pos += sizeof(uint8_t);
		--versionLength;
		if (dataSize < pos + versionLength)return false;

#ifdef ALLOW_CPP20_STL
		skeletonMetaData.version = std::string_view(reinterpret_cast<const std::string_view::value_type*>(&pData[pos]), versionLength);

		return IsVersionString(skeletonMetaData.version);
#else
		skeletonMetaData.version = &pData[pos];
		skeletonMetaData.versionLength = versionLength;

		return IsVersionString(skeletonMetaData.version, skeletonMetaData.versionLength);
#endif
	}


} /* namespace spine_file_verifier*/

spine_file_verifier::SkeletonMetadata spine_file_verifier::VerifySkeletonFileData(const unsigned char* pFileData, size_t dataLength)
{
	SkeletonMetadata skeletonMetaData;

	if (IsLikelyJsonSkeleton(pFileData, dataLength, skeletonMetaData))
	{
		skeletonMetaData.skeletonFormat = SkeletonFormat::Json;
	}
	else if (StartsWithHexHashPrecedingVersion(pFileData, dataLength, skeletonMetaData)) /* Spine 4.0と4.1のバイナリ形式 */
	{
		skeletonMetaData.skeletonFormat = SkeletonFormat::Binary;
	}
	else if (StartsWithStringHashPrecedingVersion(pFileData, dataLength, skeletonMetaData)) /* 上記以外の版のバイナリ形式 */
	{
		skeletonMetaData.skeletonFormat = SkeletonFormat::Binary;
	}

    return skeletonMetaData;
}
