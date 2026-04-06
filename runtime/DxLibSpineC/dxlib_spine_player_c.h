#ifndef DXLIB_SPINE_PLAYER_C_H_
#define DXLIB_SPINE_PLAYER_C_H_

#include "spine_player_c.h"

class CDxLibSpinePlayerC : public CSpinePlayerC
{
public:
	CDxLibSpinePlayerC();
	~CDxLibSpinePlayerC();

	void draw();

	DxLib::MATRIX calculateTransformMatrix() const noexcept;
	DxLib::FLOAT4 getCurrentBoundingBox() const;
	DxLib::FLOAT4 getCurrentBoundingBoxOfSlot(const std::string& slotName) const;
private:
	void workOutDefaultScale() override;
	void workOutDefaultOffset() override;
};
#endif // !DXLIB_SPINE_PLAYER_C_H_
