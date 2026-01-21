
/* To calculate bounding box */
#include <float.h>

#include <spine/extension.h>

#include "dxlib_spine_c.h"

_SP_ARRAY_IMPLEMENT_TYPE_NO_CONTAINS(spDxLibVertexArray, DxLib::VERTEX2D)

#if	defined(_WIN32) && defined(_UNICODE)
static wchar_t* WidenPath(const char* path)
{
	int iCharCode = DxLib::GetUseCharCodeFormat();
	int iWcharCode = DxLib::Get_wchar_t_CharCodeFormat();

	size_t nLen = strlen(path);
	wchar_t* pResult = static_cast<wchar_t*>(malloc((nLen + 1LL) * sizeof(wchar_t)));
	if (pResult == nullptr)return nullptr;
	memset(pResult, L'\0', nLen * sizeof(wchar_t));

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

	wchar_t* pTemp = static_cast<wchar_t*>(realloc(pResult, iLen));
	if (pTemp != nullptr)
	{
		pResult = pTemp;
	}

	return pResult;
}
#endif

/* ==================== Implementations for <extension.h> ==================== */

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

/* ==================== end of implementations for <extension.h> ==================== */

CDxLibSpineDrawableC::CDxLibSpineDrawableC(spSkeletonData* pSkeletonData, spAnimationStateData* pAnimationStateData)
{
	spBone_setYDown(-1);

	m_worldVertices = spFloatArray_create(128);
	m_dxLibVertices = spDxLibVertexArray_create(128);

	m_skeleton = spSkeleton_create(pSkeletonData);
	if (pAnimationStateData == nullptr)
	{
		pAnimationStateData = spAnimationStateData_create(pSkeletonData);
		m_hasOwnAnimationStateData = true;
	}
	m_animationState = spAnimationState_create(pAnimationStateData);
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

CDxLibSpineDrawableC::~CDxLibSpineDrawableC()
{
	if (m_worldVertices != nullptr)
	{
		spFloatArray_dispose(m_worldVertices);
	}
	if (m_dxLibVertices != nullptr)
	{
		spDxLibVertexArray_dispose(m_dxLibVertices);
	}

	if (m_animationState != nullptr)
	{
		if (m_hasOwnAnimationStateData)
		{
			spAnimationStateData_dispose(m_animationState->data);
		}

		spAnimationState_dispose(m_animationState);
	}
	if (m_skeleton != nullptr)
	{
		spSkeleton_dispose(m_skeleton);
	}
	if (m_clipper != nullptr)
	{
		spSkeletonClipping_dispose(m_clipper);
	}

	clearLeaveOutList();
}

spSkeleton* CDxLibSpineDrawableC::skeleton() const noexcept
{
	return m_skeleton;
}

spAnimationState* CDxLibSpineDrawableC::animationState() const noexcept
{
	return m_animationState;
}

void CDxLibSpineDrawableC::premultiplyAlpha(bool premultiplied) noexcept
{
	m_isAlphaPremultiplied = premultiplied;
}

bool CDxLibSpineDrawableC::isAlphaPremultiplied() const noexcept
{
	return m_isAlphaPremultiplied;
}

void CDxLibSpineDrawableC::forceBlendModeNormal(bool toForce) noexcept
{
	m_isToForceBlendModeNormal = toForce;
}

bool CDxLibSpineDrawableC::isBlendModeNormalForced() const noexcept
{
	return m_isToForceBlendModeNormal;
}

void CDxLibSpineDrawableC::update(float fDelta)
{
	if (m_skeleton == nullptr || m_animationState == nullptr)return;

	spAnimationState_update(m_animationState, fDelta);
	spAnimationState_apply(m_animationState, m_skeleton);

	/* Spine 4.1 Does not have "spSkeleton_update()" */
#if !defined(SPINE_4_1_OR_LATER) || defined (SPINE_4_2_OR_LATER)
	spSkeleton_update(m_skeleton, fDelta);
#endif

#ifdef SPINE_4_2_OR_LATER
	spSkeleton_updateWorldTransform(m_skeleton, spPhysics::SP_PHYSICS_UPDATE);
#else
	spSkeleton_updateWorldTransform(m_skeleton);
#endif
}

void CDxLibSpineDrawableC::draw()
{
	if (m_skeleton == nullptr || m_animationState == nullptr)return;
	if (m_skeleton->color.a == 0) return;

	static unsigned short quadIndices[] = { 0, 1, 2, 2, 3, 0 };

	for (int i = 0; i < m_skeleton->slotsCount; ++i)
	{
		spSlot* pSlot = m_skeleton->drawOrder[i];
		spAttachment* pAttachment = pSlot->attachment;
#ifdef SPINE_3_8_OR_LATER
		if (pAttachment == nullptr || pSlot->color.a == 0 || !pSlot->bone->active)
#else
		if (pAttachment == nullptr || pSlot->color.a == 0)
#endif
		{
			spSkeletonClipping_clipEnd(m_clipper, pSlot);
			continue;
		}

		if (isSlotToBeLeftOut(pSlot->data->name))
		{
			spSkeletonClipping_clipEnd(m_clipper, pSlot);
			continue;
		}

		spFloatArray* pVertices = m_worldVertices;
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
			pAttachmentUvs = pRegionAttachment->uvs;
			pIndices = quadIndices;
			indicesCount = sizeof(quadIndices) / sizeof(unsigned short);

			spAtlasRegion* pAtlasRegion = static_cast<spAtlasRegion*>(pRegionAttachment->rendererObject);
#if defined (SPINE_4_0) || defined (SPINE_4_1_OR_LATER)
			/* true: -1, false: 0 */
			m_isAlphaPremultiplied = pAtlasRegion->page->pma == -1;
#endif
			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(pAtlasRegion->page->rendererObject)));
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
			pAttachmentUvs = pMeshAttachment->uvs;
			pIndices = pMeshAttachment->triangles;
			indicesCount = pMeshAttachment->trianglesCount;

			spAtlasRegion* pAtlasRegion = static_cast<spAtlasRegion*>(pMeshAttachment->rendererObject);
#if defined (SPINE_4_0) || defined (SPINE_4_1_OR_LATER)
			m_isAlphaPremultiplied = pAtlasRegion->page->pma == -1;
#endif
			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(pAtlasRegion->page->rendererObject)));
		}
		else if (pAttachment->type == SP_ATTACHMENT_CLIPPING)
		{
			spClippingAttachment* clip = (spClippingAttachment*)pSlot->attachment;
			spSkeletonClipping_clipStart(m_clipper, pSlot, clip);
			continue;
		}
		else
		{
			spSkeletonClipping_clipEnd(m_clipper, pSlot);
			continue;
		}

		if (spSkeletonClipping_isClipping(m_clipper))
		{
			spSkeletonClipping_clipTriangles(m_clipper, pVertices->items, pVertices->size, pIndices, indicesCount, pAttachmentUvs, 2);
			if (m_clipper->clippedTriangles->size == 0)
			{
				spSkeletonClipping_clipEnd(m_clipper, pSlot);
				continue;
			}
			pVertices = m_clipper->clippedVertices;
			pAttachmentUvs = m_clipper->clippedUVs->items;
			pIndices = m_clipper->clippedTriangles->items;
			indicesCount = m_clipper->clippedTriangles->size;
		}

		spColor tint;
		tint.r = m_skeleton->color.r * pSlot->color.r * pAttachmentColor->r;
		tint.g = m_skeleton->color.g * pSlot->color.g * pAttachmentColor->g;
		tint.b = m_skeleton->color.b * pSlot->color.b * pAttachmentColor->b;
		tint.a = m_skeleton->color.a * pSlot->color.a * pAttachmentColor->a;

		spDxLibVertexArray_setSize(m_dxLibVertices, pVertices->size / 2);
		for (int ii = 0, k = 0; ii < pVertices->size; ii += 2, ++k)
		{
			DxLib::VERTEX2D& dxLibVertex = m_dxLibVertices->items[k];

			dxLibVertex.pos.x = pVertices->items[ii];
			dxLibVertex.pos.y = pVertices->items[ii + 1LL];
			dxLibVertex.pos.z = 0.f;
			dxLibVertex.rhw = 1.f;

			dxLibVertex.dif.r = static_cast<BYTE>(tint.r * 255.f);
			dxLibVertex.dif.g = static_cast<BYTE>(tint.g * 255.f);
			dxLibVertex.dif.b = static_cast<BYTE>(tint.b * 255.f);
			dxLibVertex.dif.a = static_cast<BYTE>(tint.a * 255.f);

			dxLibVertex.u = pAttachmentUvs[ii];
			dxLibVertex.v = pAttachmentUvs[ii + 1LL];
		}

		int iDxLibBlendMode;
		spBlendMode spineBlendMode = m_isToForceBlendModeNormal ? SP_BLEND_MODE_NORMAL : pSlot->data->blendMode;
		switch (spineBlendMode)
		{
		case spBlendMode::SP_BLEND_MODE_ADDITIVE:
			iDxLibBlendMode = m_isAlphaPremultiplied ? DX_BLENDMODE_PMA_ADD : DX_BLENDMODE_SPINE_ADDITIVE;
			break;
		case spBlendMode::SP_BLEND_MODE_MULTIPLY:
			iDxLibBlendMode = DX_BLENDMODE_CUSTOM;
			break;
		case spBlendMode::SP_BLEND_MODE_SCREEN:
			iDxLibBlendMode = DX_BLENDMODE_SPINE_SCREEN;
			break;
		default:
			iDxLibBlendMode = m_isAlphaPremultiplied ? DX_BLENDMODE_PMA_ALPHA : DX_BLENDMODE_SPINE_NORMAL;
			break;
		}
		DxLib::SetDrawBlendMode(iDxLibBlendMode, 255);
		DxLib::DrawPolygonIndexed2D
		(
			m_dxLibVertices->items,
			m_dxLibVertices->size,
			pIndices,
			indicesCount / 3,
			iDxLibTexture, TRUE
		);

		spSkeletonClipping_clipEnd(m_clipper, pSlot);
	}
	spSkeletonClipping_clipEnd2(m_clipper);
}

void CDxLibSpineDrawableC::setLeaveOutList(const char** list, int listCount)
{
	clearLeaveOutList();

	m_leaveOutList = CALLOC(char*, listCount);
	if (m_leaveOutList == nullptr)return;

	m_leaveOutListCount = listCount;
	for (int i = 0; i < m_leaveOutListCount; ++i)
	{
		MALLOC_STR(m_leaveOutList[i], list[i]);
	}
}

DxLib::FLOAT4 CDxLibSpineDrawableC::getBoundingBox() const
{
	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;
	float fMaxX = -FLT_MAX;
	float fMaxY = -FLT_MAX;

	spFloatArray* pTempVertices = spFloatArray_create(128);

	for (int i = 0; i < m_skeleton->slotsCount; ++i)
	{
		spSlot* pSlot = m_skeleton->drawOrder[i];
		spAttachment* pAttachment = pSlot->attachment;

		if (pAttachment == nullptr)continue;

		if (pAttachment->type == SP_ATTACHMENT_REGION)
		{
			spRegionAttachment* pRegionAttachment = (spRegionAttachment*)pAttachment;

			spFloatArray_setSize(pTempVertices, 8);
#ifdef SPINE_4_1_OR_LATER
			spRegionAttachment_computeWorldVertices(pRegionAttachment, pSlot, pTempVertices->items, 0, 2);
#else
			spRegionAttachment_computeWorldVertices(pRegionAttachment, pSlot->bone, pTempVertices->items, 0, 2);
#endif
		}
		else if (pAttachment->type == SP_ATTACHMENT_MESH)
		{
			spMeshAttachment* pMeshAttachment = (spMeshAttachment*)pAttachment;

			spFloatArray_setSize(pTempVertices, pMeshAttachment->super.worldVerticesLength);
			spVertexAttachment_computeWorldVertices(SUPER(pMeshAttachment), pSlot, 0, pMeshAttachment->super.worldVerticesLength, pTempVertices->items, 0, 2);
		}
		else
		{
			continue;
		}

		for (size_t i = 0; i < pTempVertices->size; i += 2)
		{
			float fX = pTempVertices->items[i];
			float fY = pTempVertices->items[i + 1LL];

			fMinX = fMinX < fX ? fMinX : fX;
			fMinY = fMinY < fY ? fMinY : fY;
			fMaxX = fMaxX > fX ? fMaxX : fX;
			fMaxY = fMaxY > fY ? fMaxY : fY;
		}
	}

	if (pTempVertices != nullptr)spFloatArray_dispose(pTempVertices);

	return DxLib::FLOAT4{ fMinX, fMinY, fMaxX - fMinX, fMaxY - fMinY };
}

DxLib::FLOAT4 CDxLibSpineDrawableC::getBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found) const
{
	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;
	float fMaxX = -FLT_MAX;
	float fMaxY = -FLT_MAX;

	spFloatArray* pTempVertices = spFloatArray_create(128);

	if (m_skeleton != nullptr)
	{
		for (int i = 0; i < m_skeleton->slotsCount; ++i)
		{
			spSlot* pSlot = m_skeleton->drawOrder[i];
			spAttachment* pAttachment = pSlot->attachment;
			if (pAttachment == nullptr)continue;

			size_t nLen = strlen(pSlot->data->name);
			if (nLen != nameLength)continue;

			if (::memcmp(pSlot->data->name, slotName, nLen) == 0)
			{
				if (pAttachment->type == SP_ATTACHMENT_REGION)
				{
					spRegionAttachment* pRegionAttachment = (spRegionAttachment*)pAttachment;

					spFloatArray_setSize(pTempVertices, 8);
#ifdef SPINE_4_1_OR_LATER
					spRegionAttachment_computeWorldVertices(pRegionAttachment, pSlot, pTempVertices->items, 0, 2);
#else
					spRegionAttachment_computeWorldVertices(pRegionAttachment, pSlot->bone, pTempVertices->items, 0, 2);
#endif
				}
				else if (pAttachment->type == SP_ATTACHMENT_MESH)
				{
					spMeshAttachment* pMeshAttachment = (spMeshAttachment*)pAttachment;

					spFloatArray_setSize(pTempVertices, pMeshAttachment->super.worldVerticesLength);
					spVertexAttachment_computeWorldVertices(SUPER(pMeshAttachment), pSlot, 0, pMeshAttachment->super.worldVerticesLength, pTempVertices->items, 0, 2);
				}
				else
				{
					continue;
				}

				for (size_t i = 0; i < pTempVertices->size; i += 2)
				{
					float fX = pTempVertices->items[i];
					float fY = pTempVertices->items[i + 1LL];

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

	if (pTempVertices != nullptr)spFloatArray_dispose(pTempVertices);

	return DxLib::FLOAT4{ fMinX, fMinY, fMaxX - fMinX, fMaxY - fMinY };
}

void CDxLibSpineDrawableC::clearLeaveOutList()
{
	if (m_leaveOutList != nullptr)
	{
		for (int i = 0; i < m_leaveOutListCount; ++i)
		{
			if (m_leaveOutList[i] != nullptr)FREE(m_leaveOutList[i]);
		}
		FREE(m_leaveOutList);
	}
	m_leaveOutList = nullptr;
	m_leaveOutListCount = 0;
}

bool CDxLibSpineDrawableC::isSlotToBeLeftOut(const char* slotName)
{
	if (m_pLeaveOutCallback != nullptr)
	{
		return m_pLeaveOutCallback(slotName, strlen(slotName));
	}
	else
	{
		for (int i = 0; i < m_leaveOutListCount; ++i)
		{
			if (strcmp(slotName, m_leaveOutList[i]) == 0)return true;
		}
	}

	return false;
}
