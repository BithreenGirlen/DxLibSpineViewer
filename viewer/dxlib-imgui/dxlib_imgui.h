#ifndef DXLIB_IMGUI_H_
#define DXLIB_IMGUI_H_

class CDxLibImgui
{
public:
	CDxLibImgui(const char* defaultFontfilePath = nullptr, float fontSize = DefaultFontSize);
	~CDxLibImgui();

	bool hasBeenInitialised() const { return m_hasBeenInitialised; }

	static void NewFrame();
	static void Render();

	static void UpdateAndRenderViewPorts();
private:
	static constexpr float DefaultFontSize = 20.f;
	bool m_hasBeenInitialised = false;
};

#endif // !DXLIB_IMGUI_H_

