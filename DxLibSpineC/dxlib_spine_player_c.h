#ifndef DXLIB_SPINE_PLAYER_C_H_
#define DXLIB_SPINE_PLAYER_C_H_

#include "spine_player_c.h"

class CDxLibSpinePlayerC : public CSpinePlayerC
{
public:
	CDxLibSpinePlayerC();
	~CDxLibSpinePlayerC();

	void Redraw();

	DxLib::FLOAT4 GetCurrentBounding() const;
private:
	virtual void WorkOutDefaultScale();
	virtual void WorkOutDefaultOffset();

	void SetTransformMatrix() const;
};
#endif // !DXLIB_SPINE_PLAYER_C_H_
