

#include "dxlib_imgui.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>
#include <imgui.h>

#if defined _WIN32
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
/* if DX_NON_DIRECT3D9 is defined in DxCompileConfig.h, there is no need to consider DirectX9 backend. */
#ifndef DX_NON_DIRECT3D9
#include <backends/imgui_impl_dx9.h>
#endif
#elif defined __ANDROID__

#elif defined __APPLE__

#endif

#if defined _WIN32
struct DxLibImguiWin32
{
	void (*RenderDrawData)(ImDrawData*) = nullptr;
	void (*NewFrame)(void) = nullptr;
	void (*RendererShutdown)(void) = nullptr;
};
static DxLibImguiWin32 g_dxLibImguiWin32;

class CDxLibImguiImplWin32 abstract final
{
public:
	static bool CreateContext()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		io.ConfigViewportsNoAutoMerge = true;

		ImGui::StyleColorsLight();
		ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.875f;

		return true;
	}

	static bool Initialise()
	{
		bool bRet = ImGui_ImplWin32_Init(DxLib::GetMainWindowHandle());
		if (!bRet)return false;

		int d3Version = DxLib::GetUseDirect3DVersion();
		if (d3Version == DX_DIRECT3D_11)
		{
			g_dxLibImguiWin32.NewFrame = &ImGui_ImplDX11_NewFrame;
			g_dxLibImguiWin32.RenderDrawData = &ImGui_ImplDX11_RenderDrawData;
			g_dxLibImguiWin32.RendererShutdown = &ImGui_ImplDX11_Shutdown;

			ID3D11Device* pD3D11Device = static_cast<ID3D11Device*>(const_cast<void*>(DxLib::GetUseDirect3D11Device()));
			ID3D11DeviceContext* pD3D11DeviceContext = static_cast<ID3D11DeviceContext*>(const_cast<void*>(DxLib::GetUseDirect3D11DeviceContext()));
			return ImGui_ImplDX11_Init(pD3D11Device, pD3D11DeviceContext);
		}
#ifndef DX_NON_DIRECT3D9
		else if (d3Version == DX_DIRECT3D_9 || d3Version == DX_DIRECT3D_9EX)
		{
			g_dxLibImguiWin32.NewFrame = &ImGui_ImplDX9_NewFrame;
			g_dxLibImguiWin32.RenderDrawData = &ImGui_ImplDX9_RenderDrawData;
			g_dxLibImguiWin32.RendererShutdown = &ImGui_ImplDX9_Shutdown;

			IDirect3DDevice9* pD3D9Device = static_cast<IDirect3DDevice9*>(const_cast<void*>(DxLib::GetUseDirect3DDevice9()));
			return ImGui_ImplDX9_Init(pD3D9Device);
		}
#endif
		return false;
	}

	static void NewFrame()
	{
		if (g_dxLibImguiWin32.NewFrame != nullptr)
		{
			g_dxLibImguiWin32.NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}
	}

	static void Render()
	{
		if (g_dxLibImguiWin32.NewFrame != nullptr)
		{
			ImGui::Render();
			g_dxLibImguiWin32.RenderDrawData(ImGui::GetDrawData());
		}
	}

	static void Shutdown()
	{
		if (g_dxLibImguiWin32.RendererShutdown != nullptr)
		{
			g_dxLibImguiWin32.RendererShutdown();
		}
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
};

#endif /* WIN32 */

CDxLibImgui::CDxLibImgui(const char* defaultFontfilePath, float fontSize)
{
#if defined _WIN32
	m_bInitialised = CDxLibImguiImplWin32::CreateContext();
	m_bInitialised &= CDxLibImguiImplWin32::Initialise();

	if (defaultFontfilePath != nullptr)
	{
		ImGuiIO& io = ImGui::GetIO();
		const auto& fontAtlas = io.Fonts;
		const ImWchar* glyph = fontAtlas->GetGlyphRangesChineseFull();
		fontAtlas->AddFontFromFileTTF(defaultFontfilePath, fontSize, nullptr, glyph);
	}
#endif
}

CDxLibImgui::~CDxLibImgui()
{
#if defined _WIN32
	CDxLibImguiImplWin32::Shutdown();
#endif
}

void CDxLibImgui::NewFrame()
{
#if defined _WIN32
	CDxLibImguiImplWin32::NewFrame();
#endif
}

void CDxLibImgui::Render()
{
#if defined _WIN32
	DxLib::RenderVertex();
	CDxLibImguiImplWin32::Render();
#endif
}

void CDxLibImgui::UpdateAndRenderViewPorts()
{
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}
