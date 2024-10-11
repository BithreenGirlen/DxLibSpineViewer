#ifndef DXLIB_SPINE_C_H_
#define DXLIB_SPINE_C_H_

#include <spine/spine.h>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

/*Backport from spine-c 4.1*/
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

class CDxLibSpineDrawerC
{
public:
	CDxLibSpineDrawerC(spSkeletonData* pSkeletonData, spAnimationStateData* pAnimationStateData = nullptr);
	~CDxLibSpineDrawerC();

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

	spSkeletonClipping* m_clipper = nullptr;

    char** m_leaveOutList = nullptr;
    int m_leaveOutListCount = 0;

    void ClearLeaveOutList();
    bool IsToBeLeftOut(const char* slotName);
};
#endif // !DXLIB_SPINE_C_H_
