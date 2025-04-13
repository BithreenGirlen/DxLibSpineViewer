#ifndef WIN_IMAGE_H_
#define WIN_IMAGE_H_

#include <vector>

struct SImageFrame
{
	unsigned int uiWidth = 0;
	unsigned int uiHeight = 0;
	int iStride = 0;
	std::vector<unsigned char> pixels;
};

namespace win_image
{
	enum class ERotation
	{
		None, Deg90, Deg180, Deg270
	};
	bool LoadImageToMemory(const wchar_t* pwzFilePath, SImageFrame* pImageFrame, float fScale = 1.f, ERotation rotation = ERotation::None);
	bool SkimImageSize(const wchar_t* pwzFilePath, unsigned int* uiWidth, unsigned int* uiHeight);

	bool SaveImageAsPng(const wchar_t* pwzFilePath, SImageFrame* pImageFrame);

	class CWicGifEncoder
	{
	public:
		CWicGifEncoder();
		~CWicGifEncoder();

		bool Initialise(const wchar_t* pwzFilePath);
		bool CommitFrame(SImageFrame* pImageFrame);
		bool End();
	private:
		class Impl;
		Impl* m_impl = nullptr;
	};
}
#endif // !WIN_IMAGE_H_
