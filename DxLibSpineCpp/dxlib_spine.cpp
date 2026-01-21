
/* To calculate bounding box */
#include <float.h>

#include "dxlib_spine.h"

namespace spine
{
	SpineExtension* getDefaultExtension()
	{
		return new DefaultSpineExtension();
	}
}

CDxLibSpineDrawable::CDxLibSpineDrawable(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData)
{
	spine::Bone::setYDown(true);

	m_dxLibVertices.ensureCapacity(128);

	m_skeleton = new spine::Skeleton(pSkeletonData);

	if (pAnimationStateData == nullptr)
	{
		pAnimationStateData = new spine::AnimationStateData(pSkeletonData);
		m_hasOwnAnimationStateData = true;
	}
	m_animationState = new spine::AnimationState(pAnimationStateData);

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

CDxLibSpineDrawable::~CDxLibSpineDrawable()
{
	if (m_animationState != nullptr)
	{
		if (m_hasOwnAnimationStateData)
		{
			delete m_animationState->getData();
		}

		delete m_animationState;
	}
	if (m_skeleton != nullptr)
	{
		delete m_skeleton;
	}
}

spine::Skeleton* CDxLibSpineDrawable::skeleton() const noexcept
{
	return m_skeleton;
}

spine::AnimationState* CDxLibSpineDrawable::animationState() const noexcept
{
	return m_animationState;
}

void CDxLibSpineDrawable::premultiplyAlpha(bool premultiplied) noexcept
{
	m_isAlphaPremultiplied = premultiplied;
}

bool CDxLibSpineDrawable::isAlphaPremultiplied() const noexcept
{
	return m_isAlphaPremultiplied;
}

void CDxLibSpineDrawable::forceBlendModeNormal(bool toForce) noexcept
{
	m_isToForceBlendModeNormal = toForce;
}

bool CDxLibSpineDrawable::isBlendModeNormalForced() const noexcept
{
	return m_isToForceBlendModeNormal;
}

void CDxLibSpineDrawable::update(float fDelta)
{
	if (m_skeleton != nullptr && m_animationState != nullptr)
	{
		m_animationState->update(fDelta);
		m_animationState->apply(*m_skeleton);

		/* Spine 4.1 Does not have "Skeleton::update()" */
#if !defined(SPINE_4_1_OR_LATER) || defined (SPINE_4_2_OR_LATER)
		m_skeleton->update(fDelta);
#endif

#ifdef SPINE_4_2_OR_LATER
		m_skeleton->updateWorldTransform(spine::Physics::Physics_Update);
#else
		m_skeleton->updateWorldTransform();
#endif
	}
}

void CDxLibSpineDrawable::draw()
{
	if (m_skeleton == nullptr || m_animationState == nullptr)return;

	if (m_skeleton->getColor().a == 0)return;

	for (size_t i = 0; i < m_skeleton->getSlots().size(); ++i)
	{
		spine::Slot& slot = *m_skeleton->getDrawOrder()[i];
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

#ifdef SPINE_4_1_OR_LATER
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pRegionAttachment->getRegion());
			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(pAtlasRegion->page->texture)));
#else
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pRegionAttachment->getRendererObject());
#ifdef SPINE_4_0
			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
#endif
			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(pAtlasRegion->page->getRendererObject())));
#endif // SPINE_4_1_OR_LATER
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
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pMeshAttachment->getRegion());
			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(pAtlasRegion->page->texture)));
#else
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pMeshAttachment->getRendererObject());
#ifdef SPINE_4_0
			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
#endif
			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(pAtlasRegion->page->getRendererObject())));
#endif // SPINE_4_1_OR_LATER
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

		const spine::Color& skeletonColor = m_skeleton->getColor();
		const spine::Color& slotColor = slot.getColor();
		const spine::Color tint
		(
			skeletonColor.r * slotColor.r * pAttachmentColor->r,
			skeletonColor.g * slotColor.g * pAttachmentColor->g,
			skeletonColor.b * slotColor.b * pAttachmentColor->b,
			skeletonColor.a * slotColor.a * pAttachmentColor->a
		);

		m_dxLibVertices.setSize(pVertices->size() / 2, {});
		for (int ii = 0, k = 0; ii < pVertices->size(); ii += 2, ++k)
		{
			DxLib::VERTEX2D& dxLibVertex = m_dxLibVertices[k];

			dxLibVertex.pos.x = (*pVertices)[ii];
			dxLibVertex.pos.y = (*pVertices)[ii + 1LL];
			dxLibVertex.pos.z = 0.f;
			dxLibVertex.rhw = 1.f;

			dxLibVertex.dif.r = static_cast<BYTE>(tint.r * 255.f);
			dxLibVertex.dif.g = static_cast<BYTE>(tint.g * 255.f);
			dxLibVertex.dif.b = static_cast<BYTE>(tint.b * 255.f);
			dxLibVertex.dif.a = static_cast<BYTE>(tint.a * 255.f);

			dxLibVertex.u = (*pAttachmentUvs)[ii];
			dxLibVertex.v = (*pAttachmentUvs)[ii + 1LL];
		}

		int iDxLibBlendMode;
		spine::BlendMode spineBlendMode = m_isToForceBlendModeNormal ? spine::BlendMode::BlendMode_Normal : slot.getData().getBlendMode();
		switch (spineBlendMode)
		{
		case spine::BlendMode_Additive:
			iDxLibBlendMode = m_isAlphaPremultiplied ? DX_BLENDMODE_PMA_ADD : DX_BLENDMODE_SPINE_ADDITIVE;
			break;
		case spine::BlendMode_Multiply:
			iDxLibBlendMode = DX_BLENDMODE_CUSTOM;
			break;
		case spine::BlendMode_Screen:
			iDxLibBlendMode = DX_BLENDMODE_SPINE_SCREEN;
			break;
		default:
			iDxLibBlendMode = m_isAlphaPremultiplied ? DX_BLENDMODE_PMA_ALPHA : DX_BLENDMODE_SPINE_NORMAL;
			break;
		}

		DxLib::SetDrawBlendMode(iDxLibBlendMode, 255);
		DxLib::DrawPolygonIndexed2D
		(
			m_dxLibVertices.buffer(),
			static_cast<int>(m_dxLibVertices.size()),
			pIndices->buffer(),
			static_cast<int>(pIndices->size() / 3),
			iDxLibTexture, TRUE
		);
		m_clipper.clipEnd(slot);
	}
	m_clipper.clipEnd();
}

void CDxLibSpineDrawable::setLeaveOutList(spine::Vector<spine::String>& list)
{
	/*There are some slots having mask or nuisance effect; exclude them from rendering.*/
	m_leaveOutList.clearAndAddAll(list);
}

DxLib::FLOAT4 CDxLibSpineDrawable::getBoundingBox() const
{
	DxLib::FLOAT4 boundingBox{};

	if (m_skeleton != nullptr)
	{
		spine::Vector<float> tempVertices;
		m_skeleton->getBounds(boundingBox.x, boundingBox.y, boundingBox.z, boundingBox.w, tempVertices);
	}

	return boundingBox;
}

DxLib::FLOAT4 CDxLibSpineDrawable::getBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found) const
{
	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;
	float fMaxX = -FLT_MAX;
	float fMaxY = -FLT_MAX;

	if (m_skeleton != nullptr)
	{
		for (size_t i = 0; i < m_skeleton->getSlots().size(); ++i)
		{
			spine::Slot& slot = *m_skeleton->getDrawOrder()[i];
			const spine::String& slotDataName = slot.getData().getName();
			if (nameLength != slotDataName.length())continue;

			if (::memcmp(slotDataName.buffer(), slotName, slotDataName.length()) == 0)
			{
				spine::Attachment* pAttachment = slot.getAttachment();
				if (pAttachment != nullptr)
				{
					spine::Vector<float> tempVertices;
					if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
					{
						spine::RegionAttachment* pRegionAttachment = static_cast<spine::RegionAttachment*>(pAttachment);

						tempVertices.setSize(8, 0);
#ifdef SPINE_4_1_OR_LATER
						pRegionAttachment->computeWorldVertices(slot, tempVertices, 0, 2);
#else
						pRegionAttachment->computeWorldVertices(slot.getBone(), tempVertices, 0, 2);
#endif
					}
					else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
					{
						spine::MeshAttachment* pMeshAttachment = static_cast<spine::MeshAttachment*>(pAttachment);
						tempVertices.setSize(pMeshAttachment->getWorldVerticesLength(), 0);
						pMeshAttachment->computeWorldVertices(slot, 0, pMeshAttachment->getWorldVerticesLength(), tempVertices, 0, 2);
					}
					else
					{
						continue;
					}

					for (size_t i = 0; i < tempVertices.size(); i += 2)
					{
						float fX = tempVertices[i];
						float fY = tempVertices[i + 1LL];

						fMinX = fMinX < fX ? fMinX : fX;
						fMinY = fMinY < fY ? fMinY : fY;
						fMaxX = fMaxX > fX ? fMaxX : fX;
						fMaxY = fMaxY > fY ? fMaxY : fY;
					}

					if (found != nullptr)*found = true;
					break;
				}
			}
		}
	}

	return DxLib::FLOAT4{ fMinX, fMinY, fMaxX - fMinX, fMaxY - fMinY };
}

bool CDxLibSpineDrawable::IsToBeLeftOut(const spine::String& slotName)
{
	if (m_pLeaveOutCallback != nullptr)
	{
		return m_pLeaveOutCallback(slotName.buffer(), slotName.length());
	}
	else
	{
		return m_leaveOutList.contains(slotName);
	}
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
