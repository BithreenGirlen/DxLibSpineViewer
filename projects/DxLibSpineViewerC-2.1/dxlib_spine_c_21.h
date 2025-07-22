#ifndef DXLIB_SPINE_C_21_H_
#define DXLIB_SPINE_C_21_H_

#include <spine/spine.h>
#include <spine/array.h>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

_SP_ARRAY_DECLARE_TYPE(spDxLibVertexArray, DxLib::VERTEX2D)

class CDxLibSpineDrawableC21
{
public:
	CDxLibSpineDrawableC21(spSkeletonData* pSkeletonData, spAnimationStateData* pAnimationStateData = nullptr);
	~CDxLibSpineDrawableC21();

	spSkeleton* skeleton = nullptr;
	spAnimationState* animationState = nullptr;

	bool isAlphaPremultiplied = true;
	bool isToForceBlendModeNormal = false;

	void Update(float fDelta);
	void Draw();

	void SetLeaveOutList(const char** list, int listCount);
	void SetLeaveOutCallback(bool (*pFunc)(const char*, size_t)) { m_pLeaveOutCallback = pFunc; }

	DxLib::FLOAT4 GetBoundingBox() const;
private:
	bool m_hasOwnAnimationStateData = false;

	spFloatArray* m_worldVertices = nullptr;
	spDxLibVertexArray* m_dxLibVertices = nullptr;
	spUnsignedShortArray* m_dxLibIndices = nullptr;

	char** m_leaveOutList = nullptr;
	int m_leaveOutListCount = 0;
	bool (*m_pLeaveOutCallback)(const char*, size_t) = nullptr;

	void ClearLeaveOutList();
	bool IsToBeLeftOut(const char* slotName);
};
#endif // !DXLIB_SPINE_C_21_H_
