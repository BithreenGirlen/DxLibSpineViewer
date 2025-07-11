
/* FREE() and MALLOC_STR() macro */
#include <spine/extension.h>

#include "spine_player_c.h"
#include "spine_loader_c.h"

CSpinePlayerC::CSpinePlayerC()
{

}

CSpinePlayerC::~CSpinePlayerC()
{

}

/*ファイル取り込み*/
bool CSpinePlayerC::LoadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
{
	if (atlasPaths.size() != skelPaths.size())return false;
	ClearDrawables();

	for (size_t i = 0; i < atlasPaths.size(); ++i)
	{
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonPath = skelPaths[i];

		std::shared_ptr<spAtlas> atlas = spine_loader_c::CreateAtlasFromFile(strAtlasPath.c_str(), nullptr);
		if (atlas.get() == nullptr)continue;

		std::shared_ptr<spSkeletonData> skeletonData = isBinarySkel ?
			spine_loader_c::ReadBinarySkeletonFromFile(strSkeletonPath.c_str(), atlas.get()) :
			spine_loader_c::ReadTextSkeletonFromFile(strSkeletonPath.c_str(), atlas.get());
		if (skeletonData.get() == nullptr)continue;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return SetupDrawables();
}
/*メモリ取り込み*/
bool CSpinePlayerC::LoadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool isBinarySkel)
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

		std::shared_ptr<spSkeletonData> skeletonData = isBinarySkel ?
			spine_loader_c::ReadBinarySkeletonFromMemory(reinterpret_cast<const unsigned char*>((strSkeletonData.c_str())), static_cast<int>(strSkeletonData.size()), atlas.get()) :
			spine_loader_c::ReadTextSkeletonFromMemory(strSkeletonData.c_str(), atlas.get());
		if (skeletonData.get() == nullptr)continue;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return SetupDrawables();
}
/*ファイルから追加*/
bool CSpinePlayerC::AddSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool bBinary)
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
	if (m_isDrawOrderReversed)
	{
		std::rotate(m_drawables.rbegin(), m_drawables.rbegin() + 1, m_drawables.rend());
	}

	RestartAnimation();
	ResetScale();

	return true;
}

size_t CSpinePlayerC::GetNumberOfSpines() const
{
	return m_drawables.size();
}

bool CSpinePlayerC::HasSpineBeenLoaded() const
{
	return !m_drawables.empty();
}

void CSpinePlayerC::Update(float fDelta)
{
	for (const auto& drawable : m_drawables)
	{
		drawable->Update(fDelta * m_fTimeScale);
	}
}
/*拡縮変更*/
void CSpinePlayerC::RescaleSkeleton(bool bUpscale)
{
	if (bUpscale)
	{
		m_fSkeletonScale += kfScaleFactor;
	}
	else
	{
		m_fSkeletonScale -= kfScaleFactor;
		if (m_fSkeletonScale < kfMinScale)m_fSkeletonScale = kfMinScale;
	}
}

void CSpinePlayerC::RescaleCanvas(bool bUpscale)
{
	if (bUpscale)
	{
		m_fCanvasScale += kfScaleFactor;
	}
	else
	{
		m_fCanvasScale -= kfScaleFactor;
		if (m_fCanvasScale < kfMinScale)m_fCanvasScale = kfMinScale;
	}
}
/*時間尺度変更*/
void CSpinePlayerC::RescaleTime(bool bHasten)
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
}

void CSpinePlayerC::ResetScale()
{
	m_fTimeScale = 1.0f;
	m_fSkeletonScale = m_fDefaultScale;
	m_fCanvasScale = m_fDefaultScale;
	m_fOffset = m_fDefaultOffset;

	UpdatePosition();
}
/*位置移動*/
void CSpinePlayerC::MoveViewPoint(int iX, int iY)
{
	m_fOffset.x += iX / m_fSkeletonScale;
	m_fOffset.y += iY / m_fSkeletonScale;
	UpdatePosition();
}
/*動作移行*/
void CSpinePlayerC::ShiftAnimation()
{
	++m_nAnimationIndex;
	if (m_nAnimationIndex >= m_animationNames.size())m_nAnimationIndex = 0;

	ClearAnimationTracks();
	RestartAnimation();
}
/*装い移行*/
void CSpinePlayerC::ShiftSkin()
{
	if (m_skinNames.empty())return;

	++m_nSkinIndex;
	if (m_nSkinIndex >= m_skinNames.size())m_nSkinIndex = 0;

	SetupSkin();
}

void CSpinePlayerC::SetAnimationByIndex(size_t nIndex)
{
	if (nIndex < m_animationNames.size())
	{
		m_nAnimationIndex = nIndex;
		RestartAnimation();
	}
}
void CSpinePlayerC::SetAnimationByName(const char* szAnimationName)
{
	if (szAnimationName != nullptr)
	{
		const auto& iter = std::find(m_animationNames.begin(), m_animationNames.end(), szAnimationName);
		if (iter != m_animationNames.cend())
		{
			m_nAnimationIndex = std::distance(m_animationNames.begin(), iter);
			RestartAnimation();
		}
	}
}
/*動作適用*/
void CSpinePlayerC::RestartAnimation(bool loop)
{
	if (m_nAnimationIndex >= m_animationNames.size())return;
	const char* szAnimationName = m_animationNames[m_nAnimationIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spAnimation* pAnimation = spSkeletonData_findAnimation(pDrawable->skeleton->data, szAnimationName);
		if (pAnimation != nullptr)
		{
			spAnimationState_setAnimationByName(pDrawable->animationState, 0, pAnimation->name, loop ? -1 : 0);
		}
	}
}

void CSpinePlayerC::SetSkinByIndex(size_t nIndex)
{
	if (nIndex < m_skinNames.size())
	{
		m_nSkinIndex = nIndex;
		SetupSkin();
	}
}

void CSpinePlayerC::SetSkinByName(const char* szSkinName)
{
	if (szSkinName != nullptr)
	{
		const auto& iter = std::find(m_skinNames.begin(), m_skinNames.end(), szSkinName);
		if (iter != m_skinNames.cend())
		{
			m_nSkinIndex = std::distance(m_skinNames.begin(), iter);
			SetupSkin();
		}
	}
}

void CSpinePlayerC::SetupSkin()
{
	if (m_nSkinIndex >= m_skinNames.size())return;
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
void CSpinePlayerC::TogglePma()
{
	for (const auto& drawable : m_drawables)
	{
		drawable->isAlphaPremultiplied ^= true;
	}
}
/*槽溝指定合成方法採択可否*/
void CSpinePlayerC::ToggleBlendModeAdoption()
{
	for (const auto& drawable : m_drawables)
	{
		drawable->isToForceBlendModeNormal ^= true;
	}
}
/* 乗算済み是否 */
bool CSpinePlayerC::IsAlphaPremultiplied(size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		return m_drawables[nDrawableIndex]->isAlphaPremultiplied;
	}

	return false;
}
/* 通常混色法強制是否*/
bool CSpinePlayerC::IsBlendModeNormalForced(size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		return m_drawables[nDrawableIndex]->isToForceBlendModeNormal;
	}

	return false;
}

bool CSpinePlayerC::IsDrawOrderReversed() const
{
	return m_isDrawOrderReversed;
}

bool CSpinePlayerC::PremultiplyAlpha(bool isToBePremultiplied, size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		m_drawables[nDrawableIndex]->isAlphaPremultiplied = isToBePremultiplied;
		return true;
	}

	return false;
}

bool CSpinePlayerC::ForceBlendModeNormal(bool isToForce, size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		m_drawables[nDrawableIndex]->isToForceBlendModeNormal = isToForce;
		return true;
	}

	return false;
}

void CSpinePlayerC::SetDrawOrder(bool isToBeReversed)
{
	m_isDrawOrderReversed = isToBeReversed;
}

std::string CSpinePlayerC::GetCurrentAnimationName()
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
					return pAnimation->name;
				}
			}
		}
	}

	return std::string();
}

void CSpinePlayerC::GetCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd)
{
	for (const auto& pDrawable : m_drawables)
	{
		for (size_t i = 0; i < pDrawable->animationState->tracksCount; ++i)
		{
			spTrackEntry* pTrackEntry = pDrawable->animationState->tracks[i];
			if (pTrackEntry != nullptr)
			{
				spAnimation* pAnimation = pTrackEntry->animation;
				if (pAnimation != nullptr)
				{
#ifdef SPINE_2_1
					if (fTrack != nullptr)*fTrack = pTrackEntry->time;
					/* spTrackEntry::lastTime is the same as pTrackEntry->time */
					if (fLast != nullptr)*fLast = ::fmodf(pTrackEntry->time, pTrackEntry->endTime);
					if (fStart != nullptr)*fStart = pTrackEntry->delay;
					if (fEnd != nullptr)*fEnd = pTrackEntry->endTime;
#else
					if (fTrack != nullptr)*fTrack = pTrackEntry->trackTime;
					if (fLast != nullptr)*fLast = pTrackEntry->animationLast;
					if (fStart != nullptr)*fStart = pTrackEntry->animationStart;
					if (fEnd != nullptr)*fEnd = pTrackEntry->animationEnd;
#endif
				}
			}
		}
	}
}
/*槽溝名称引き渡し*/
const std::vector<std::string>& CSpinePlayerC::GetSlotNames() const
{
	return m_slotNames;
}
/*装い名称引き渡し*/
const std::vector<std::string>& CSpinePlayerC::GetSkinNames() const
{
	return m_skinNames;
}
/*動作名称引き渡し*/
const std::vector<std::string>& CSpinePlayerC::GetAnimationNames() const
{
	return m_animationNames;
}
/*描画除外リスト設定*/
void CSpinePlayerC::SetSlotsToExclude(const std::vector<std::string>& slotNames)
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
void CSpinePlayerC::MixSkins(const std::vector<std::string>& skinNames)
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
void CSpinePlayerC::MixAnimations(const std::vector<std::string>& animationNames, bool loop)
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
					spAnimationState_addAnimation(pDrawble->animationState, iTrack, pAnimationToAdd, loop ? - 1: 0, 0.f);
					++iTrack;
				}
			}
		}
	}
}
/*差し替え可能な槽溝名称取得*/
std::unordered_map<std::string, std::vector<std::string>> CSpinePlayerC::GetSlotNamesWithTheirAttachments()
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
bool CSpinePlayerC::ReplaceAttachment(const char* szSlotName, const char* szAttachmentName)
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
FPoint2 CSpinePlayerC::GetBaseSize() const
{
	return m_fBaseSize;
}

FPoint2 CSpinePlayerC::GetOffset() const
{
	return m_fOffset;
}

float CSpinePlayerC::GetSkeletonScale() const
{
	return m_fSkeletonScale;
}

void CSpinePlayerC::SetSkeletonScale(float fScale)
{
	m_fSkeletonScale = fScale;
}

float CSpinePlayerC::GetCanvasScale() const
{
	return m_fCanvasScale;
}
void CSpinePlayerC::SetCanvasScale(float fScale)
{
	m_fCanvasScale = fScale;
}

float CSpinePlayerC::GetTimeScale() const
{
	return m_fTimeScale;
}

void CSpinePlayerC::SetTimeScale(float fTimeScale)
{
	m_fTimeScale = fTimeScale;
}
/*消去*/
void CSpinePlayerC::ClearDrawables()
{
	m_drawables.clear();
	m_atlases.clear();
	m_skeletonData.clear();

	m_animationNames.clear();
	m_nAnimationIndex = 0;

	m_skinNames.clear();
	m_nSkinIndex = 0;

	m_slotNames.clear();
}
/*描画物追加*/
bool CSpinePlayerC::AddDrawable(spSkeletonData* const pSkeletonData)
{
	auto pDrawable = std::make_shared<CSpineDrawableC>(pSkeletonData);
	if (pDrawable.get() == nullptr)false;

	pDrawable->skeleton->x = m_fBaseSize.x / 2;
	pDrawable->skeleton->y = m_fBaseSize.y / 2;
	pDrawable->Update(0.f);

	m_drawables.push_back(std::move(pDrawable));

	return true;
}

bool CSpinePlayerC::SetupDrawables()
{
	WorkOutDefaultSize();
	WorkOutDefaultScale();

	for (const auto& pSkeletonDatum : m_skeletonData)
	{
		bool bRet = AddDrawable(pSkeletonDatum.get());
		if (!bRet)continue;

		for (size_t i = 0; i < pSkeletonDatum->animationsCount; ++i)
		{
			const char* szAnimationName = pSkeletonDatum->animations[i]->name;
			if (szAnimationName == nullptr)continue;

			const auto& iter = std::find(m_animationNames.begin(), m_animationNames.end(), szAnimationName);
			if (iter == m_animationNames.cend())m_animationNames.push_back(szAnimationName);
		}

		for (size_t i = 0; i < pSkeletonDatum->skinsCount; ++i)
		{
			const char* szSkinName = pSkeletonDatum->skins[i]->name;
			if (szSkinName == nullptr)continue;

			const auto& iter = std::find(m_skinNames.begin(), m_skinNames.end(), szSkinName);
			if (iter == m_skinNames.cend())m_skinNames.push_back(szSkinName);
		}

		for (size_t i = 0; i < pSkeletonDatum->slotsCount; ++i)
		{
			const char* szSlotName = pSkeletonDatum->slots[i]->name;
			if (szSlotName == nullptr)continue;

			const auto& iter = std::find(m_slotNames.begin(), m_slotNames.end(), szSlotName);
			if (iter == m_slotNames.cend())m_slotNames.push_back(szSlotName);
		}
	}

	WorkOutDefaultOffset();

	RestartAnimation();

	ResetScale();

	return m_animationNames.size() > 0;
}
/*標準寸法算出*/
void CSpinePlayerC::WorkOutDefaultSize()
{
	if (m_skeletonData.empty())return;

	float fMaxSize = 0.f;
	const auto CompareDimention = [this, &fMaxSize](float fWidth, float fHeight)
		-> bool
		{
			if (fWidth > 0.f && fHeight > 0.f && fWidth * fHeight > fMaxSize)
			{
				m_fBaseSize.x = fWidth;
				m_fBaseSize.y = fHeight;
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
void CSpinePlayerC::WorkOutDefaultScale()
{
	m_fDefaultScale = 1.f;
	m_fDefaultOffset = {};

	int iSkeletonWidth = static_cast<int>(m_fBaseSize.x);
	int iSkeletonHeight = static_cast<int>(m_fBaseSize.y);

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
void CSpinePlayerC::UpdatePosition()
{
	for (const auto& drawable : m_drawables)
	{
		drawable->skeleton->x = m_fBaseSize.x / 2 - m_fOffset.x;
		drawable->skeleton->y = m_fBaseSize.y / 2 - m_fOffset.y;
	}
}
/*合成動作消去*/
void CSpinePlayerC::ClearAnimationTracks()
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
