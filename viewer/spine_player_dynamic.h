#ifndef SPINE_PLAYER_DYNAMIC_H_
#define SPINE_PLAYER_DYNAMIC_H_

#include <memory>

#include "../runtime/dxlib_spine_dll.h"
#include "../runtime/spine_player_dll.h"

class DynamicSpinePlayerEntry;

class CSpinePlayerDynamic
{
public:
	CSpinePlayerDynamic();
	~CSpinePlayerDynamic();

	bool initialise();
	bool hasBeenInitialised() const noexcept;

	enum class ESpineVersionIndex : uint8_t
	{
		NotImplemented = static_cast<uint8_t>(-1U),
		Spine21 = 0,
		Spine35,
		Spine36,
		Spine37,
		Spine38,
		Spine40,
		Spine41,
		Spine42,
		kMax
	};
	long long findVersionIndex(const char* version) const noexcept;
	bool setPlayerToUse(size_t index) noexcept;
	uint8_t versionIndexInUse() const noexcept;

	ISpinePlayer* getByIndex(size_t index) const;
	ISpinePlayer* get() const;
private:
	static constexpr size_t NumOfVersions = 8;
	static_assert(NumOfVersions == static_cast<uint8_t>(ESpineVersionIndex::kMax), "Spine version count does not match enumeration.");

	std::unique_ptr<DynamicSpinePlayerEntry> m_spinePlayers[NumOfVersions];
	uint8_t m_nSpinePlayerIndexInUse = static_cast<uint8_t>(ESpineVersionIndex::Spine38);
	bool m_hasLoadedAllDlls = true;
};

#endif // !SPINE_PLAYER_DYNAMIC_H_
