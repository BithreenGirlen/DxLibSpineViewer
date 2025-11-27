#ifndef DXLIB_SPINE_PLAYER_C_H_
#define DXLIB_SPINE_PLAYER_C_H_

#include "spine_player_c.h"

class CDxLibSpinePlayerC : public CSpinePlayerC
{
public:
	CDxLibSpinePlayerC();
	~CDxLibSpinePlayerC();

	void Redraw();

	DxLib::MATRIX CalculateTransformMatrix() const;
	DxLib::FLOAT4 GetCurrentBoundingOfSlot(const std::string& slotName) const;
private:
	virtual void WorkOutDefaultScale();
	virtual void WorkOutDefaultOffset();
};
#endif // !DXLIB_SPINE_PLAYER_C_H_
