
#include "spine_player.h"
#include "spine_loader.h"

CSpinePlayer::CSpinePlayer()
{

}

CSpinePlayer::~CSpinePlayer()
{

}

/*ファイル取り込み*/
bool CSpinePlayer::loadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
{
	if (atlasPaths.size() != skelPaths.size())return false;
	clearDrawables();

	for (size_t i = 0; i < atlasPaths.size(); ++i)
	{
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonPath = skelPaths[i];

		std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(strAtlasPath.c_str(), &m_textureLoader);
		if (atlas.get() == nullptr)continue;

		std::shared_ptr<spine::SkeletonData> skeletonData = isBinarySkel ?
			spine_loader::ReadBinarySkeletonFromFile(strSkeletonPath.c_str(), atlas.get(), 1.f) :
			spine_loader::ReadTextSkeletonFromFile(strSkeletonPath.c_str(), atlas.get(), 1.f);
		if (skeletonData.get() == nullptr)return false;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return setupDrawables();
}
/*メモリ取り込み*/
bool CSpinePlayer::loadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool isBinarySkel)
{
	if (atlasData.size() != skelData.size() || atlasData.size() != atlasPaths.size())return false;
	clearDrawables();

	for (size_t i = 0; i < atlasData.size(); ++i)
	{
		const std::string& strAtlasDatum = atlasData[i];
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonData = skelData[i];

		std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(strAtlasDatum.c_str(), static_cast<int>(strAtlasDatum.size()), strAtlasPath.c_str(), &m_textureLoader);
		if (atlas.get() == nullptr)continue;

		std::shared_ptr<spine::SkeletonData> skeletonData = isBinarySkel ?
			spine_loader::ReadBinarySkeletonFromMemory(reinterpret_cast<const unsigned char*>(strSkeletonData.data()), static_cast<int>(strSkeletonData.size()), atlas.get(), 1.f) :
			spine_loader::ReadTextSkeletonFromMemory(strSkeletonData.data(), atlas.get(), 1.f);
		if (skeletonData.get() == nullptr)return false;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return setupDrawables();
}

bool CSpinePlayer::addSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool isBinarySkel)
{
	if (m_drawables.empty() || szAtlasPath == nullptr || szSkelPath == nullptr)return false;

	std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(szAtlasPath, &m_textureLoader);
	if (atlas.get() == nullptr)return false;

	std::shared_ptr<spine::SkeletonData> skeletonData = isBinarySkel ?
		spine_loader::ReadBinarySkeletonFromFile(szSkelPath, atlas.get(), 1.f) :
		spine_loader::ReadTextSkeletonFromFile(szSkelPath, atlas.get(), 1.f);
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

size_t CSpinePlayer::getNumberOfSpines() const noexcept
{
	return m_drawables.size();
}

bool CSpinePlayer::hasSpineBeenLoaded() const noexcept
{
	return !m_drawables.empty();
}
/*状態更新*/
void CSpinePlayer::update(float fDelta)
{
	for (const auto& drawable : m_drawables)
	{
		drawable->update(fDelta * m_fTimeScale);
	}
}
/*速度・尺度・視点初期化*/
void CSpinePlayer::resetScale()
{
	m_fTimeScale = 1.0f;

	m_fSkeletonScale = m_fDefaultScale;
	m_fCanvasScale = m_fDefaultScale;
	m_fOffset = m_fDefaultOffset;

	updatePosition();
}
/*視点移動*/
void CSpinePlayer::addOffset(int iX, int iY)
{
	m_fOffset.x += iX / m_fSkeletonScale;
	m_fOffset.y += iY / m_fSkeletonScale;
	updatePosition();
}
/*動作移行*/
void CSpinePlayer::shiftAnimation()
{
	++m_nAnimationIndex;
	if (m_nAnimationIndex >= m_animationNames.size())m_nAnimationIndex = 0;

	clearAnimationTracks();
	restartAnimation();
}
/*装い移行*/
void CSpinePlayer::shiftSkin()
{
	if (m_skinNames.empty())return;

	++m_nSkinIndex;
	if (m_nSkinIndex >= m_skinNames.size())m_nSkinIndex = 0;

	setupSkin();
}

void CSpinePlayer::setAnimationByIndex(size_t nIndex)
{
	if (nIndex < m_animationNames.size())
	{
		m_nAnimationIndex = nIndex;
		restartAnimation();
	}
}

void CSpinePlayer::setAnimationByName(const char* szAnimationName)
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
void CSpinePlayer::restartAnimation(bool loop)
{
	if (m_nAnimationIndex >= m_animationNames.size())return;
	const char* szAnimationName = m_animationNames[m_nAnimationIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spine::Animation* pAnimation = pDrawable->skeleton()->getData()->findAnimation(szAnimationName);
		if (pAnimation != nullptr)
		{
			pDrawable->animationState()->setAnimation(0, pAnimation->getName(), loop);
		}
	}
}

void CSpinePlayer::setSkinByIndex(size_t nIndex)
{
	if (nIndex < m_skinNames.size())
	{
		m_nSkinIndex = nIndex;
		setupSkin();
	}
}

void CSpinePlayer::setSkinByName(const char* szSkinName)
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

void CSpinePlayer::setupSkin()
{
	if (m_nSkinIndex >= m_skinNames.size())return;
	const char* szSkinName = m_skinNames[m_nSkinIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spine::Skin* skin = pDrawable->skeleton()->getData()->findSkin(szSkinName);
		if (skin != nullptr)
		{
			pDrawable->skeleton()->setSkin(skin);
			pDrawable->skeleton()->setSlotsToSetupPose();
		}
	}
}
/*乗算済み透過度有効・無効切り替え*/
void CSpinePlayer::togglePma()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->premultiplyAlpha(!pDrawable->isAlphaPremultiplied());
	}
}
/*槽溝指定合成方法採択可否*/
void CSpinePlayer::toggleBlendModeAdoption()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->forceBlendModeNormal(!pDrawable->isBlendModeNormalForced());
	}
}
/* 乗算済み是否 */
bool CSpinePlayer::isAlphaPremultiplied(size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		return m_drawables[nDrawableIndex]->isAlphaPremultiplied();
	}

	return false;
}
/* 通常混色法強制是否*/
bool CSpinePlayer::isBlendModeNormalForced(size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		return m_drawables[nDrawableIndex]->isBlendModeNormalForced();
	}

	return false;
}

bool CSpinePlayer::isDrawOrderReversed() const noexcept
{
	return m_isDrawOrderReversed;
}

bool CSpinePlayer::premultiplyAlpha(bool premultiplied, size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		m_drawables[nDrawableIndex]->premultiplyAlpha(premultiplied);
		return true;
	}

	return false;
}

bool CSpinePlayer::forceBlendModeNormal(bool toForce, size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		m_drawables[nDrawableIndex]->forceBlendModeNormal(toForce);
		return true;
	}

	return false;
}

void CSpinePlayer::setDrawOrder(bool reversed)
{
	m_isDrawOrderReversed = reversed;
}
/*現在の動作名と経過時間取得*/
std::string CSpinePlayer::getCurrentAnimationName()
{
	for (const auto& pDrawable : m_drawables)
	{
		auto& tracks = pDrawable->animationState()->getTracks();
		for (size_t i = 0; i < tracks.size(); ++i)
		{
			spine::Animation* pAnimation = tracks[i]->getAnimation();
			if (pAnimation != nullptr)
			{
				return pAnimation->getName().buffer();
			}
		}
	}

	return std::string();
}

void CSpinePlayer::getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd)
{
	for (const auto& pDrawable : m_drawables)
	{
		auto& tracks = pDrawable->animationState()->getTracks();
		for (size_t i = 0; i < tracks.size(); ++i)
		{
			spine::Animation* pAnimation = tracks[i]->getAnimation();
			if (pAnimation != nullptr)
			{
				if (fTrack != nullptr)*fTrack = tracks[i]->getTrackTime();
				if (fLast != nullptr)*fLast = tracks[i]->getAnimationLast();
				if (fStart != nullptr)*fStart = tracks[i]->getAnimationStart();
				if (fEnd != nullptr)*fEnd = tracks[i]->getAnimationEnd();

				return;
			}
		}
	}
}

float CSpinePlayer::getAnimationDuration(const char* animationName)
{
	for (const auto& pDrawable : m_drawables)
	{
		spine::Animation* pAnimation = pDrawable->skeleton()->getData()->findAnimation(animationName);
		if (pAnimation != nullptr)
		{
			return pAnimation->getDuration();
		}
	}

	return 0.f;
}

/*槽溝名称引き渡し*/
const std::vector<std::string>& CSpinePlayer::getSlotNames() const noexcept
{
	return m_slotNames;
}
/*装い名称引き渡し*/
const std::vector<std::string>& CSpinePlayer::getSkinNames() const noexcept
{
	return m_skinNames;
}
/*動作名称引き渡し*/
const std::vector<std::string>& CSpinePlayer::getAnimationNames() const noexcept
{
	return m_animationNames;
}
/*描画除外リスト設定*/
void CSpinePlayer::setSlotsToExclude(const std::vector<std::string>& slotNames)
{
	spine::Vector<spine::String> leaveOutList;
	for (const auto& slotName : slotNames)
	{
		leaveOutList.add(slotName.c_str());
	}

	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->setLeaveOutList(leaveOutList);
	}
}
/*装い合成*/
void CSpinePlayer::mixSkins(const std::vector<std::string>& skinNames)
{
	if (m_nSkinIndex >= m_skinNames.size())return;
	const auto& currentSkinName = m_skinNames[m_nSkinIndex];

	for (const auto& pDrawble : m_drawables)
	{
		spine::Skin* skinToSet = pDrawble->skeleton()->getData()->findSkin(currentSkinName.c_str());
		if (skinToSet == nullptr)continue;

		for (const auto& skinName : skinNames)
		{
			if (currentSkinName != skinName)
			{
				spine::Skin* skinToAdd = pDrawble->skeleton()->getData()->findSkin(skinName.c_str());
				if (skinToAdd != nullptr)
				{
					skinToSet->addSkin(skinToAdd);
				}
			}
		}
		pDrawble->skeleton()->setSkin(skinToSet);
		pDrawble->skeleton()->setSlotsToSetupPose();
	}
}
/* 動作予約 */
void CSpinePlayer::addAnimationTracks(const std::vector<std::string>& animationNames, bool loop)
{
	clearAnimationTracks();

	if (m_nAnimationIndex >= m_animationNames.size())return;
	const auto& currentAnimationName = m_animationNames[m_nAnimationIndex];

	for (const auto& pDrawable : m_drawables)
	{
		if (pDrawable->skeleton()->getData()->findAnimation(currentAnimationName.c_str()) == nullptr)continue;

		int iTrack = 1;
		for (const auto& animationName : animationNames)
		{
			if (animationName != currentAnimationName)
			{
				spine::Animation* animation = pDrawable->skeleton()->getData()->findAnimation(animationName.c_str());
				if (animation != nullptr)
				{
					pDrawable->animationState()->addAnimation(iTrack, animation, loop, 0.f);
					++iTrack;
				}
			}
		}
	}
}

void CSpinePlayer::mixAnimations(const char* fadeOutAnimationName, const char* fadeInAnimationName, float mixTime)
{
	for (const auto& pDrawable : m_drawables)
	{
		spine::Animation* fadeOutAnimation = pDrawable->skeleton()->getData()->findAnimation(fadeOutAnimationName);
		spine::Animation* fadeInAnimation = pDrawable->skeleton()->getData()->findAnimation(fadeInAnimationName);
		if (fadeOutAnimation != nullptr && fadeInAnimation != nullptr)
		{
			const auto& animationStateData = pDrawable->animationState()->getData();
			animationStateData->setMix(fadeOutAnimation, fadeInAnimation, mixTime);
		}
	}
}

void CSpinePlayer::clearMixedAnimation()
{
#ifdef SPINE_4_1_OR_LATER
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->animationState()->getData()->clear();
	}
#endif
}

void CSpinePlayer::setSlotExcludeCallback(bool(*pFunc)(const char*, size_t))
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->setLeaveOutCallback(pFunc);
	}
}

std::unordered_map<std::string, std::vector<std::string>> CSpinePlayer::getSlotNamesWithTheirAttachments()
{
	std::unordered_map<std::string, std::vector<std::string>> slotAttachmentMap;

	/* Default skin, if exists, contains all the attachments including those not attached to any slots. */
	for (const auto& skeletonDatum : m_skeletonData)
	{
		spine::Skin* pSkin = skeletonDatum->getDefaultSkin();

		auto& slots = skeletonDatum->getSlots();
		for (size_t i = 0; i < slots.size(); ++i)
		{
			spine::Vector<spine::Attachment*> pAttachments;
			pSkin->findAttachmentsForSlot(i, pAttachments);
			if (pAttachments.size() > 1)
			{
				std::vector<std::string> attachmentNames;

				for (size_t ii = 0; ii < pAttachments.size(); ++ii)
				{
					const char* szName = pAttachments[ii]->getName().buffer();
					const auto& iter = std::find(attachmentNames.begin(), attachmentNames.end(), szName);
					if (iter == attachmentNames.cend())attachmentNames.push_back(szName);
				}

				slotAttachmentMap.insert({ slots[i]->getName().buffer(), attachmentNames });
			}
		}
	}

	return slotAttachmentMap;
}

bool CSpinePlayer::replaceAttachment(const char* szSlotName, const char* szAttachmentName)
{
	if (szSlotName == nullptr || szAttachmentName == nullptr)return false;

	const auto FindSlot = [&szSlotName](spine::Skeleton* const skeleton)
		-> spine::Slot*
		{
			for (size_t i = 0; i < skeleton->getSlots().size(); ++i)
			{
				const spine::String& slotName = skeleton->getDrawOrder()[i]->getData().getName();
				if (!slotName.isEmpty() && slotName == szSlotName)
				{
					return skeleton->getDrawOrder()[i];
				}
			}
			return nullptr;
		};

	const auto FindAttachment = [&szAttachmentName](spine::SkeletonData* const skeletonDatum)
		->spine::Attachment*
		{
			spine::Skin::AttachmentMap::Entries attachmentMapEntries = skeletonDatum->getDefaultSkin()->getAttachments();
			for (; attachmentMapEntries.hasNext();)
			{
				spine::Skin::AttachmentMap::Entry attachmentMapEntry = attachmentMapEntries.next();

				if (attachmentMapEntry._name == szAttachmentName)
				{
					return attachmentMapEntry._attachment;
				}
			}
			return nullptr;
		};

	for (const auto& pDrawable : m_drawables)
	{
		spine::Slot* pSlot = FindSlot(pDrawable->skeleton());
		if (pSlot == nullptr)continue;

		spine::Attachment* pAttachment = FindAttachment(pDrawable->skeleton()->getData());
		if (pAttachment == nullptr)continue;

		/* Replace attachment name in spine::AttachmentTimeline if exists. */
		if (pSlot->getAttachment() != nullptr)
		{
			const char* animationName = m_animationNames[m_nAnimationIndex].c_str();
			spine::Animation* pAnimation = pDrawable->skeleton()->getData()->findAnimation(animationName);
			if (pAnimation == nullptr)continue;

			spine::Vector<spine::Timeline*>& timelines = pAnimation->getTimelines();
			for (size_t i = 0; i < timelines.size(); ++i)
			{
				if (timelines[i]->getRTTI().isExactly(spine::AttachmentTimeline::rtti))
				{
					const auto& attachmentTimeline = static_cast<spine::AttachmentTimeline*>(timelines[i]);

					spine::Vector<spine::String>& attachmentNames = attachmentTimeline->getAttachmentNames();
					for (size_t ii = 0; ii < attachmentNames.size(); ++ii)
					{
						const char* szName = attachmentNames[ii].buffer();
						if (szName == nullptr)continue;

						if (strcmp(szName, pSlot->getAttachment()->getName().buffer()) == 0)
						{
							attachmentNames[ii] = szAttachmentName;
						}
					}
				}
			}
		}

		pSlot->setAttachment(pAttachment);
	}

	return true;
}
/*寸法受け渡し*/
FPoint2 CSpinePlayer::getBaseSize() const noexcept
{
	return m_fBaseSize;
}

void CSpinePlayer::setBaseSize(float fWidth, float fHeight)
{
	m_fBaseSize = { fWidth, fHeight };
	workOutDefaultScale();
	m_fDefaultOffset = m_fOffset;

	resetScale();
}

void CSpinePlayer::resetBaseSize()
{
	workOutDefaultSize();
	workOutDefaultScale();

	m_fOffset = {};
	updatePosition();
	for (const auto& drawable : m_drawables)
	{
		drawable->animationState()->setEmptyAnimations(0.f);
		drawable->update(0.f);
	}

	workOutDefaultOffset();
	resetScale();
	restartAnimation();
}

FPoint2 CSpinePlayer::getOffset() const noexcept
{
	return m_fOffset;
}

void CSpinePlayer::setOffset(float fX, float fY) noexcept
{
	m_fOffset.x = fX;
	m_fOffset.y = fY;
}

float CSpinePlayer::getSkeletonScale() const noexcept
{
	return m_fSkeletonScale;
}

void CSpinePlayer::setSkeletonScale(float fScale)
{
	m_fSkeletonScale = fScale;
}

float CSpinePlayer::getCanvasScale() const noexcept
{
	return m_fCanvasScale;
}
void CSpinePlayer::setCanvasScale(float fScale) noexcept
{
	m_fCanvasScale = fScale;
}

float CSpinePlayer::getTimeScale() const noexcept
{
	return m_fTimeScale;
}

void CSpinePlayer::setTimeScale(float fTimeScale) noexcept
{
	m_fTimeScale = fTimeScale;
}
/*消去*/
void CSpinePlayer::clearDrawables()
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

bool CSpinePlayer::addDrawable(spine::SkeletonData* pSkeletonData)
{
	if (pSkeletonData == nullptr)return false;
	auto pDrawable = std::make_unique<CSpineDrawable>(pSkeletonData);
	if (pDrawable.get() == nullptr)return false;

	pDrawable->skeleton()->setPosition(m_fBaseSize.x / 2, m_fBaseSize.y / 2);
	pDrawable->update(0.f);

	m_drawables.push_back(std::move(pDrawable));

	return true;
}

bool CSpinePlayer::setupDrawables()
{
	workOutDefaultSize();
	workOutDefaultScale();

	for (const auto& pSkeletonDatum : m_skeletonData)
	{
		bool bRet = addDrawable(pSkeletonDatum.get());
		if (!bRet)continue;

		auto& animations = pSkeletonDatum->getAnimations();
		for (size_t i = 0; i < animations.size(); ++i)
		{
			const char* szAnimationName = animations[i]->getName().buffer();
			if (szAnimationName == nullptr)continue;

			const auto& iter = std::find(m_animationNames.begin(), m_animationNames.end(), szAnimationName);
			if (iter == m_animationNames.cend())m_animationNames.push_back(szAnimationName);
		}

		auto& skins = pSkeletonDatum->getSkins();
		for (size_t i = 0; i < skins.size(); ++i)
		{
			const char* szSkinName = skins[i]->getName().buffer();
			if (szSkinName == nullptr)continue;

			const auto& iter = std::find(m_skinNames.begin(), m_skinNames.end(), szSkinName);
			if (iter == m_skinNames.cend())m_skinNames.push_back(szSkinName);
		}

		auto& slots = pSkeletonDatum->getSlots();
		for (size_t ii = 0; ii < slots.size(); ++ii)
		{
			const char* szName = slots[ii]->getName().buffer();
			const auto& iter = std::find(m_slotNames.begin(), m_slotNames.end(), szName);
			if (iter == m_slotNames.cend())m_slotNames.push_back(szName);
		}
	}

	workOutDefaultOffset();

	restartAnimation();

	resetScale();

	return m_animationNames.size() > 0;
}
/*基準寸法算出*/
void CSpinePlayer::workOutDefaultSize()
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
		if (pSkeletonData.get()->getWidth() > 0 && pSkeletonData.get()->getHeight())
		{
			CompareDimention(pSkeletonData.get()->getWidth(), pSkeletonData.get()->getHeight());
		}
		else
		{
			const auto FindDefaultSkinAttachment = [&pSkeletonData]()
				-> spine::Attachment*
				{
					spine::Skin::AttachmentMap::Entries attachmentMapEntries = pSkeletonData.get()->getDefaultSkin()->getAttachments();
					for (; attachmentMapEntries.hasNext();)
					{
						spine::Skin::AttachmentMap::Entry attachmentMapEntry = attachmentMapEntries.next();
						if (attachmentMapEntry._slotIndex == 0)
						{
							return attachmentMapEntry._attachment;
						}
					}
					return nullptr;
				};

			spine::Attachment* pAttachment = FindDefaultSkinAttachment();
			if (pAttachment == nullptr)continue;

			if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
			{
				spine::RegionAttachment* pRegionAttachment = (spine::RegionAttachment*)pAttachment;

				CompareDimention(pRegionAttachment->getWidth() * pRegionAttachment->getScaleX(), pRegionAttachment->getHeight() * pRegionAttachment->getScaleY());
			}
			else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
			{
				spine::MeshAttachment* pMeshAttachment = (spine::MeshAttachment*)pAttachment;

				float fScale =
					::isgreater(pMeshAttachment->getWidth(), Constants::kMinAtlas) &&
					::isgreater(pMeshAttachment->getHeight(), Constants::kMinAtlas) ? 1.f : 2.f;

				CompareDimention(pMeshAttachment->getWidth() * fScale, pMeshAttachment->getHeight() * fScale);
			}
		}
	}
}

/*位置適用*/
void CSpinePlayer::updatePosition()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->skeleton()->setPosition(m_fBaseSize.x / 2 - m_fOffset.x, m_fBaseSize.y / 2 - m_fOffset.y);
	}
}
/*合成動作消去*/
void CSpinePlayer::clearAnimationTracks()
{
	for (const auto& pDrawable : m_drawables)
	{
		const auto& trackEntry = pDrawable->animationState()->getTracks();
		for (size_t iTrack = 1; iTrack < trackEntry.size(); ++iTrack)
		{
			pDrawable->animationState()->setEmptyAnimation(iTrack, 0.f);
		}
	}
}
