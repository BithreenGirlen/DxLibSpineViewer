#ifndef DXLIB_RECORDER_H_
#define DXLIB_RECORDER_H_

class CDxLibRecorder
{
public:
	CDxLibRecorder();
	~CDxLibRecorder();

	enum class EOutputType
	{
		Unknown,
		Pngs,
		Gif,
		Video,
	};
	bool Start(EOutputType outputType, unsigned int fps = kDefaultFps);
	EOutputType GetOutputType() const;
	int GetFps() const;

	enum class EState
	{
		Idle,
		UnderRecording,
		InitialisingVideoStream,
	};
	EState GetState() const;

	bool CaptureFrame(const wchar_t* pwzFileName = nullptr);

	bool End(const wchar_t* pwzFilePath);
private:
	static constexpr unsigned int kDefaultFps = 30;

	class Impl;
	Impl* m_impl = nullptr;
};
#endif // !DXLIB_RECORDER_H_
