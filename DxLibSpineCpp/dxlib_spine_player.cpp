

#include "dxlib_spine_player.h"
#include "spine_loader.h"

CDxLibSpinePlayer::CDxLibSpinePlayer()
{

}

CDxLibSpinePlayer::~CDxLibSpinePlayer()
{

}

/*ファイル取り込み*/
bool CDxLibSpinePlayer::LoadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
{
	if (atlasPaths.size() != skelPaths.size())return false;
	ClearDrawables();

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

	return SetupDrawer();
}
/*メモリ取り込み*/
bool CDxLibSpinePlayer::LoadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool isBinarySkel)
{
	if (atlasData.size() != skelData.size() || atlasData.size() != atlasPaths.size())return false;
	ClearDrawables();

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

	return SetupDrawer();
}
/*ファイルから追加*/
bool CDxLibSpinePlayer::AddSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool bBinary)
{
	if (m_drawables.empty() || szAtlasPath == nullptr || szSkelPath == nullptr)return false;

	std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(szAtlasPath, &m_textureLoader);
	if (atlas.get() == nullptr)return false;

	std::shared_ptr<spine::SkeletonData> skeletonData = bBinary ?
		spine_loader::ReadBinarySkeletonFromFile(szSkelPath, atlas.get(), 1.f) :
		spine_loader::ReadTextSkeletonFromFile(szSkelPath, atlas.get(), 1.f);
	if (skeletonData.get() == nullptr)return false;

	bool bRet = AddDrawable(skeletonData.get());
	if (!bRet)return false;

	m_atlases.push_back(std::move(atlas));
	m_skeletonData.push_back(std::move(skeletonData));
	if (m_bDrawOrderReversed)
	{
		std::rotate(m_drawables.rbegin(), m_drawables.rbegin() + 1, m_drawables.rend());
	}

	RestartAnimation();
	ResetScale();

	return true;
}

size_t CDxLibSpinePlayer::GetNumberOfSpines() const
{
	return m_drawables.size();
}

bool CDxLibSpinePlayer::HasLoaded() const
{
	return !m_drawables.empty();
}

void CDxLibSpinePlayer::Update(float fDelta)
{
	for (const auto& drawable : m_drawables)
	{
		drawable->Update(fDelta * m_fTimeScale);
	}
}
/*再描画*/
void CDxLibSpinePlayer::Redraw()
{
	if (!m_drawables.empty())
	{
		SetTransformMatrix();

		if (!m_bDrawOrderReversed)
		{
			for (size_t i = 0; i < m_drawables.size(); ++i)
			{
				m_drawables[i]->Draw();
			}
		}
		else
		{
			for (long long i = m_drawables.size() - 1; i >= 0; --i)
			{
				m_drawables[i]->Draw();
			}
		}

		DxLib::ResetTransformTo2D();
	}
}
/*拡縮変更*/
void CDxLibSpinePlayer::RescaleSkeleton(bool bUpscale)
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

void CDxLibSpinePlayer::RescaleCanvas(bool bUpscale)
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
void CDxLibSpinePlayer::RescaleTime(bool bHasten)
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
/*速度・尺度・視点初期化*/
void CDxLibSpinePlayer::ResetScale()
{
	m_fTimeScale = 1.0f;
	m_fSkeletonScale = m_fDefaultScale;
	m_fCanvasScale = m_fDefaultScale;
	m_fOffset = m_fDefaultOffset;

	UpdatePosition();
}
/*位置移動*/
void CDxLibSpinePlayer::MoveViewPoint(int iX, int iY)
{
	m_fOffset.u += iX / m_fSkeletonScale;
	m_fOffset.v += iY / m_fSkeletonScale;
	UpdatePosition();
}
/*動作移行*/
void CDxLibSpinePlayer::ShiftAnimation()
{
	++m_nAnimationIndex;
	if (m_nAnimationIndex >= m_animationNames.size())m_nAnimationIndex = 0;

	ClearAnimationTracks();
	RestartAnimation();
}
/*装い移行*/
void CDxLibSpinePlayer::ShiftSkin()
{
	if (m_skinNames.empty())return;

	++m_nSkinIndex;
	if (m_nSkinIndex >= m_skinNames.size())m_nSkinIndex = 0;

	const char* szSkinName = m_skinNames[m_nSkinIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spine::Skin* skin = pDrawable->skeleton->getData()->findSkin(szSkinName);
		if (skin != nullptr)
		{
			pDrawable->skeleton->setSkin(skin);
			pDrawable->skeleton->setSlotsToSetupPose();
		}
	}
}

void CDxLibSpinePlayer::SetAnimationByIndex(size_t nIndex)
{
	if (nIndex < m_animationNames.size())
	{
		m_nAnimationIndex = nIndex;
		RestartAnimation();
	}
}
/*動作適用*/
void CDxLibSpinePlayer::RestartAnimation()
{
	if (m_nAnimationIndex >= m_animationNames.size())return;
	const char* szAnimationName = m_animationNames[m_nAnimationIndex].c_str();

	for (const auto& pDrawable : m_drawables)
	{
		spine::Animation* pAnimation = pDrawable->skeleton->getData()->findAnimation(szAnimationName);
		if (pAnimation != nullptr)
		{
			pDrawable->animationState->setAnimation(0, pAnimation->getName(), true);
		}
	}
}
/*乗算済み透過度有効・無効切り替え*/
void CDxLibSpinePlayer::TogglePma()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->isAlphaPremultiplied ^= true;
	}
}
/*槽溝指定合成方法採択可否*/
void CDxLibSpinePlayer::ToggleBlendModeAdoption()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->isToForceBlendModeNormal ^= true;
	}
}
/*描画順切り替え*/
void CDxLibSpinePlayer::ToggleDrawOrder()
{
	m_bDrawOrderReversed ^= true;
}
/* 現在設定されている動作名取得*/
std::string CDxLibSpinePlayer::GetCurrentAnimationName()
{
	for (const auto& pDrawable : m_drawables)
	{
		auto& tracks = pDrawable->animationState->getTracks();
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

void CDxLibSpinePlayer::GetCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd)
{
	for (const auto& pDrawable : m_drawables)
	{
		auto& tracks = pDrawable->animationState->getTracks();
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
/*槽溝名称取得*/
std::vector<std::string> CDxLibSpinePlayer::GetSlotNames()
{
	std::vector<std::string> slotNames;
	for (const auto& skeletonDatum : m_skeletonData)
	{
		auto& slots = skeletonDatum->getSlots();
		for (size_t ii = 0; ii < slots.size(); ++ii)
		{
			const char* szName = slots[ii]->getName().buffer();
			const auto iter = std::find(slotNames.begin(), slotNames.end(), szName);
			if (iter == slotNames.cend())slotNames.push_back(szName);
		}
	}

	return slotNames;
}
/*装い名称引き渡し*/
const std::vector<std::string>& CDxLibSpinePlayer::GetSkinNames() const
{
	return m_skinNames;
}
/*動作名称引き渡し*/
const std::vector<std::string>& CDxLibSpinePlayer::GetAnimationNames() const
{
	return m_animationNames;
}
/*描画除外リスト設定*/
void CDxLibSpinePlayer::SetSlotsToExclude(const std::vector<std::string>& slotNames)
{
	spine::Vector<spine::String> leaveOutList;
	for (const auto& slotName : slotNames)
	{
		leaveOutList.add(slotName.c_str());
	}

	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->SetLeaveOutList(leaveOutList);
	}
}
/*装い合成*/
void CDxLibSpinePlayer::MixSkins(const std::vector<std::string>& skinNames)
{
	if (m_nSkinIndex >= m_skinNames.size())return;
	const auto& currentSkinName = m_skinNames[m_nSkinIndex];

	for (const auto& pDrawble : m_drawables)
	{
		spine::Skin* skinToSet = pDrawble->skeleton->getData()->findSkin(currentSkinName.c_str());
		if (skinToSet == nullptr)continue;

		for (const auto& skinName : skinNames)
		{
			if (currentSkinName != skinName)
			{
				spine::Skin* skinToAdd = pDrawble->skeleton->getData()->findSkin(skinName.c_str());
				if (skinToAdd != nullptr)
				{
					skinToSet->addSkin(skinToAdd);
				}
			}
		}
		pDrawble->skeleton->setSkin(skinToSet);
		pDrawble->skeleton->setSlotsToSetupPose();
	}
}
/*動作合成*/
void CDxLibSpinePlayer::MixAnimations(const std::vector<std::string>& animationNames)
{
	ClearAnimationTracks();

	if (m_nAnimationIndex >= m_animationNames.size())return;
	const auto& currentAnimationName = m_animationNames[m_nAnimationIndex];

	for (const auto& pDrawable : m_drawables)
	{
		if (pDrawable->skeleton->getData()->findAnimation(currentAnimationName.c_str()) == nullptr)continue;

		int iTrack = 1;
		for (const auto& animationName : animationNames)
		{
			if (animationName != currentAnimationName)
			{
				spine::Animation* animation = pDrawable->skeleton->getData()->findAnimation(animationName.c_str());
				if (animation != nullptr)
				{
					pDrawable->animationState->addAnimation(iTrack, animation, false, 0.f);
					++iTrack;
				}
			}
		}
	}
}
/*差し替え可能な槽溝名称取得*/
std::unordered_map<std::string, std::vector<std::string>> CDxLibSpinePlayer::GetSlotNamesWithTheirAttachments()
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
/*差し替え*/
bool CDxLibSpinePlayer::ReplaceAttachment(const char* szSlotName, const char* szAttachmentName)
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
		spine::Slot* pSlot = FindSlot(pDrawable->skeleton);
		if (pSlot == nullptr)continue;

		spine::Attachment* pAttachment = FindAttachment(pDrawable->skeleton->getData());
		if (pAttachment == nullptr)continue;

		/* Replace attachment name in spine::AttachmentTimeline if exists. */
		if (pSlot->getAttachment() != nullptr)
		{
			const char* animationName = m_animationNames[m_nAnimationIndex].c_str();
			spine::Animation* pAnimation = pDrawable->skeleton->getData()->findAnimation(animationName);
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
void CDxLibSpinePlayer::GetBaseSize(float* fWidth, float* fHeight) const
{
	if (fWidth != nullptr)*fWidth = m_fBaseSize.u;
	if (fHeight != nullptr)*fHeight = m_fBaseSize.v;
}
/*尺度受け渡し*/
float CDxLibSpinePlayer::GetCanvasScale() const
{
	return m_fCanvasScale;
}
/*消去*/
void CDxLibSpinePlayer::ClearDrawables()
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
bool CDxLibSpinePlayer::AddDrawable(spine::SkeletonData* const pSkeletonData)
{
	if (pSkeletonData == nullptr)return false;
	auto pDrawable = std::make_shared<CDxLibSpineDrawer>(pSkeletonData);
	if (pDrawable.get() == nullptr)return false;

	pDrawable->skeleton->setPosition(m_fBaseSize.u / 2, m_fBaseSize.v / 2);
	pDrawable->Update(0.f);

	m_drawables.push_back(std::move(pDrawable));

	return true;
}
/*描画器設定*/
bool CDxLibSpinePlayer::SetupDrawer()
{
	WorkOutDefaultSize();
	WorkOutDefaultScale();

	for (const auto& pSkeletonDatum : m_skeletonData)
	{
		bool bRet = AddDrawable(pSkeletonDatum.get());
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
	}

	DxLib::FLOAT4 rect = GetBoundingBox();
	m_fDefaultOffset.u = rect.x;
	m_fDefaultOffset.v = rect.y;

	RestartAnimation();

	ResetScale();

	return m_animationNames.size() > 0;
}
DxLib::FLOAT4 CDxLibSpinePlayer::GetBoundingBox()
{
	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;
	float fMaxWidth = -FLT_MAX;
	float fMaxHeight = -FLT_MAX;

	for (const auto& m_drawable : m_drawables)
	{
		DxLib::FLOAT4 rect = m_drawable->GetBoundingBox();
		fMinX = (std::min)(fMinX, rect.x);
		fMinY = (std::min)(fMinY, rect.y);
		fMaxWidth = (std::max)(fMaxWidth, rect.z);
		fMaxHeight = (std::max)(fMaxHeight, rect.w);
	}

	DxLib::FLOAT4 bounds =
	{
		fMinX == FLT_MAX ? 0 : fMinX,
		fMinY == FLT_MAX ? 0 : fMinY,
		fMaxWidth == -FLT_MAX ? 0 : fMaxWidth,
		fMaxHeight == -FLT_MAX ? 0 : fMaxHeight,
	};

	return bounds;
}
/*標準寸法算出*/
void CDxLibSpinePlayer::WorkOutDefaultSize()
{
	if (m_skeletonData.empty())return;

	float fMaxSize = 0.f;
	const auto CompareDimention = [this, &fMaxSize](float fWidth, float fHeight)
		-> void
		{
			if (fWidth > 0.f && fHeight > 0.f && fWidth * fHeight > fMaxSize)
			{
				m_fBaseSize.u = fWidth;
				m_fBaseSize.v = fHeight;
				fMaxSize = fWidth * fHeight;
			}
		};

	for (const auto& pSkeletonData : m_skeletonData)
	{
		if (pSkeletonData->getWidth() > 0.f && pSkeletonData->getHeight() > 0.f)
		{
			CompareDimention(pSkeletonData->getWidth(), pSkeletonData->getHeight());
		}
		else
		{
			const auto FindDefaultSkinAttachment = [&pSkeletonData]()
				-> spine::Attachment*
				{
					spine::Skin::AttachmentMap::Entries attachmentMapEntries = pSkeletonData->getDefaultSkin()->getAttachments();
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
			if (pAttachment != nullptr)
			{
				if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
				{
					spine::RegionAttachment* pRegionAttachment = (spine::RegionAttachment*)pAttachment;

					CompareDimention(pRegionAttachment->getWidth() * pRegionAttachment->getScaleX(), pRegionAttachment->getHeight() * pRegionAttachment->getScaleY());
				}
				else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
				{
					spine::MeshAttachment* pMeshAttachment = (spine::MeshAttachment*)pAttachment;

					float fScale =::isgreater(pMeshAttachment->getWidth(), Constants::kMinAtlas) &&
						::isgreater(pMeshAttachment->getHeight(), Constants::kMinAtlas) ? 1.f : 2.f;

					CompareDimention(pMeshAttachment->getWidth() * fScale, pMeshAttachment->getHeight() * fScale);
				}
			}
		}
	}
}
/*既定尺度算出*/
void CDxLibSpinePlayer::WorkOutDefaultScale()
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
void CDxLibSpinePlayer::UpdatePosition()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->skeleton->setPosition(m_fBaseSize.u / 2 - m_fOffset.u, m_fBaseSize.v / 2 - m_fOffset.v);
	}
}
/*合成動作消去*/
void CDxLibSpinePlayer::ClearAnimationTracks()
{
	for (const auto& pDrawable : m_drawables)
	{
		const auto& trackEntry = pDrawable->animationState->getTracks();
		for (size_t iTrack = 1; iTrack < trackEntry.size(); ++iTrack)
		{
			pDrawable->animationState->setEmptyAnimation(iTrack, 0.f);
		}
	}
}

void CDxLibSpinePlayer::SetTransformMatrix() const
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
