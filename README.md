# DxLibSpineViewer
Spine viewer with practical manipulation feature, based on runtime for [DxLib](https://dxlib.xsrv.jp/index.html).

## Spine runtime

- Spine runtime for DxLib provides functionality to (1) load texture and (2) render skeleton.
- There are two kind of runtimes depending on the official generic runtimes to be used with.
  - `dxlib_spine.cpp` and `dxlib_spine.h` are to be used with `spine-cpp`. (`3.8` to `4.1`)
  - `dxlib_spine_c.cpp` and `dxlib_spine_c.h` are to be used with `spine-c`. (`3.5` to `4.1`)
    - Class is used because DxLib is C++ library, but STL is avoided.

Besides, there is a runtime for spine `2.1.27` under `projects/DxLibSpineViewerC-2.1`. But note that transformation method is totally [different](https://en.esotericsoftware.com/forum/d/3462-spines-non-skewing-transforms) from later versions.

## Spine viewer

The viewer is built with Spine generic runtime `2.1`, `3.5`, `3.6`, `3.7`, `3.8`, `4.0`, and `4.1`. 
### Feature
- Multiple Spines rendering
  - Rendered in filename ascending or descending order.
- Runtime manipulation
  - Exclude slot
  - Mix skins
  - Mix animations
  - Replace attachment
- Media file export
  - As `PNG`, `GIF`, `JPG`, and `H264`.
    - `H264` encoding is available only for Intel CPU.
- Transparent/borderless window style

### Demonstration

<details><summary>Multiple rendering</summary>
 
https://github.com/user-attachments/assets/c1b44202-94f8-4a8b-befa-5ee2b4abbf70

</details>

<details><summary>Mix animations</summary>

https://github.com/user-attachments/assets/4a3abb0e-63d7-4402-b929-4f9c2671c94d
 
</details>

<details><summary>Exclude slot</summary>

https://github.com/user-attachments/assets/f3db7d40-4912-416c-8a39-03b38923d63f

</details>

<details><summary>Replace attachment</summary>
 
https://github.com/user-attachments/assets/36c40c5c-8314-410c-9905-77255fa96a17

</details>

<details><summary>Media export</summary>

https://github.com/user-attachments/assets/4498830d-8fc4-4333-a3d9-2a506777ec7d

</details>
<details><summary>Transparent window</summary>

https://github.com/user-attachments/assets/b73a0010-d21b-4386-9d1b-084ee2dd29c0

</details>

The following sections are on the viewer.

## Menu functions

| Entry | Item | Action |
| ---- | ---- | ---- | 
| File | `Open folder` | Open folder-select-dialogue. |
| - | `Setting` | Open a dialogue to set atlas/skelton extensions to pick up when opening folder. |
| - | `Select files` | Pick up atlas and skeleton files one by one regardless of their extension. |
| Image | `Manipulation` | Open a dialogue to specify slots to be excluded, skins or animations to be mixed. |
| - | `Re-attachment` | Open a dialogue to replace attachment. |
| Window | `Through-seen` | Toggle window's transparancy. |
| - | `Allow manual sizing` | Allow/forbid manual sizing of window. _Default: forbidden_ |
| - | `Move view on release` | Toggle view-point behaviour; move while dragging or when dragged button is released. |
 
### Load spine(s) via `Open folder` 
1. In the `Setting` dialogue, specify atlas and skeleton extensions.
2. From `Open folder`, select a folder containing atlas/skel(s) with specified extensions.

### Load spine(s) via `Select files`
1. From `Select files`, first select atlas file(s) to load. 
2. In the secoend dialogue, select skel file(s) which is/are pair(s) of atlas.

## Context-menu function

- The context-menu appears only when spine is loaded, and the items of which varies depending on whether it is under recording or not.

### Idle state

| Menu item | Action |
| ---- | ---- |
| `Snap as PNG` | Save the current screen as PNG. |
| `Snap as JPG` | Save the current screen as JPG. |
| `Start image recording` | Start storing screen frames at intervals. |
| `Start video recording` | Start recording screen as H264. |

### Image storing state

| Menu item | Action |
| ---- | ---- |
| `Save as GIF` | Save stored frames into a single GIF file. |
| `Save as PNGs` | Save stored frames as separate PNG files. |

### Video recording state

| Menu item | Action |
| ---- | ---- |
| `End recording` | End video recording. |

- The files are saved in the subdirectory of the execution file.
  -  The folder is named after folder-name when loaded via `Open folder`, and the first atlas filename when via `Select files`.
- PNG and JPG file will be named like `home_4.475018.png` where `home` is animation name, and `4.475018` is animation frame when saved.
- GIF file will be named like `wait.gif` where `wait` is animation name.
- Mind that `width * height * 4` byte of memory will be consumed every recording frame.
- H264 file will be named like `fp.mp4` where `fp` is animation name.

## Mouse functions

| Input | Action |
| ---- | ---- |
| Wheel scroll | Scale up/down. Combining with `Ctrl` to retain window size. |
| Left pressed + wheel scroll | Speed up/down the animation. |
| Left click | Switch the animation. |
| Left drag | Move view-point. |
| Middle click | Reset scale, animation speed, and view-point to default. |
| Right pressed + middle click | Hide/show window's border and menu. Having hidden, the window goes to the origin of display. |
| Right pressed + left click | Start moving borderless window. Left click to end.  |
| Right pressed + wheel scroll | Switch the skin. |

### Tip on transparent window
1. Check menu item `Window->Through-seen` to make window transparent.
2. Right pressed + middle click to make window borderless.
3. Right pressed + left click to move borderless window.

## Keyboard functions

| Input | Action |
| --- | --- |
| <kbd>Esc</kbd> | Close the application. |
| <kbd>Up</kbd> | Open the previpus folder. |
| <kbd>Down</kbd> | Open the next folder. |
| <kbd>A</kbd> | Enable/disable premultiplied alpha. _Default: enabled_. | 
| <kbd>B</kbd> | Prefer/ignore blned-mode specified by slots. _Default: preferred_. | 
| <kbd>R</kbd> | Toggle draw-order between filename asc/descending order. _Default: ascending order_. | 
| <kbd>Z</kbd> | Enable/disable depth-buffer. _Default: disabled_. |  

- <kbd>Up</kbd> and <kbd>Down</kbd> key are valid only when the current spine(s) is/are loaded via `Open folder`.
- Disable `PMA` with <kbd>A</kbd> if it is too bright, and enable if darkish.
- Force `normal` blend mode with <kbd>B</kbd> if `multiply` is not well represented.
 
## External libraries

- [DxLib](https://dxlib.xsrv.jp/dxdload.html)
- [Spine Runtimes](https://github.com/EsotericSoftware/spine-runtimes)

## Build
1. Run `shared-src/deps/CMakeLists.txt`.
2. Open `DxLibSpineViewer.sln` with Visual Studio.

The `CMakeLists.txt` modifies some of the external sources as well as obtains them.
- For spine-c `3.5`, renames some of the functions which lack `sp` prefix in `extension.c` and `extension.h` so as to be consistent with those of `3.6` and later.
- For spine-c `2.1`, supplies binary skeleton reader which is lacking in official `2.1.25` runtime, and overwrites some of the files with those from [here](https://github.com/BithreenGirlen/spine-c-2.1.27).

<details><summary>deps directory will be as follows:</summary>

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
│  │  ├ spine-c-x.x // Spine C generic runtime for version x.x
│  │  │  ├ include
│  │  │  │  └ ...
│  │  │  └ src
│  │  │     └ ...
│  │  ├ ...
│  │  ├ spine-cpp-x.x // Spine C++ generic runtime for version x.x
│  │  │  ├ include
│  │  │  │  └ ...
│  │  │  └ src
│  │  │     └ ...
│  │  └ ...
│  └ ...
├ DxLibSpineViewer.sln
└ ...
</pre>

 </details>
