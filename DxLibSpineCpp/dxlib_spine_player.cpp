

#include "dxlib_spine_player.h"
#include "spine_loader.h"

CDxLibSpinePlayer::CDxLibSpinePlayer()
{

}

CDxLibSpinePlayer::~CDxLibSpinePlayer()
{

}

void CDxLibSpinePlayer::SetRenderWindow(HWND hRenderWnd)
{
	m_hRenderWnd = hRenderWnd;
}
/*ファイル取り込み*/
bool CDxLibSpinePlayer::SetSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary)
{
	if (atlasPaths.size() != skelPaths.size())return false;
	ClearDrawables();

	for (size_t i = 0; i < atlasPaths.size(); ++i)
	{
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonPath = skelPaths[i];

		std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(strAtlasPath.c_str(), &m_textureLoader);
		if (atlas.get() == nullptr)continue;

		std::shared_ptr<spine::SkeletonData> skeletonData = bIsBinary ?
			spine_loader::ReadBinarySkeletonFromFile(strSkeletonPath.c_str(), atlas.get(), 1.f) :
			spine_loader::ReadTextSkeletonFromFile(strSkeletonPath.c_str(), atlas.get(), 1.f);
		if (skeletonData.get() == nullptr)return false;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	WorkOutDefaultScale();

	return SetupDrawer();
}
/*メモリ取り込み*/
bool CDxLibSpinePlayer::SetSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary)
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

		std::shared_ptr<spine::SkeletonData> skeletonData = bIsBinary ? 
			spine_loader::ReadBinarySkeletonFromMemory(strSkeletonData, atlas.get(), 1.f) :
			spine_loader::ReadTextSkeletonFromMemory(strSkeletonData, atlas.get(), 1.f);
		if (skeletonData.get() == nullptr)return false;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	WorkOutDefaultScale();

	return SetupDrawer();
}
/*再描画*/
void CDxLibSpinePlayer::Redraw(float fDelta)
{
	if (!m_drawables.empty())
	{
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
	}
}
/*表示形式変更通知*/
void CDxLibSpinePlayer::OnStyleChanged()
{
	ResizeWindow();
}
/*拡縮変更*/
void CDxLibSpinePlayer::RescaleSkeleton(bool bUpscale, bool bWindowToBeResized)
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

	UpdateScaletonScale();
	if (bWindowToBeResized)
	{
		ResizeWindow();
		AdjustViewOffset();
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

	UpdateTimeScale();
}
/*速度・尺度・視点初期化*/
void CDxLibSpinePlayer::ResetScale()
{
	m_fTimeScale = 1.0f;
	m_fSkeletonScale = m_fDefaultScale;
	m_fOffset = m_fDefaultOffset;
	m_fViewOffset = DxLib::FLOAT2{};

	UpdateScaletonScale();
	UpdateTimeScale();
	ResizeWindow();
	AdjustViewOffset();
}
/*視点移動*/
void CDxLibSpinePlayer::MoveViewPoint(int iX, int iY)
{
	m_fOffset.u += iX;
	m_fOffset.v += iY;
	UpdatePosition();
}
/*動作移行*/
void CDxLibSpinePlayer::ShiftAnimation()
{
	++m_nAnimationIndex;
	if (m_nAnimationIndex >= m_animationNames.size())m_nAnimationIndex = 0;

	ClearAnimationTracks();
	UpdateAnimation();
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
/*乗算済み透過度有効・無効切り替え*/
void CDxLibSpinePlayer::SwitchPma()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->SwitchPma();
	}
}
/*槽溝指定合成方法採択可否*/
void CDxLibSpinePlayer::SwitchBlendModeAdoption()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->SwitchBlendModeAdoption();
	}
}
/*奥行き表現有効無効切り替え*/
bool CDxLibSpinePlayer::SwitchDepthBufferValidity()
{
	int iRet = DxLib::SetUseZBufferFlag(m_bDepthBufferEnabled ? FALSE : TRUE);
	if (iRet == -1)return false;

	iRet = DxLib::SetWriteZBufferFlag(m_bDepthBufferEnabled ? FALSE : TRUE);
	if (iRet == -1)return false;

	m_bDepthBufferEnabled ^= true;
	return true;
}
/*描画順切り替え*/
void CDxLibSpinePlayer::SwitchDrawOrder()
{
	m_bDrawOrderReversed ^= true;
}
/*現在の動作名と経過時間取得*/
std::string CDxLibSpinePlayer::GetCurrentAnimationNameWithTrackTime(float* fTrackTime)
{
	for (const auto& pDrawable : m_drawables)
	{
		auto& tracks = pDrawable->animationState->getTracks();
		for (size_t i = 0; i < tracks.size(); ++i)
		{
			spine::Animation* pAnimation = tracks[i]->getAnimation();
			if (pAnimation != nullptr)
			{
				if (fTrackTime != nullptr)
				{
					*fTrackTime = tracks[i]->getTrackTime();
				}
				return pAnimation->getName().buffer();
			}
		}
	}

	return std::string();
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

	const auto FindSlot = [this, &szSlotName]()
		->spine::Slot*
		{
			for (const auto& pDrawable : m_drawables)
			{
				for (size_t i = 0; i < pDrawable->skeleton->getSlots().size();++i)
				{
					const spine::String& slotName = pDrawable->skeleton->getDrawOrder()[i]->getData().getName();
					if (!slotName.isEmpty() && slotName == szSlotName)
					{
						return pDrawable->skeleton->getDrawOrder()[i];
					}
				}
			}

			return nullptr;
		};

	spine::Slot* pSlot = FindSlot();
	if (pSlot == nullptr)return false;

	const auto FindAttachment = [this, &szAttachmentName]()
		-> spine::Attachment*
		{
			for (const auto& skeletonDatum : m_skeletonData)
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
			}
			return nullptr;
		};

	spine::Attachment* pAttachment = FindAttachment();
	if (pAttachment == nullptr)return false;

	/* Replace attachment name in spine::AttachmentTimeline if exists. */
	for (const auto& skeletonDatum : m_skeletonData)
	{
		const auto& animationName = m_animationNames[m_nAnimationIndex];
		spine::Animation* pAnimation = skeletonDatum->findAnimation(animationName.c_str());
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

	return true;
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
/*描画器設定*/
bool CDxLibSpinePlayer::SetupDrawer()
{
	for (const auto& pSkeletonDatum : m_skeletonData)
	{
		auto pDrawable = std::make_shared<CDxLibSpineDrawer>(pSkeletonDatum.get());
		if (pDrawable.get() == nullptr)continue;

		pDrawable->timeScale = 1.0f;
		pDrawable->skeleton->setPosition(m_fBaseSize.u / 2, m_fBaseSize.v / 2);
		pDrawable->skeleton->setToSetupPose();
		pDrawable->skeleton->updateWorldTransform();

		m_drawables.push_back(std::move(pDrawable));

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

	UpdateAnimation();

	ResetScale();

	return m_animationNames.size() > 0;
}
/*既定尺度算出*/
void CDxLibSpinePlayer::WorkOutDefaultScale()
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
			/*If skeletonData does not store size, deduce from the attachment of the default skin.*/
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

					float fScale = pMeshAttachment->getWidth() > Constants::kMinAtlas && pMeshAttachment->getHeight() > Constants::kMinAtlas ? 1.f : 2.f;

					CompareDimention(pMeshAttachment->getWidth() * fScale, pMeshAttachment->getHeight() * fScale);
				}
			}
		}
	}

	m_fDefaultScale = 1.f;
	m_fDefaultOffset = DxLib::FLOAT2{};

	int iSkeletonWidth = static_cast<int>(m_fBaseSize.u);
	int iSkeletonHeight = static_cast<int>(m_fBaseSize.v);

	int iDesktopWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int iDesktopHeight = ::GetSystemMetrics(SM_CYSCREEN);

	if (iSkeletonWidth > iDesktopWidth || iSkeletonHeight > iDesktopHeight)
	{
		float fScaleX = static_cast<float>(iDesktopWidth) / iSkeletonWidth;
		float fScaleY = static_cast<float>(iDesktopHeight) / iSkeletonHeight;

		if (fScaleX > fScaleY)
		{
			m_fDefaultScale = fScaleY;

			/*
			* The sum of
			* (1) Centre-point difference between world- and locale-view.
			* (2) deviance from ideal scaling. (This becomes zero when ideally scaled.)
			* results in
			* (iSkeletonWidth - iDesktopWidth) / 2.f + (iDesktopWidth - iSkeletonWidth * fScaleY) / 2.f
			*  = (iSkeletonWidth * (1 - fScaleY)) / 2.f
			*/
			m_fDefaultOffset.u = iSkeletonWidth > iDesktopWidth ? (iSkeletonWidth * (1 - fScaleY)) / 2.f : 0.f;
			m_fDefaultOffset.v = iSkeletonHeight > iDesktopHeight ? (iSkeletonHeight - iDesktopHeight) / 2.f : 0.f;
		}
		else
		{
			m_fDefaultScale = fScaleX;

			m_fDefaultOffset.u = iSkeletonWidth > iDesktopWidth ? (iSkeletonWidth - iDesktopWidth) / 2.f : 0.f;
			m_fDefaultOffset.v = iSkeletonHeight > iDesktopHeight ? (iSkeletonHeight * (1 - fScaleX)) / 2.f : 0.f;
		}

	}
}
/*視点補正*/
void CDxLibSpinePlayer::AdjustViewOffset()
{
	if (m_hRenderWnd != nullptr)
	{
		RECT rc;
		::GetClientRect(m_hRenderWnd, &rc);

		int iClientWidth = rc.right - rc.left;
		int iClientHeight = rc.bottom - rc.top;

		int iDesktopWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int iDesktopHeight = ::GetSystemMetrics(SM_CYSCREEN);

		iClientWidth = iClientWidth < iDesktopWidth ? iClientWidth : iDesktopWidth;
		iClientHeight = iClientHeight < iDesktopHeight ? iClientHeight : iDesktopHeight;

		m_fViewOffset.u = (m_fBaseSize.u * m_fDefaultScale - iClientWidth) / 2.f;
		m_fViewOffset.v = (m_fBaseSize.v * m_fDefaultScale - iClientHeight) / 2.f;
	}
	UpdatePosition();
}
/*位置適用*/
void CDxLibSpinePlayer::UpdatePosition()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->skeleton->setPosition(m_fBaseSize.u / 2 - m_fOffset.u - m_fViewOffset.u, m_fBaseSize.v / 2 - m_fOffset.v - m_fViewOffset.v);
	}
}
/*尺度適用*/
void CDxLibSpinePlayer::UpdateScaletonScale()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->skeleton->setScaleX(m_fSkeletonScale);
		pDrawable->skeleton->setScaleY(m_fSkeletonScale);
	}
}
/*速度適用*/
void CDxLibSpinePlayer::UpdateTimeScale()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->timeScale = m_fTimeScale;
	}
}
/*動作適用*/
void CDxLibSpinePlayer::UpdateAnimation()
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
/*窓寸法調整*/
void CDxLibSpinePlayer::ResizeWindow()
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
		const auto HasWindowMenu = [&lStyle]()
			-> bool
			{
				return !((lStyle & WS_CAPTION) && (lStyle & WS_SYSMENU));
			};
		::AdjustWindowRect(&rect, lStyle, HasWindowMenu() ? FALSE : TRUE);
		::SetWindowPos(m_hRenderWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
	}
}
