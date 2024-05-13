

#include "dxlib_spine_player.h"
#include "spine_loader.h"

CDxLibSpinePlayer::CDxLibSpinePlayer()
{

}

CDxLibSpinePlayer::~CDxLibSpinePlayer()
{
	if (m_iDxLibInitialised != -1)
	{
		DxLib::DxLib_End();
	}
}
/*初期設定*/
bool CDxLibSpinePlayer::SetupDxLib(HWND hRenderWnd)
{
	if (m_iDxLibInitialised == 0)return true;

	int iRet = -1;
	iRet = DxLib::SetOutApplicationLogValidFlag(FALSE);
	if (iRet == -1)return false;

	if (hRenderWnd != nullptr)
	{
		iRet = DxLib::SetUserWindow(hRenderWnd);
		if (iRet == -1)return false;
	}
	iRet = DxLib::SetUserWindowMessageProcessDXLibFlag(hRenderWnd != nullptr ? FALSE : TRUE);
	if (iRet == -1)return false;

	iRet = DxLib::SetChangeScreenModeGraphicsSystemResetFlag(hRenderWnd !=  nullptr ? FALSE : TRUE);
	if (iRet == -1)return false;
	m_hRenderWnd = hRenderWnd;

	iRet = DxLib::ChangeWindowMode(TRUE);
	if (iRet == -1)return false;

	iRet = DxLib::SetDrawMode(DX_DRAWMODE_BILINEAR);
	if (iRet == -1)return false;

	iRet = DxLib::SetMultiThreadFlag(TRUE);
	if (iRet == -1)return false;

	iRet = DxLib::SetUseTransColor(FALSE);
	if (iRet == -1)return false;

	m_iDxLibInitialised = DxLib::DxLib_Init();
	if (m_iDxLibInitialised == -1)return false;

	iRet = DxLib::SetDrawScreen(DX_SCREEN_BACK);
	if (iRet == -1)
	{
		DxLib::DxLib_End();
		m_iDxLibInitialised = -1;
	}

	return m_iDxLibInitialised != -1;
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
		/*PeekMessageは扱いにくい*/
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
	if (m_iDxLibInitialised != -1)
	{
		int iRet = DxLib::SetUseZBufferFlag(m_bDepthBufferEnabled ? FALSE : TRUE);
		if (iRet == -1)return false;

		iRet = DxLib::SetWriteZBufferFlag(m_bDepthBufferEnabled ? FALSE : TRUE);
		if (iRet == -1)return false;

		m_bDepthBufferEnabled ^= true;
		return true;
	}
	return false;
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

	if (m_skeletonData.at(0).get()->getWidth() > 0.f && m_skeletonData.at(0).get()->getHeight() > 0.f)
	{
		for (size_t i = 0; i < m_skeletonData.size(); ++i)
		{
			float fWidth = m_skeletonData.at(i).get()->getWidth();
			float fHeight = m_skeletonData.at(i).get()->getHeight();
			m_fBaseSize.u = m_fBaseSize.u > fWidth ? m_fBaseSize.u : fWidth;
			m_fBaseSize.v = m_fBaseSize.v > fHeight ? m_fBaseSize.v : fHeight;
		}
	}
	else
	{
		/*If skeletonData does not store size, deduce from the attachment of the default skin.*/
		spine::Attachment* pAttachment = m_skeletonData.at(0).get()->getDefaultSkin()->getAttachments().next()._attachment;
		if (pAttachment != nullptr)
		{
			if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
			{
				spine::RegionAttachment* pRegionAttachment = (spine::RegionAttachment*)pAttachment;
				if (pRegionAttachment->getWidth() > 0.f && pRegionAttachment->getHeight() > 0.f)
				{
					m_fBaseSize.u = pRegionAttachment->getWidth() * pRegionAttachment->getScaleX();
					m_fBaseSize.v = pRegionAttachment->getHeight() * pRegionAttachment->getScaleY();
				}
			}
			else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
			{
				spine::MeshAttachment* pMeshAttachment = (spine::MeshAttachment*)pAttachment;
				if (pMeshAttachment->getWidth() > 0.f && pMeshAttachment->getHeight() > 0.f)
				{
					m_fBaseSize.u = pMeshAttachment->getWidth() * 2.f;
					m_fBaseSize.v = pMeshAttachment->getHeight() * 2.f;
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
			m_fThresholdScale = static_cast<float>(iDesktopWidth) / iSkeletonWidth;
		}
		else
		{
			m_fDefaultWindowScale = fScaleX;
			m_fThresholdScale = static_cast<float>(iDesktopHeight) / iSkeletonHeight;
		}

		m_fDefaultOffset.u = iSkeletonWidth > iDesktopWidth ? (iSkeletonWidth - iDesktopWidth) * fScaleX : 0.f;
		m_fDefaultOffset.v = iSkeletonHeight > iDesktopHeight ? (iSkeletonHeight - iDesktopHeight) * fScaleY : 0.f;
	}
}
/*尺度適用*/
void CDxLibSpinePlayer::UpdateScaletonScale()
{
	float fOffset = m_fSkeletonScale - m_fThresholdScale > 0.f ? m_fSkeletonScale - m_fThresholdScale : 0;
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
/*窓寸法調整*/
void CDxLibSpinePlayer::ResizeWindow()
{
	if (m_hRenderWnd != nullptr)
	{
		bool bBarHidden = bIsWidowBarHidden();
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
bool CDxLibSpinePlayer::bIsWidowBarHidden()
{
	if (m_hRenderWnd != nullptr)
	{
		LONG lStyle = ::GetWindowLong(m_hRenderWnd, GWL_STYLE);
		return !((lStyle & WS_CAPTION) && (lStyle & WS_SYSMENU));
	}
	return false;
}
