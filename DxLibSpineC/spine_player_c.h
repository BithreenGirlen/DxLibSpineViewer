#ifndef SPINE_PLAYER_C_H_
#define SPINE_PLAYER_C_H_

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

/* DxLib::FLOAT2 is composed of {u,v} variables that it is not compatible with other rendering library. */
using FPoint2 = struct { float x, y; };
#ifdef SPINE_2_1
#include "../projects//DxLibSpineViewerC-2.1/dxlib_spine_c_21.h"
using CSpineDrawableC = CDxLibSpineDrawableC21;
#else
#include "dxlib_spine_c.h"
using CSpineDrawableC = CDxLibSpineDrawableC;
#endif


class CSpinePlayerC
{
public:
	CSpinePlayerC();
	virtual ~CSpinePlayerC();

	bool LoadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel);
	bool LoadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool isBinarySkel);

	bool AddSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool isBinarySkel);

	size_t GetNumberOfSpines() const noexcept;
	bool HasSpineBeenLoaded() const noexcept;

	void Update(float fDelta);

	void RescaleSkeleton(bool upscale);
	void RescaleCanvas(bool upscale);
	void RescaleTime(bool hasten);

	void ResetScale();

	void MoveViewPoint(int iX, int iY);

	void ShiftAnimation();
	void ShiftSkin();

	void SetAnimationByIndex(size_t nIndex);
	void SetAnimationByName(const char* szAnimationName);
	void RestartAnimation(bool loop = true);

	void SetSkinByIndex(size_t nIndex);
	void SetSkinByName(const char* szSkinName);
	void SetupSkin();

	/// @brief Toggle the state of all drawables
	void TogglePma();
	void ToggleBlendModeAdoption();

	/// @return current state. If it were out of range, return false.
	bool IsAlphaPremultiplied(size_t nDrawableIndex = 0);
	bool IsBlendModeNormalForced(size_t nDrawableIndex = 0);
	bool IsDrawOrderReversed() const noexcept;

	/// @return false if it were out of range.
	bool PremultiplyAlpha(bool isToBePremultiplied, size_t nDrawableIndex = 0);
	bool ForceBlendModeNormal(bool isToForce, size_t nDrawableIndex = 0);
	void SetDrawOrder(bool isToBeReversed);

	std::string GetCurrentAnimationName();
	/// @brief Get animation time actually entried in track.
	/// @param fTrack elapsed time since the track was entried.
	/// @param fLast current timeline position.
	/// @param fStart timeline start position.
	/// @param fEnd timeline end position.
	void GetCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd);

	const std::vector<std::string>& GetSlotNames() const noexcept;
	const std::vector<std::string>& GetSkinNames() const noexcept;
	const std::vector<std::string>& GetAnimationNames() const noexcept;

	void SetSlotsToExclude(const std::vector<std::string>& slotNames);
	void MixSkins(const std::vector<std::string>& skinNames);
	void MixAnimations(const std::vector<std::string>& animationNames, bool loop = false);

	void SetSlotExcludeCallback(bool (*pFunc)(const char*, size_t));

	/// @brief Searches slots having multiple attachments. If each slot is associated with only single attachment, returns empty.
	/// @return slot name as key and attachment names as values.
	std::unordered_map<std::string, std::vector<std::string>> GetSlotNamesWithTheirAttachments();
	bool ReplaceAttachment(const char* szSlotName, const char* szAttachmentName);

	FPoint2 GetBaseSize() const noexcept;
	void SetBaseSize(float fWidth, float fHeight);
	void ResetBaseSize();

	FPoint2 GetOffset() const noexcept;

	float GetSkeletonScale() const noexcept;
	void SetSkeletonScale(float fScale);

	float GetCanvasScale() const noexcept;
	void SetCanvasScale(float fScale);

	float GetTimeScale() const noexcept;
	void SetTimeScale(float fTimeScale);
protected:
	static constexpr float kfScaleFactor = 0.025f;
	static constexpr float kfMinScale = 0.15f;
	enum Constants { kBaseWidth = 1280, kBaseHeight = 720 };

	std::vector<std::shared_ptr<spAtlas>> m_atlases;
	std::vector<std::shared_ptr<spSkeletonData>> m_skeletonData;
	std::vector<std::shared_ptr<CSpineDrawableC>> m_drawables;
	FPoint2 m_fBaseSize = FPoint2{ kBaseWidth, kBaseHeight };

	float m_fDefaultScale = 1.f;
	FPoint2 m_fDefaultOffset{};

	float m_fTimeScale = 1.f;
	float m_fSkeletonScale = 1.f;
	float m_fCanvasScale = 1.f;
	FPoint2 m_fOffset{};

	std::vector<std::string> m_animationNames;
	size_t m_nAnimationIndex = 0;

	std::vector<std::string> m_skinNames;
	size_t m_nSkinIndex = 0;

	std::vector<std::string> m_slotNames;

	bool m_isDrawOrderReversed = false;

	void ClearDrawables();
	bool AddDrawable(spSkeletonData* const pSkeletonData);
	bool SetupDrawables();

	void WorkOutDefaultSize();
	virtual void WorkOutDefaultScale() = 0;
	virtual void WorkOutDefaultOffset() = 0;

	void UpdatePosition();

	void ClearAnimationTracks();
};

#endif // !SPINE_PLAYER_C_H_
