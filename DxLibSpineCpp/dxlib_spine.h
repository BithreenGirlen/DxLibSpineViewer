#ifndef DXLIB_SPINE_H_
#define DXLIB_SPINE_H_

/*avoid conflict between <MathUtils.h> and <Windows.h>*/
#undef min
#undef max
#include <spine/spine.h>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

class CDxLibSpineDrawable
{
public:
	CDxLibSpineDrawable(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData = nullptr);
	~CDxLibSpineDrawable();

	spine::Skeleton* skeleton = nullptr;
	spine::AnimationState* animationState = nullptr;

	/// @brief Whether alpha is premultiplied or not. For Spine 4.0 and later, this property is exported with atlas file,
	///	       but for Spine 3.8, should be configured based on other means.
	bool isAlphaPremultiplied = true; 
	bool isToForceBlendModeNormal = false;

	void Update(float fDelta);
	void Draw();

	/// @brief Set slots to be excluded from rendering
	void SetLeaveOutList(spine::Vector<spine::String> &list);
	void SetLeaveOutCallback(bool (*pFunc)(const char*, size_t)) { m_pLeaveOutCallback = pFunc; }

	DxLib::FLOAT4 GetBoundingBox() const;
private:
	bool m_hasOwnAnimationStateData = false;

	spine::Vector<float> m_worldVertices;
	spine::Vector<DxLib::VERTEX2D> m_dxLibVertices;

	spine::Vector<unsigned short> m_quadIndices;

	spine::SkeletonClipping m_clipper;

	spine::Vector<spine::String> m_leaveOutList;
	bool (*m_pLeaveOutCallback)(const char*, size_t) = nullptr;

	bool IsToBeLeftOut(const spine::String& slotName);
};

class CDxLibTextureLoader : public spine::TextureLoader
{
public:
	CDxLibTextureLoader() {};
	virtual ~CDxLibTextureLoader() {};

	virtual void load(spine::AtlasPage& page, const spine::String& path);
	virtual void unload(void* texture);
};

#endif // DXLIB_SPINE_H_
