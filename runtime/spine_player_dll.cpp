
#include "spine_player_dll.h"

#if defined(SPINE_C)
#include "DxLibSpineC/dxlib_spine_player_c.h"
using CDxLibSpinePlayer = CDxLibSpinePlayerC;
#elif defined(SPINE_CPP)
#include "DxLibSpineCpp/dxlib_spine_player.h"
#endif

#if defined(SPINE_21)
#define SPINE_VERSION 21
#elif defined(SPINE_35)
#define SPINE_VERSION 35
#elif defined(SPINE_36)
#define SPINE_VERSION 36
#elif defined(SPINE_37)
#define SPINE_VERSION 37
#elif defined(SPINE_38)
#define SPINE_VERSION 38
#elif defined(SPINE_40)
#define SPINE_VERSION 40
#elif defined(SPINE_41)
#define SPINE_VERSION 41
#elif defined(SPINE_42)
#define SPINE_VERSION 42
#else
#define SPINE_VERSION 38
#endif

#define SPINE_PLAYER(VERSION) CSpinePlayer##VERSION
#define SPINE_PLAYER_VERSION(VERSION) SPINE_PLAYER(VERSION)

#define SPCLASS SPINE_PLAYER_VERSION(SPINE_VERSION)

class SPCLASS : public ISpinePlayer
{
public:
	SPCLASS() = default;
	virtual ~SPCLASS() = default;

	bool loadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel) override;
	bool loadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& textureDirectories, const std::vector<std::string>& skelData, bool isBinarySkel) override;
	bool addSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool isBinarySkel) override;

	size_t getNumberOfSpines() const noexcept override;
	bool hasSpineBeenLoaded() const noexcept override;

	void update(float fDelta) override;
	void draw() override;

	void resetScale() override;

	void addOffset(int iX, int iY) override;

	void shiftAnimation() override;
	void shiftSkin() override;

	void setAnimationByIndex(size_t nIndex) override;
	void setAnimationByName(const char* szAnimationName) override;
	void restartAnimation(bool loop = true) override;

	void setSkinByIndex(size_t nIndex) override;
	void setSkinByName(const char* szSkinName) override;
	void setupSkin() override;

	void togglePma() override;
	void toggleBlendModeAdoption() override;
	void togglePause() override;
	void toggleVisibility() override;

	bool isAlphaPremultiplied(size_t nDrawableIndex = 0) override;
	bool premultiplyAlpha(bool isToBePremultiplied, size_t nDrawableIndex = 0) override;

	bool isBlendModeNormalForced(size_t nDrawableIndex = 0) override;
	bool forceBlendModeNormal(bool isToForce, size_t nDrawableIndex = 0) override;

	bool isPaused(size_t nDrawableIndex = 0) override;
	bool setPause(bool paused, size_t nDrawableIndex = 0) override;

	bool isVisible(size_t nDrawableIndex = 0) override;
	bool setVisibility(bool visible, size_t nDrawableIndex = 0) override;

	void setDrawOrder(bool isToBeReversed) override;
	bool isDrawOrderReversed() const noexcept override;

	std::string getCurrentAnimationName() override;
	void getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd) override;
	void setCurrentAnimationTime(float animationTime) override;
	float getAnimationDuration(const char* animationName) override;

	const std::vector<std::string>& getSlotNames() const noexcept override;
	const std::vector<std::string>& getSkinNames() const noexcept override;
	const std::vector<std::string>& getAnimationNames() const noexcept override;

	void setSlotsToExclude(const std::vector<std::string>& slotNames) override;
	void mixSkins(const std::vector<std::string>& skinNames) override;
	void addAnimationTracks(const std::vector<std::string>& animationNames, bool loop = false) override;
	void mixAnimations(const char* fadeOutAnimationName, const char* fadeInAnimationName, float mixTime) override;
	void clearMixedAnimation() override;

	void setSlotExcludeCallback(bool (*pFunc)(const char*, size_t)) override;

	std::unordered_map<std::string, std::vector<std::string>> getSlotNamesWithTheirAttachments() override;
	bool replaceAttachment(const char* szSlotName, const char* szAttachmentName) override;

	DxLib::FLOAT2 getBaseSize() const noexcept override;
	void setBaseSize(float fWidth, float fHeight) override;
	void resetBaseSize() override;

	DxLib::FLOAT2 getOffset() const noexcept override;
	void setOffset(float fX, float fY) noexcept override;

	float getSkeletonScale() const noexcept override;
	void setSkeletonScale(float fScale) override;

	float getCanvasScale() const noexcept override;
	void setCanvasScale(float fScale) override;

	float getTimeScale() const noexcept override;
	void setTimeScale(float fTimeScale) override;

	DxLib::MATRIX calculateTransformMatrix() const noexcept override;
	DxLib::FLOAT4 getCurrentBoundingOfSlot(const std::string& slotName) const override;
private:
	CDxLibSpinePlayer m_dxLibSpinePlayer;
};

#define CREATE_SPINE_PLAYER(VERSION) CreateSpinePlayer##VERSION
#define CREATE_SPINE_PLAYER_VERSION(VERSION) CREATE_SPINE_PLAYER(VERSION)
#define SPCREATE CREATE_SPINE_PLAYER_VERSION(SPINE_VERSION)

ISpinePlayer* SPCREATE()
{
	return new SPCLASS();
}

#define DESTROY_SPINE_PLAYER(VERSION) DestroySpinePlayer##VERSION
#define DESTROY_SPINE_PLAYER_VERSION(VERSION) DESTROY_SPINE_PLAYER(VERSION)
#define SPDESTROY DESTROY_SPINE_PLAYER_VERSION(SPINE_VERSION)

void SPDESTROY(ISpinePlayer* pSpinePlayer)
{
	delete pSpinePlayer;
}

bool SPCLASS::loadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
{
	return m_dxLibSpinePlayer.loadSpineFromFile(atlasPaths, skelPaths, isBinarySkel);
}

bool SPCLASS::loadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& textureDirectories, const std::vector<std::string>& skelData, bool isBinarySkel)
{
	return m_dxLibSpinePlayer.loadSpineFromMemory(atlasData, textureDirectories, skelData, isBinarySkel);
}

bool SPCLASS::addSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool isBinarySkel)
{
	return m_dxLibSpinePlayer.addSpineFromFile(szAtlasPath, szSkelPath, isBinarySkel);
}

size_t SPCLASS::getNumberOfSpines() const noexcept
{
	return m_dxLibSpinePlayer.getNumberOfSpines();
}

bool SPCLASS::hasSpineBeenLoaded() const noexcept
{
	return m_dxLibSpinePlayer.hasSpineBeenLoaded();
}

void SPCLASS::update(float fDelta)
{
	m_dxLibSpinePlayer.update(fDelta);
}

void SPCLASS::draw()
{
	m_dxLibSpinePlayer.draw();
}

void SPCLASS::resetScale()
{
	m_dxLibSpinePlayer.resetScale();
}

void SPCLASS::addOffset(int iX, int iY)
{
	m_dxLibSpinePlayer.addOffset(iX, iY);
}

void SPCLASS::shiftAnimation()
{
	m_dxLibSpinePlayer.shiftAnimation();
}

void SPCLASS::shiftSkin()
{
	m_dxLibSpinePlayer.shiftSkin();
}

void SPCLASS::setAnimationByIndex(size_t nIndex)
{
	m_dxLibSpinePlayer.setAnimationByIndex(nIndex);
}

void SPCLASS::setAnimationByName(const char* szAnimationName)
{
	m_dxLibSpinePlayer.setAnimationByName(szAnimationName);
}

void SPCLASS::restartAnimation(bool loop)
{
	m_dxLibSpinePlayer.restartAnimation(loop);
}

void SPCLASS::setSkinByIndex(size_t nIndex)
{
	m_dxLibSpinePlayer.setSkinByIndex(nIndex);
}

void SPCLASS::setSkinByName(const char* szSkinName)
{
	m_dxLibSpinePlayer.setSkinByName(szSkinName);
}

void SPCLASS::setupSkin()
{
	m_dxLibSpinePlayer.setupSkin();
}

void SPCLASS::togglePma()
{
	m_dxLibSpinePlayer.togglePma();
}

void SPCLASS::toggleBlendModeAdoption()
{
	m_dxLibSpinePlayer.toggleBlendModeAdoption();
}

void SPCLASS::togglePause()
{
	m_dxLibSpinePlayer.togglePause();
}

void SPCLASS::toggleVisibility()
{
	m_dxLibSpinePlayer.toggleVisibility();
}

bool SPCLASS::isAlphaPremultiplied(size_t nDrawableIndex)
{
	return m_dxLibSpinePlayer.isAlphaPremultiplied(nDrawableIndex);
}

bool SPCLASS::premultiplyAlpha(bool isToBePremultiplied, size_t nDrawableIndex)
{
	return m_dxLibSpinePlayer.premultiplyAlpha(isToBePremultiplied, nDrawableIndex);
}

bool SPCLASS::isBlendModeNormalForced(size_t nDrawableIndex)
{
	return m_dxLibSpinePlayer.isBlendModeNormalForced(nDrawableIndex);
}

bool SPCLASS::forceBlendModeNormal(bool isToForce, size_t nDrawableIndex)
{
	return m_dxLibSpinePlayer.forceBlendModeNormal(isToForce, nDrawableIndex);
}

bool SPCLASS::isPaused(size_t nDrawableIndex)
{
	return m_dxLibSpinePlayer.isPaused(nDrawableIndex);
}

bool SPCLASS::setPause(bool paused, size_t nDrawableIndex)
{
	return m_dxLibSpinePlayer.setPause(paused, nDrawableIndex);
}

bool SPCLASS::isVisible(size_t nDrawableIndex)
{
	return m_dxLibSpinePlayer.isVisible(nDrawableIndex);
}

bool SPCLASS::setVisibility(bool visible, size_t nDrawableIndex)
{
	return m_dxLibSpinePlayer.setVisibility(visible, nDrawableIndex);
}

bool SPCLASS::isDrawOrderReversed() const noexcept
{
	return m_dxLibSpinePlayer.isDrawOrderReversed();
}

void SPCLASS::setDrawOrder(bool isToBeReversed)
{
	m_dxLibSpinePlayer.setDrawOrder(isToBeReversed);
}

std::string SPCLASS::getCurrentAnimationName()
{
	return m_dxLibSpinePlayer.getCurrentAnimationName();
}

float SPCLASS::getAnimationDuration(const char* animationName)
{
	return m_dxLibSpinePlayer.getAnimationDuration(animationName);
}

void SPCLASS::getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd)
{
	m_dxLibSpinePlayer.getCurrentAnimationTime(fTrack, fLast, fStart, fEnd);
}

void SPCLASS::setCurrentAnimationTime(float animationTime)
{
	m_dxLibSpinePlayer.setCurrentAnimationTime(animationTime);
}

const std::vector<std::string>& SPCLASS::getSlotNames() const noexcept
{
	return m_dxLibSpinePlayer.getSlotNames();
}

const std::vector<std::string>& SPCLASS::getSkinNames() const noexcept
{
	return m_dxLibSpinePlayer.getSkinNames();
}

const std::vector<std::string>& SPCLASS::getAnimationNames() const noexcept
{
	return m_dxLibSpinePlayer.getAnimationNames();
}

void SPCLASS::setSlotsToExclude(const std::vector<std::string>& slotNames)
{
	m_dxLibSpinePlayer.setSlotsToExclude(slotNames);
}

void SPCLASS::mixSkins(const std::vector<std::string>& skinNames)
{
	m_dxLibSpinePlayer.mixSkins(skinNames);
}

void SPCLASS::addAnimationTracks(const std::vector<std::string>& animationNames, bool loop)
{
	m_dxLibSpinePlayer.addAnimationTracks(animationNames, loop);
}

void SPCLASS::mixAnimations(const char* fadeOutAnimationName, const char* fadeInAnimationName, float mixTime)
{
	m_dxLibSpinePlayer.mixAnimations(fadeOutAnimationName, fadeInAnimationName, mixTime);
}

void SPCLASS::clearMixedAnimation()
{
	m_dxLibSpinePlayer.clearMixedAnimation();
}

void SPCLASS::setSlotExcludeCallback(bool(*pFunc)(const char*, size_t))
{
	m_dxLibSpinePlayer.setSlotExcludeCallback(pFunc);
}

std::unordered_map<std::string, std::vector<std::string>> SPCLASS::getSlotNamesWithTheirAttachments()
{
	return m_dxLibSpinePlayer.getSlotNamesWithTheirAttachments();
}

bool SPCLASS::replaceAttachment(const char* szSlotName, const char* szAttachmentName)
{
	return m_dxLibSpinePlayer.replaceAttachment(szSlotName, szAttachmentName);
}

DxLib::FLOAT2 SPCLASS::getBaseSize() const noexcept
{
	const auto& size = m_dxLibSpinePlayer.getBaseSize();
	return { size.x, size.y };
}

void SPCLASS::setBaseSize(float fWidth, float fHeight)
{
	m_dxLibSpinePlayer.setBaseSize(fWidth, fHeight);
}

void SPCLASS::resetBaseSize()
{
	m_dxLibSpinePlayer.resetBaseSize();
}

DxLib::FLOAT2 SPCLASS::getOffset() const noexcept
{
	const auto& offset = m_dxLibSpinePlayer.getOffset();
	return { offset.x, offset.y };
}
void SPCLASS::setOffset(float fX, float fY) noexcept
{
	m_dxLibSpinePlayer.setOffset(fX, fY);
}

float SPCLASS::getSkeletonScale() const noexcept
{
	return m_dxLibSpinePlayer.getSkeletonScale();
}

void SPCLASS::setSkeletonScale(float fScale)
{
	m_dxLibSpinePlayer.setSkeletonScale(fScale);
}

float SPCLASS::getCanvasScale() const noexcept
{
	return m_dxLibSpinePlayer.getCanvasScale();
}

void SPCLASS::setCanvasScale(float fScale)
{
	m_dxLibSpinePlayer.setCanvasScale(fScale);
}

float SPCLASS::getTimeScale() const noexcept
{
	return m_dxLibSpinePlayer.getTimeScale();
}

void SPCLASS::setTimeScale(float fTimeScale)
{
	m_dxLibSpinePlayer.setTimeScale(fTimeScale);
}

DxLib::MATRIX SPCLASS::calculateTransformMatrix() const noexcept
{
	return m_dxLibSpinePlayer.calculateTransformMatrix();
}

DxLib::FLOAT4 SPCLASS::getCurrentBoundingOfSlot(const std::string& slotName) const
{
	return m_dxLibSpinePlayer.getCurrentBoundingOfSlot(slotName);
}

