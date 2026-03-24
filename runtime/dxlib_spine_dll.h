#ifndef DXLIB_SPINE_DLL_H_
#define DXLIB_SPINE_DLL_H_

#include <DxLib.h>

#if defined(_WIN32)
#if defined(SPINE_RUNTIME_DLL_BUILD)
#define SPINE_EXTERN __declspec(dllexport)
#else
#define SPINE_EXTERN __declspec(dllimport)
#endif
#else
#define SPINE_EXTERN 
#endif

/* DxLib functions to be registered on executable side. */
struct DxLibRegerenda
{
#if defined(_WIN32) && defined(_UNICODE)
	/* To convert UTF8 to UTF16. */
	int (*GetUseCharCodeFormat)(void);
	int (*Get_wchar_t_CharCodeFormat)(void);
	int (*ConvertStringCharCodeFormat)(int SrcCharCodeFormat, const void* SrcString, int DestCharCodeFormat, void* DestStringBuffer);
#endif
	/* To create and destroy texture. */
	int (*LoadGraph)(const TCHAR* FileName, int NotUse3DFlag);
	int (*DeleteGraph)(int GrHandle);

	/* To render indexed vertices. */
	int (*SetDrawCustomBlendMode)(int BlendEnable, int SrcBlendRGB, int DestBlendRGB, int BlendOpRGB, int SrcBlendA, int DestBlendA, int BlendOpA, int BlendParam);
	int (*SetDrawBlendMode)(int BlendMode, int BlendParam);
	int (*DrawPolygonIndexed2D)(const DxLib::VERTEX2D* VertexArray, int VertexNum, const unsigned short* IndexArray, int PolygonNum, int GrHandle, int TransFlag);

	/* To calculate transform matrix. */
	int (*GetDrawScreenSize)(int* XBuf, int* YBuf);
	DxLib::MATRIX (*MGetScale)(DxLib::VECTOR Scale);
	DxLib::MATRIX (*MGetTranslate)(DxLib::VECTOR Trans);
	DxLib::MATRIX (*MMult)(DxLib::MATRIX In1, DxLib::MATRIX In2);
	int (*SetTransformTo2D)(const DxLib::MATRIX* Matrix);
	int (*ResetTransformTo2D)(void);
#if defined _WIN32
	/* This does not corresponds to the two subsequent functions in that this returns the largest one in configuration, not the current one. */
	int (*GetDisplayMaxResolution)(int* SizeX, int* SizeY, int DisplayIndex);
#elif defined __ANDROID__
	int (*GetAndroidDisplayResolution)(int* SizeX, int* SizeY);
#elif defined __APPLE__
	int (*GetDisplayResolution_iOS)(int* SizeX, int* SizeY);
#endif
};

extern "C"
{
	SPINE_EXTERN void RegisterDxLibFunctions(const DxLibRegerenda* pDxLibRegerenda);
}

#if !defined(SPINE_RUNTIME_DLL_BUILD)
const DxLibRegerenda* GetDxLibFunctonsToBeRegistered();
#endif

#endif // !DXLIB_SPINE_DLL_H_
