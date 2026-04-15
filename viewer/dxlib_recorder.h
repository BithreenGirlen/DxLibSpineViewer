#ifndef DXLIB_RECORDER_H_
#define DXLIB_RECORDER_H_

/// @brief 録画器
class CDxLibRecorder
{
public:
	CDxLibRecorder();
	~CDxLibRecorder();

	/// @brief 出力形式
	enum class EOutputType
	{
		Unknown,
		Gif,
		Video,
		Pngs,
		Jpgs,
	};
	/// @brief 録画開始
	bool start(EOutputType outputType, unsigned int fps = kDefaultFps);
	/// @brief 録画中の場合、出力形式を取得
	EOutputType getOutputType() const;
	/// @brief 録画中の場合、フレーム率を取得
	int getFps() const;

	/// @brief 動作状態
	enum class EState
	{
		Idle,
		UnderRecording,
		InitialisingVideoStream,
	};
	/// @brief 動作状態を取得
	EState getState() const;

	/// @brief 現在の描画先を複写して録画フレームとする
	/// @param imageName 個々のフレーム名。連番画像出力時のみ必要
	bool captureFrame(const wchar_t* imageName = nullptr);
	/// @brief 描画対象にできる紋理を複写して録画フレームとする
	/// @param iGraphicHandle 描画対象にできるグラフィックハンドル
	/// @param imageName 個々のフレーム名。連番画像出力時のみ必要
	bool commitFrame(const int iGraphicHandle, const wchar_t* imageName = nullptr);
	/// @brief 1フレームでも有しているか
	bool hasFrames() const;

	/// @brief 録画終了
	/// @param filePath 拡張子を除いたファイル経路。拡張子は出力形式に応じて付与
	bool end(const wchar_t* filePath);
private:
	static constexpr unsigned int kDefaultFps = 30;

	class Impl;
	Impl* m_impl = nullptr;
};
#endif // !DXLIB_RECORDER_H_
