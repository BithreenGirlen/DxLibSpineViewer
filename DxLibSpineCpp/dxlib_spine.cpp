

#include "dxlib_spine.h"

namespace spine
{
	SpineExtension* getDefaultExtension()
	{
		return new DefaultSpineExtension();
	}
}

CDxLibSpineDrawer::CDxLibSpineDrawer(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData)
{
	spine::Bone::setYDown(true);

	m_dxLibVertices.ensureCapacity(128);

	skeleton = new spine::Skeleton(pSkeletonData);

	if (pAnimationStateData == nullptr)
	{
		pAnimationStateData = new spine::AnimationStateData(pSkeletonData);
		m_bHasOwnAnimationStateData = true;
	}
	animationState = new spine::AnimationState(pAnimationStateData);

	m_quadIndices.add(0);
	m_quadIndices.add(1);
	m_quadIndices.add(2);
	m_quadIndices.add(2);
	m_quadIndices.add(3);
	m_quadIndices.add(0);

	/*
	* Here custom blend mode is used to avoid the pixels drawn with blend-mode-multiply to be transparent.
	* ---------- Formula for blend-mode-multiply ----------
	*   dstRGB = (srcRGB * dstRGB) + (dstRGB * (1-srcA))
	*   dstA = dstA
	* -----------------------------------------------------
	* The fomula above retains destination alpha, so in case initial alpha of the screen being 0,
	* it happens that drawn pixels are seen on the screen but unseen when saved as PNG.
	*
	* The solution taken here, not definitive though, is to overwrite alpha formula with that of blend-mode-normal.
	*/
	DxLib::SetDrawCustomBlendMode
	(
		TRUE,
		DX_BLEND_DEST_COLOR,
		DX_BLEND_INV_SRC_ALPHA,
		DX_BLENDOP_ADD,
		DX_BLEND_ONE,
		DX_BLEND_INV_SRC_ALPHA,
		DX_BLENDOP_ADD,
		255
	);
}

CDxLibSpineDrawer::~CDxLibSpineDrawer()
{
	if (animationState != nullptr)
	{
		if (m_bHasOwnAnimationStateData)
		{
			delete animationState->getData();
		}

		delete animationState;
	}
	if (skeleton != nullptr)
	{
		delete skeleton;
	}
}

void CDxLibSpineDrawer::Update(float fDelta)
{
	if (skeleton != nullptr && animationState != nullptr)
	{
#ifndef SPINE_4_1_OR_LATER
		skeleton->update(fDelta);
#endif
		animationState->update(fDelta * timeScale);
		animationState->apply(*skeleton);
		skeleton->updateWorldTransform();
	}
}

void CDxLibSpineDrawer::Draw(float fDepth)
{
	if (skeleton == nullptr || animationState == nullptr)return;

	if (skeleton->getColor().a == 0)return;

	for (size_t i = 0; i < skeleton->getSlots().size(); ++i)
	{
		spine::Slot& slot = *skeleton->getDrawOrder()[i];
		spine::Attachment* pAttachment = slot.getAttachment();

		if (pAttachment == nullptr || slot.getColor().a == 0 || !slot.getBone().isActive())
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		if (IsToBeLeftOut(slot.getData().getName()))
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		spine::Vector<float>* pVertices = &m_worldVertices;
		spine::Vector<float>* pAttachmentUvs = nullptr;
		spine::Vector<unsigned short>* pIndices = nullptr;

		spine::Color* pAttachmentColor = nullptr;

		int iDxLibTexture = -1;

		if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
		{
			spine::RegionAttachment* pRegionAttachment = (spine::RegionAttachment*)pAttachment;
			pAttachmentColor = &pRegionAttachment->getColor();

			if (pAttachmentColor->a == 0)
			{
				m_clipper.clipEnd(slot);
				continue;
			}

			m_worldVertices.setSize(8, 0);
#ifdef SPINE_4_1_OR_LATER
			pRegionAttachment->computeWorldVertices(slot, m_worldVertices, 0, 2);
#else
			pRegionAttachment->computeWorldVertices(slot.getBone(), m_worldVertices, 0, 2);
#endif
			pAttachmentUvs = &pRegionAttachment->getUVs();
			pIndices = &m_quadIndices;

			/*Fetch texture handle stored in AltasPage*/
#ifdef SPINE_4_1_OR_LATER
			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(static_cast<spine::AtlasRegion*>(pRegionAttachment->getRegion())->rendererObject)));
#else
			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(static_cast<spine::AtlasRegion*>(pRegionAttachment->getRendererObject())->page->getRendererObject())));
#endif
		}
		else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
		{
			spine::MeshAttachment* pMeshAttachment = (spine::MeshAttachment*)pAttachment;
			pAttachmentColor = &pMeshAttachment->getColor();

			if (pAttachmentColor->a == 0)
			{
				m_clipper.clipEnd(slot);
				continue;
			}
			m_worldVertices.setSize(pMeshAttachment->getWorldVerticesLength(), 0);
			pMeshAttachment->computeWorldVertices(slot, 0, pMeshAttachment->getWorldVerticesLength(), m_worldVertices, 0, 2);
			pAttachmentUvs = &pMeshAttachment->getUVs();
			pIndices = &pMeshAttachment->getTriangles();

#ifdef SPINE_4_1_OR_LATER
			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(static_cast<spine::AtlasRegion*>(pMeshAttachment->getRegion())->rendererObject)));
#else
			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(static_cast<spine::AtlasRegion*>(pMeshAttachment->getRendererObject())->page->getRendererObject())));
#endif
		}
		else if (pAttachment->getRTTI().isExactly(spine::ClippingAttachment::rtti))
		{
			spine::ClippingAttachment* clip = (spine::ClippingAttachment*)slot.getAttachment();
			m_clipper.clipStart(slot, clip);
			continue;
		}
		else
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		if (m_clipper.isClipping())
		{
			m_clipper.clipTriangles(m_worldVertices, *pIndices, *pAttachmentUvs, 2);
			if (m_clipper.getClippedTriangles().size() == 0)
			{
				m_clipper.clipEnd(slot);
				continue;
			}
			pVertices = &m_clipper.getClippedVertices();
			pAttachmentUvs = &m_clipper.getClippedUVs();
			pIndices = &m_clipper.getClippedTriangles();
		}

		const spine::Color& skeletonColor = skeleton->getColor();
		const spine::Color& slotColor = slot.getColor();
		spine::Color tint
		(
			skeletonColor.r * slotColor.r * pAttachmentColor->r,
			skeletonColor.g * slotColor.g * pAttachmentColor->g,
			skeletonColor.b * slotColor.b * pAttachmentColor->b,
			skeletonColor.a * slotColor.a * pAttachmentColor->a
		);

		/*Convert to DxLib's structure*/
		m_dxLibVertices.clear();
		for (int ii = 0; ii < pVertices->size(); ii +=2)
		{
			DxLib::VERTEX2D dxLibVertex{};
			dxLibVertex.pos.x = (*pVertices)[ii];
			dxLibVertex.pos.y = (*pVertices)[ii + 1LL];
			dxLibVertex.pos.z = fDepth;
			dxLibVertex.rhw = 1.f;
			dxLibVertex.dif.r = (BYTE)(tint.r * 255.f);
			dxLibVertex.dif.g = (BYTE)(tint.g * 255.f);
			dxLibVertex.dif.b = (BYTE)(tint.b * 255.f);
			dxLibVertex.dif.a = (BYTE)(tint.a * 255.f);
			dxLibVertex.u = (*pAttachmentUvs)[ii];
			dxLibVertex.v = (*pAttachmentUvs)[ii + 1LL];
			m_dxLibVertices.add(dxLibVertex);
		}
		m_dxLibIndices.clear();
		for (int ii = 0; ii < pIndices->size(); ++ii)
		{
			m_dxLibIndices.add((*pIndices)[ii]);
		}

		int iDxLibBlendMode;
		spine::BlendMode spineBlendMode = m_bForceBlendModeNormal ? spine::BlendMode::BlendMode_Normal : slot.getData().getBlendMode();
		switch (spineBlendMode)
		{
		case spine::BlendMode_Additive:
			iDxLibBlendMode = m_bAlphaPremultiplied ? DX_BLENDMODE_PMA_ADD : DX_BLENDMODE_SPINE_ADDITIVE;
			break;
		case spine::BlendMode_Multiply:
			iDxLibBlendMode = DX_BLENDMODE_CUSTOM;
			break;
		case spine::BlendMode_Screen:
			iDxLibBlendMode = DX_BLENDMODE_SPINE_SCREEN;
			break;
		default:
			iDxLibBlendMode = m_bAlphaPremultiplied ? DX_BLENDMODE_PMA_ALPHA : DX_BLENDMODE_SPINE_NORMAL;
			break;
		}

		DxLib::SetDrawBlendMode(iDxLibBlendMode, 255);
		DxLib::DrawPolygonIndexed2D
		(
			m_dxLibVertices.buffer(),
			static_cast<int>(m_dxLibVertices.size()),
			m_dxLibIndices.buffer(),
			static_cast<int>(m_dxLibIndices.size() / 3),
			iDxLibTexture, TRUE
		);
		m_clipper.clipEnd(slot);
	}
	m_clipper.clipEnd();
}

void CDxLibSpineDrawer::SetLeaveOutList(spine::Vector<spine::String>& list)
{
	/*There are some slots having mask or nuisance effect; exclude them from rendering.*/
	m_leaveOutList.clear();
	for (size_t i = 0; i < list.size(); ++i)
	{
		m_leaveOutList.add(list[i].buffer());
	}
}

bool CDxLibSpineDrawer::IsToBeLeftOut(const spine::String& slotName)
{
	/*The comparison method depends on what should be excluded; the precise matching or just containing.*/
	for (size_t i = 0; i < m_leaveOutList.size(); ++i)
	{
		if (strcmp(slotName.buffer(), m_leaveOutList[i].buffer()) == 0)return true;
	}
	return false;
}

void CDxLibTextureLoader::load(spine::AtlasPage& page, const spine::String& path)
{
#if	defined(_WIN32) && defined(_UNICODE)
	const auto WidenPath = [&path]()
		-> spine::Vector<wchar_t>
		{
			int iCharCode = DxLib::GetUseCharCodeFormat();
			int iWcharCode = DxLib::Get_wchar_t_CharCodeFormat();

			spine::Vector<wchar_t> vBuffer;
			vBuffer.setSize(path.length() * sizeof(wchar_t), L'\0');

			int iLen = DxLib::ConvertStringCharCodeFormat
			(
				iCharCode,
				path.buffer(),
				iWcharCode,
				vBuffer.buffer()
			);
			if (iLen != -1)
			{
				/*The defualt value is neglected when shrinking.*/
				vBuffer.setSize(iLen, L'\0');
			}
			return vBuffer;
		};
	spine::Vector<wchar_t> wcharPath = WidenPath();
	int iDxLibTexture = DxLib::LoadGraph(wcharPath.buffer());
#else
	int iDxLibTexture = DxLib::LoadGraph(path.buffer());
#endif
	if (iDxLibTexture == -1)return;

	/*In case atlas size does not coincide with that of png, overwriting will collapse the layout.*/
	if (page.width == 0 && page.height == 0)
	{
		int iWidth = 0;
		int iHeight = 0;
		DxLib::GetGraphSize(iDxLibTexture, &iWidth, &iHeight);
		page.width = iWidth;
		page.height = iHeight;
	}
	void* p = reinterpret_cast<void*>(static_cast<unsigned long long>(iDxLibTexture));

#ifdef SPINE_4_1_OR_LATER
	page.texture = p;
#else
	page.setRendererObject(p);
#endif
}

void CDxLibTextureLoader::unload(void* texture)
{
	DxLib::DeleteGraph(static_cast<int>(reinterpret_cast<unsigned long long>(texture)));
}
