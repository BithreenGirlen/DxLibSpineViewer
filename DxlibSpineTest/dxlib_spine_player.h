﻿#ifndef DXLIB_SPINE_PLAYER_H_
#define DXLIB_SPINE_PLAYER_H_

#include <Windows.h>

#include <string>
#include <vector>
#include <memory>

#include "dxlib_spine.h"

class CDxLibSpinePlayer
{
public:
	CDxLibSpinePlayer();
	~CDxLibSpinePlayer();

	bool SetupDxLib(HWND hRenderWnd);

	bool SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary);
	bool SetSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary);

	void OnStyleChanged();

	void RescaleSkeleton(bool bUpscale);
	void RescaleTime(bool bHasten);
	void ResetScale();

	void MoveViewPoint(int iX, int iY);
	void ShiftAnimation();
	void ShiftSkin();

	void Redraw(float fDelta);

	void SwitchPma();
	void SwitchBlendModeAdoption();
	bool SwitchDepthBufferValidity();

	std::vector<std::string> GetSlotList();
	std::vector<std::string> GetSkinList() const;
	std::vector<std::string> GetAnimationList() const;

	void SetSlotsToExclude(const std::vector<std::string>& slotNames);
	void MixSkins(const std::vector<std::string>& skinNames);
	void MixAnimations(const std::vector<std::string>& animationNames);
private:
	HWND m_hRenderWnd = nullptr;
	int m_iDxLibInitialised = -1;

	enum Constants { kBaseWidth = 1280, kBaseHeight = 720, kMinAtlas = 1024, };

	CDxLibTextureLoader m_textureLoader;
	std::vector<std::unique_ptr<spine::Atlas>> m_atlases;
	std::vector<std::shared_ptr<spine::SkeletonData>> m_skeletonData;
	std::vector<std::shared_ptr<CDxLibSpineDrawer>> m_drawables;

	DxLib::FLOAT2 m_fBaseSize = DxLib::FLOAT2{ kBaseWidth, kBaseHeight };

	float m_fDefaultWindowScale = 1.f;
	DxLib::FLOAT2 m_fDefaultOffset{};

	float m_fTimeScale = 1.f;
	float m_fSkeletonScale = 1.f;
	DxLib::FLOAT2 m_fOffset{};

	std::vector<std::string> m_animationNames;
	size_t m_nAnimationIndex = 0;

	std::vector<std::string> m_skinNames;
	size_t m_nSkinIndex = 0;

	bool m_bDepthBufferEnabled = false;

	void ClearDrawables();
	bool SetupDrawer();
	void WorkOutDefaultScale();

	void UpdateScaletonScale();
	void UpdateTimeScale();

	void ClearAnimationTracks();

	void ResizeWindow();
	void ResizeBuffer();

	bool IsWidowBarHidden();
};
#endif // !DXLIB_SPINE_PLAYER_H_
