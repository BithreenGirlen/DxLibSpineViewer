
/* FREE() and MALLOC_STR() macro */
#include <spine/extension.h>

#include "dxlib_spine_player_c.h"
#include "spine_loader_c.h"

CDxLibSpinePlayerC::CDxLibSpinePlayerC()
{

}

CDxLibSpinePlayerC::~CDxLibSpinePlayerC()
{

}

/*ファイル取り込み*/
bool CDxLibSpinePlayerC::SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary)
{
	if (atlasPaths.size() != skelPaths.size())return false;
	ClearDrawables();

	for (size_t i = 0; i < atlasPaths.size(); ++i)
	{
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonPath = skelPaths[i];

		std::shared_ptr<spAtlas> atlas = spine_loader_c::CreateAtlasFromFile(strAtlasPath.c_str(), nullptr);
		if (atlas.get() == nullptr)continue;

		std::shared_ptr<spSkeletonData> skeletonData = bIsBinary ?
			spine_loader_c::ReadBinarySkeletonFromFile(strSkeletonPath.c_str(), atlas.get()) :
			spine_loader_c::ReadTextSkeletonFromFile(strSkeletonPath.c_str(), atlas.get());
		if (skeletonData.get() == nullptr)continue;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
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
		const std::string& strAtlasDatum = atlasData[i];
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonData = skelData[i];

		std::shared_ptr<spAtlas> atlas = spine_loader_c::CreateAtlasFromMemory(strAtlasDatum.c_str(), static_cast<int>(strAtlasDatum.size()), strAtlasPath.c_str(), nullptr);
		if (atlas.get() == nullptr)continue;

		std::shared_ptr<spSkeletonData> skeletonData = bIsBinary ?
			spine_loader_c::ReadBinarySkeletonFromMemory(reinterpret_cast<const unsigned char*>((strSkeletonData.c_str())), static_cast<int>(strSkeletonData.size()), atlas.get()) :
			spine_loader_c::ReadTextSkeletonFromMemory(strSkeletonData.c_str(), atlas.get());
		if (skeletonData.get() == nullptr)continue;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	WorkOutDefaultSize();
	WorkOutDefaultScale();

	return SetupDrawer();
}
/*ファイルから追加*/
bool CDxLibSpinePlayerC::AddSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool bBinary)
{
	if (m_drawables.empty() || szAtlasPath == nullptr || szSkelPath == nullptr)return false;

	std::shared_ptr<spAtlas> atlas = spine_loader_c::CreateAtlasFromFile(szAtlasPath, nullptr);
	if (atlas.get() == nullptr)return false;

	std::shared_ptr<spSkeletonData> skeletonData = bBinary ?
		spine_loader_c::ReadBinarySkeletonFromFile(szSkelPath, atlas.get()) :
		spine_loader_c::ReadTextSkeletonFromFile(szSkelPath, atlas.get());
	if (skeletonData.get() == nullptr)return false;

	bool bRet = AddDrawable(skeletonData.get());
	if (!bRet)return false;

	m_atlases.push_back(std::move(atlas));
	m_skeletonData.push_back(std::move(skeletonData));
	m_drawables.back()->SetPma(false);
	if (m_bDrawOrderReversed)
	{
		std::rotate(m_drawables.rbegin(), m_drawables.rbegin() + 1, m_drawables.rend());
	}

	UpdateAnimation();
	ResetScale();

	return true;
}
/*再描画*/
void CDxLibSpinePlayerC::Redraw(float fDelta)
{
	if (!m_drawables.empty())
	{
		SetTransformMatrix();

		if (!m_bDrawOrderReversed)
		{
			for (size_t i = 0; i < m_drawables.size(); ++i)
			{
				m_drawables[i]->Update(fDelta);
				m_drawables[i]->Draw(m_bDepthBufferEnabled ? 0.1f * (i + 1) : 0.f);
			}
		}
		else
		{
			for (long long i = m_drawables.size() - 1; i >= 0; --i)
			{
				m_drawables[i]->Update(fDelta);
				m_drawables[i]->Draw(m_bDepthBufferEnabled ? 0.1f * (i + 1) : 0.f);
			}
		}

		DxLib::ResetTransformTo2D();
	}
}
/*拡縮変更*/
void CDxLibSpinePlayerC::RescaleSkeleton(bool bUpscale)
{
	if (bUpscale)
	{
		m_fSkeletonScale += kfScalePortion;
	}
	else
	{
		m_fSkeletonScale -= kfScalePortion;
		if (m_fSkeletonScale < kfMinScale)m_fSkeletonScale = kfMinScale;
	}
}

void CDxLibSpinePlayerC::RescaleCanvas(bool bUpscale)
{
	if (bUpscale)
	{
		m_fCanvasScale += kfScalePortion;
	}
	else
	{
		m_fCanvasScale -= kfScalePortion;
		if (m_fCanvasScale < kfMinScale)m_fCanvasScale = kfMinScale;
	}
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
	m_fCanvasScale = m_fDefaultScale;
	m_fOffset = m_fDefaultOffset;

	UpdateTimeScale();
	UpdatePosition();
}
/*位置移動*/
void CDxLibSpinePlayerC::MoveViewPoint(int iX, int iY)
{
	m_fOffset.u += iX / m_fSkeletonScale;
	m_fOffset.v += iY / m_fSkeletonScale;
	UpdatePosition();
}
/*動作移行*/
void CDxLibSpinePlayerC::ShiftAnimation()
{
	++m_nAnimationIndex;
	if (m_nAnimationIndex >= m_animationNames.size())m_nAnimationIndex = 0;

	ClearAnimationTracks();
	UpdateAnimation();
}
/*装い移行*/
void CDxLibSpinePlayerC::ShiftSkin()
{
	if (m_skinNames.empty())return;

	++m_nSkinIndex;
	if (m_nSkinIndex >= m_skinNames.size())m_nSkinIndex = 0;

	const char* szSkinName = m_skinNames[m_nSkinIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spSkin* pSkin = spSkeletonData_findSkin(pDrawable->skeleton->data, szSkinName);
		if (pSkin != nullptr)
		{
			spSkeleton_setSkin(pDrawable->skeleton, pSkin);
			spSkeleton_setToSetupPose(pDrawable->skeleton);
		}
	}
}
/*乗算済み透過度有効・無効切り替え*/
void CDxLibSpinePlayerC::TogglePma()
{
	for (const auto& drawable : m_drawables)
	{
		drawable->SetPma(!drawable->GetPma());
	}
}
/*槽溝指定合成方法採択可否*/
void CDxLibSpinePlayerC::ToggleBlendModeAdoption()
{
	for (const auto& drawable : m_drawables)
	{
		drawable->SetForceBlendModeNormal(!drawable->GetForceBlendModeNormal());
	}
}
/*奥行き表現有効無効切り替え*/
bool CDxLibSpinePlayerC::ToggleDepthBufferValidity()
{
	int iRet = DxLib::SetUseZBufferFlag(m_bDepthBufferEnabled ? FALSE : TRUE);
	if (iRet == -1)return false;

	iRet = DxLib::SetWriteZBufferFlag(m_bDepthBufferEnabled ? FALSE : TRUE);
	if (iRet == -1)return false;

	m_bDepthBufferEnabled ^= true;
	return true;
}
/*描画順切り替え*/
void CDxLibSpinePlayerC::ToggleDrawOrder()
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
#ifdef SPINE_2_1
						*fTrackTime = pTrackEntry->time;
#else
						*fTrackTime = pTrackEntry->trackTime;
#endif
					}
					return pAnimation->name;
				}
			}
		}
	}

	return std::string();
}
/*槽溝名称取得*/
std::vector<std::string> CDxLibSpinePlayerC::GetSlotNames()
{
	std::vector<std::string> slotNames;
	for (const auto& skeletonDatum : m_skeletonData)
	{
		for (size_t i = 0; i < skeletonDatum->slotsCount; ++i)
		{
			const auto iter = std::find(slotNames.begin(), slotNames.end(), skeletonDatum->slots[i]->name);
			if (iter == slotNames.cend())slotNames.push_back(skeletonDatum->slots[i]->name);
		}
	}

	return slotNames;
}
/*装い名称引き渡し*/
const std::vector<std::string>& CDxLibSpinePlayerC::GetSkinNames() const
{
	return m_skinNames;
}
/*動作名称引き渡し*/
const std::vector<std::string>& CDxLibSpinePlayerC::GetAnimationNames() const
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
	const auto& currentSkinName = m_skinNames[m_nSkinIndex];

	for (const auto& pDrawble : m_drawables)
	{
		spSkin* skinToSet = spSkeletonData_findSkin(pDrawble->skeleton->data, currentSkinName.c_str());
		if (skinToSet == nullptr)continue;

		for (const auto& skinName : skinNames)
		{
			if (currentSkinName != skinName)
			{
				spSkin* skinToAdd = spSkeletonData_findSkin(pDrawble->skeleton->data, skinName.c_str());
				if (skinToAdd != nullptr)
				{
					spSkin_addSkin(skinToSet, skinToAdd);
				}
			}
		}
		spSkeleton_setSkin(pDrawble->skeleton, skinToSet);
		spSkeleton_setToSetupPose(pDrawble->skeleton);
	}
#endif
}
/*動作合成*/
void CDxLibSpinePlayerC::MixAnimations(const std::vector<std::string>& animationNames)
{
	ClearAnimationTracks();

	if (m_nAnimationIndex >= m_animationNames.size())return;
	const auto& currentAnimationName = m_animationNames[m_nAnimationIndex];

	for (const auto& pDrawble : m_drawables)
	{
		spAnimation* pCurrentAnimation = spSkeletonData_findAnimation(pDrawble->skeleton->data, currentAnimationName.c_str());
		if (pCurrentAnimation == nullptr)continue;

		int iTrack = 1;
		for (const auto& animationName : animationNames)
		{
			if (animationName != currentAnimationName)
			{
				spAnimation* pAnimationToAdd = spSkeletonData_findAnimation(pDrawble->skeleton->data, animationName.c_str());
				if (pAnimationToAdd != nullptr)
				{
					spAnimationState_addAnimation(pDrawble->animationState, iTrack, pAnimationToAdd, false, 0.f);
					++iTrack;
				}
			}
		}
	}
}
/*差し替え可能な槽溝名称取得*/
std::unordered_map<std::string, std::vector<std::string>> CDxLibSpinePlayerC::GetSlotNamesWithTheirAttachments()
{
	std::unordered_map<std::string, std::vector<std::string>> slotAttachmentMap;

	for (const auto& pSkeletonDatum : m_skeletonData)
	{
		spSkin* pSkin = pSkeletonDatum->defaultSkin;
		if (pSkin == nullptr)continue;

		for (int iSlotIndex = 0; iSlotIndex < pSkeletonDatum->slotsCount; ++iSlotIndex)
		{
			std::vector<std::string> attachmentNames;

			for (int iAttachmentIndex = 0;; ++iAttachmentIndex)
			{
				const char* attachmentName = spSkin_getAttachmentName(pSkeletonDatum->defaultSkin, iSlotIndex, iAttachmentIndex);
				if (attachmentName == nullptr)break;

				const auto& iter = std::find(attachmentNames.begin(), attachmentNames.end(), attachmentName);
				if (iter == attachmentNames.cend())attachmentNames.push_back(attachmentName);
			}
			
			if (attachmentNames.size() > 1)
			{
				slotAttachmentMap.insert({ pSkeletonDatum->slots[iSlotIndex]->name, attachmentNames });
			}
		}
	}

	return slotAttachmentMap;
}
/*差し替え*/
bool CDxLibSpinePlayerC::ReplaceAttachment(const char* szSlotName, const char* szAttachmentName)
{
	if (szSlotName == nullptr || szAttachmentName == nullptr)return false;

	const auto FindSlot = [&szSlotName](spSkeleton* const pSkeleton)
		-> spSlot*
		{
			for (size_t i = 0; i < pSkeleton->slotsCount; ++i)
			{
				const char* slotName = pSkeleton->drawOrder[i]->data->name;
				if (slotName != nullptr && strcmp(slotName, szSlotName) == 0)
				{
					return pSkeleton->drawOrder[i];
				}
			}

			return nullptr;
		};

	const auto FindAttachment = [&szAttachmentName](spSkeletonData* const pSkeletonDatum)
		-> spAttachment*
		{
			spSkin* pSkin = pSkeletonDatum->defaultSkin;
			if (pSkin != nullptr)
			{
				for (int iSlotIndex = 0; iSlotIndex < pSkeletonDatum->slotsCount; ++iSlotIndex)
				{
					for (int iAttachmentIndex = 0;; ++iAttachmentIndex)
					{
						const char* attachmentName = spSkin_getAttachmentName(pSkeletonDatum->defaultSkin, iSlotIndex, iAttachmentIndex);
						if (attachmentName == nullptr)break;

						if (strcmp(attachmentName, szAttachmentName) == 0)
						{
							spAttachment* pAttachment = spSkin_getAttachment(pSkeletonDatum->defaultSkin, iSlotIndex, attachmentName);
							if (pAttachment != nullptr)
							{
								return pAttachment;
							}
						}
					}
				}
			}

			return nullptr;
		};

	for (const auto& pDrawable : m_drawables)
	{
		spSlot* pSlot = FindSlot(pDrawable->skeleton);
		if (pSlot == nullptr)continue;

		spAttachment* pAttachment = FindAttachment(pDrawable->skeleton->data);
		if (pAttachment == nullptr)continue;

		/* Replace attachment name in spAttachmentTimeline if exists. */
		if (pSlot->attachment != nullptr)
		{
			const char* animationName = m_animationNames[m_nAnimationIndex].c_str();
			spAnimation* pAnimation = spSkeletonData_findAnimation(pDrawable->skeleton->data, animationName);
			if (pAnimation == nullptr)continue;

#ifndef SPINE_4_1_OR_LATER
			for (size_t i = 0; i < pAnimation->timelinesCount; ++i)
			{
				if (pAnimation->timelines[i]->type == SP_TIMELINE_ATTACHMENT)
				{
					spAttachmentTimeline* pAttachmentTimeline = (spAttachmentTimeline*)pAnimation->timelines[i];
					for (size_t ii = 0; ii < pAttachmentTimeline->framesCount; ++ii)
					{
						const char* szName = pAttachmentTimeline->attachmentNames[ii];
						if (szName == nullptr)continue;

						if (strcmp(szName, pSlot->attachment->name) == 0)
						{
							FREE(pAttachmentTimeline->attachmentNames[ii]);
							MALLOC_STR(pAttachmentTimeline->attachmentNames[ii], szAttachmentName);
						}
					}
				}
			}
#else
			for (size_t i = 0; i < pAnimation->timelines->size; ++i)
			{
				if (pAnimation->timelines->items[i]->type == SP_TIMELINE_ATTACHMENT)
				{
					spAttachmentTimeline* pAttachmentTimeline = (spAttachmentTimeline*)pAnimation->timelines->items[i];
					for (size_t ii = 0; ii < pAnimation->timelines->items[i]->frameCount; ++ii)
					{
						const char* szName = pAttachmentTimeline->attachmentNames[ii];
						if (szName == nullptr)continue;

						if (strcmp(szName, pSlot->attachment->name) == 0)
						{
							FREE(pAttachmentTimeline->attachmentNames[ii]);
							MALLOC_STR(pAttachmentTimeline->attachmentNames[ii], szAttachmentName);
						}
					}
				}
			}
#endif
		}

		spSlot_setAttachment(pSlot, pAttachment);
	}

	return true;
}
/*寸法受け渡し*/
void CDxLibSpinePlayerC::GetBaseSize(float* fWidth, float* fHeight) const
{
	if (fWidth != nullptr)*fWidth = m_fBaseSize.u;
	if (fHeight != nullptr)*fHeight = m_fBaseSize.v;
}
/*尺度受け渡し*/
float CDxLibSpinePlayerC::GetCanvasScale() const
{
	return m_fCanvasScale;
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
/*描画物追加*/
bool CDxLibSpinePlayerC::AddDrawable(spSkeletonData* const pSkeletonData)
{
	auto pDrawable = std::make_shared<CDxLibSpineDrawerC>(pSkeletonData);
	if (pDrawable.get() == nullptr)false;

	pDrawable->timeScale = 1.0f;
	pDrawable->skeleton->x = m_fBaseSize.u / 2;
	pDrawable->skeleton->y = m_fBaseSize.v / 2;
	spSkeleton_setToSetupPose(pDrawable->skeleton);
	spSkeleton_updateWorldTransform(pDrawable->skeleton);

	m_drawables.push_back(std::move(pDrawable));

	return true;
}
/*描画器設定*/
bool CDxLibSpinePlayerC::SetupDrawer()
{
	for (const auto& pSkeletonDatum : m_skeletonData)
	{
		bool bRet = AddDrawable(pSkeletonDatum.get());
		if (!bRet)continue;

		for (size_t i = 0; i < pSkeletonDatum->animationsCount; ++i)
		{
			const char* szAnimationName = pSkeletonDatum->animations[i]->name;
			if (szAnimationName == nullptr)continue;

			const auto &iter = std::find(m_animationNames.begin(), m_animationNames.end(), szAnimationName);
			if (iter == m_animationNames.cend())m_animationNames.push_back(szAnimationName);
		}

		for (size_t i = 0; i < pSkeletonDatum->skinsCount; ++i)
		{
			const char* szSkinName = pSkeletonDatum->skins[i]->name;
			if (szSkinName == nullptr)continue;
			
			const auto &iter = std::find(m_skinNames.begin(), m_skinNames.end(), szSkinName);
			if (iter == m_skinNames.cend())m_skinNames.push_back(szSkinName);
		}
	}

	UpdateAnimation();

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

	int iDisplayWidth = 0;
	int iDisplayHeight = 0;
#if defined _WIN32
	DxLib::GetDisplayMaxResolution(&iDisplayWidth, &iDisplayHeight);
#elif defined __ANDROID__
	DxLib::GetAndroidDisplayResolution(&iDisplayWidth, &iDisplayHeight);
#elif defined __APPLE__
	DxLib::GetDisplayResolution_iOS(&iDisplayWidth, &iDisplayHeight);
#endif
	if (iDisplayWidth == 0 || iDisplayHeight == 0)return;

	if (iSkeletonWidth > iDisplayWidth || iSkeletonHeight > iDisplayHeight)
	{
		float fScaleX = static_cast<float>(iDisplayWidth) / iSkeletonWidth;
		float fScaleY = static_cast<float>(iDisplayHeight) / iSkeletonHeight;

		if (fScaleX > fScaleY)
		{
			m_fDefaultScale = fScaleY;
		}
		else
		{
			m_fDefaultScale = fScaleX;
		}
	}
}
/*位置適用*/
void CDxLibSpinePlayerC::UpdatePosition()
{
	for (const auto& drawable : m_drawables)
	{
		drawable->skeleton->x = m_fBaseSize.u / 2 - m_fOffset.u;
		drawable->skeleton->y = m_fBaseSize.v / 2 - m_fOffset.v;
	}
}
/*速度適用*/
void CDxLibSpinePlayerC::UpdateTimeScale()
{
	for (const auto& pDrawble : m_drawables)
	{
		pDrawble->timeScale = m_fTimeScale;
	}
}
/*動作適用*/
void CDxLibSpinePlayerC::UpdateAnimation()
{
	if (m_nAnimationIndex >= m_animationNames.size())return;
	const char* szAnimationName = m_animationNames[m_nAnimationIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spAnimation* pAnimation = spSkeletonData_findAnimation(pDrawable->skeleton->data, szAnimationName);
		if (pAnimation != nullptr)
		{
			spAnimationState_setAnimationByName(pDrawable->animationState, 0, pAnimation->name, 1);
		}
	}
}
/*合成動作消去*/
void CDxLibSpinePlayerC::ClearAnimationTracks()
{
	for (const auto& pDdrawble : m_drawables)
	{
#ifdef SPINE_2_1
		/*This clears 0-th track as well.*/
		spAnimationState_clearTracks(pDdrawble->animationState);
#else
		for (int iTrack = 1; iTrack < pDdrawble->animationState->tracksCount; ++iTrack)
		{
			spAnimationState_setEmptyAnimation(pDdrawble->animationState, iTrack, 0.f);
		}
#endif
	}
}

void CDxLibSpinePlayerC::SetTransformMatrix() const
{
	int iClientWidth = 0;
	int iClientHeight = 0;
	DxLib::GetScreenState(&iClientWidth, &iClientHeight, nullptr);
	float fX = (m_fBaseSize.u * m_fSkeletonScale - iClientWidth) / 2;
	float fY = (m_fBaseSize.v * m_fSkeletonScale - iClientHeight) / 2;

	DxLib::MATRIX matrix = DxLib::MGetScale(DxLib::VGet(m_fSkeletonScale, m_fSkeletonScale, 1.f));
	DxLib::MATRIX tranlateMatrix = DxLib::MGetTranslate(DxLib::VGet(-fX, -fY, 0.f));
	matrix = DxLib::MMult(matrix, tranlateMatrix);

	DxLib::SetTransformTo2D(&matrix);
}
