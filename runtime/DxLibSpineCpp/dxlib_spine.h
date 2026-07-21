#ifndef DXLIB_SPINE_H_
#define DXLIB_SPINE_H_

/* Avoid conflict between <MathUtils.h> and <Windows.h> for Spine 4.0 and older */
#undef min
#undef max
#include <spine/spine.h>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

#if defined(SPINE_RUNTIME_DLL_BUILD)
#include "../dxlib_spine_dll.h"
#endif

class CDxLibSpineDrawable
{
public:
	CDxLibSpineDrawable(spine::SkeletonData* pSkeletonData);
	~CDxLibSpineDrawable();

	spine::Skeleton* skeleton() const noexcept;
	spine::AnimationState* animationState() const noexcept;

	void premultiplyAlpha(bool premultiplied) noexcept;
	bool isAlphaPremultiplied() const noexcept;

	void forceBlendModeNormal(bool toForce) noexcept;
	bool isBlendModeNormalForced() const noexcept;

	void setPause(bool paused) noexcept;
	bool isPaused() const noexcept;

	void setVisibility(bool visible) noexcept;
	bool isVisible() const noexcept;

	/// @brief 物理演算法
	enum class Physics : unsigned char
	{
		None = 0, /* 物理演算を行わない */
		Reset, /* 1フレーム前までの影響をリセットして新たに物理演算を開始する */
		Update, /* 物理演算を行い、通算の影響を反映させる */
		Pose /* 1フレーム前の状態で静止させる */
	};

	/// @brief 物理演算方法指定。Spine4.2以降でのみ有効
	void setPhysics(Physics physics);
	Physics getPhysics() const noexcept;

	/// @brief Add animation time and update world transform.
	/// @remark Even if it is paused or 0.0f is passed, world transform will be updated.
	void update(float fDelta);
	void draw();

	/// @brief Set slots to be excluded from rendering
	void setLeaveOutList(spine::Vector<spine::String> &list);
	void setLeaveOutCallback(bool (*pFunc)(const char*, size_t)) { m_pLeaveOutCallback = pFunc; }

	DxLib::FLOAT4 getBoundingBox();
	DxLib::FLOAT4 getBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found = nullptr);
private:
	bool m_isAlphaPremultiplied = true;
	bool m_isToForceBlendModeNormal = false;
	bool m_isVisible = true;
	bool m_isPaused = false;
	Physics m_physics = Physics::Update;

	spine::Skeleton* m_skeleton = nullptr;
	spine::AnimationState* m_animationState = nullptr;

	spine::Vector<float> m_worldVertices;
	spine::Vector<DxLib::VERTEX2D> m_dxLibVertices;
	spine::Vector<unsigned short> m_quadIndices;
	spine::SkeletonClipping m_clipper;

	/// @brief A buffer to be used to calculate bounding box.
	spine::Vector<float> m_tempVertices;

	spine::Vector<spine::String> m_leaveOutList;
	bool (*m_pLeaveOutCallback)(const char*, size_t) = nullptr;

	bool IsToBeLeftOut(const spine::String& slotName);
};

class CDxLibTextureLoader : public spine::TextureLoader
{
public:
	void load(spine::AtlasPage& page, const spine::String& path) override;
	void unload(void* texture) override;
};

#endif // DXLIB_SPINE_H_
