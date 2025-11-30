# DxLibSpineViewer

Spine viewer for Windows based on [Runtime for DxLib](#spine-runtime-for-dxlib) (`2.1`, `3.5` to `4.2`).

### Feature
- Multiple rendering
- Runtime manipulation
  - Exclude slot
  - Mix skins
  - Mix animations
  - Replace attachment
- Media export
- Transparent/borderless window style

## Demonstration

<details><summary>Multiple rendering-1 (via selecting multiple files)</summary>
 
https://github.com/user-attachments/assets/8ca281a2-9fe5-4d4d-a888-f636e06f0608

</details>

<details><summary>Multiple rendering-2 (via opening a folder)</summary>

https://github.com/user-attachments/assets/6787f39b-3dd2-47f6-8941-8809ddec6b03
 
</details>

<details><summary>Mix animations</summary>

https://github.com/user-attachments/assets/47ef9037-94d4-4354-8397-b04081dc6697

</details>

<details><summary>Exclude slot</summary>

https://github.com/user-attachments/assets/96f44b4b-1054-4aaf-83a6-1a604160cae4

</details>

<details><summary>Replace attachment</summary>
 
https://github.com/user-attachments/assets/009406d5-71d2-4012-9095-0adc198850d0

</details>

<details><summary>Media export</summary>

https://github.com/user-attachments/assets/fbfa7fb7-6ebf-4660-8d88-d29cf8d16840

</details>
<details><summary>Transparent window</summary>

https://github.com/user-attachments/assets/b67e6ead-7b2b-4035-b1d3-894274bdfafb

</details>

<details><summary>Add effect</summary>

https://github.com/user-attachments/assets/5cd49bca-2fe5-4cd9-9404-6a7381b5364a

</details>

<details><summary>Import Cocos-1 (Json containing both atlas and skeleton)</summary>

https://github.com/user-attachments/assets/d85f0677-d16b-454b-b8e6-8c47af28cb3b

</details>

<details><summary>Import Cocos-2 (Json containing only atlas and requiring skeleton separately)</summary>

https://github.com/user-attachments/assets/3033fef6-aa30-420f-9a2a-5cb1976780e3

</details>


## How to load

### Load via `Open files`

1. First select atlas file(s) to load. 
2. Then select skel file(s) which is/are pair(s) of atlas.

### Load via `Open folder` 

1. From `File->Extension setting`, specify atlas and skeleton extensions.
2. From `File->Open folder`, select a folder containing atlas/skel(s) with specified extensions.

- `Open folder` is __to load all the Spine files in a folder and render them synchronically.__
  - Of cource it is appropriate to open a folder containing only one set of Spine, but it is not appropriate to open a folder containing more than two sets of Spine unless their animations have synchronised timelines.

### Load via `Import Cocos`

- This is to load json:
  - which contains atlas at `[5][0][2]`, and skeleton at `[5][0][4]`.
  - or that which contains atlas at `[5][0][2]` and requires binary skeleton separately. 

## Mouse functions

| Input | Action |
| ---- | ---- |
| Wheel scroll | Scale up/down. Combinating with `Ctrl` to zoom in/out. |
| L-pressed + wheel scroll | Speed up/down the animation. |
| L-click | Switch the animation. |
| L-drag | Move view-point. |
| M-click | Reset scale, animation speed, and view-point to default. |
| R-pressed + wheel scroll | Switch the skin. |
| R-pressed + M-click | Hide/show the border of window. |
| R-pressed + L-click | Start moving borderless window. L-click again to end. |


## Keyboard functions

| Input | Action |
| --- | --- |
| <kbd>Esc</kbd> | Close the application. |
| <kbd>Up</kbd> | Open the previpus folder. |
| <kbd>Down</kbd> | Open the next folder. |
| <kbd>A</kbd> | Enable/disable premultiplied alpha. _Default: enabled_. | 
| <kbd>B</kbd> | Prefer/ignore blned-mode specified by slots. _Default: preferred_. | 
| <kbd>R</kbd> | Toggle draw-order between filename asc/descending order. _Default: ascending order_. | 

- <kbd>Up</kbd> and <kbd>Down</kbd> key are valid only when files are loaded via `Open folder`.
- Toggling `PMA` is permitted only for Spine version `3.8` and older.
  - For Spine version `4.0` and later, runtime applies pma property read from atlas file.
  - Disable `PMA` if it seems too bright, and enable if darkish.
- Force `normal` blend mode if `multiply` is not well represented.


## Context menu functions

| Menu item | Action |
| ---- | ---- |
| Snap as PNG | Save the current screen as `PNG`. |
| Snap as JPG | Save the current screen as `JPG`. |
| Export as GIF | Restart the current animation and export as `GIF`. |
| Export as H264 | Restart the current animation and export as `MP4`. |
| Export as PNGs | Restart the current animation and export as sequential `PNG`s. |
| Export as JPGs | Restart the current animation and export as sequential `PNG`s. |

- Context menu appears only when Spine is loaded.
- By unchecking `Export per anim.` option from `Tool->Export setting`, when to start and end recording will be delegated to user.

<details><summary>Note on filename</summary>

- The files are saved in the subdirectory of the execution file.
  -  The folder is named after folder-name when loaded via `Open folder`, and the first atlas filename when via `Select files`.
- `PNG` and `JPG` file will be named like `home_4.475.png` where `home` is animation name, and `4.475` is animation time.
- `GIF` file will be named like `wait.gif` where `wait` is animation name.
- `H264` file will be named like `fp.mp4` where `fp` is animation name.

</details>

## External libraries

- [DxLib](https://dxlib.xsrv.jp/)
- [Spine Runtimes](https://github.com/EsotericSoftware/spine-runtimes)
- [Dear ImGui](https://github.com/ocornut/imgui)

## Build Spine viewer

Visual Studio is required.  

1. Open `shared-src/deps` folder with Visual Studio.
2. Wait for the CMake configuration to be done.
    - The configuration downloads external libraries and modifies older Spine `extensions`.
      - For spine-c `3.5`, renames some of the functions which lack `sp` prefix so as to be consistent with `3.6` and later.
      - For spine-c `2.1`, supplies binary skeleton reader from [here](https://github.com/BithreenGirlen/spine-c-2.1.27).
3. Install Spine generic runtimes both for `x64-debug` and `x64-release`.
4. Open `DxLibSpineViewer.sln` and build.

## Spine runtime for DxLib

- For minimal integration, only `dxlib_spine.cpp/h` or `dxlib_spine_c.cpp` are suffice.
  - [dxlib_spine.cpp](/DxLibSpineCpp/dxlib_spine.cpp) is to be used with `spine-cpp`. (`3.8` to `4.2`)
  - [dxlib_spine_c.cpp](/DxLibSpineC/dxlib_spine_c.cpp) is to be used with `spine-c`. (`3.5` to `4.2`)
    - Class is used because DxLib is C++ library, but STL is avoided.
  - There is a runtime for Spine `2.1` under [projects/DxLibSpineViewerC-2.1](/projects/DxLibSpineViewerC-2.1). But note that transformation method is totally [different](https://en.esotericsoftware.com/forum/d/3462-spines-non-skewing-transforms) from later versions.
- For more functionalities, use all the files under `DxLibSpineCpp` or `DxLibSpineC`.
  - The functionalities are as follows:

| File | Functionality | Dependency |
| --- | --- | --- |
| [dxlib_spine.cpp/h](/DxLibSpineCpp/dxlib_spine.h) | Load texture and render skeleton. | DxLib + Spine generic runtime |
| [dxlib_spine_player.cpp/h](/DxLibSpineCpp/dxlib_spine_player.h) | Adjust scale and translation using matrix. | DxLib; derived from `CSpinePlayer` |
| [spine_loader.cpp/h](/DxLibSpineCpp/spine_loader.h) | Load atlas or skeleton file. | C++14 STL + Spine generic runtime |
| [spine_player.cpp/h](/DxLibSpineCpp/spine_player.h) | Manage Spine resources and manipulation. | C++14 STL + Spine generic runtime |

- In order to build these files in a project using DxLib, it is required to define macro depending on versions of Spine generic runtime to be used with.

| Version | Macro to be defined |
| --- | --- |
| `2.1` | `SPINE_2_1` |
| `3.5` | - |
| `3.6` | - |
| `3.7` | - |
| `3.8` | `SPINE_3_8_OR_LATER` provided that C runtime is to be used. |
| `4.0` | `SPINE_4_0` |
| `4.1` | `SPINE_4_1_OR_LATER` |
| `4.2` | `SPINE_4_1_OR_LATER`, `SPINE_4_2_OR_LATER` |
