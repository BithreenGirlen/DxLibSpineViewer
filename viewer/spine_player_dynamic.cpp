
#include "spine_player_dynamic.h"

class DynamicSpinePlayerEntry
{
public:
	DynamicSpinePlayerEntry() = default;
	~DynamicSpinePlayerEntry()
	{
		destroy();
	}

	bool initialise(const wchar_t* dllFileName, const char* createFuncName, const char* destroyFuncName);
	bool hasLoadedLibrary() const
	{
		return m_spinePlayerDll.get() != nullptr && m_pSpinePlayer != nullptr;
	}

	ISpinePlayer* getSpinePlayer() const { return m_pSpinePlayer; }
private:
	using UniqueModule = std::unique_ptr<std::remove_pointer_t<HMODULE>, decltype(&::FreeLibrary)>;
	using CreatePlayerFunc = ISpinePlayer * (*)();
	using DestroyPlayerFunc = void(*)(ISpinePlayer*);

	/* Not inclined to introduce WIL only to manage DLLs. */
	UniqueModule m_spinePlayerDll{ nullptr, ::FreeLibrary };
	DestroyPlayerFunc m_pDestroySpinePlayerFunc = nullptr;
	ISpinePlayer* m_pSpinePlayer = nullptr;

	void destroy()
	{
		if (m_pSpinePlayer != nullptr && m_pDestroySpinePlayerFunc != nullptr)
		{
			m_pDestroySpinePlayerFunc(m_pSpinePlayer);
			m_pSpinePlayer = nullptr;
			m_pDestroySpinePlayerFunc = nullptr;
		}
	}
};

bool DynamicSpinePlayerEntry::initialise(const wchar_t* dllFileName, const char* createFuncName, const char* destroyFuncName)
{
	using pRegisterFunc = void(*)(const DxLibRegerenda* pDxLibRegerenda);

	m_spinePlayerDll.reset(::LoadLibraryW(dllFileName));
	if (m_spinePlayerDll.get() == nullptr)return false;

	pRegisterFunc pRegisterDxLibFunctions = reinterpret_cast<pRegisterFunc>(::GetProcAddress(m_spinePlayerDll.get(), "RegisterDxLibFunctions"));
	if (pRegisterDxLibFunctions == nullptr)return false;
	pRegisterDxLibFunctions(GetDxLibFunctonsToBeRegistered());

	CreatePlayerFunc pCreateSpinePlayer = reinterpret_cast<CreatePlayerFunc>(::GetProcAddress(m_spinePlayerDll.get(), createFuncName));
	if (pCreateSpinePlayer == nullptr)return false;

	m_pDestroySpinePlayerFunc = reinterpret_cast<DestroyPlayerFunc>(::GetProcAddress(m_spinePlayerDll.get(), destroyFuncName));
	if (m_pDestroySpinePlayerFunc == nullptr)return false;

	m_pSpinePlayer = pCreateSpinePlayer();

	return m_pSpinePlayer != nullptr;
}

CSpinePlayerDynamic::CSpinePlayerDynamic()
{
	initialise();
}

CSpinePlayerDynamic::~CSpinePlayerDynamic()
{

}

bool CSpinePlayerDynamic::initialise()
{
	struct SpineRuntimeDllExport
	{
		const wchar_t* filename;
		const char* createFuncName;
		const char* destroyFuncName;
	};

	/* Prefer static strings to dynamic formatting. */
	static constexpr const SpineRuntimeDllExport spineRuntimeDllExports[NumOfVersions]
	{
		{L"DxLibSpinePlayerC21.dll", "CreateSpinePlayer21", "DestroySpinePlayer21"},
		{L"DxLibSpinePlayerC35.dll", "CreateSpinePlayer35", "DestroySpinePlayer35"},
		{L"DxLibSpinePlayerC36.dll", "CreateSpinePlayer36", "DestroySpinePlayer36"},
		{L"DxLibSpinePlayerC37.dll", "CreateSpinePlayer37", "DestroySpinePlayer37"},
		{L"DxLibSpinePlayer38.dll", "CreateSpinePlayer38", "DestroySpinePlayer38"},
		{L"DxLibSpinePlayer40.dll", "CreateSpinePlayer40", "DestroySpinePlayer40"},
		{L"DxLibSpinePlayer41.dll", "CreateSpinePlayer41", "DestroySpinePlayer41"},
		{L"DxLibSpinePlayer42.dll", "CreateSpinePlayer42", "DestroySpinePlayer42"}
	};

	m_hasLoadedAllDlls = true;
	for (size_t i = 0; i < NumOfVersions; ++i)
	{
		const auto& spineRuntimeDllExport = spineRuntimeDllExports[i];
		auto& spinePlayer = m_spinePlayers[i];
		spinePlayer = std::make_unique<DynamicSpinePlayerEntry>();
		m_hasLoadedAllDlls &= spinePlayer->initialise(spineRuntimeDllExport.filename, spineRuntimeDllExport.createFuncName, spineRuntimeDllExport.destroyFuncName);
	}

	return m_hasLoadedAllDlls;
}

bool CSpinePlayerDynamic::hasBeenInitialised() const noexcept
{
	return m_hasLoadedAllDlls;
}

long long CSpinePlayerDynamic::findVersionIndex(const char* version) const noexcept
{
	static constexpr size_t majorVersionLength = 3;
	static constexpr const char majorVersions[][majorVersionLength + 1] =
	{
		"2.1", "3.5", "3.6", "3.7", "3.8", "4.0", "4.1", "4.2"
	};
	static constexpr long long versionCount = sizeof(majorVersions) / sizeof(majorVersions[0]);

	for (long long i = 0; i < versionCount; ++i)
	{
		const auto& majorVersion = majorVersions[i];
		if (strncmp(version, majorVersion, majorVersionLength) == 0)
		{
			return i;
		}
	}

	return -1;
}

bool CSpinePlayerDynamic::setPlayerToUse(size_t index) noexcept
{
	if (index >= NumOfVersions)return false;

	m_nSpinePlayerIndexInUse = index;

	return true;
}

uint8_t CSpinePlayerDynamic::versionIndexInUse() const noexcept
{
	return m_nSpinePlayerIndexInUse;
}

ISpinePlayer* CSpinePlayerDynamic::getByIndex(size_t index) const
{
	if (index < NumOfVersions)
	{
		return m_spinePlayers[index]->getSpinePlayer();
	}

	return nullptr;
}

ISpinePlayer* CSpinePlayerDynamic::get() const
{
	return m_spinePlayers[m_nSpinePlayerIndexInUse]->getSpinePlayer();
}
