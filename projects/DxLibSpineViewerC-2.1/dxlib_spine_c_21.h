﻿#ifndef DXLIB_SPINE_C_21_H_
#define DXLIB_SPINE_C_21_H_

#include <spine/spine.h>
#include <spine/array.h>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

_SP_ARRAY_DECLARE_TYPE(spDxLibVertexArray, DxLib::VERTEX2D)

class CDxLibSpineDrawerC21
{
public:
	CDxLibSpineDrawerC21(spSkeletonData* pSkeletonData, spAnimationStateData* pAnimationStateData = nullptr);
	~CDxLibSpineDrawerC21();

	spSkeleton* skeleton = nullptr;
	spAnimationState* animationState = nullptr;
	float timeScale = 1.f;

	void Update(float fDelta);
	void Draw(float fDepth = 0.f, float fScale = 1.f);

	void SwitchPma() { m_bAlphaPremultiplied ^= true; }
	void SwitchBlendModeAdoption() { m_bForceBlendModeNormal ^= true; }
	void SetLeaveOutList(const char** list, int listCount);
private:
	bool m_bHasOwnAnimationStateData = false;
	bool m_bAlphaPremultiplied = true;
	bool m_bForceBlendModeNormal = false;

	spFloatArray* m_worldVertices = nullptr;
	spDxLibVertexArray* m_dxLibVertices = nullptr;
	spUnsignedShortArray* m_dxLibIndices = nullptr;

	char** m_leaveOutList = nullptr;
	int m_leaveOutListCount = 0;

	void ClearLeaveOutList();
	bool IsToBeLeftOut(const char* slotName);
};
#endif // !DXLIB_SPINE_C_21_H_
