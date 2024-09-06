

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
		const std::string& strAtlasPath = atlasPaths.at(i);
		const std::string& strSkeletonPath = skelPaths.at(i);

		m_atlases.emplace_back(std::make_unique<spine::Atlas>(strAtlasPath.c_str(), &m_textureLoader));

		std::shared_ptr<spine::SkeletonData> skeletonData = bIsBinary ? spine_loader::readBinarySkeletonFromFile(strSkeletonPath.c_str(), m_atlases.back().get(), 1.f) : spine_loader::readTextSkeletonFromFile(strSkeletonPath.c_str(), m_atlases.back().get(), 1.f);
		if (skeletonData == nullptr)return false;

		m_skeletonData.emplace_back(skeletonData);
	}

	if (m_skeletonData.empty())return false;

	WorkOutDefaultScale();
	ResizeWindow();

	return SetupDrawer();
}
/*メモリ取り込み*/
bool CDxLibSpinePlayer::SetSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary)
{
	if (atlasData.size() != skelData.size() || atlasData.size() != atlasPaths.size())return false;
	ClearDrawables();

	for (size_t i = 0; i < atlasData.size(); ++i)
	{
		const std::string& strAtlasDatum = atlasData.at(i);
		const std::string& strAtlasPath = atlasPaths.at(i);
		const std::string& strSkeletonData = skelData.at(i);

		m_atlases.emplace_back(std::make_unique<spine::Atlas>(strAtlasDatum.c_str(), static_cast<int>(strAtlasDatum.size()), strAtlasPath.c_str(), &m_textureLoader));

		std::shared_ptr<spine::SkeletonData> skeletonData = bIsBinary ? spine_loader::readBinarySkeletonFromMemory(strSkeletonData, m_atlases.back().get(), 1.f) : spine_loader::readTextSkeletonFromMemory(strSkeletonData, m_atlases.back().get(), 1.f);
		if (skeletonData == nullptr)return false;

		m_skeletonData.emplace_back(skeletonData);
	}

	if (m_skeletonData.empty())return false;

	WorkOutDefaultScale();
	ResizeWindow();

	return SetupDrawer();
}
/*表示形式変更通知*/
void CDxLibSpinePlayer::OnStyleChanged()
{
	ResizeWindow();
}
/*拡縮変更*/
void CDxLibSpinePlayer::RescaleSkeleton(bool bUpscale)
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
	m_fSkeletonScale = m_fDefaultWindowScale;
	m_fOffset = m_fDefaultOffset;

	UpdateScaletonScale();
	UpdateTimeScale();
	MoveViewPoint(0, 0);
	ResizeWindow();
}
/*視点移動*/
void CDxLibSpinePlayer::MoveViewPoint(int iX, int iY)
{
	m_fOffset.u += iX;
	m_fOffset.v += iY;
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->skeleton->setPosition(m_fBaseSize.u / 2 - m_fOffset.u, m_fBaseSize.v / 2 - m_fOffset.v);
	}
}
/*動作移行*/
void CDxLibSpinePlayer::ShiftAnimation()
{
	++m_nAnimationIndex;
	if (m_nAnimationIndex > m_animationNames.size() - 1)m_nAnimationIndex = 0;

	ClearAnimationTracks();

	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		spine::Animation* animation = m_skeletonData.at(i).get()->findAnimation(m_animationNames.at(m_nAnimationIndex).c_str());
		if (animation != nullptr)
		{
			m_drawables.at(i).get()->state->setAnimation(0, animation->getName(), true);
		}
	}
}
/*装い移行*/
void CDxLibSpinePlayer::ShiftSkin()
{
	++m_nSkinIndex;
	if (m_nSkinIndex > m_skinNames.size() - 1)m_nSkinIndex = 0;
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		spine::Skin *skin = m_skeletonData.at(i).get()->findSkin(m_skinNames.at(m_nSkinIndex).c_str());
		if (skin != nullptr)
		{
			m_drawables.at(i).get()->skeleton->setSkin(skin);
		}
		m_drawables.at(i).get()->skeleton->setSlotsToSetupPose();
	}
}
/*再描画*/
void CDxLibSpinePlayer::Redraw(float fDelta)
{
	if (!m_drawables.empty())
	{
		DxLib::ClearDrawScreen();
		for (size_t i = 0; i < m_drawables.size(); ++i)
		{
			m_drawables.at(i).get()->Update(fDelta);
			m_drawables.at(i).get()->Draw(m_bDepthBufferEnabled ? 0.1f * (i + 1) : 0.f);
		}
		DxLib::ScreenFlip();

		if (m_hRenderWnd != nullptr)
		{
			::InvalidateRect(m_hRenderWnd, nullptr, FALSE);
		}
	}
}
/*乗算済み透過度有効・無効切り替え*/
void CDxLibSpinePlayer::SwitchPma()
{
	for (size_t i = 0; i < m_drawables.size();++i)
	{
		m_drawables.at(i).get()->SwitchPma();
	}
}
/*槽溝指定合成方法採択可否*/
void CDxLibSpinePlayer::SwitchBlendModeAdoption()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->SwitchBlendModeAdoption();
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
/*槽溝名称引き渡し*/
std::vector<std::string> CDxLibSpinePlayer::GetSlotList()
{
	std::vector<std::string> slotNames;
	for (size_t i = 0; i < m_skeletonData.size(); ++i)
	{
		auto& slots = m_skeletonData.at(i).get()->getSlots();
		for (size_t ii = 0; ii < slots.size(); ++ii)
		{
			const std::string& strName = slots[ii]->getName().buffer();
			const auto iter = std::find(slotNames.begin(), slotNames.end(), strName);
			if (iter == slotNames.cend())slotNames.push_back(strName);
		}
	}
	return slotNames;
}
/*装い名称引き渡し*/
std::vector<std::string> CDxLibSpinePlayer::GetSkinList() const
{
	return m_skinNames;
}
/*動作名称引き渡し*/
std::vector<std::string> CDxLibSpinePlayer::GetAnimationList() const
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

	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->SetLeaveOutList(leaveOutList);
	}
}
/*装い合成*/
void CDxLibSpinePlayer::MixSkins(const std::vector<std::string>& skinNames)
{
	const auto& currentSkinName = m_skinNames.at(m_nSkinIndex);

	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		spine::Skin* skinToSet = m_skeletonData.at(i).get()->findSkin(currentSkinName.c_str());
		if (skinToSet != nullptr)
		{
			for (const auto& skinName : skinNames)
			{
				if (currentSkinName != skinName)
				{
					spine::Skin* skinToAdd = m_skeletonData.at(i).get()->findSkin(skinName.c_str());
					if (skinToAdd != nullptr)
					{
						skinToSet->addSkin(skinToAdd);
					}
				}
			}
			m_drawables.at(i).get()->skeleton->setSkin(skinToSet);
			m_drawables.at(i).get()->skeleton->setSlotsToSetupPose();
		}
	}
}
/*動作合成*/
void CDxLibSpinePlayer::MixAnimations(const std::vector<std::string>& animationNames)
{
	ClearAnimationTracks();

	const auto& currentAnimationName = m_animationNames.at(m_nAnimationIndex);

	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		if (m_skeletonData.at(i).get()->findAnimation(currentAnimationName.c_str()) == nullptr)continue;

		int iTrack = 1;
		for (const auto& animationName : animationNames)
		{
			if (animationName != currentAnimationName)
			{
				spine::Animation* animation = m_skeletonData.at(i).get()->findAnimation(animationName.c_str());
				if (animation != nullptr)
				{
					m_drawables.at(i).get()->state->addAnimation(iTrack, animation, false, 0.f);
					++iTrack;
				}
			}
		}
	}
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
	for (size_t i = 0; i < m_skeletonData.size(); ++i)
	{
		m_drawables.emplace_back(std::make_shared<CDxLibSpineDrawer>(m_skeletonData.at(i).get()));

		CDxLibSpineDrawer* drawable = m_drawables.at(i).get();
		drawable->timeScale = 1.0f;
		drawable->skeleton->setPosition(m_fBaseSize.u / 2, m_fBaseSize.v / 2);
		drawable->skeleton->updateWorldTransform();

		auto& animations = m_skeletonData.at(i).get()->getAnimations();
		for (size_t ii = 0; ii < animations.size(); ++ii)
		{
			const std::string& strAnimationName = animations[ii]->getName().buffer();
			const auto iter = std::find(m_animationNames.begin(), m_animationNames.end(), strAnimationName);
			if (iter == m_animationNames.cend())m_animationNames.push_back(strAnimationName);
		}

		auto& skins = m_skeletonData.at(i).get()->getSkins();
		for (size_t ii = 0; ii < skins.size(); ++ii)
		{
			const std::string& strName = skins[ii]->getName().buffer();
			const auto iter = std::find(m_skinNames.begin(), m_skinNames.end(), strName);
			if (iter == m_skinNames.cend())m_skinNames.push_back(strName);
		}
	}

	if (!m_animationNames.empty())
	{
		for (size_t i = 0; i < m_skeletonData.size(); ++i)
		{
			spine::Animation *animation = m_skeletonData.at(i).get()->findAnimation(m_animationNames.at(0).c_str());
			if (animation != nullptr)
			{
				m_drawables.at(i).get()->state->setAnimation(0, animation->getName(), true);
			}
		}
	}

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
			if (fWidth * fHeight > fMaxSize)
			{
				m_fBaseSize.u = fWidth;
				m_fBaseSize.v = fHeight;
				fMaxSize = fWidth * fHeight;
			}
		};

	for (size_t i = 0; i < m_skeletonData.size(); ++i)
	{
		if (m_skeletonData.at(i).get()->getWidth() > 0.f && m_skeletonData.at(i).get()->getHeight() > 0.f)
		{
			CompareDimention(m_skeletonData.at(i).get()->getWidth(), m_skeletonData.at(i).get()->getHeight());
		}
		else
		{
			/*If skeletonData does not store size, deduce from the attachment of the default skin.*/
			spine::Attachment* pAttachment = m_skeletonData.at(i).get()->getDefaultSkin()->getAttachments().next()._attachment;
			if (pAttachment != nullptr)
			{
				if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
				{
					spine::RegionAttachment* pRegionAttachment = (spine::RegionAttachment*)pAttachment;
					if (pRegionAttachment->getWidth() > 0.f && pRegionAttachment->getHeight() > 0.f)
					{
						CompareDimention
						(
							pRegionAttachment->getWidth() * pRegionAttachment->getScaleX(),
							pRegionAttachment->getHeight() * pRegionAttachment->getScaleY()
						);
					}
				}
				else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
				{
					spine::MeshAttachment* pMeshAttachment = (spine::MeshAttachment*)pAttachment;
					if (pMeshAttachment->getWidth() > 0.f && pMeshAttachment->getHeight() > 0.f)
					{
						float fScale = pMeshAttachment->getWidth() > Constants::kMinAtlas && pMeshAttachment->getHeight() > Constants::kMinAtlas ? 1.f : 2.f;
						CompareDimention(pMeshAttachment->getWidth() * fScale, pMeshAttachment->getHeight() * fScale);
					}
				}
			}
		}
	}

	m_fDefaultWindowScale = 1.f;
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
			m_fDefaultWindowScale = fScaleY;
		}
		else
		{
			m_fDefaultWindowScale = fScaleX;
		}

		m_fDefaultOffset.u = iSkeletonWidth > iDesktopWidth ? (iSkeletonWidth - iDesktopWidth) * fScaleX : 0.f;
		m_fDefaultOffset.v = iSkeletonHeight > iDesktopHeight ? (iSkeletonHeight - iDesktopHeight) * fScaleY : 0.f;
	}
}
/*尺度適用*/
void CDxLibSpinePlayer::UpdateScaletonScale()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->skeleton->setScaleX(m_fSkeletonScale);
		m_drawables.at(i).get()->skeleton->setScaleY(m_fSkeletonScale);
	}

	ResizeWindow();
}
/*速度適用*/
void CDxLibSpinePlayer::UpdateTimeScale()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		m_drawables.at(i).get()->timeScale = m_fTimeScale;
	}
}
/*合成動作消去*/
void CDxLibSpinePlayer::ClearAnimationTracks()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
	{
		const auto& trackEntry = m_drawables.at(i).get()->state->getTracks();
		for (size_t iTrack = 1; iTrack < trackEntry.size(); ++iTrack)
		{
			m_drawables.at(i).get()->state->setEmptyAnimation(iTrack, 0.f);
		}
	}
}
/*窓寸法調整*/
void CDxLibSpinePlayer::ResizeWindow()
{
	if (m_hRenderWnd != nullptr)
	{
		bool bBarHidden = IsWidowBarHidden();
		RECT rect;
		if (!bBarHidden)
		{
			::GetWindowRect(m_hRenderWnd, &rect);
		}
		else
		{
			::GetClientRect(m_hRenderWnd, &rect);
		}

		float fDpiScale = ::GetDpiForWindow(m_hRenderWnd) / 96.f;
		int iX = static_cast<int>(m_fBaseSize.u * m_fSkeletonScale);
		int iY = static_cast<int>(m_fBaseSize.v * m_fSkeletonScale);
		rect.right = iX + rect.left;
		rect.bottom = iY + rect.top;
		if (!bBarHidden)
		{
			LONG lStyle = ::GetWindowLong(m_hRenderWnd, GWL_STYLE);
			::AdjustWindowRect(&rect, lStyle, TRUE);
			::SetWindowPos(m_hRenderWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
		}
		else
		{
			RECT rc;
			::GetWindowRect(m_hRenderWnd, &rc);
			::MoveWindow(m_hRenderWnd, rc.left, rc.top, rect.right, rect.bottom, TRUE);
		}

		ResizeBuffer();
	}
}
/*緩衝容量再設定*/
void CDxLibSpinePlayer::ResizeBuffer()
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
/*枠縁有無*/
bool CDxLibSpinePlayer::IsWidowBarHidden()
{
	if (m_hRenderWnd != nullptr)
	{
		LONG lStyle = ::GetWindowLong(m_hRenderWnd, GWL_STYLE);
		return !((lStyle & WS_CAPTION) && (lStyle & WS_SYSMENU));
	}
	return false;
}
