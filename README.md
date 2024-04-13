# DxlibSpineTest
Implementation of Spine rendering with [DXライブラリ](https://dxlib.xsrv.jp/index.html)

## Demonstration
 Built on spine-cpp 3.8.
 
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
  │  └ spine-cpp // spine-cpp 3.8
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
Folder| Open | Opens folder select dialogue.
 -| Setting | Opens dialogue for setting file extensions to pick up.
Image| Through-seen | Switches the window's transparancy.

## Mouse functions

| Command | Action |
----|---- 
Mouse wheel| Scales up/down
Left button + mouse wheel| Speeds up/down the animation.
Left button click| Switches the animation.
Left button drag|Moves view point.
Middle button|Resets scale, speed, and view point to default.
Right button + middle button|Hides/shows window's title and menu bar. Having hidden, the window goes to the origin of the primary display.
Right button + left button|Moves window. This works only when the window's title/bar are hidden.

## Key functions

| Input  | Action  |
| --- | --- |
| Esc | Close the application. |
| A | Enable/disable premultiplied alpha.|  
