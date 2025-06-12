
/* To calculate bounding box */
#include <float.h>

#include <spine/extension.h>

#include "dxlib_spine_c_21.h"

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

	wchar_t* pTemp = static_cast<wchar_t*>(realloc(pResult, iLen));
	if (pTemp != nullptr)
	{
		pResult = pTemp;
	}

	return pResult;
}

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

CDxLibSpineDrawerC21::CDxLibSpineDrawerC21(spSkeletonData* pSkeletonData, spAnimationStateData* pAnimationStateData)
{
	spBone_setYDown(-1);

	m_worldVertices = spFloatArray_create(128);
	m_dxLibVertices = spDxLibVertexArray_create(128);
	m_dxLibIndices = spUnsignedShortArray_create(128);

	skeleton = spSkeleton_create(pSkeletonData);
	if (pAnimationStateData == nullptr)
	{
		pAnimationStateData = spAnimationStateData_create(pSkeletonData);
		m_hasOwnAnimationStateData = true;
	}
	animationState = spAnimationState_create(pAnimationStateData);
}

CDxLibSpineDrawerC21::~CDxLibSpineDrawerC21()
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
		if (m_hasOwnAnimationStateData)
		{
			spAnimationStateData_dispose(animationState->data);
		}

		spAnimationState_dispose(animationState);
	}
	if (skeleton != nullptr)
	{
		spSkeleton_dispose(skeleton);
	}

	ClearLeaveOutList();
}

void CDxLibSpineDrawerC21::Update(float fDelta)
{
	if (skeleton == nullptr || animationState == nullptr)return;

	spSkeleton_update(skeleton, fDelta);
	spAnimationState_update(animationState, fDelta);
	spAnimationState_apply(animationState, skeleton);
	spSkeleton_updateWorldTransform(skeleton);
}

void CDxLibSpineDrawerC21::Draw()
{
	if (m_worldVertices == nullptr || skeleton == nullptr || animationState == nullptr)return;

	if (skeleton->a == 0) return;

	static int quadIndices[] = { 0, 1, 2, 2, 3, 0 };

	for (int i = 0; i < skeleton->slotsCount; ++i)
	{
		spSlot* pSlot = skeleton->drawOrder[i];
		spAttachment* pAttachment = pSlot->attachment;

		if (pAttachment == nullptr)
		{
			continue;
		}

		if (IsToBeLeftOut(pSlot->data->name))
		{
			continue;
		}

		spFloatArray* pVertices = m_worldVertices;
		float* pAttachmentUvs = nullptr;
		int* pIndices = nullptr;
		int indicesCount = 0;

		DxLib::COLOR_F attachmentColour{};

		int iDxLibTexture = -1;

		if (pAttachment->type == SP_ATTACHMENT_REGION)
		{
			spRegionAttachment* pRegionAttachment = (spRegionAttachment*)pAttachment;

			attachmentColour.r = pRegionAttachment->r;
			attachmentColour.g = pRegionAttachment->g;
			attachmentColour.b = pRegionAttachment->b;
			attachmentColour.a = pRegionAttachment->a;

			spFloatArray_setSize(pVertices, 8);
			spRegionAttachment_computeWorldVertices(pRegionAttachment, pSlot->bone, pVertices->items);
			pAttachmentUvs = pRegionAttachment->uvs;
			pIndices = quadIndices;
			indicesCount = sizeof(quadIndices) / sizeof(int);

			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(static_cast<spAtlasRegion*>(pRegionAttachment->rendererObject)->page->rendererObject)));
		}
		else if (pAttachment->type == SP_ATTACHMENT_MESH)
		{
			spMeshAttachment* pMeshAttachment = (spMeshAttachment*)pAttachment;

			attachmentColour.r = pMeshAttachment->r;
			attachmentColour.g = pMeshAttachment->g;
			attachmentColour.b = pMeshAttachment->b;
			attachmentColour.a = pMeshAttachment->a;

			spFloatArray_setSize(pVertices, pMeshAttachment->verticesCount);
			spMeshAttachment_computeWorldVertices(pMeshAttachment, pSlot, pVertices->items);
			pAttachmentUvs = pMeshAttachment->uvs;
			pIndices = pMeshAttachment->triangles;
			indicesCount = pMeshAttachment->trianglesCount;

			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(static_cast<spAtlasRegion*>(pMeshAttachment->rendererObject)->page->rendererObject)));
		}
		else if (pAttachment->type == SP_ATTACHMENT_SKINNED_MESH)
		{
			spSkinnedMeshAttachment* pSkinnedMeshAttachment = (spSkinnedMeshAttachment*)pAttachment;

			attachmentColour.r = pSkinnedMeshAttachment->r;
			attachmentColour.g = pSkinnedMeshAttachment->g;
			attachmentColour.b = pSkinnedMeshAttachment->b;
			attachmentColour.a = pSkinnedMeshAttachment->a;

			spFloatArray_setSize(pVertices, pSkinnedMeshAttachment->uvsCount);
			spSkinnedMeshAttachment_computeWorldVertices(pSkinnedMeshAttachment, pSlot, pVertices->items);
			pAttachmentUvs = pSkinnedMeshAttachment->uvs;
			pIndices = pSkinnedMeshAttachment->triangles;
			indicesCount = pSkinnedMeshAttachment->trianglesCount;

			iDxLibTexture = (static_cast<int>(reinterpret_cast<unsigned long long>(static_cast<spAtlasRegion*>(pSkinnedMeshAttachment->rendererObject)->page->rendererObject)));
		}
		else
		{
			continue;
		}

		DxLib::COLOR_F tint{};
		tint.r = skeleton->r * pSlot->r * attachmentColour.r;
		tint.g = skeleton->g * pSlot->g * attachmentColour.g;
		tint.b = skeleton->b * pSlot->b * attachmentColour.b;
		tint.a = skeleton->a * pSlot->a * attachmentColour.a;

		spDxLibVertexArray_clear(m_dxLibVertices);
		for (int ii = 0; ii < pVertices->size; ii += 2)
		{
			DxLib::VERTEX2D dxLibVertex{};
			dxLibVertex.pos.x = pVertices->items[ii];
			dxLibVertex.pos.y = pVertices->items[ii + 1LL];
			dxLibVertex.pos.z = 0.f;
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
		if (!isToForceBlendModeNormal && pSlot->data->additiveBlending)
		{
			iDxLibBlendMode = isAlphaPremultiplied ? DX_BLENDMODE_PMA_ADD : DX_BLENDMODE_SPINE_ADDITIVE;
		}
		else
		{
			iDxLibBlendMode = isAlphaPremultiplied ? DX_BLENDMODE_PMA_ALPHA : DX_BLENDMODE_SPINE_NORMAL;
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
	}
}

void CDxLibSpineDrawerC21::SetLeaveOutList(const char** list, int listCount)
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

DxLib::FLOAT4 CDxLibSpineDrawerC21::GetBoundingBox() const
{
	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;
	float fMaxX = -FLT_MAX;
	float fMaxY = -FLT_MAX;

	spFloatArray* pTempVertices = spFloatArray_create(128);

	for (int i = 0; i < skeleton->slotsCount; ++i)
	{
		spSlot* pSlot = skeleton->drawOrder[i];
		spAttachment* pAttachment = pSlot->attachment;

		if (pAttachment == nullptr)continue;

		if (pAttachment->type == SP_ATTACHMENT_REGION)
		{
			spRegionAttachment* pRegionAttachment = (spRegionAttachment*)pAttachment;

			spFloatArray_setSize(pTempVertices, 8);
			spRegionAttachment_computeWorldVertices(pRegionAttachment, pSlot->bone, pTempVertices->items);
		}
		else if (pAttachment->type == SP_ATTACHMENT_MESH)
		{
			spMeshAttachment* pMeshAttachment = (spMeshAttachment*)pAttachment;

			spFloatArray_setSize(pTempVertices, pMeshAttachment->verticesCount);
			spMeshAttachment_computeWorldVertices(pMeshAttachment, pSlot, pTempVertices->items);
		}
		else if (pAttachment->type == SP_ATTACHMENT_SKINNED_MESH)
		{
			spSkinnedMeshAttachment* pSkinnedMeshAttachment = (spSkinnedMeshAttachment*)pAttachment;

			spFloatArray_setSize(pTempVertices, pSkinnedMeshAttachment->uvsCount);
			spSkinnedMeshAttachment_computeWorldVertices(pSkinnedMeshAttachment, pSlot, pTempVertices->items);
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

void CDxLibSpineDrawerC21::ClearLeaveOutList()
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

bool CDxLibSpineDrawerC21::IsToBeLeftOut(const char* slotName)
{
	for (int i = 0; i < m_leaveOutListCount; ++i)
	{
		if (strcmp(slotName, m_leaveOutList[i]) == 0)return true;
	}
	return false;
}
