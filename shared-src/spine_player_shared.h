#ifndef SPINE_PLAYER_SHARED_H_
#define SPINE_PLAYER_SHARED_H_

#ifdef DXLIB_SPINE_CPP
#include "../DxLibSpineCpp/dxlib_spine_player.h"
#elif DXLIB_SPINE_C
#include "../DxLibSpineC/dxlib_spine_player_c.h"
using CDxLibSpinePlayer = CDxLibSpinePlayerC;
#endif

#endif // !SPINE_PLAYER_SHARED_H_
