#ifndef DXLIB_IMAGE_ENCODER_H_
#define DXLIB_IMAGE_ENCODER_H_

#include <vector>

/// @brief 画像保存処理
namespace dxlib_image_encoder
{
	/// @brief 現在の描画対象をJPGとして保存
	bool SaveScreenAsJpg(const wchar_t* filePath);
	/// @brief 現在の描画対象をPNGとして保存
	bool SaveScreenAsPng(const wchar_t* filePath);
	/// @brief 描画対象にできるグラフィックハンドルをJPGとして保存
	bool SaveRenderTextureAsJpg(int iGraphicHandle, const wchar_t* filePath);
	/// @brief 描画対象にできるグラフィックハンドルをPNGとして保存
	bool SaveRenderTextureAsPng(int iGraphicHandle, const wchar_t* filePath);
	/// @brief 現在の描画対象の画素配列を取得
	/// @param toBeRgba true時RGBA32配列として取得。false時BGRA32として取得
	bool GetScreenPixels(int* iWidth, int* iHeight, int* iStride, std::vector<unsigned char>& pixels, bool toBeRgba = true);
}

#endif // !DXLIB_IMAGE_ENCODER_H_
