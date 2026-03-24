#ifndef WIN_IMAGE_H_
#define WIN_IMAGE_H_

#include <vector>

namespace win_image
{
	struct SImageFrame
	{
		unsigned int uiWidth = 0;
		unsigned int uiHeight = 0;
		unsigned int uiStride = 0;
		std::vector<unsigned char> pixels;
	};

	enum class ERotation
	{
		None, Deg90, Deg180, Deg270
	};

	bool LoadImageToMemory(const wchar_t* filePath, SImageFrame* pImageFrame, float fScale = 1.f, ERotation rotation = ERotation::None);
	/// @brief Pass `IWICBitmap**` to the second argument.
	bool LoadImageToWicBitmap(const wchar_t* filePath, void** pWicBitmap, float fScale = 1.f, ERotation rotation = ERotation::None);

	bool SkimImageSize(const wchar_t* filePath, unsigned int* width, unsigned int* height);

	bool SaveImageAsPng(const wchar_t* filePath, unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels, bool hasAlpha);
	bool SaveImageAsJpg(const wchar_t* filePath, unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels, bool hasAlpha);

	class CWicGifEncoder
	{
	public:
		CWicGifEncoder();
		~CWicGifEncoder();

		bool Initialise(const wchar_t* filePath);
		bool HasBeenInitialised() const;

		bool CommitFrame(unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels, bool hasAlpha, float delayInSeconds);

		bool Finalise();
	private:
		class Impl;
		Impl* m_impl = nullptr;
	};
}
#endif // !WIN_IMAGE_H_
