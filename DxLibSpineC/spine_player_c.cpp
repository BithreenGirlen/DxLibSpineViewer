
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
bool CSpinePlayerC::loadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
{
	if (atlasPaths.size() != skelPaths.size())return false;
	clearDrawables();

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

	return setupDrawables();
}
/*メモリ取り込み*/
bool CSpinePlayerC::loadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool isBinarySkel)
{
	if (atlasData.size() != skelData.size() || atlasData.size() != atlasPaths.size())return false;
	clearDrawables();

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

	return setupDrawables();
}
/*ファイルから追加*/
bool CSpinePlayerC::addSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool bBinary)
{
	if (m_drawables.empty() || szAtlasPath == nullptr || szSkelPath == nullptr)return false;

	std::shared_ptr<spAtlas> atlas = spine_loader_c::CreateAtlasFromFile(szAtlasPath, nullptr);
	if (atlas.get() == nullptr)return false;

	std::shared_ptr<spSkeletonData> skeletonData = bBinary ?
		spine_loader_c::ReadBinarySkeletonFromFile(szSkelPath, atlas.get()) :
		spine_loader_c::ReadTextSkeletonFromFile(szSkelPath, atlas.get());
	if (skeletonData.get() == nullptr)return false;

	bool bRet = addDrawable(skeletonData.get());
	if (!bRet)return false;

	m_atlases.push_back(std::move(atlas));
	m_skeletonData.push_back(std::move(skeletonData));
	if (m_isDrawOrderReversed)
	{
		std::rotate(m_drawables.rbegin(), m_drawables.rbegin() + 1, m_drawables.rend());
	}

	restartAnimation();
	resetScale();

	return true;
}

size_t CSpinePlayerC::getNumberOfSpines() const noexcept
{
	return m_drawables.size();
}

bool CSpinePlayerC::hasSpineBeenLoaded() const noexcept
{
	return !m_drawables.empty();
}

void CSpinePlayerC::update(float fDelta)
{
	for (const auto& drawable : m_drawables)
	{
		drawable->update(fDelta * m_fTimeScale);
	}
}

void CSpinePlayerC::resetScale()
{
	m_fTimeScale = 1.0f;
	m_fSkeletonScale = m_fDefaultScale;
	m_fCanvasScale = m_fDefaultScale;
	m_fOffset = m_fDefaultOffset;

	updatePosition();
}
/*位置移動*/
void CSpinePlayerC::addOffset(int iX, int iY)
{
	m_fOffset.x += iX / m_fSkeletonScale;
	m_fOffset.y += iY / m_fSkeletonScale;
	updatePosition();
}
/*動作移行*/
void CSpinePlayerC::shiftAnimation()
{
	++m_nAnimationIndex;
	if (m_nAnimationIndex >= m_animationNames.size())m_nAnimationIndex = 0;

	clearAnimationTracks();
	restartAnimation();
}
/*装い移行*/
void CSpinePlayerC::shiftSkin()
{
	if (m_skinNames.empty())return;

	++m_nSkinIndex;
	if (m_nSkinIndex >= m_skinNames.size())m_nSkinIndex = 0;

	setupSkin();
}

void CSpinePlayerC::setAnimationByIndex(size_t nIndex)
{
	if (nIndex < m_animationNames.size())
	{
		m_nAnimationIndex = nIndex;
		restartAnimation();
	}
}
void CSpinePlayerC::setAnimationByName(const char* szAnimationName)
{
	if (szAnimationName != nullptr)
	{
		const auto& iter = std::find(m_animationNames.begin(), m_animationNames.end(), szAnimationName);
		if (iter != m_animationNames.cend())
		{
			m_nAnimationIndex = std::distance(m_animationNames.begin(), iter);
			restartAnimation();
		}
	}
}
/*動作適用*/
void CSpinePlayerC::restartAnimation(bool loop)
{
	if (m_nAnimationIndex >= m_animationNames.size())return;
	const char* szAnimationName = m_animationNames[m_nAnimationIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spAnimation* pAnimation = spSkeletonData_findAnimation(pDrawable->skeleton()->data, szAnimationName);
		if (pAnimation != nullptr)
		{
			spAnimationState_setAnimationByName(pDrawable->animationState(), 0, pAnimation->name, loop ? -1 : 0);
		}
	}
}

void CSpinePlayerC::setSkinByIndex(size_t nIndex)
{
	if (nIndex < m_skinNames.size())
	{
		m_nSkinIndex = nIndex;
		setupSkin();
	}
}

void CSpinePlayerC::setSkinByName(const char* szSkinName)
{
	if (szSkinName != nullptr)
	{
		const auto& iter = std::find(m_skinNames.begin(), m_skinNames.end(), szSkinName);
		if (iter != m_skinNames.cend())
		{
			m_nSkinIndex = std::distance(m_skinNames.begin(), iter);
			setupSkin();
		}
	}
}

void CSpinePlayerC::setupSkin()
{
	if (m_nSkinIndex >= m_skinNames.size())return;
	const char* szSkinName = m_skinNames[m_nSkinIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spSkin* pSkin = spSkeletonData_findSkin(pDrawable->skeleton()->data, szSkinName);
		if (pSkin != nullptr)
		{
			spSkeleton_setSkin(pDrawable->skeleton(), pSkin);
			spSkeleton_setToSetupPose(pDrawable->skeleton());
		}
	}
}
/*乗算済み透過度有効・無効切り替え*/
void CSpinePlayerC::togglePma()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->premultiplyAlpha(!pDrawable->isAlphaPremultiplied());
	}
}
/*槽溝指定合成方法採択可否*/
void CSpinePlayerC::toggleBlendModeAdoption()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->forceBlendModeNormal(!pDrawable->isBlendModeNormalForced());
	}
}
/* 乗算済み是否 */
bool CSpinePlayerC::isAlphaPremultiplied(size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		return m_drawables[nDrawableIndex]->isAlphaPremultiplied();
	}

	return false;
}
/* 通常混色法強制是否*/
bool CSpinePlayerC::isBlendModeNormalForced(size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		return m_drawables[nDrawableIndex]->isBlendModeNormalForced();
	}

	return false;
}

bool CSpinePlayerC::isDrawOrderReversed() const noexcept
{
	return m_isDrawOrderReversed;
}

bool CSpinePlayerC::premultiplyAlpha(bool premultiplied, size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		m_drawables[nDrawableIndex]->premultiplyAlpha(premultiplied);
		return true;
	}

	return false;
}

bool CSpinePlayerC::forceBlendModeNormal(bool toForce, size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		m_drawables[nDrawableIndex]->forceBlendModeNormal(toForce);
		return true;
	}

	return false;
}

void CSpinePlayerC::setDrawOrder(bool reversed)
{
	m_isDrawOrderReversed = reversed;
}

std::string CSpinePlayerC::getCurrentAnimationName()
{
	for (const auto& pDrawable : m_drawables)
	{
		for (size_t i = 0; i < pDrawable->animationState()->tracksCount; ++i)
		{
			spTrackEntry* pTrackEntry = pDrawable->animationState()->tracks[i];
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

void CSpinePlayerC::getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd)
{
	for (const auto& pDrawable : m_drawables)
	{
		for (size_t i = 0; i < pDrawable->animationState()->tracksCount; ++i)
		{
			spTrackEntry* pTrackEntry = pDrawable->animationState()->tracks[i];
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
const std::vector<std::string>& CSpinePlayerC::getSlotNames() const noexcept
{
	return m_slotNames;
}
/*装い名称引き渡し*/
const std::vector<std::string>& CSpinePlayerC::getSkinNames() const noexcept
{
	return m_skinNames;
}
/*動作名称引き渡し*/
const std::vector<std::string>& CSpinePlayerC::getAnimationNames() const noexcept
{
	return m_animationNames;
}
/*描画除外リスト設定*/
void CSpinePlayerC::setSlotsToExclude(const std::vector<std::string>& slotNames)
{
	std::vector<const char*> vBuffer;
	vBuffer.resize(slotNames.size());
	for (size_t i = 0; i < slotNames.size(); ++i)
	{
		vBuffer[i] = slotNames[i].data();
	}
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->setLeaveOutList(vBuffer.data(), static_cast<int>(vBuffer.size()));
	}
}
/*装い合成*/
void CSpinePlayerC::mixSkins(const std::vector<std::string>& skinNames)
{
	/*spine-c 3.6 does not have spSkin_addSkin(). It was added since spine-c 3.8*/
#ifdef SPINE_3_8_OR_LATER
	if (m_nSkinIndex >= m_skinNames.size())return;
	const auto& currentSkinName = m_skinNames[m_nSkinIndex];

	for (const auto& pDrawble : m_drawables)
	{
		spSkin* skinToSet = spSkeletonData_findSkin(pDrawble->skeleton()->data, currentSkinName.c_str());
		if (skinToSet == nullptr)continue;

		for (const auto& skinName : skinNames)
		{
			if (currentSkinName != skinName)
			{
				spSkin* skinToAdd = spSkeletonData_findSkin(pDrawble->skeleton()->data, skinName.c_str());
				if (skinToAdd != nullptr)
				{
					spSkin_addSkin(skinToSet, skinToAdd);
				}
			}
		}
		spSkeleton_setSkin(pDrawble->skeleton, skinToSet);
		spSkeleton_setToSetupPose(pDrawble->skeleton());
	}
#endif
}
/*動作合成*/
void CSpinePlayerC::addAnimationTracks(const std::vector<std::string>& animationNames, bool loop)
{
	clearAnimationTracks();

	if (m_nAnimationIndex >= m_animationNames.size())return;
	const auto& currentAnimationName = m_animationNames[m_nAnimationIndex];

	for (const auto& pDrawble : m_drawables)
	{
		spAnimation* pCurrentAnimation = spSkeletonData_findAnimation(pDrawble->skeleton()->data, currentAnimationName.c_str());
		if (pCurrentAnimation == nullptr)continue;

		int iTrack = 1;
		for (const auto& animationName : animationNames)
		{
			if (animationName != currentAnimationName)
			{
				spAnimation* pAnimationToAdd = spSkeletonData_findAnimation(pDrawble->skeleton()->data, animationName.c_str());
				if (pAnimationToAdd != nullptr)
				{
					spAnimationState_addAnimation(pDrawble->animationState(), iTrack, pAnimationToAdd, loop ? - 1: 0, 0.f);
					++iTrack;
				}
			}
		}
	}
}

void CSpinePlayerC::setSlotExcludeCallback(bool(*pFunc)(const char*, size_t))
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->setLeaveOutCallback(pFunc);
	}
}
/*差し替え可能な槽溝名称取得*/
std::unordered_map<std::string, std::vector<std::string>> CSpinePlayerC::getSlotNamesWithTheirAttachments()
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
bool CSpinePlayerC::replaceAttachment(const char* szSlotName, const char* szAttachmentName)
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
		spSlot* pSlot = FindSlot(pDrawable->skeleton());
		if (pSlot == nullptr)continue;

		spAttachment* pAttachment = FindAttachment(pDrawable->skeleton()->data);
		if (pAttachment == nullptr)continue;

		/* Replace attachment name in spAttachmentTimeline if exists. */
		if (pSlot->attachment != nullptr)
		{
			const char* animationName = m_animationNames[m_nAnimationIndex].c_str();
			spAnimation* pAnimation = spSkeletonData_findAnimation(pDrawable->skeleton()->data, animationName);
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

FPoint2 CSpinePlayerC::getBaseSize() const noexcept
{
	return m_fBaseSize;
}

void CSpinePlayerC::setBaseSize(float fWidth, float fHeight)
{
	m_fBaseSize = { fWidth, fHeight };
	workOutDefaultScale();
	m_fDefaultOffset = m_fOffset;

	resetScale();
}

void CSpinePlayerC::resetBaseSize()
{
	workOutDefaultSize();
	workOutDefaultScale();

	m_fOffset = {};
	updatePosition();
	for (const auto& drawable : m_drawables)
	{
		/* Spine 2.1 does not have empty animation, so cannot be returned to default state without reloading. */
#ifndef SPINE_2_1
		spAnimationState_setEmptyAnimations(drawable->animationState(), 0.f);
#endif
		drawable->update(0.f);
	}

	workOutDefaultOffset();
	resetScale();
	restartAnimation();
}

FPoint2 CSpinePlayerC::getOffset() const noexcept
{
	return m_fOffset;
}

void CSpinePlayerC::setOffset(float fX, float fY) noexcept
{
	m_fOffset.x = fX;
	m_fOffset.y = fY;
}

float CSpinePlayerC::getSkeletonScale() const noexcept
{
	return m_fSkeletonScale;
}

void CSpinePlayerC::setSkeletonScale(float fScale) noexcept
{
	m_fSkeletonScale = fScale;
}

float CSpinePlayerC::getCanvasScale() const noexcept
{
	return m_fCanvasScale;
}
void CSpinePlayerC::setCanvasScale(float fScale) noexcept
{
	m_fCanvasScale = fScale;
}

float CSpinePlayerC::getTimeScale() const noexcept
{
	return m_fTimeScale;
}

void CSpinePlayerC::setTimeScale(float fTimeScale) noexcept
{
	m_fTimeScale = fTimeScale;
}
/*消去*/
void CSpinePlayerC::clearDrawables()
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
bool CSpinePlayerC::addDrawable(spSkeletonData* pSkeletonData)
{
	auto pDrawable = std::make_unique<CSpineDrawableC>(pSkeletonData);
	if (pDrawable.get() == nullptr)false;

	pDrawable->skeleton()->x = m_fBaseSize.x / 2;
	pDrawable->skeleton()->y = m_fBaseSize.y / 2;
	pDrawable->update(0.f);

	m_drawables.push_back(std::move(pDrawable));

	return true;
}

bool CSpinePlayerC::setupDrawables()
{
	workOutDefaultSize();
	workOutDefaultScale();

	for (const auto& pSkeletonDatum : m_skeletonData)
	{
		bool bRet = addDrawable(pSkeletonDatum.get());
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

	workOutDefaultOffset();

	restartAnimation();

	resetScale();

	return m_animationNames.size() > 0;
}
/*標準寸法算出*/
void CSpinePlayerC::workOutDefaultSize()
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
void CSpinePlayerC::workOutDefaultScale()
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
void CSpinePlayerC::updatePosition()
{
	for (const auto& drawable : m_drawables)
	{
		drawable->skeleton()->x = m_fBaseSize.x / 2 - m_fOffset.x;
		drawable->skeleton()->y = m_fBaseSize.y / 2 - m_fOffset.y;
	}
}
/*合成動作消去*/
void CSpinePlayerC::clearAnimationTracks()
{
	for (const auto& pDdrawble : m_drawables)
	{
		for (int iTrack = 1; iTrack < pDdrawble->animationState()->tracksCount; ++iTrack)
		{
#ifdef SPINE_2_1
			spAnimationState_clearTrack(pDdrawble->animationState(), iTrack);
#else
			spAnimationState_setEmptyAnimation(pDdrawble->animationState(), iTrack, 0.f);
#endif
		}
	}
}
