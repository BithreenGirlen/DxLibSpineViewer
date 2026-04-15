#ifndef DXLIB_INIT_H_
#define DXLIB_INIT_H_

/// @brief DxLib初期化・後始末
struct SDxLibInit
{
	/// @param pWindowHandle Windows OS上で自前ウィンドウを用いる場合はHWND
	SDxLibInit(void* pWindowHandle = nullptr);
	~SDxLibInit();

	/// @brief 初期化関数戻り値
	int iDxLibInitialised = -1;
};

#endif // !DXLIB_INIT_H_
