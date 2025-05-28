#ifndef DXLIB_RECORDER_H_
#define DXLIB_RECORDER_H_

class CDxLibRecorder
{
public:
	CDxLibRecorder();
	~CDxLibRecorder();

	enum class EOption
	{
		kNone,
		kAsVideo
	};
	bool Start(EOption eOption = EOption::kNone, unsigned int fps = kDefaultFps);

	enum class EState
	{
		Idle,
		StoringImages,
		InitialisingVideoStream,
		RecordingVideo
	};
	EState GetState() const;

	bool HasTimePassed() const;
	bool CaptureFrame(const wchar_t* pwzFileName = nullptr);

	enum class EOutputType
	{
		kPngs,
		kGif,
		kVideo,
	};
	bool End(EOutputType eOutputType, const wchar_t* pwzFilePath);
private:
	static constexpr unsigned int kDefaultFps = 30;

	class Impl;
	Impl* m_impl = nullptr;
};
#endif // !DXLIB_RECORDER_H_
