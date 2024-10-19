
#include <spine/extension.h>

#include "dxlib_spine_c.h"

_SP_ARRAY_IMPLEMENT_TYPE_NO_CONTAINS(spDxLibVertexArray, DxLib::VERTEX2D)

static wchar_t* WidenPath(const char* path)
{
	int iCharCode = DxLib::GetUseCharCodeFormat();
	int iWcharCode = DxLib::Get_wchar_t_CharCodeFormat();

	size_t nLen = strlen(path);
	wchar_t* pResult = static_cast<wchar_t*>(malloc((nLen + 1LL) * sizeof(wchar_t)));
	if (pResult == nullptr)return nullptr;
	wmemset(pResult, L'\0', nLen);

	int iLen = DxLib::ConvertStringCharCodeFormat
	(
		iCharCode,
		path,
		iWcharCode,
		pResult
	);
	if (iLen == -1)
	{
		free(pResult);
		return nullptr;
	}

	wchar_t* pTemp = static_cast<wchar_t*>(realloc(pResult, (iLen + 1LL) * sizeof(wchar_t)));
	if (pTemp != nullptr)
	{
		pResult = pTemp;
	}
	*(pResult + iLen) = L'\0';
	return pResult;
}

/*Implementations for <extension.h>*/

void _spAtlasPage_createTexture(spAtlasPage* pAtlasPage, const char* path)
{
#if	defined(_WIN32) && defined(_UNICODE)
	wchar_t* wcharPath = WidenPath(path);
	if (wcharPath == nullptr)return;
	int iDxLibTexture = DxLib::LoadGraph(wcharPath);
	free(wcharPath);
	wcharPath = nullptr;
#else
	int iDxLibTexture = DxLib::LoadGraph(path);
#endif
	if (iDxLibTexture == -1)return;

	/*In case atlas size does not coincide with that of png, overwriting will collapse the layout.*/
	if (pAtlasPage->width == 0 && pAtlasPage->height == 0)
	{
		int iWidth = 0;
		int iHeight = 0;
		DxLib::GetGraphSize(iDxLibTexture, &iWidth, &iHeight);
		pAtlasPage->width = iWidth;
		pAtlasPage->height = iHeight;
	}

	void* p = reinterpret_cast<void*>(static_cast<unsigned long long>(iDxLibTexture));

	pAtlasPage->rendererObject = p;
}

void _spAtlasPage_disposeTexture(spAtlasPage* pAtlasPage)
{
	DxLib::DeleteGraph(static_cast<int>(reinterpret_cast<unsigned long long>(pAtlasPage->rendererObject)));
}

char* _spUtil_readFile(const char* path, int* length)
{
	return _spReadFile(path, length);
}
// end of implementations for <extension.h>

CDxLibSpineDrawerC::CDxLibSpineDrawerC(spSkeletonData* pSkeletonData, spAnimationStateData* pAnimationStateData)
{
	spBone_setYDown(1);

	m_worldVertices = spFloatArray_create(128);
	m_dxLibVertices = spDxLibVertexArray_create(128);
	m_dxLibIndices = spUnsignedShortArray_create(128);

	skeleton = spSkeleton_create(pSkeletonData);
	if (pAnimationStateData == nullptr)
	{
		pAnimationStateData = spAnimationStateData_create(pSkeletonData);
		m_bHasOwnAnimationStateData = true;
	}
	animationState = spAnimationState_create(pAnimationStateData);
	m_clipper = spSkeletonClipping_create();

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

CDxLibSpineDrawerC::~CDxLibSpineDrawerC()
{
	if (m_worldVertices != nullptr)
	{
		spFloatArray_dispose(m_worldVertices);
	}
	if (m_dxLibVertices != nullptr)
	{
		spDxLibVertexArray_dispose(m_dxLibVertices);
	}
	if (m_dxLibIndices != nullptr)
	{
		spUnsignedShortArray_dispose(m_dxLibIndices);
	}

	if (animationState != nullptr)
	{
		if (m_bHasOwnAnimationStateData)
		{
			spAnimationStateData_dispose(animationState->data);
		}

		spAnimationState_dispose(animationState);
	}
	if (skeleton != nullptr)
	{
		spSkeleton_dispose(skeleton);
	}
	if (m_clipper != nullptr)
	{
		spSkeletonClipping_dispose(m_clipper);
	}

	ClearLeaveOutList();
}

void CDxLibSpineDrawerC::Update(float fDelta)
{
	if (skeleton == nullptr || animationState == nullptr)return;
#ifndef SPINE_4_1_OR_LATER
	spSkeleton_update(skeleton, fDelta);
#endif
	spAnimationState_update(animationState, fDelta * timeScale);
	spAnimationState_apply(animationState, skeleton);
	spSkeleton_updateWorldTransform(skeleton);
}

void CDxLibSpineDrawerC::Draw(float fDepth, float fScale)
{
	if (m_worldVertices == nullptr || m_clipper == nullptr || skeleton == nullptr || animationState == nullptr)return;

	if (skeleton->color.a == 0) return;

	static unsigned short quadIndices[] = { 0, 1, 2, 2, 3, 0 };

	for (int i = 0; i < skeleton->slotsCount; ++i)
	{
		spSlot* pSlot = skeleton->drawOrder[i];
		spAttachment* pAttachment = pSlot->attachment;
		/*spine-c 3.6 lacks pSlot->bone->active*/
		if (pAttachment == nullptr || pSlot->color.a == 0)
		{
			spSkeletonClipping_clipEnd(m_clipper, pSlot);
			continue;
		}

		if (IsToBeLeftOut(pSlot->data->name))
		{
			spSkeletonClipping_clipEnd(m_clipper, pSlot);
			continue;
		}

		spFloatArray* pVertices = m_worldVertices;
		int verticesCount = 0;
		float* pAttachmentUvs = nullptr;

		unsigned short* pIndices = nullptr;
		int indicesCount = 0;

		spColor* pAttachmentColor = nullptr;

		int iDxLibTexture = -1;

		if (pAttachment->type == SP_ATTACHMENT_REGION)
		{
			spRegionAttachment* pRegionAttachment = (spRegionAttachment*)pAttachment;
			pAttachmentColor = &pRegionAttachment->color;

			if (pAttachmentColor->a == 0)
			{
				spSkeletonClipping_clipEnd(m_clipper, pSlot);
				continue;
			}

			spFloatArray_setSize(pVertices, 8);
#ifdef SPINE_4_1_OR_LATER
			spRegionAttachment_computeWorldVertices(pRegionAttachment, pSlot, pVertices->items, 0, 2);
#else
			spRegionAttachment_computeWorldVertices(pRegionAttachment, pSlot->bone, pVertices->items, 0, 2);
#endif
			verticesCount = 4;
			pAttachmentUvs = pRegionAttachment->uvs;
			pIndices = quadIndices;
			indicesCount = 6;

			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(static_cast<spAtlasRegion*>(pRegionAttachment->rendererObject)->page->rendererObject)));
		}
		else if (pAttachment->type == SP_ATTACHMENT_MESH)
		{
			spMeshAttachment* pMeshAttachment = (spMeshAttachment*)pAttachment;
			pAttachmentColor = &pMeshAttachment->color;

			if (pAttachmentColor->a == 0)
			{
				spSkeletonClipping_clipEnd(m_clipper, pSlot);
				continue;
			}
			spFloatArray_setSize(pVertices, pMeshAttachment->super.worldVerticesLength);
			spVertexAttachment_computeWorldVertices(SUPER(pMeshAttachment), pSlot, 0, pMeshAttachment->super.worldVerticesLength, pVertices->items, 0, 2);
			verticesCount = pMeshAttachment->super.worldVerticesLength / 2;
			pAttachmentUvs = pMeshAttachment->uvs;
			pIndices = pMeshAttachment->triangles;
			indicesCount = pMeshAttachment->trianglesCount;

			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(static_cast<spAtlasRegion*>(pMeshAttachment->rendererObject)->page->rendererObject)));

		}
		else if (pAttachment->type == SP_ATTACHMENT_CLIPPING)
		{
			spClippingAttachment* clip = (spClippingAttachment*)pSlot->attachment;
			spSkeletonClipping_clipStart(m_clipper, pSlot, clip);
			continue;
		}
		else
		{
			continue;
		}

		if (spSkeletonClipping_isClipping(m_clipper))
		{
			spSkeletonClipping_clipTriangles(m_clipper, pVertices->items, verticesCount / 2, pIndices, indicesCount, pAttachmentUvs, 2);
			pVertices = m_clipper->clippedVertices;
			verticesCount = m_clipper->clippedVertices->size / 2;
			pAttachmentUvs = m_clipper->clippedUVs->items;
			pIndices = m_clipper->clippedTriangles->items;
			indicesCount = m_clipper->clippedTriangles->size;
		}

		spColor tint;
		tint.r = skeleton->color.r * pSlot->color.r * pAttachmentColor->r;
		tint.g = skeleton->color.g * pSlot->color.g * pAttachmentColor->g;
		tint.b = skeleton->color.b * pSlot->color.b * pAttachmentColor->b;
		tint.a = skeleton->color.a * pSlot->color.a * pAttachmentColor->a;

		spDxLibVertexArray_clear(m_dxLibVertices);
		for (int ii = 0; ii < verticesCount * 2; ii += 2)
		{
			DxLib::VERTEX2D dxLibVertex{};
			dxLibVertex.pos.x = pVertices->items[ii] * fScale;
			dxLibVertex.pos.y = pVertices->items[ii + 1LL] * fScale;
			dxLibVertex.pos.z = fDepth;
			dxLibVertex.rhw = 1.f;
			dxLibVertex.dif.r = (BYTE)(tint.r * 255.f);
			dxLibVertex.dif.g = (BYTE)(tint.g * 255.f);
			dxLibVertex.dif.b = (BYTE)(tint.b * 255.f);
			dxLibVertex.dif.a = (BYTE)(tint.a * 255.f);
			dxLibVertex.u = pAttachmentUvs[ii];
			dxLibVertex.v = pAttachmentUvs[ii + 1LL];
			spDxLibVertexArray_add(m_dxLibVertices, dxLibVertex);
		}

		spUnsignedShortArray_clear(m_dxLibIndices);
		for (int ii = 0; ii < indicesCount; ++ii)
		{
			spUnsignedShortArray_add(m_dxLibIndices, pIndices[ii]);
		}

		int iDxLibBlendMode;
		switch (pSlot->data->blendMode)
		{
		case spBlendMode::SP_BLEND_MODE_ADDITIVE:
			iDxLibBlendMode = m_bAlphaPremultiplied ? DX_BLENDMODE_PMA_ADD : DX_BLENDMODE_SPINE_ADDITIVE;
			break;
		case spBlendMode::SP_BLEND_MODE_MULTIPLY:
			iDxLibBlendMode = DX_BLENDMODE_CUSTOM;
			break;
		case spBlendMode::SP_BLEND_MODE_SCREEN:
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
			m_dxLibVertices->items,
			m_dxLibVertices->size,
			m_dxLibIndices->items,
			m_dxLibIndices->size / 3,
			iDxLibTexture, TRUE
		);

		spSkeletonClipping_clipEnd(m_clipper, pSlot);
	}
	spSkeletonClipping_clipEnd2(m_clipper);
}

void CDxLibSpineDrawerC::SetLeaveOutList(const char** list, int listCount)
{
	ClearLeaveOutList();

	m_leaveOutList = CALLOC(char*, listCount);
	if (m_leaveOutList == nullptr)return;

	m_leaveOutListCount = listCount;
	for (int i = 0; i < m_leaveOutListCount; ++i)
	{
		MALLOC_STR(m_leaveOutList[i], list[i]);
	}
}

void CDxLibSpineDrawerC::ClearLeaveOutList()
{
	if (m_leaveOutList != nullptr)
	{
		for (int i = 0; i < m_leaveOutListCount; ++i)
		{
			if(m_leaveOutList[i] != nullptr)FREE(m_leaveOutList[i]);
		}
		FREE(m_leaveOutList);
	}
	m_leaveOutList = nullptr;
	m_leaveOutListCount = 0;
}

bool CDxLibSpineDrawerC::IsToBeLeftOut(const char* slotName)
{
	for (int i = 0; i < m_leaveOutListCount; ++i)
	{
		if (strcmp(slotName, m_leaveOutList[i]) == 0)return true;
	}
	return false;
}
