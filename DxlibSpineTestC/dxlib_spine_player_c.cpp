

#include "dxlib_spine_player_c.h"
#include "spine_loader_c.h"

CDxLibSpinePlayerC::CDxLibSpinePlayerC()
{

}

CDxLibSpinePlayerC::~CDxLibSpinePlayerC()
{

}

void CDxLibSpinePlayerC::SetRenderWindow(HWND hRenderWnd)
{
	m_hRenderWnd = hRenderWnd;
}
/*ファイル取り込み*/
bool CDxLibSpinePlayerC::SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary)
{
	if (atlasPaths.size() != skelPaths.size())return false;
	ClearDrawables();

	for (size_t i = 0; i < atlasPaths.size(); ++i)
	{
		const std::string& strAtlasPath = atlasPaths.at(i);
		const std::string& strSkeletonPath = skelPaths.at(i);

		std::shared_ptr<spAtlas> atlas = spine_loader_c::CreateAtlasFromFile(strAtlasPath.c_str(), nullptr);
		if (atlas.get() == nullptr)continue;

		std::shared_ptr<spSkeletonData> skeletonData = bIsBinary ?
			spine_loader_c::ReadBinarySkeletonFromFile(strSkeletonPath.c_str(), atlas.get()) :
			spine_loader_c::ReadTextSkeletonFromFile(strSkeletonPath.c_str(), atlas.get());
		if (skeletonData.get() == nullptr)continue;

		m_atlases.push_back(atlas);
		m_skeletonData.push_back(skeletonData);
	}

	WorkOutDefaultSize();
	WorkOutDefaultScale();

	return SetupDrawer();
}
/*メモリ取り込み*/
bool CDxLibSpinePlayerC::SetSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary)
{
	if (atlasData.size() != skelData.size() || atlasData.size() != atlasPaths.size())return false;
	ClearDrawables();

	for (size_t i = 0; i < atlasData.size(); ++i)
	{
		const std::string& strAtlasDatum = atlasData.at(i);
		const std::string& strAtlasPath = atlasPaths.at(i);
		const std::string& strSkeletonData = skelData.at(i);

		std::shared_ptr<spAtlas> atlas = spine_loader_c::CreateAtlasFromMemory(strAtlasDatum.c_str(), static_cast<int>(strAtlasDatum.size()), strAtlasPath.c_str(), nullptr);
		if (atlas.get() == nullptr)continue;

		std::shared_ptr<spSkeletonData> skeletonData = bIsBinary ?
			spine_loader_c::ReadBinarySkeletonFromMemory(reinterpret_cast<const unsigned char*>((strSkeletonData.c_str())), static_cast<int>(strSkeletonData.size()), atlas.get()) :
			spine_loader_c::ReadTextSkeletonFromMemory(strSkeletonData.c_str(), atlas.get());
		if (skeletonData.get() == nullptr)continue;

		m_atlases.push_back(atlas);
		m_skeletonData.push_back(skeletonData);
	}

	if (m_skeletonData.empty())return false;

	WorkOutDefaultSize();
	WorkOutDefaultScale();

	return SetupDrawer();
}
/*再描画*/
void CDxLibSpinePlayerC::Redraw(float fDelta)
{
	if (!m_drawables.empty())
	{
		DxLib::ClearDrawScreen();

		if (!m_bDrawOrderReversed)
		{
			for (size_t i = 0; i < m_drawables.size(); ++i)
			{
				m_drawables.at(i).get()->Update(fDelta);
#ifdef SPINE_3_7_OR_LATER
				m_drawables.at(i).get()->Draw(m_bDepthBufferEnabled ? 0.1f * (i + 1) : 0.f);
#else 
				/*
				* Implementation of scaling by multiplying factor and vertice.
				* A method suggested on official forum thread 4918.
				*/
				m_drawables.at(i).get()->Draw(m_bDepthBufferEnabled ? 0.1f * (i + 1) : 0.f, m_fSkeletonScale);
#endif
			}
		}
		else
		{
			for (long long i = m_drawables.size() - 1; i >= 0; --i)
			{
				m_drawables.at(i).get()->Update(fDelta);
#ifdef SPINE_3_7_OR_LATER
				m_drawables.at(i).get()->Draw(m_bDepthBufferEnabled ? 0.1f * (i + 1) : 0.f);
#else 
				m_drawables.at(i).get()->Draw(m_bDepthBufferEnabled ? 0.1f * (i + 1) : 0.f, m_fSkeletonScale);
#endif
			}
		}

		DxLib::ScreenFlip();

		if (m_hRenderWnd != nullptr)
		{
			::InvalidateRect(m_hRenderWnd, nullptr, FALSE);
		}
	}
}
/*表示形式変更通知*/
void CDxLibSpinePlayerC::OnStyleChanged()
{
	ResizeWindow();
}
/*拡縮変更*/
void CDxLibSpinePlayerC::RescaleSkeleton(bool bUpscale)
{
	constexpr float kfScalePortion = 0.025f;
	constexpr float kfMinScale = 0.15f;
	if (bUpscale)
	{
		m_fSkeletonScale += kfScalePortion;
	}
	else
	{
		m_fSkeletonScale -= kfScalePortion;
		if (m_fSkeletonScale < kfMinScale)m_fSkeletonScale = kfMinScale;
	}
#ifdef SPINE_3_7_OR_LATER
	AdjustViewOffset();
#endif
	UpdateScaletonScale();
	ResizeWindow();
}
/*時間尺度変更*/
void CDxLibSpinePlayerC::RescaleTime(bool bHasten)
{
	constexpr float kfTimeScalePortion = 0.05f;
	if (bHasten)
	{
		m_fTimeScale += kfTimeScalePortion;
	}
	else
	{
		m_fTimeScale -= kfTimeScalePortion;
	}
	if (m_fTimeScale < 0.f)m_fTimeScale = 0.f;

	UpdateTimeScale();
}

void CDxLibSpinePlayerC::ResetScale()
{
	m_fTimeScale = 1.0f;
	m_fSkeletonScale = m_fDefaultScale;
	m_fOffset = m_fDefaultOffset;

	UpdateScaletonScale();
	UpdateTimeScale();
	ResizeWindow();
	AdjustViewOffset();
}
/*視点移動*/
void CDxLibSpinePlayerC::MoveViewPoint(int iX, int iY)
{
	m_fOffset.u += iX;
	m_fOffset.v += iY;
	UpdatePosition();
}
/*動作移行*/
void CDxLibSpinePlayerC::ShiftAnimation()
{
	if (m_animationNames.empty())return;

	++m_nAnimationIndex;
	if (m_nAnimationIndex > m_animationNames.size() - 1)m_nAnimationIndex = 0;

	ClearAnimationTracks();

	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		const std::string& animationName = m_animationNames.at(m_nAnimationIndex);
		spAnimation *pAnimation = spSkeletonData_findAnimation(m_skeletonData.at(i).get(), animationName.c_str());
		if (pAnimation != nullptr)
		{
			spAnimationState_setAnimationByName(m_drawables.at(i)->animationState, 0, pAnimation->name, 1);
		}
	}
}
/*装い移行*/
void CDxLibSpinePlayerC::ShiftSkin()
{
	if (m_skinNames.empty())return;

	++m_nSkinIndex;
	if (m_nSkinIndex > m_skinNames.size() - 1)m_nSkinIndex = 0;

	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		const std::string& skinName = m_skinNames.at(m_nSkinIndex);
		spSkin* pSkin = spSkeletonData_findSkin(m_skeletonData.at(i).get(), skinName.c_str());
		if (pSkin != nullptr)
		{
			spSkeleton_setSkin(m_drawables.at(i)->skeleton, pSkin);
			spSkeleton_setToSetupPose(m_drawables.at(i)->skeleton);
		}
	}
}
/*乗算済み透過度有効・無効切り替え*/
void CDxLibSpinePlayerC::SwitchPma()
{
	for (const auto& drawable : m_drawables)
	{
		drawable->SwitchPma();
	}
}
/*槽溝指定合成方法採択可否*/
void CDxLibSpinePlayerC::SwitchBlendModeAdoption()
{
	for (const auto& drawable : m_drawables)
	{
		drawable->SwitchBlendModeAdoption();
	}
}
/*奥行き表現有効無効切り替え*/
bool CDxLibSpinePlayerC::SwitchDepthBufferValidity()
{
	int iRet = DxLib::SetUseZBufferFlag(m_bDepthBufferEnabled ? FALSE : TRUE);
	if (iRet == -1)return false;

	iRet = DxLib::SetWriteZBufferFlag(m_bDepthBufferEnabled ? FALSE : TRUE);
	if (iRet == -1)return false;

	m_bDepthBufferEnabled ^= true;
	return true;
}
/*描画順切り替え*/
void CDxLibSpinePlayerC::SwitchDrawOrder()
{
	m_bDrawOrderReversed ^= true;
}
/*現在の動作名と経過時間取得*/
std::string CDxLibSpinePlayerC::GetCurrentAnimationNameWithTrackTime(float* fTrackTime)
{
	for (const auto& pDrawable : m_drawables)
	{
		for (size_t i = 0; i < pDrawable->animationState->tracksCount; ++i)
		{
			spTrackEntry* pTrackEntry = pDrawable->animationState->tracks[i];
			if (pTrackEntry != nullptr)
			{
				spAnimation* pAnimation = pTrackEntry->animation;
				if (pAnimation != nullptr && pAnimation->name != nullptr)
				{
					if (fTrackTime != nullptr)
					{
						*fTrackTime = pTrackEntry->trackTime;
					}
					return pAnimation->name;
				}
			}
		}
	}

	return std::string();
}
/*槽溝名称引き渡し*/
std::vector<std::string> CDxLibSpinePlayerC::GetSlotList()
{
	std::vector<std::string> slotNames;
	for (const auto& skeletonData: m_skeletonData)
	{
		for (size_t i = 0; i < skeletonData->slotsCount; ++i)
		{
			const auto iter = std::find(slotNames.begin(), slotNames.end(), skeletonData->slots[i]->name);
			if (iter == slotNames.cend())slotNames.push_back(skeletonData->slots[i]->name);
		}
	}

	return slotNames;
}
/*装い名称引き渡し*/
std::vector<std::string> CDxLibSpinePlayerC::GetSkinList() const
{
	return m_skinNames;
}
/*動作名称引き渡し*/
std::vector<std::string> CDxLibSpinePlayerC::GetAnimationList() const
{
	return m_animationNames;
}
/*描画除外リスト設定*/
void CDxLibSpinePlayerC::SetSlotsToExclude(const std::vector<std::string>& slotNames)
{
	std::vector<const char*> vBuffer;
	vBuffer.resize(slotNames.size());
	for (size_t i = 0; i < slotNames.size(); ++i)
	{
		vBuffer[i] = slotNames[i].data();
	}
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->SetLeaveOutList(vBuffer.data(), static_cast<int>(vBuffer.size()));
	}
}
/*装い合成*/
void CDxLibSpinePlayerC::MixSkins(const std::vector<std::string>& skinNames)
{
	/*spine-c 3.6 does not have spSkin_addSkin(). It was added since spine-c 3.8*/
#ifdef SPINE_3_8_OR_LATER
	if (m_nSkinIndex >= m_skinNames.size())return;
	const auto& currentSkinName = m_skinNames.at(m_nSkinIndex);

	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		spSkin *skinToSet = spSkeletonData_findSkin(m_skeletonData.at(i).get(), currentSkinName.c_str());
		if (skinToSet != nullptr)
		{
			for (const auto& skinName : skinNames)
			{
				if (currentSkinName != skinName)
				{
					spSkin* skinToAdd = spSkeletonData_findSkin(m_skeletonData.at(i).get(), skinName.c_str());
					if (skinToAdd != nullptr)
					{
						spSkin_addSkin(skinToSet, skinToAdd);
					}
				}
			}
			spSkeleton_setSkin(m_drawables.at(i)->skeleton, skinToSet);
			spSkeleton_setToSetupPose(m_drawables.at(i)->skeleton);
		}
	}
#endif
}
/*動作合成*/
void CDxLibSpinePlayerC::MixAnimations(const std::vector<std::string>& animationNames)
{
	ClearAnimationTracks();

	if (m_nAnimationIndex >= m_animationNames.size())return;
	const auto& currentAnimationName = m_animationNames.at(m_nAnimationIndex);

	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		spAnimation* pCurrentAnimation = spSkeletonData_findAnimation(m_skeletonData.at(i).get(), currentAnimationName.c_str());
		if (pCurrentAnimation == nullptr)
		{
			continue;
		}

		int iTrack = 1;
		for (const auto& animationName : animationNames)
		{
			if (animationName != currentAnimationName)
			{
				spAnimation *pAnimationToAdd = spSkeletonData_findAnimation(m_skeletonData.at(i).get(), animationName.c_str());
				if (pAnimationToAdd != nullptr)
				{
					spAnimationState_addAnimation(m_drawables.at(i)->animationState, iTrack, pAnimationToAdd, false, 0.f);
					++iTrack;
				}
			}
		}
	}
}
/*消去*/
void CDxLibSpinePlayerC::ClearDrawables()
{
	m_drawables.clear();
	m_atlases.clear();
	m_skeletonData.clear();

	m_animationNames.clear();
	m_nAnimationIndex = 0;

	m_skinNames.clear();
	m_nSkinIndex = 0;
}
/*描画器設定*/
bool CDxLibSpinePlayerC::SetupDrawer()
{
	for (const auto& pSkeletonData : m_skeletonData)
	{
		const auto pDrawable = std::make_shared<CDxLibSpineDrawerC>(pSkeletonData.get());
		if (pDrawable.get() == nullptr)continue;

		pDrawable->timeScale = 1.0f;
		pDrawable->skeleton->x = m_fBaseSize.u / 2;
		pDrawable->skeleton->y = m_fBaseSize.v / 2;
		spSkeleton_setToSetupPose(pDrawable->skeleton);
		spSkeleton_updateWorldTransform(pDrawable->skeleton);

		m_drawables.push_back(pDrawable);

		for (size_t i = 0; i < pSkeletonData->animationsCount; ++i)
		{
			const std::string &strAnimationName = pSkeletonData->animations[i]->name;
			auto iter = std::find(m_animationNames.begin(), m_animationNames.end(), strAnimationName);
			if (iter == m_animationNames.cend())m_animationNames.push_back(strAnimationName);
		}

		for (size_t i = 0; i < pSkeletonData->skinsCount; ++i)
		{
			const std::string& strSkinName = pSkeletonData->skins[i]->name;
			auto iter = std::find(m_skinNames.begin(), m_skinNames.end(), strSkinName);
			if (iter == m_skinNames.cend())m_skinNames.push_back(strSkinName);
		}
	}

	if (!m_animationNames.empty())
	{
		for (size_t i = 0; i < m_skeletonData.size(); ++i)
		{
			spAnimationState_setAnimationByName(m_drawables.at(i)->animationState, 0, m_animationNames.at(0).c_str(), true);
		}
	}

	ResetScale();

	return m_animationNames.size() > 0;
}
/*標準寸法算出*/
void CDxLibSpinePlayerC::WorkOutDefaultSize()
{
	if (m_skeletonData.empty())return;

	float fMaxSize = 0.f;
	const auto CompareDimention = [this, &fMaxSize](float fWidth, float fHeight)
		-> bool
		{
			if (fWidth > 0.f && fHeight > 0.f && fWidth * fHeight > fMaxSize)
			{
				m_fBaseSize.u = fWidth;
				m_fBaseSize.v = fHeight;
				fMaxSize = fWidth * fHeight;
				return true;
			}

			return false;
		};

	for (const auto& pSkeletonData : m_skeletonData)
	{
		if (pSkeletonData->defaultSkin == nullptr)continue;

		const char* attachmentName = spSkin_getAttachmentName(pSkeletonData->defaultSkin, 0, 0);
		if (attachmentName == nullptr)continue;

		spAttachment* pAttachment = spSkin_getAttachment(pSkeletonData->defaultSkin, 0, attachmentName);
		if (pAttachment == nullptr)continue;

		if (pAttachment->type == SP_ATTACHMENT_REGION)
		{
			spRegionAttachment* pRegionAttachment = (spRegionAttachment*)pAttachment;

			CompareDimention(pRegionAttachment->width * pRegionAttachment->scaleX, pRegionAttachment->height * pRegionAttachment->scaleY);
		}
		else if (pAttachment->type == SP_ATTACHMENT_MESH)
		{
			spMeshAttachment* pMeshAttachment = (spMeshAttachment*)pAttachment;

			spSlotData* pSlotData = spSkeletonData_findSlot(pSkeletonData.get(), attachmentName);

			float fScaleX = pSlotData != nullptr ? pSlotData->boneData->scaleX : 1.f;
			float fScaleY = pSlotData != nullptr ? pSlotData->boneData->scaleY : 1.f;

			CompareDimention(pMeshAttachment->width * fScaleX, pMeshAttachment->height * fScaleY);
		}
	}

	for (const auto& pSkeletonData : m_skeletonData)
	{
		CompareDimention(pSkeletonData->width, pSkeletonData->height);
	}
}
/*標準尺度算出*/
void CDxLibSpinePlayerC::WorkOutDefaultScale()
{
	m_fDefaultScale = 1.f;
	m_fDefaultOffset = DxLib::FLOAT2{};

	int iSkeletonWidth = static_cast<int>(m_fBaseSize.u);
	int iSkeletonHeight = static_cast<int>(m_fBaseSize.v);

	int iDesktopWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int iDesktopHeight = ::GetSystemMetrics(SM_CYSCREEN);

	if (iSkeletonWidth > iDesktopWidth || iSkeletonHeight > iDesktopHeight)
	{
		float fScaleX = static_cast<float>(iDesktopWidth) / iSkeletonWidth;
		if (fScaleX < 0.49f)fScaleX = 0.5f;
		float fScaleY = static_cast<float>(iDesktopHeight) / iSkeletonHeight;
		if (fScaleY < 0.49f)fScaleY = 0.5f;

		if (iDesktopWidth > iDesktopHeight)
		{
			m_fDefaultScale = fScaleY;
		}
		else
		{
			m_fDefaultScale = fScaleX;
		}

		m_fDefaultOffset.u = iSkeletonWidth > iDesktopWidth ? (iSkeletonWidth - iDesktopWidth) * fScaleX : 0.f;
		m_fDefaultOffset.v = iSkeletonHeight > iDesktopHeight ? (iSkeletonHeight - iDesktopHeight) * fScaleY : 0.f;
	}
}
/*視点補正*/
void CDxLibSpinePlayerC::AdjustViewOffset()
{
#ifdef SPINE_3_7_OR_LATER
	if (m_hRenderWnd != nullptr)
	{
		RECT rc;
		::GetClientRect(m_hRenderWnd, &rc);

		int iClientWidth = rc.right - rc.left;
		int iClientHeight = rc.bottom - rc.top;

		int iDesktopWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int iDesktopHeight = ::GetSystemMetrics(SM_CYSCREEN);
		if (iClientWidth < iDesktopWidth && iClientHeight < iDesktopHeight)
		{
			m_fViewOffset.u = m_fBaseSize.u * m_fDefaultScale * (1 - m_fSkeletonScale) / 2.f;
			m_fViewOffset.v = m_fBaseSize.v * m_fDefaultScale * (1 - m_fSkeletonScale) / 2.f;
		}
	}
#endif
	UpdatePosition();
}
/*位置適用*/
void CDxLibSpinePlayerC::UpdatePosition()
{
	for (const auto& drawable : m_drawables)
	{
		drawable->skeleton->x = m_fBaseSize.u / 2 - m_fOffset.u - m_fViewOffset.u;
		drawable->skeleton->y = m_fBaseSize.v / 2 - m_fOffset.v - m_fViewOffset.v;
	}
}
/*尺度適用*/
void CDxLibSpinePlayerC::UpdateScaletonScale()
{
	/*
	* scaleX and scaleY in spSkeleton were added since spine-3.7.
	* It is true that skeleton->root has scaleX and scaleY, but these values will be reset to 1.f
	* on _spScaleTimeline_apply() if the animation has no appropriate timeline properties.
	*/
#ifdef SPINE_3_7_OR_LATER
	for (const auto& drawbale : m_drawables)
	{
		drawbale->skeleton->scaleX = m_fSkeletonScale;
		drawbale->skeleton->scaleY = m_fSkeletonScale;
	}
#endif
}
/*速度適用*/
void CDxLibSpinePlayerC::UpdateTimeScale()
{
	for (const auto& pDrawble : m_drawables)
	{
		pDrawble->timeScale = m_fTimeScale;
	}
}
/*合成動作消去*/
void CDxLibSpinePlayerC::ClearAnimationTracks()
{
	for (const auto& pDdrawble : m_drawables)
	{
		for (int iTrack = 1; iTrack < pDdrawble->animationState->tracksCount;++iTrack)
		{
			spAnimationState_setEmptyAnimation(pDdrawble->animationState, iTrack, 0.f);
		}
	}
}
/*窓寸法調整*/
void CDxLibSpinePlayerC::ResizeWindow()
{
	if (m_hRenderWnd != nullptr)
	{
		RECT rect;
		::GetWindowRect(m_hRenderWnd, &rect);
		int iX = static_cast<int>(m_fBaseSize.u * m_fSkeletonScale);
		int iY = static_cast<int>(m_fBaseSize.v * m_fSkeletonScale);

		rect.right = iX + rect.left;
		rect.bottom = iY + rect.top;
		LONG lStyle = ::GetWindowLong(m_hRenderWnd, GWL_STYLE);
		const auto HasWindowMenu = [this, &lStyle]()
			-> bool
			{
				return !((lStyle & WS_CAPTION) && (lStyle & WS_SYSMENU));
			};
		::AdjustWindowRect(&rect, lStyle, HasWindowMenu() ? FALSE : TRUE);
		::SetWindowPos(m_hRenderWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);

		ResizeBuffer();
	}
}

void CDxLibSpinePlayerC::ResizeBuffer()
{
	if (m_hRenderWnd != nullptr)
	{
		RECT rc;
		::GetClientRect(m_hRenderWnd, &rc);

		int iClientWidth = rc.right - rc.left;
		int iClientHeight = rc.bottom - rc.top;

		int iDesktopWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int iDesktopHeight = ::GetSystemMetrics(SM_CYSCREEN);

		DxLib::SetGraphMode
		(
			iClientWidth < iDesktopWidth ? iClientWidth : iDesktopWidth,
			iClientHeight < iDesktopHeight ? iClientHeight : iDesktopHeight,
			32
		);
	}
}
