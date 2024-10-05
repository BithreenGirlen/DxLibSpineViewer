# DxlibSpineTest
Spine runtime for [DxLib](https://dxlib.xsrv.jp/index.html), accompanied with a viewer for Windows.

## Demonstration
- Built on spine-cpp 3.8.
 
https://github.com/BithreenGirlen/DxlibSpineTest/assets/152838289/0ab643de-73fb-42f3-8143-a871a9382f51

## Build dependency

- [DXライブラリ](https://dxlib.xsrv.jp/dxdload.html)
- [Spine Runtimes](https://github.com/EsotericSoftware/spine-runtimes)

When building, supply the above libraries under `/shared-src/deps`. 
<pre>
...
├ DxlibSpineTest
│  └ ...
├ DxlibSpineTestC
│  └ ...
├ shared-src
│  ├ deps
│  │  ├ dxlib // static libraries and headers of DxLib
│  │  │ └ ...
│  │  ├ spine-c // sources and headers of spine-c
│  │  │ ├ include
│  │  │ │ └ ...
│  │  │ └ src
│  │  │   └ ...
│  │  └ spine-cpp // sources and headers of spine-cpp
│  │    ├ include
│  │    │ └ ...
│  │    └ src
│  │      └ ...
│  ├ dpiAwareness.manifest
│  └ ...
├ DxlibSpineTest.sln
└ ...
</pre>

## Brief explanation of files

The files under `/DxlibSpineTest` are to be used with `spine-cpp` runtime. 
- `dxlib_spine.cpp`
  - Spine texture loader and skeleton renderer.
- `spine_loader.cpp`
  - Atlas/skeleton loader.
- `dxlib_spine_player.cpp`
  - Manager for spine resources and some of its parameters.

There are `spine-c` runtime equivalents under `/DxlibSpineTestC`, which are intended to be used for spine 3.6 and older because there was not C++ runtime.  

The setup for DxLib is written in `shared-src/dxlib_init.cpp`. Other files are for viewer.

## Menu functions

| Entry | Item | Action |
----|---- |---- 
File| Open folder | Open folder-select-dialogue.
 -| Setting | Open a dialogue to set atlas/skelton extensions to pick up when opening folder.
 -| Select files | Pick up atlas and skeleton files one by one regardless of their extension.
Image| Through-seen | Switch window's transparancy.
 -| Manipulation | Open a dialogue to specify slots to be excluded, skins or animations to be mixed.

### Load spine(s) via `Open folder` 
1. In the `Setting` dialogue, specify atlas and skeleton extensions.
2. Uncheck `Binary` if skeketon is text.format.
3. In the dialogue appeared when clicking `Open folder`, select a folder containing atlas/skel(s) with specified extensions.

### Load spine(s) via `Select files`
1. If skeleton is text format, uncheck `Binary` in the `Setting` dialogue.
2. In the first dialogue appeared when clicking `Select files`, select atlas file(s) to load. 
3. In the second dialogue, select skel file(s) which is/are pair(s) of atlas.

## Context-menu function

| Item | Action |
|---- |---- 
| Snap as PNG | Save the current screen as PNG.
| Start recording | Start storing screen frames at intervals.
| Save as GIF | Save stored frames into a single GIF file.
| Save as PNGs | Save stored frames as separate PNG files.

- The context-menu appears only when spine is loaded, and the items of which varies depending on whether it is under recording or not.
- The files are saved in the subdirectory of the execution file.
  -  The folder is named after folder-name when loaded via `Open folder`, and the first atlas filename when via `Select files`.
- The PNG file will be named like `home_4.475018.png` where `home` is animation name, and `4.475018` is animation frame when saved.
- The GIF file will be named like `wait.gif` where `wait` is animation name.

## Mouse functions

| Command | Action |
----|---- 
Mouse wheel| Scale up/down
Left button + mouse wheel| Speed up/down the animation.
Left button click| Switch the animation.
Left button drag| Move view-point.
Middle button| Reset scale, speed, and view-point to default.
Right button + middle button| Hide/show window's frame and menu. Having hidden, the window goes to the origin of the primary display.
Right button + left button| Move window. This works only when the window's frame/menu are hidden.
Right button + mouse wheel| Switch the skin.

## Keyboard functions

| Input  | Action  |
| --- | --- |
| Esc | Close the application. |
| Up | Open the previpus folder. |
| Down | Open the next folder. |
| A | Enable/disable premultiplied alpha.| 
| B | Prefer/ignore blned-mode specified by slots.| 
| R | Switch draw-order between filename asc/descending order.| 
| Z | Enable/disable depth-buffer.|  

- `Up` and `Down` key are valid only when the current spine(s) is/are loaded via `Open folder`.

## Extra-demo
- Depth-buffer
  - When rendering two spines, setting depth-buffer ensures one of them is always rendered in front of the other.

https://github.com/BithreenGirlen/DxlibSpineTest/assets/152838289/7075f77a-afa0-43fe-93d3-aec1ebdf2d1c

- Mix animations
  - Some spines require mixing skins or animations to supplement facial parts.

https://github.com/BithreenGirlen/DxlibSpineTest/assets/152838289/a5ca45df-ad1c-4f9e-b968-b2cd36f069f5
