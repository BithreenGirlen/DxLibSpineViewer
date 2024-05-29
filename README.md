# DxlibSpineTest
Implementation of Spine rendering with [DXライブラリ](https://dxlib.xsrv.jp/index.html)

## Demonstration
- Built on spine-cpp 3.8.
 
https://github.com/BithreenGirlen/DxlibSpineTest/assets/152838289/0ab643de-73fb-42f3-8143-a871a9382f51

## Libraries

- [DXライブラリ](https://dxlib.xsrv.jp/dxdload.html)
- [Spine Runtimes](https://github.com/EsotericSoftware/spine-runtimes)

When building, supply the above libraries under the project directory as following. 
<pre>
DxlibSpineTest
  ├ deps
  │  ├ dxlib // static libraries and headers of DXライブラリ
  │  │ └ ...
  │  └ spine-cpp // sources and headers of spine-cpp
  │    ├ include
  │    │ └ ...
  │    └ src
  │      └ ...
  ├ dpiAwareness.manifest
  └ ...
</pre>

## Brief explanation of files

- `dxlib_spine.cpp`
  - Texture loader and spine rendering impelementation with DxLib.
  - Win32API is used in order to convert `char` to `wchar_t` in `TextureLoader`. 
```cpp
void CDxLibTextureLoader::load(spine::AtlasPage& page, const spine::String& path)
{
	//
	std::wstring wstrPath = win_text::WidenANSI(path.buffer());
	int iDxLibTexture = DxLib::LoadGraph(wstrPath.c_str());
	//
}
```
  - This dependency can be removed compiling with multi-byte character set; or feel free to replace the conversion part with other suitable libraries.

The following are not indispensable for rendering.
- `spine_loader.cpp`
  - spine resource loader functions.
- `dxlib_spine_player.cpp`
  - Manages spine resources and some of its parameters; also sets up DxLib.

Other files are for GUI.

## Menu functions

| Entry | Item | Action |
----|---- |---- 
File| Open folder | Open folder-select-dialogue.
 -| Setting | Open a dialogue to set atlas/skelton extensions to pick up when opening folder.
 -| Select files | Pick up atlas and skeleton file one by one regardless of its extension.
Image| Through-seen | Switch the window's transparancy.

`Open folder` is basically for rendering multiple spines, `Select files` is for rendering single spine.  
In both case, whether the skeleton is binary or not should be configured through setting dialogue.

## Mouse functions

| Command | Action |
----|---- 
Mouse wheel| Scale up/down
Left button + mouse wheel| Speed up/down the animation.
Left button click| Switch the animation.
Left button drag| Move view point.
Middle button| Reset scale, speed, and view point to default.
Right button + middle button| Hide/show window's title and menu bar. Having hidden, the window goes to the origin of the primary display.
Right button + left button| Move window. This works only when the window's title/bar are hidden.
Right button + mouse wheel| Switch the skin.

## Key functions

| Input  | Action  |
| --- | --- |
| Esc | Close the application. |
| A | Enable/disable premultiplied alpha.|  
| B | Prefer/ignore blned-mode specified by slots.|  
| Z | Enable/disable depth-buffer.|  

## Extra-demo
- Depth-buffer
  - When rendering two spines, setting depth-buffer ensures one of them is always rendered in front of the other.

https://github.com/BithreenGirlen/DxlibSpineTest/assets/152838289/7075f77a-afa0-43fe-93d3-aec1ebdf2d1c

- Mix animations
  - Some spines require mixing skins or animations to supplement facial parts.

https://github.com/BithreenGirlen/DxlibSpineTest/assets/152838289/a5ca45df-ad1c-4f9e-b968-b2cd36f069f5
