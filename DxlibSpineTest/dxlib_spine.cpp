

#include "dxlib_spine.h"

/*Windows OS dependency*/
#include "win_text.h"

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

	m_dxLibVertices.ensureCapacity(pSkeletonData->getBones().size() * sizeof(DxLib::VERTEX2D) / sizeof(float));

	skeleton = new(__FILE__, __LINE__) spine::Skeleton(pSkeletonData);

	if (pAnimationStateData == nullptr)
	{
		pAnimationStateData = new(__FILE__, __LINE__) spine::AnimationStateData(pSkeletonData);
		m_bHasOwnAnimationStateData = true;
	}
	state = new(__FILE__, __LINE__) spine::AnimationState(pAnimationStateData);

	m_quadIndices.add(0);
	m_quadIndices.add(1);
	m_quadIndices.add(2);
	m_quadIndices.add(2);
	m_quadIndices.add(3);
	m_quadIndices.add(0);
}

CDxLibSpineDrawer::~CDxLibSpineDrawer()
{
	if (state != nullptr)
	{
		if (m_bHasOwnAnimationStateData)
		{
			delete state->getData();
		}

		delete state;
	}
	if (skeleton != nullptr)
	{
		delete skeleton;
	}
}

void CDxLibSpineDrawer::Update(float fDelta)
{
	if (skeleton != nullptr && state != nullptr)
	{
		skeleton->update(fDelta);
		state->update(fDelta * timeScale);
		state->apply(*skeleton);
		skeleton->updateWorldTransform();
	}
}

void CDxLibSpineDrawer::Draw()
{
	if (skeleton == nullptr || state == nullptr)return;

	if (skeleton->getColor().a == 0)return; // Invisible case

	for (size_t i = 0; i < skeleton->getSlots().size(); ++i)
	{
		spine::Slot& slot = *skeleton->getDrawOrder()[i];
		spine::Attachment* pAttachment = slot.getAttachment();

		if (pAttachment == nullptr || slot.getColor().a == 0 || !slot.getBone().isActive())
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		spine::Vector<float>* pVertices = &m_worldVertices;
		int verticesCount = 0;
		spine::Vector<float>* pAttachmentUvs = nullptr;

		spine::Vector<unsigned short>* pIndices = nullptr;
		int indicesCount = 0;

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
			/*Fetch texture handle stored in AltasPage*/
			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(static_cast<spine::AtlasRegion*>(pRegionAttachment->getRendererObject())->page->getRendererObject())));

			/*
			* In SDL, SFML and Cocos2d, it is assumed the stride being 2 byte, and as float 4 byte, they set 8 here.
			* This float type vertice should be converted to the engine's vertex later as does Cocos2d to V3F_C4B_T2F.
			* In DxLib, this will be converted to DxLib::VERTEX2D.
			*/
			m_worldVertices.setSize(8, 0);
			/*Depends on spine's version whether the first argument is slot or bone.*/
			pRegionAttachment->computeWorldVertices(slot.getBone(), m_worldVertices, 0, 2);
			verticesCount = 4;
			pAttachmentUvs = &pRegionAttachment->getUVs();
			pIndices = &m_quadIndices;
			indicesCount = 6;
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
			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(static_cast<spine::AtlasRegion*>(pMeshAttachment->getRendererObject())->page->getRendererObject())));

			m_worldVertices.setSize(pMeshAttachment->getWorldVerticesLength(), 0);
			pMeshAttachment->computeWorldVertices(slot, 0, pMeshAttachment->getWorldVerticesLength(), m_worldVertices, 0, 2);
			verticesCount = static_cast<int>(pMeshAttachment->getWorldVerticesLength() / 2);
			pAttachmentUvs = &pMeshAttachment->getUVs();
			pIndices = &pMeshAttachment->getTriangles();
			indicesCount = static_cast<int>(pIndices->size());
		}
		else if (pAttachment->getRTTI().isExactly(spine::ClippingAttachment::rtti))
		{
			spine::ClippingAttachment* clip = (spine::ClippingAttachment*)slot.getAttachment();
			m_clipper.clipStart(slot, clip);
			continue;
		}
		else continue;

		if (m_clipper.isClipping())
		{
			m_clipper.clipTriangles(m_worldVertices, *pIndices, *pAttachmentUvs, 2);
			pVertices = &m_clipper.getClippedVertices();
			verticesCount = static_cast<int>(m_clipper.getClippedVertices().size() / 2);
			pAttachmentUvs = &m_clipper.getClippedUVs();
			pIndices = &m_clipper.getClippedTriangles();
			indicesCount = static_cast<int>(m_clipper.getClippedTriangles().size());
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
		for (int ii = 0; ii < verticesCount * 2; ii +=2)
		{
			DxLib::VERTEX2D dxLibVertex{};
			dxLibVertex.pos.x = (*pVertices)[ii];
			dxLibVertex.pos.y = (*pVertices)[ii + 1LL];
			dxLibVertex.pos.z = 0.f;
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
		switch (slot.getData().getBlendMode())
		{
		case spine::BlendMode_Additive:
			iDxLibBlendMode = m_bAlphaPremultiplied ? DX_BLENDMODE_PMA_ADD : DX_BLENDMODE_SPINE_ADDITIVE;
			break;
		case spine::BlendMode_Multiply:
			iDxLibBlendMode = DX_BLENDMODE_SPINE_MULTIPLY;
			break;
		case spine::BlendMode_Screen:
			iDxLibBlendMode = DX_BLENDMODE_SPINE_SCREEN;
			break;
		default:
			iDxLibBlendMode = m_bAlphaPremultiplied ? DX_BLENDMODE_PMA_ALPHA : DX_BLENDMODE_SPINE_NORMAL;
			break;
		}
		if (m_bForceBlendModeNormal)
		{
			iDxLibBlendMode = m_bAlphaPremultiplied ? DX_BLENDMODE_PMA_ALPHA : DX_BLENDMODE_SPINE_NORMAL;
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

void CDxLibTextureLoader::load(spine::AtlasPage& page, const spine::String& path)
{
	/*Actually this shall be platform-independet as does SFML in <Utf.hpp>.*/
	std::wstring wstrPath = win_text::WidenANSI(path.buffer());
	int iDxLibTexture = DxLib::LoadGraph(wstrPath.c_str());
	/*In case atlas size does not coincide with that of png, overwriting will collapse the layout.*/
	if (iDxLibTexture != -1 && page.width == 0 && page.height == 0)
	{
		int iWidth = 0;
		int iHeight = 0;
		DxLib::GetGraphSize(iDxLibTexture, &iWidth, &iHeight);
		page.width = iWidth;
		page.height = iHeight;
	}
	void* p = reinterpret_cast<void*>(static_cast<unsigned long long>(iDxLibTexture));

	page.setRendererObject(p);
}

void CDxLibTextureLoader::unload(void* texture)
{
	DxLib::DeleteGraph(static_cast<int>(reinterpret_cast<unsigned long long>(texture)));
}
