#ifndef DXLIB_SPINE_PLAYER_C_H_
#define DXLIB_SPINE_PLAYER_C_H_

#include <Windows.h>

#include <string>
#include <vector>
#include <memory>

#include "dxlib_spine_c.h"

class CDxLibSpinePlayerC
{
public:
	CDxLibSpinePlayerC();
	~CDxLibSpinePlayerC();

	void SetRenderWindow(HWND hRenderWnd);

	bool SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary);
	bool SetSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary);

	void Redraw(float fDelta);

	void OnStyleChanged();

	void RescaleSkeleton(bool bUpscale);
	void RescaleTime(bool bHasten);
	void ResetScale();

	void MoveViewPoint(int iX, int iY);
	void ShiftAnimation();
	void ShiftSkin();

	void SwitchPma();
	void SwitchBlendModeAdoption();
	bool SwitchDepthBufferValidity();
	void SwitchDrawOrder();

	std::string GetCurrentAnimationNameWithTrackTime(float *fTrackTime = nullptr);

	std::vector<std::string> GetSlotList();
	std::vector<std::string> GetSkinList() const;
	std::vector<std::string> GetAnimationList() const;

	void SetSlotsToExclude(const std::vector<std::string>& slotNames);
	void MixSkins(const std::vector<std::string>& skinNames);
	void MixAnimations(const std::vector<std::string>& animationNames);
private:
	HWND m_hRenderWnd = nullptr;

	enum Constants { kBaseWidth = 1280, kBaseHeight = 720 };

	std::vector<std::shared_ptr<spAtlas>> m_atlases;
	std::vector<std::shared_ptr<spSkeletonData>> m_skeletonData;
	std::vector<std::shared_ptr<CDxLibSpineDrawerC>> m_drawables;

	DxLib::FLOAT2 m_fBaseSize = DxLib::FLOAT2{ kBaseWidth, kBaseHeight };

	float m_fDefaultScale = 1.f;
	DxLib::FLOAT2 m_fDefaultOffset{};

	float m_fTimeScale = 1.f;
	float m_fSkeletonScale = 1.f;
	DxLib::FLOAT2 m_fOffset{};
	DxLib::FLOAT2 m_fViewOffset{};

	std::vector<std::string> m_animationNames;
	size_t m_nAnimationIndex = 0;

	std::vector<std::string> m_skinNames;
	size_t m_nSkinIndex = 0;

	bool m_bDepthBufferEnabled = false;
	bool m_bDrawOrderReversed = false;

	void ClearDrawables();
	bool SetupDrawer();

	void WorkOutDefaultSize();
	void WorkOutDefaultScale();
	void AdjustViewOffset();

	void UpdatePosition();
	void UpdateScaletonScale();
	void UpdateTimeScale();

	void ClearAnimationTracks();

	void ResizeWindow();
	void ResizeBuffer();
};
#endif // !DXLIB_SPINE_PLAYER_C_H_
