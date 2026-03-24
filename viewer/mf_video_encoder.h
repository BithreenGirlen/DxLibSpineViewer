#ifndef MF_VIDEO_ENCODER_H_
#define MF_VIDEO_ENCODER_H_

class CMfVideoEncoder
{
public:
	CMfVideoEncoder();
	~CMfVideoEncoder();

	/// @brief ストリーム初期化
	/// @remark 省電力系AMD製CPU(5800U等)では幅と高さを4の倍数長にしておかないと、初期化やフレーム書き込みには成功しても
	///			"IMFSinkWriter::Finalize()"がラップしているDLL内部でハングするので予め切り捨てておくこと。
	bool initialise(const wchar_t* filePath, unsigned int width, unsigned int height, unsigned int frameRate);
	bool hasBeenInitialised() const;

	/// @brief RGBA32もしくはBGRA32配列を想定
	bool commitCpuFrame(unsigned char* pPixels, unsigned long pixelSize, bool isRgba = true);
#ifdef MF_GPU_TEXTURE
	/// @brief DXGI_FORMAT_B8G8R8A8_UNORM形式のID3D11Texture2Dを想定
	bool commitGpuFrame(void* pD3D11Texture2D);
#endif
	void finalise();
private:
	static constexpr unsigned int DefaultFrameRate = 60;

	class Impl;
	Impl* m_impl = nullptr;
};
#endif // !MF_VIDEO_ENCODER_H_
