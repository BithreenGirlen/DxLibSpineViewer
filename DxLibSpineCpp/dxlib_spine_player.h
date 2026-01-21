#ifndef DXLIB_SPINE_PLAYER_H_
#define DXLIB_SPINE_PLAYER_H_

#include "spine_player.h"

class CDxLibSpinePlayer : public CSpinePlayer
{
public:
	CDxLibSpinePlayer();
	~CDxLibSpinePlayer();

	void redraw();

	DxLib::MATRIX calculateTransformMatrix() const noexcept;
	DxLib::FLOAT4 getCurrentBoundingOfSlot(const std::string& slotName) const;
private:
	void workOutDefaultScale() override;
	void workOutDefaultOffset() override;
};
#endif // !DXLIB_SPINE_PLAYER_H_
