﻿#ifndef DXLIB_SPINE_PLAYER_C_H_
#define DXLIB_SPINE_PLAYER_C_H_

#include <Windows.h>

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

	void SetRenderWindow(HWND hRenderWnd);

	bool SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary);
	bool SetSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary);

	void Redraw(float fDelta);

	void OnStyleChanged();

	void RescaleSkeleton(bool bUpscale, bool bWindowToBeResized = true);
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

	std::vector<std::string> GetSlotNames();
	std::vector<std::string> GetSkinNames() const;
	std::vector<std::string> GetAnimationNames() const;

	void SetSlotsToExclude(const std::vector<std::string>& slotNames);
	void MixSkins(const std::vector<std::string>& skinNames);
	void MixAnimations(const std::vector<std::string>& animationNames);

	std::unordered_map<std::string, std::vector<std::string>> GetSlotNamesWithTheirAttachments();
	bool ReplaceAttachment(const char* szSlotName, const char* szAttachmentName);
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
	void UpdateAnimation();

	void ClearAnimationTracks();

	void ResizeWindow();
	void ResizeBuffer();
};
#endif // !DXLIB_SPINE_PLAYER_C_H_
