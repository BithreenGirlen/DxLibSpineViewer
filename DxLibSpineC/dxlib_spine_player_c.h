#ifndef DXLIB_SPINE_PLAYER_C_H_
#define DXLIB_SPINE_PLAYER_C_H_

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#ifdef SPINE_2_1
#include "../projects//DxLibSpineViewerC-2.1/dxlib_spine_c_21.h"
using CDxLibSpineDrawerC = CDxLibSpineDrawerC21;
#else
#include "dxlib_spine_c.h"
#endif

class CDxLibSpinePlayerC
{
public:
	CDxLibSpinePlayerC();
	~CDxLibSpinePlayerC();

	bool LoadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary);
	bool LoadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary);

	bool AddSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool bBinary);

	size_t GetNumberOfSpines() const;
	bool HasLoaded() const;

	void Update(float fDelta);
	void Redraw();

	void RescaleSkeleton(bool bUpscale);
	void RescaleCanvas(bool bUpscale);
	void RescaleTime(bool bHasten);

	void ResetScale();

	void MoveViewPoint(int iX, int iY);

	void ShiftAnimation();
	void ShiftSkin();

	void SetAnimationByIndex(size_t nIndex);
	void RestartAnimation();

	void TogglePma();
	void ToggleBlendModeAdoption();
	void ToggleDrawOrder();

	std::string GetCurrentAnimationName();
	/// @brief Get time of animation actually entried in track.
	/// @param fTrack elapsed time since the track was entried.
	/// @param fLast current timeline position.
	/// @param fStart timeline start position.
	/// @param fEnd timeline end position.
	void GetCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd);

	std::vector<std::string> GetSlotNames();
	const std::vector<std::string>& GetSkinNames() const;
	const std::vector<std::string>& GetAnimationNames() const;

	void SetSlotsToExclude(const std::vector<std::string>& slotNames);
	void MixSkins(const std::vector<std::string>& skinNames);
	void MixAnimations(const std::vector<std::string>& animationNames);

	std::unordered_map<std::string, std::vector<std::string>> GetSlotNamesWithTheirAttachments();
	bool ReplaceAttachment(const char* szSlotName, const char* szAttachmentName);

	void GetBaseSize(float* fWidth, float* fHeight) const;
	float GetCanvasScale() const;
private:
	static constexpr float kfScaleFactor = 0.025f;
	static constexpr float kfMinScale = 0.15f;
	enum Constants { kBaseWidth = 1280, kBaseHeight = 720 };

	std::vector<std::shared_ptr<spAtlas>> m_atlases;
	std::vector<std::shared_ptr<spSkeletonData>> m_skeletonData;
	std::vector<std::shared_ptr<CDxLibSpineDrawerC>> m_drawables;

	DxLib::FLOAT2 m_fBaseSize = DxLib::FLOAT2{ kBaseWidth, kBaseHeight };

	float m_fDefaultScale = 1.f;
	DxLib::FLOAT2 m_fDefaultOffset{};

	float m_fTimeScale = 1.f;
	float m_fSkeletonScale = 1.f;
	float m_fCanvasScale = 1.f;
	DxLib::FLOAT2 m_fOffset{};

	std::vector<std::string> m_animationNames;
	size_t m_nAnimationIndex = 0;

	std::vector<std::string> m_skinNames;
	size_t m_nSkinIndex = 0;

	bool m_bDrawOrderReversed = false;

	void ClearDrawables();
	bool AddDrawable(spSkeletonData* const pSkeletonData);
	bool SetupDrawer();

	void WorkOutDefaultSize();
	void WorkOutDefaultScale();

	void UpdatePosition();
	void UpdateTimeScale();

	void ClearAnimationTracks();

	void SetTransformMatrix() const;
};
#endif // !DXLIB_SPINE_PLAYER_C_H_
