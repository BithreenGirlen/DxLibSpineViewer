# DxLibSpineViewer

Spine viewer for Windows based on [Runtime for DxLib](#spine-runtime-for-dxlib) plus official generic runtime `2.1`, `3.5`, `3.6`, `3.7`, `3.8`, `4.0`, and `4.1`.

## Feature
- Multiple rendering
- Runtime manipulation
  - Exclude slot
  - Mix skins
  - Mix animations
  - Replace attachment
- Media file export
  - As `PNG`, `GIF`, `JPG`, and `H264`.
- Transparent/borderless window style

## Demonstration

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

<details><summary>Add effect</summary>

https://github.com/user-attachments/assets/d6682127-e01f-444d-838d-78b3bddac121

</details>

## How to load

### Load via `Open folder` 
1. From `File->Setting`, specify atlas and skeleton extensions.
2. From `File->Open folder`, select a folder containing atlas/skel(s) with specified extensions.

### Load via `Select files`
1. From `File->Select files`, first select atlas file(s) to load. 
2. Then select skel file(s) which is/are pair(s) of atlas.

## How to export

### Export as single image

1. Right click on the window.
2. Select `Snap as PNG` or `Snap as JPG`.

### Export as multiple images

1. Right click on the window.
2. Select `Start image recording`.
3. Right click again and select `Save as GIF` or `Save as PNGs`.

### Export as video

1. Right click on the window.
2. Select `Start video recording`.
3. Right click again and select `End recording`.

</details>
<details><summary>Note on filename</summary>

- The files are saved in the subdirectory of the execution file.
  -  The folder is named after folder-name when loaded via `Open folder`, and the first atlas filename when via `Select files`.
- `PNG` and `JPG` file will be named like `home_4.475018.png` where `home` is animation name, and `4.475018` is animation frame when saved.
- `GIF` file will be named like `wait.gif` where `wait` is animation name.
- `H264` file will be named like `fp.mp4` where `fp` is animation name.

</details>

## Mouse functions

| Input | Action |
| ---- | ---- |
| Wheel scroll | Scale up/down. Combinating with `Ctrl` to retain window size. |
| Left pressed + wheel scroll | Speed up/down the animation. |
| Left click | Switch the animation. |
| Left drag | Move view-point. |
| Middle click | Reset scale, animation speed, and view-point to default. |
| Right pressed + wheel scroll | Switch the skin. |

<details><summary>Tip on transparent window</summary>

1. Check menu item `Window->Through-seen` to make window transparent.
2. `Right pressed + middle click` to make window borderless.
3. `Right pressed + left click` to move borderless window.

</details>

## Keyboard functions

| Input | Action |
| --- | --- |
| <kbd>Esc</kbd> | Close the application. |
| <kbd>Up</kbd> | Open the previpus folder. |
| <kbd>Down</kbd> | Open the next folder. |
| <kbd>A</kbd> | Enable/disable premultiplied alpha. _Default: enabled_. | 
| <kbd>B</kbd> | Prefer/ignore blned-mode specified by slots. _Default: preferred_. | 
| <kbd>R</kbd> | Toggle draw-order between filename asc/descending order. _Default: ascending order_. | 

- <kbd>Up</kbd> and <kbd>Down</kbd> key are valid only when the current spine(s) is/are loaded via `Open folder`.

</details>
<details><summary>Tip on PMA and blend mode</summary>
  
- Disable `PMA` with <kbd>A</kbd> if it is too bright, and enable if darkish.
- Force `normal` blend mode with <kbd>B</kbd> if `multiply` is not well represented.
  
</details>
 
## External libraries

- [DxLib](https://dxlib.xsrv.jp/)
- [Spine Runtimes](https://github.com/EsotericSoftware/spine-runtimes)

## Build
1. Run `shared-src/deps/CMakeLists.txt`.
2. Open `DxLibSpineViewer.sln` with Visual Studio.
3. Select `Build Solution` on menu item.

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

## Spine runtime for DxLib

- Spine runtime for DxLib provides functionality to (1) load texture and (2) render skeleton.
- There are two kind of runtimes depending on the official generic runtimes to be used with.
  - [dxlib_spine.cpp](/DxLibSpineCpp/dxlib_spine.cpp) is to be used with `spine-cpp`. (`3.8` to `4.1`)
  - [dxlib_spine_c.cpp](/DxLibSpineC/dxlib_spine_c.cpp) is to be used with `spine-c`. (`3.5` to `4.1`)
    - Class is used because DxLib is C++ library, but STL is avoided.

Besides, there is a runtime for spine `2.1.27` under [projects/DxLibSpineViewerC-2.1](/projects/DxLibSpineViewerC-2.1). But note that transformation method is totally [different](https://en.esotericsoftware.com/forum/d/3462-spines-non-skewing-transforms) from later versions.
