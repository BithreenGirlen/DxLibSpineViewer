#ifndef DXLIB_SPINE_PLAYER_H_
#define DXLIB_SPINE_PLAYER_H_

#include "spine_player.h"

class CDxLibSpinePlayer : public CSpinePlayer
{
public:
	CDxLibSpinePlayer();
	~CDxLibSpinePlayer();

	virtual void Redraw();
private:
	virtual void WorkOutDefaultScale();
	virtual void WorkOutDefaultOffset();

	void SetTransformMatrix() const;
};
#endif // !DXLIB_SPINE_PLAYER_H_
