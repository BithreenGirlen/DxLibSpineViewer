#ifndef DXLIB_SPINE_PLAYER_H_
#define DXLIB_SPINE_PLAYER_H_

#include "spine_player.h"

class CDxLibSpinePlayer : public CSpinePlayer
{
public:
	CDxLibSpinePlayer() = default;
	virtual ~CDxLibSpinePlayer() = default;

	void draw();

	DxLib::MATRIX calculateTransformMatrix() const noexcept;
	DxLib::FLOAT4 getCurrentBoundingBox() const;
	DxLib::FLOAT4 getCurrentBoundingBoxOfSlot(const std::string& slotName) const;
private:
	void workOutDefaultScale() override;
	void workOutDefaultSizeAndOffset() override;
};
#endif // !DXLIB_SPINE_PLAYER_H_
