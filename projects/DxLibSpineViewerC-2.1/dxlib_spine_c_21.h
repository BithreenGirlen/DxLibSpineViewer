#ifndef DXLIB_SPINE_C_21_H_
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

	bool isAlphaPremultiplied = true;
	bool isToForceBlendModeNormal = false;

	void Update(float fDelta);
	void Draw();

	void SetLeaveOutList(const char** list, int listCount);

	DxLib::FLOAT4 GetBoundingBox() const;
private:
	bool m_hasOwnAnimationStateData = false;

	spFloatArray* m_worldVertices = nullptr;
	spDxLibVertexArray* m_dxLibVertices = nullptr;
	spUnsignedShortArray* m_dxLibIndices = nullptr;

	char** m_leaveOutList = nullptr;
	int m_leaveOutListCount = 0;

	void ClearLeaveOutList();
	bool IsToBeLeftOut(const char* slotName);
};
#endif // !DXLIB_SPINE_C_21_H_
