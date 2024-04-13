#ifndef DXLIB_SPINE_H_
#define DXLIB_SPINE_H_

/*avoid conflict between <MathUtils.h> and <Windows.h>*/
#undef min
#undef max
#include <spine/spine.h>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

class CDxLibSpineDrawer
{
public:
	CDxLibSpineDrawer(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData = nullptr);
	~CDxLibSpineDrawer();

	spine::Skeleton* skeleton = nullptr;
	spine::AnimationState* state = nullptr;
	float timeScale = 1.f;

	void Update(float fDelta);
	void Draw();
	void SwitchPma()const { m_bAlphaPremultiplied ^= true; }
private:
	mutable bool m_bHasOwnAnimationStateData = false;
	mutable bool m_bAlphaPremultiplied = true;

	mutable spine::Vector<float> m_worldVertices;
	mutable spine::Vector<DxLib::VERTEX2D> m_dxLibVertices;
	mutable spine::Vector<unsigned short> m_dxLibIndices;

	spine::SkeletonClipping m_clipper;

	spine::Vector<unsigned short> m_quadIndices;
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
