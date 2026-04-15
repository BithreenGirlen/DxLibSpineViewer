#ifndef JSON_MINIMAL_H_
#define JSON_MINIMAL_H_

/// @brief Json extracter
namespace json_minimal
{
	bool FindNextObject(const char** src, const char* name, const char** start, const char** end);
	bool FindNextArray(const char** src, const char* name, const char** start, const char** end);

	bool FindValueByName(const char* src, const char* name, const char** start, const char** end, int* iDepth = nullptr);
	bool FindArrayValueByIndices(const char* src, const size_t* indices, size_t indices_size, const char** start, const char** end);

	namespace util
	{
		bool ReadNextKeyInObject(const char** src, const char** keyStart, const char** keyEnd, const char** valueStart, const char** valueEnd);

		bool ReadNextValueInArray(const char** src, const char** start, const char** end);
	}
}

#endif //JSON_MINIMAL_H_