# DxLibSpineViewer
Spine runtime for [DxLib](https://dxlib.xsrv.jp/index.html), accompanied with a viewer for Windows.

## Demonstration
- Built on spine-cpp 3.8.
 
https://github.com/BithreenGirlen/DxlibSpineTest/assets/152838289/0ab643de-73fb-42f3-8143-a871a9382f51

## Runtime files

- Spine runtime for DxLib provides functionality to (1) load texture and (2) render skeleton.
- There are two kind of runtimes depending on the official generic runtimes to be used with.
  - `dxlib_spine.cpp` and `dxlib_spine.h` are to be used with `spine-cpp`. (`3.8` to `4.1`)
  - `dxlib_spine_c.cpp` and `dxlib_spine_c.h` are to be used with `spine-c`. (`3.5` to `4.1`)
    - Class is used because DxLib is C++ library, but STL is avoided.

Besides, there is a runtime for spine `2.1.27` under `projects/DxLibSpineViewerC-2.1`. But note that transformation method is totally [different](https://ja.esotericsoftware.com/forum/d/3462-spines-non-skewing-transforms) from later versions.

## Viewer for Windows

The viewer is built with Spine generic runtime `2.1`, `3.5`, `3.6`, `3.7`, `3.8`, `4.0`, and `4.1`. 
### Feature
- Multiple Spines rendering
  - Animation names should be shared among all the skeleton files.
  - Rendering order is either in filename ascending or descending order.
- Runtime manipulation
  - Able to exclude slots, mix skins, mix animations, and change attachment.
    - Mixing skins is available for versions later than `3.8`.
    - Changing attachment is possible only when there are slots associated with multiple attachments.
      - Even if it is possible to change attachment, it depends on timelines whether it is appropriate or not.
    - Mixing skins and changing attachment will overwrite the animation state.
      - It may be required to reload files in order to back to default state.
- Export to media file.
  - As `PNG`, `GIF`, `JPG`, and `H264`.
    - `H264` encoding is available only for Intel CPU.

The following sections explain how to use the viewer.

## Menu functions

| Entry | Item | Action |
----|---- |---- 
File | Open folder | Open folder-select-dialogue.
 -| Setting | Open a dialogue to set atlas/skelton extensions to pick up when opening folder.
 -| Select files | Pick up atlas and skeleton files one by one regardless of their extension.
Image | Manipulation | Open a dialogue to specify slots to be excluded, skins or animations to be mixed.
 -| Re-attachment | Open a dialogue to change attachment.
 Window | Through-seen | Switch window's transparancy.
 -| Pan smoothly | Switch view-point behaviour; pan while dragging or when dragged button is released.
 
### Load spine(s) via `Open folder` 
1. In the `Setting` dialogue, specify atlas and skeleton extensions.
2. Uncheck `Binary` if skeketon file is json format.
3. From `Open folder`, select a folder containing atlas/skel(s) with specified extensions.

### Load spine(s) via `Select files`
1. Uncheck `Binary` in the `Setting` dialogue if skeleton file is json format.
2. From `Select files`, first select atlas file(s) to load. 
3. In the secoend dialogue, select skel file(s) which is/are pair(s) of atlas.

## Context-menu function

| Item | Action |
|---- |---- 
| Snap as PNG | Save the current screen as PNG.
| Snap as JPG | Save the current screen as JPG.
| Start image recording | Start storing screen frames at intervals.
| Save as GIF | Save stored frames into a single GIF file.
| Save as PNGs | Save stored frames as separate PNG files.

- The context-menu appears only when spine is loaded, and the items of which varies depending on whether it is under recording or not.
- The files are saved in the subdirectory of the execution file.
  -  The folder is named after folder-name when loaded via `Open folder`, and the first atlas filename when via `Select files`.
- PNG and JPG file will be named like `home_4.475018.png` where `home` is animation name, and `4.475018` is animation frame when saved.
- GIF file will be named like `wait.gif` where `wait` is animation name.
- Mind that `width * height * 4` byte of memory will be consumed every recording frame.

The additional items below appear only for Intel CPU. 
| Item | Action |
|---- |---- 
| Start video recording | Start recording screen as H264.
| End recording | End video recording.

- H264 file will be named like `fp.mp4` where `fp` is animation name.

## Mouse functions

| Command | Action |
----|---- 
Mouse wheel| Scale up/down. Combining with `Ctrl` to retain window size.
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

- Slot exclusion
  - Sometimes it is desirable to leave out visually nuisance effect.

https://github.com/user-attachments/assets/f4b5e1fa-faf7-4711-918f-2d0fbd2bb859

## External libraries

- [DXライブラリ](https://dxlib.xsrv.jp/dxdload.html)
- [Spine Runtimes](https://github.com/EsotericSoftware/spine-runtimes)

## Build
1. Run `shared-src/deps/CMakeLists.txt`.
2. Open `DxLibSpineViewer.sln` with Visual Studio.

The `CMakeLists.txt` modifies some of the external sources as well as obtains them.
- For spine-c `3.5`, renames some of the functions which lack `sp` prefix in `extension.c` and `extension.h` so as to be consistent with those of `3.6` and later.
- For spine-c `2.1`, supplies binary skeleton reader which is lacking in official `2.1.25` runtime, and overwrites some of the files with those from [here](https://github.com/BithreenGirlen/spine-c-2.1.27).

The `deps` directory will be as follows:
<pre>
...
├ DxLibSpineC
│  └ ...
├ DxLibSpineCpp
│  └ ...
├ projects
│  └ ...
├ shared-src
│  ├ deps
│  │  ├ dxlib // static libraries and headers of DxLib for VC
│  │  │  └ ...
│  │  ├ spine-c-x.x // sources and headers of spine-c x.x
│  │  │  ├ include
│  │  │  │  └ ...
│  │  │  └ src
│  │  │     └ ...
│  │  ├ ...
│  │  ├ spine-cpp-x.x // sources and headers of spine-cpp x.x
│  │  │  ├ include
│  │  │  │  └ ...
│  │  │  └ src
│  │  │     └ ...
│  │  └ ...
│  └ ...
├ DxLibSpineViewer.sln
└ ...
</pre>

