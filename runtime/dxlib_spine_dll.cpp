
#include "dxlib_spine_dll.h"

#if defined(SPINE_RUNTIME_DLL_BUILD)
static DxLibRegerenda g_dxLibRegerenda;

namespace DxLib
{
	#if	defined(_WIN32) && defined(_UNICODE)
	extern int GetUseCharCodeFormat(void)
	{
		return g_dxLibRegerenda.GetUseCharCodeFormat();
	}

	extern int Get_wchar_t_CharCodeFormat(void)
	{
		return g_dxLibRegerenda.Get_wchar_t_CharCodeFormat();
	}

	extern int ConvertStringCharCodeFormat(int SrcCharCodeFormat, const void* SrcString, int DestCharCodeFormat, void* DestStringBuffer)
	{
		return g_dxLibRegerenda.ConvertStringCharCodeFormat(SrcCharCodeFormat, SrcString, DestCharCodeFormat, DestStringBuffer);
	}
	#endif

	extern int LoadGraph(const TCHAR* FileName, int NotUse3DFlag)
	{
		return g_dxLibRegerenda.LoadGraph(FileName, NotUse3DFlag);
	}

	extern int DeleteGraph(int GrHandle)
	{
		return g_dxLibRegerenda.DeleteGraph(GrHandle);
	}

	extern int SetDrawCustomBlendMode(int BlendEnable, int SrcBlendRGB, int DestBlendRGB, int BlendOpRGB, int SrcBlendA, int DestBlendA, int BlendOpA, int BlendParam)
	{
		return g_dxLibRegerenda.SetDrawCustomBlendMode(BlendEnable, SrcBlendRGB, DestBlendRGB, BlendOpRGB, SrcBlendA, DestBlendA, BlendOpA, BlendParam);
	}

	extern int SetDrawBlendMode(int BlendMode, int BlendParam)
	{
		return g_dxLibRegerenda.SetDrawBlendMode(BlendMode, BlendParam);
	}

	extern int DrawPolygonIndexed2D(const VERTEX2D* VertexArray, int VertexNum, const unsigned short* IndexArray, int PolygonNum, int GrHandle, int TransFlag)
	{
		return g_dxLibRegerenda.DrawPolygonIndexed2D(VertexArray, VertexNum, IndexArray, PolygonNum, GrHandle, TransFlag);
	}

	extern int GetDrawScreenSize(int* XBuf, int* YBuf)
	{
		return g_dxLibRegerenda.GetDrawScreenSize(XBuf, YBuf);
	}

	extern MATRIX MGetScale(VECTOR Scale)
	{
		return g_dxLibRegerenda.MGetScale(Scale);
	}

	extern MATRIX MGetTranslate(VECTOR Trans)
	{
		return g_dxLibRegerenda.MGetTranslate(Trans);
	}

	extern MATRIX MMult(MATRIX In1, MATRIX In2)
	{
		return g_dxLibRegerenda.MMult(In1, In2);
	}

	extern int SetTransformTo2D(const MATRIX* Matrix)
	{
		return g_dxLibRegerenda.SetTransformTo2D(Matrix);
	}

	extern int ResetTransformTo2D(void)
	{
		return g_dxLibRegerenda.ResetTransformTo2D();
	}

#if defined _WIN32
	extern int GetDisplayMaxResolution(int* SizeX, int* SizeY, int DisplayIndex)
	{
		return g_dxLibRegerenda.GetDisplayMaxResolution(SizeX, SizeY, DisplayIndex);
	}
#elif defined __ANDROID__
	extern int GetAndroidDisplayResolution(int* SizeX, int* SizeY)
	{
		return g_dxLibRegerenda.GetAndroidDisplayResolution(SizeX, SizeY);
	}
#elif defined __APPLE__
	extern int GetDisplayResolution_iOS(int* SizeX, int* SizeY)
	{
		return g_dxLibRegerenda.GetDisplayResolution_iOS(SizeX, SizeY);
	}
#endif
}

void RegisterDxLibFunctions(const DxLibRegerenda* pDxLibRegerenda)
{
	g_dxLibRegerenda = *pDxLibRegerenda;
}

#else /* Scope for executable side */
const DxLibRegerenda* GetDxLibFunctonsToBeRegistered()
{
	static DxLibRegerenda s_dxLibRegerenda;

	s_dxLibRegerenda.GetUseCharCodeFormat = &DxLib::GetUseCharCodeFormat;
	s_dxLibRegerenda.Get_wchar_t_CharCodeFormat = &DxLib::Get_wchar_t_CharCodeFormat;
	s_dxLibRegerenda.ConvertStringCharCodeFormat = &DxLib::ConvertStringCharCodeFormat;

	s_dxLibRegerenda.LoadGraph = &DxLib::LoadGraph;
	s_dxLibRegerenda.DeleteGraph = &DxLib::DeleteGraph;

	s_dxLibRegerenda.SetDrawCustomBlendMode = &DxLib::SetDrawCustomBlendMode;
	s_dxLibRegerenda.SetDrawBlendMode = &DxLib::SetDrawBlendMode;
	s_dxLibRegerenda.DrawPolygonIndexed2D = &DxLib::DrawPolygonIndexed2D;

	s_dxLibRegerenda.GetDrawScreenSize = &DxLib::GetDrawScreenSize;
	s_dxLibRegerenda.MGetScale = &DxLib::MGetScale;
	s_dxLibRegerenda.MGetTranslate = &DxLib::MGetTranslate;
	s_dxLibRegerenda.MMult = &DxLib::MMult;

	s_dxLibRegerenda.SetTransformTo2D = &DxLib::SetTransformTo2D;
	s_dxLibRegerenda.ResetTransformTo2D= &DxLib::ResetTransformTo2D;

#if defined _WIN32
	s_dxLibRegerenda.GetDisplayMaxResolution = &DxLib::GetDisplayMaxResolution;
#elif defined __ANDROID__
	s_dxLibRegerenda.GetAndroidDisplayResolution = &DxLib::GetAndroidDisplayResolution;
#elif defined __APPLE__
	s_dxLibRegerenda.GetDisplayResolution_iOS = &DxLib::GetDisplayResolution_iOS;
#endif

	return &s_dxLibRegerenda;
}
#endif /* SPINE_RUNTIME_DLL_BUILD */
