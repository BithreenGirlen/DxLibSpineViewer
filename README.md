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

<details><summary>Multiple rendering-2 (via opening folder)</summary>

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

<details><summary>Import Cocos-1 (including both atlas and skeleton)</summary>

https://github.com/user-attachments/assets/d85f0677-d16b-454b-b8e6-8c47af28cb3b

</details>

<details><summary>Import Cocos-2 (including only atlas and requiring separate skeleton)</summary>

https://github.com/user-attachments/assets/3033fef6-aa30-420f-9a2a-5cb1976780e3

</details>


## How to load

### Load via `Open files`

1. First select atlas file(s) to load. 
2. Then select skel file(s) which is/are pair(s) of atlas.

### Load via `Open folder` 

1. From `File->Extension setting`, specify atlas and skeleton extensions.
2. From `File->Open folder`, select a folder containing atlas/skel(s) with specified extensions.

- `Open folder` is to load all the Spine files in the directory render them at the same time.
- Do not try to open folder including Spine files which cannot be managed simultaneously.

<details><summary>Appropriate example</summary>

<pre>
221491
├ odin_S1.atlas.txt
├ odin_S1.png
├ odin_S1.txt
├ odin_S2.atlas.txt
├ odin_S2.png
├ odin_S2.txt
└ ...
</pre>

</details>

<details><summary>Inappropriate exmaple</summary>

<pre>
spinechar20
├ 1200.atlas
├ 1200.json
├ 1200.png
├ 1201.atlas
├ 1201.json
├ 1201.png
└ ...
</pre>

</details>

### Load via `Import Cocos`

- This is to load json:
  - which contains atlas at `[5][0][2]`, and skeleton at `[5][0][4]`.
  - or that which contains atlas at `[5][0][2]` and at the same time requires binary format skeleton separately. 
- This may serve as an exmaple to load Spine from memory.

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
- Manual toggling of PMA is available for Spine version `3.8` and older.
  - For Spine version `4.0` and later, runtime applies pma property read from atlas file.

</details>
<details><summary>Tip on PMA and blend mode</summary>
  
- Disable `PMA` with <kbd>A</kbd> if it is too bright, and enable if darkish.
- Force `normal` blend mode with <kbd>B</kbd> if `multiply` is not well represented.
  
</details>

## Context menu functions

- Context menu appears only when file is loaded.

### Export as single image

1. Right click on the window.
2. Select `Snap as PNG` or `Snap as JPG`.

### Export as animation

1. Right click on the window.
2. Select `Export as GIF` or `Export as H264`.

More flexible recording is available by unchecking `Export per anim.` option from `Image->Export setting`.

<details><summary>Note on flexible recording</summary>

| State | Behaviour |
| ---- | ---- |
| `Export per anim.` checked | Restarts the animation once recording has started, and ends recording as soon as animation has ended. |
| `Export per anim.` unchecked | Recording start and end timing can be choosen by user hand. |

#### Export as multiple images

1. Right click on the window.
2. Select `Start image recording`.
3. Right click again and select `Save as GIF` or `Save as PNGs`.

#### Export as video

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
 
## External libraries

- [DxLib](https://dxlib.xsrv.jp/)
- [Spine Runtimes](https://github.com/EsotericSoftware/spine-runtimes)

## Build

Visual Studio is required.
1. Open `shared-src/deps` directory with file explorer.
2. Type `cmd` in the directory path box.
    - Command prompt will start up.
3. Type `start devenv .` in the command prompt.
     - `Visual Studio` will start up and configure CMake.
4. Wait for the CMake configuration to be finished.
5. Configuration having done, open `DxLibSpineViewer.sln` with Visual Studio.
6. Select `Build Solution` on menu item.
7. Build having succeeded, delete `out` and `.vs` folders in `shared-src/deps`.

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

- For minimal integration, only `dxlib_spine.cpp/h` or `dxlib_spine_c.cpp` are suffice.
  - [dxlib_spine.cpp](/DxLibSpineCpp/dxlib_spine.cpp) is to be used with `spine-cpp`. (`3.8` to `4.2`)
  - [dxlib_spine_c.cpp](/DxLibSpineC/dxlib_spine_c.cpp) is to be used with `spine-c`. (`3.5` to `4.2`)
    - Class is used because DxLib is C++ library, but STL is avoided.
  - There is a runtime for spine `2.1.27` under [projects/DxLibSpineViewerC-2.1](/projects/DxLibSpineViewerC-2.1). But note that transformation method is totally [different](https://en.esotericsoftware.com/forum/d/3462-spines-non-skewing-transforms) from later versions.
- For more functionalities, use all the files under `DxLibSpineCpp` or `DxLibSpineC`.
  - The functionalities are as follows:

| File | Functionality |
| --- | --- |
| dxlib_spine.cpp/h | Load texture and render skeleton based on DxLib's API。 |
| dxlib_spine_player.cpp/h | Adjust Spine scaling and translation based on DxLib's matrices. |
| spine_loader.cpp/h | Load atlas or skeleton file. |
| spine_player.cpp/h | Manage Spine resources and manipulation. |
