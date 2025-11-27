#ifndef DXLIB_SPINE_PLAYER_H_
#define DXLIB_SPINE_PLAYER_H_

#include "spine_player.h"

class CDxLibSpinePlayer : public CSpinePlayer
{
public:
	CDxLibSpinePlayer();
	~CDxLibSpinePlayer();

	void Redraw();

	DxLib::MATRIX CalculateTransformMatrix() const;
	DxLib::FLOAT4 GetCurrentBoundingOfSlot(const std::string& slotName) const;
private:
	virtual void WorkOutDefaultScale();
	virtual void WorkOutDefaultOffset();
};
#endif // !DXLIB_SPINE_PLAYER_H_
