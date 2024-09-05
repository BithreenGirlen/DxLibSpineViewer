#ifndef SPINE_PLAYER_SHARED_H_

#ifdef DXLIB_SPINE_CPP
#include "../DxlibSpineTest/dxlib_spine_player.h"
#elif DXLIB_SPINE_C
#include "../DxlibSpineTestC/dxlib_spine_player_c.h"
using CDxLibSpinePlayer = CDxLibSpinePlayerC
#endif

#endif // !SPINE_PLAYER_SHARED_H_
