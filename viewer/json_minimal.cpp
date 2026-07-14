/* Minimal JSON extractor */

#include <string.h>

#include "json_minimal.h"

namespace json_minimal
{
	static const char* SkipWhiteSpace(const char* src)
	{
		const char* p = src;

		static const char ref[] = " \t\r\n";
		return p + strspn(p, ref);
	}

	static const char* SkipString(const char* src)
	{
		const char* p = src;

		for (; *p != '\0';)
		{
			++p;
			switch (*p)
			{
			case '"':
				return p + 1;
			case '\\':
				++p;
				break;
			default:
				break;
			}
		}

		return p;
	}

	static const char* SkipPrimitive(const char* src)
	{
		static const char ref[] = " \t\r\n,]}";
		return strpbrk(src, ref);
	}

	static const char* SkipCollection(const char* src)
	{
		const char* p = src;

		if (*p != '{' && *p != '[') return p;
		const char cStart = *p;
		const char cEnd = cStart == '{' ? '}' : ']';

		int depth = 1;
		++p;

		while (*p != '\0')
		{
			if (*p == '"')
			{
				p = SkipString(p);
				continue;
			}

			if (*p == cStart)
			{
				++depth;
			}
			else if (*p == cEnd)
			{
				--depth;
				if (depth == 0)	return p + 1;
			}

			++p;
		}

		return nullptr;
	}

	static const char* SkipValue(const char* src)
	{
		const char* p = SkipWhiteSpace(src);
		if (*p == '"') return SkipString(p);
		else if (*p == '{' || *p == '[') return SkipCollection(p);

		return SkipPrimitive(p);
	}

	static const char* FindValueStart(const char* src, const char* name, const char cStart)
	{
		if (name != nullptr)
		{
			const char* p1 = strstr(src, name);
			if (p1 == nullptr) return nullptr;

			const char* p2 = strchr(p1, ':');
			if (p2 == nullptr) return nullptr;
			++p2;

			p1 = SkipWhiteSpace(p2);

			return p1;
		}
		else
		{
			return strchr(src, cStart);
		}
	}

	static bool ReadValueRange(const char* src, const char** start, const char** end)
	{
		const char* p = SkipWhiteSpace(src);

		if (*p == '"')
		{
			*start = p + 1;
			*end = SkipString(p) - 1;

			return true;
		}

		if (*src == '{' || *src == '[')
		{
			const char* q = SkipCollection(p);
			if (q == nullptr) return false;

			*start = src;
			*end = q;

			return true;
		}

		const char* q = SkipPrimitive(p);
		if (q == nullptr) q = strchr(src, '\0');

		*start = p;
		*end = q;

		return true;
	}

} /* namespace json_minimal */

bool json_minimal::FindNextObject(const char** src, const char* name, const char** start, const char** end)
{
	if (src == nullptr || start == nullptr || end == nullptr)return false;

	const char* p1 = FindValueStart(*src, name, '{');
	if (p1 == nullptr) return false;

	const char* p2 = SkipCollection(p1);
	if (p2 == nullptr) return false;

	*start = p1;
	*end = p2;
	*src = p2;

	return true;
}
bool json_minimal::FindNextArray(const char** src, const char* name, const char** start, const char** end)
{
	if (src == nullptr || start == nullptr || end == nullptr)return false;

	const char* p1 = FindValueStart(*src, name, '[');
	if (p1 == nullptr) return false;

	const char* p2 = SkipCollection(p1);
	if (p2 == nullptr) return false;

	*start = p1;
	*end = p2;
	*src = p2;

	return true;
}

bool json_minimal::FindValueByName(const char* src, const char* name, const char** start, const char** end, int* iDepth)
{
	if (src == nullptr || name == nullptr || start == nullptr || end == nullptr)return false;

	const char* p1 = strstr(src, name);
	if (p1 == nullptr) return false;

	const char* p2 = strchr(p1, ':');
	if (p2 == nullptr) return false;
	++p2;

	if (!ReadValueRange(p2, start, end))return false;

	if (iDepth != nullptr)
	{
		const char* q = nullptr;
		const char* qq = nullptr;
		const char* pRead = src;

		for (;;)
		{
			q = strchr(pRead, '}');
			if (q == nullptr)break;

			qq = strchr(pRead, '{');
			if (qq == nullptr)break;

			if (q < qq)
			{
				--(*iDepth);
				pRead = q + 1;
			}
			else
			{
				++(*iDepth);
				pRead = qq + 1;
			}

			if (pRead > *end)break;
		}
	}

	return true;
}
bool json_minimal::FindArrayValueByIndices(const char* src, const size_t* indices, size_t indices_size, const char** start, const char** end)
{
	if (src == nullptr || start == nullptr || end == nullptr)return false;

	const char* p1 = src;
	const char* p2 = nullptr;

	for (size_t i = 0; i < indices_size; ++i)
	{
		p2 = strchr(p1, '[');
		if (p2 == nullptr) return false;
		p1 = p2 + 1;

		for (size_t j = 0; j < indices[i]; ++j)
		{
			p2 = SkipValue(p1);
			if (p2 == nullptr) return false;

			p1 = SkipWhiteSpace(p2);
			if (*p1 == ',') ++p1;
		}
	}

	p2 = SkipValue(p1);
	if (p2 == nullptr) return false;

	*start = p1;
	*end = p2;

	return *start != *end;
}

bool json_minimal::util::ReadNextKeyInObject(const char** src, const char** keyStart, const char** keyEnd, const char** valueStart, const char** valueEnd)
{
	const char* p1 = SkipWhiteSpace(*src);

	if (*p1 == '{')
	{
		++p1;
		p1 = SkipWhiteSpace(p1);
	}
	if (*p1 == '\0' || *p1 == '}')
	{
		return false;
	}

	if (*p1 == '"')
	{
		const char* q = SkipString(p1);
		if (q == nullptr) return false;
		*keyStart = p1 + 1;
		*keyEnd = q - 1;
		p1 = q;
	}
	else
	{
		const char* q = strpbrk(p1, ":,} \t\r\n");
		if (q == nullptr) q = strchr(p1, '\0');
		*keyStart = p1;
		*keyEnd = q;
		p1 = q;
	}

	const char* p2 = strchr(p1, ':');
	if (p2 == nullptr) return false;
	++p2;

	if (!ReadValueRange(p2, valueStart, valueEnd)) return false;

	const char* pNext = *valueEnd;
	if (*pNext == '"') ++pNext;

	pNext = SkipWhiteSpace(pNext);
	if (*pNext == ',') ++pNext;

	*src = pNext;

	return true;
}

bool json_minimal::util::ReadNextValueInArray(const char** src, const char** start, const char** end)
{
	const char* p = SkipWhiteSpace(*src);

	if (*p == '[')
	{
		++p;
		p = SkipWhiteSpace(p);
	}
	if (*p == '\0' || *p == ']')
	{
		return false;
	}

	if (!ReadValueRange(p, start, end)) return false;

	const char* pNext = *end;
	if (*pNext == '"') ++pNext;

	pNext = SkipWhiteSpace(pNext);
	if (*pNext == ',') ++pNext;

	*src = pNext;

	return true;
}
