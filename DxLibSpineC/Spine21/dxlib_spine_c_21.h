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

	spSkeleton* skeleton() const noexcept;
	spAnimationState* animationState() const noexcept;

	void premultiplyAlpha(bool premultiplied) noexcept;
	bool isAlphaPremultiplied() const noexcept;

	void forceBlendModeNormal(bool toForce) noexcept;
	bool isBlendModeNormalForced() const noexcept;

	void update(float fDelta);
	void draw();

	void setLeaveOutList(const char** list, int listCount);
	void setLeaveOutCallback(bool (*pFunc)(const char*, size_t)) { m_pLeaveOutCallback = pFunc; }

	DxLib::FLOAT4 getBoundingBox() const;
	DxLib::FLOAT4 getBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found = nullptr) const;
private:
	bool m_hasOwnAnimationStateData = false;
	bool m_isAlphaPremultiplied = true;
	bool m_isToForceBlendModeNormal = false;

	spSkeleton* m_skeleton = nullptr;
	spAnimationState* m_animationState = nullptr;

	spFloatArray* m_worldVertices = nullptr;
	spDxLibVertexArray* m_dxLibVertices = nullptr;
	spUnsignedShortArray* m_dxLibIndices = nullptr;

	char** m_leaveOutList = nullptr;
	int m_leaveOutListCount = 0;
	bool (*m_pLeaveOutCallback)(const char*, size_t) = nullptr;

	void clearLeaveOutList();
	bool isSlotToBeLeftOut(const char* slotName);
};
#endif // !DXLIB_SPINE_C_21_H_
