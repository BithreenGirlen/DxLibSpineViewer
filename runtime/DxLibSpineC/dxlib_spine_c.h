#ifndef DXLIB_SPINE_C_H_
#define DXLIB_SPINE_C_H_

#include <spine/spine.h>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

#if defined(SPINE_RUNTIME_DLL_BUILD)
#include "../dxlib_spine_dll.h"
#endif

/* Backport from spine-c 4.1 */
#ifndef _SP_ARRAY_IMPLEMENT_TYPE_NO_CONTAINS
#define _SP_ARRAY_IMPLEMENT_TYPE_NO_CONTAINS(name, itemType) \
	name* name##_create(int initialCapacity) { \
		name* array = CALLOC(name, 1); \
		array->size = 0; \
		array->capacity = initialCapacity; \
		array->items = CALLOC(itemType, initialCapacity); \
		return array; \
	} \
	void name##_dispose(name* self) { \
		FREE(self->items); \
		FREE(self); \
	} \
	void name##_clear(name* self) { \
		self->size = 0; \
	} \
	name* name##_setSize(name* self, int newSize) { \
		self->size = newSize; \
		if (self->capacity < newSize) { \
			self->capacity = MAX(8, (int)(self->size * 1.75f)); \
			self->items = REALLOC(self->items, itemType, self->capacity); \
		} \
		return self; \
	} \
	void name##_ensureCapacity(name* self, int newCapacity) { \
		if (self->capacity >= newCapacity) return; \
		self->capacity = newCapacity; \
		self->items = REALLOC(self->items, itemType, self->capacity); \
	} \
	void name##_add(name* self, itemType value) { \
		if (self->size == self->capacity) { \
			self->capacity = MAX(8, (int)(self->size * 1.75f)); \
			self->items = REALLOC(self->items, itemType, self->capacity); \
		} \
		self->items[self->size++] = value; \
	} \
	void name##_addAll(name* self, name* other) { \
		int i = 0; \
		for (; i < other->size; i++) { \
			name##_add(self, other->items[i]); \
		} \
	} \
	void name##_addAllValues(name* self, itemType* values, int offset, int count) { \
		int i = offset, n = offset + count; \
		for (; i < n; i++) { \
			name##_add(self, values[i]); \
		} \
	} \
	void name##_removeAt(name* self, int index) { \
		self->size--; \
		memmove(self->items + index, self->items + index + 1, sizeof(itemType) * (self->size - index)); \
	} \
	itemType name##_pop(name* self) { \
		itemType item = self->items[--self->size]; \
		return item; \
	} \
	itemType name##_peek(name* self) { \
		return self->items[self->size - 1]; \
	}
#endif //_SP_ARRAY_IMPLEMENT_TYPE_NO_CONTAINS

_SP_ARRAY_DECLARE_TYPE(spDxLibVertexArray, DxLib::VERTEX2D)

class CDxLibSpineDrawableC
{
public:
	CDxLibSpineDrawableC(spSkeletonData* pSkeletonData);
	~CDxLibSpineDrawableC();

	spSkeleton* skeleton() const noexcept;
	spAnimationState* animationState() const noexcept;

	void premultiplyAlpha(bool premultiplied) noexcept;
	bool isAlphaPremultiplied() const noexcept;

	void forceBlendModeNormal(bool toForce) noexcept;
	bool isBlendModeNormalForced() const noexcept;

	void setPause(bool paused) noexcept;
	bool isPaused() const noexcept;

	void setVisibility(bool visible) noexcept;
	bool isVisible() const noexcept;

	/// @brief 物理演算法
	enum class Physics : unsigned char /* uint8_t is not defined here */
	{
		None = 0, /* 物理演算を行わない */
		Reset, /* 1フレーム前までの影響をリセットして新たに物理演算を開始する */
		Update, /* 物理演算を行い、通算の影響を反映させる */
		Pose, /* 1フレーム前の状態で静止させる */
		NotSupported = static_cast<unsigned char>(-1) /* To be used for Spine 4.1 and older */
	};

	/// @brief 物理演算方法指定。Spine4.2以降でのみ有効
	void setPhysics(Physics physics);
	Physics getPhysics() const noexcept;

	void update(float fDelta);
	void draw();

	void setLeaveOutList(const char** list, int listCount);
	void setLeaveOutCallback(bool (*pFunc)(const char*, size_t)) { m_pLeaveOutCallback = pFunc; }

	DxLib::FLOAT4 getBoundingBox();
	DxLib::FLOAT4 getBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found = nullptr);
private:
	bool m_isAlphaPremultiplied = true;
	bool m_isToForceBlendModeNormal = false;
	bool m_isVisible = true;
	bool m_isPaused = false;
	Physics m_physics = Physics::Update;

	spSkeleton* m_skeleton = nullptr;
	spAnimationState* m_animationState = nullptr;

	spFloatArray* m_worldVertices = nullptr;
	spDxLibVertexArray* m_dxLibVertices = nullptr;
	spSkeletonClipping* m_clipper = nullptr;

	/// @brief A buffer to be used to calculate bounding box.
	spFloatArray* m_tempVertices = nullptr;

	char** m_leaveOutList = nullptr;
	int m_leaveOutListCount = 0;
	bool (*m_pLeaveOutCallback)(const char*, size_t) = nullptr;

	void clearLeaveOutList();
	bool isSlotToBeLeftOut(const char* slotName);
};
#endif // !DXLIB_SPINE_C_H_
