#pragma once

/* Windows headers */
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <CommCtrl.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <atlbase.h>
/* Define __int64 prior to the definition of LONG_PTR in "DxDataTypeWin.h" */
#include <intsafe.h>

/* C headers */

/* C++ headers */
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>

/* Externals */
#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

#include <spine/spine.h>
