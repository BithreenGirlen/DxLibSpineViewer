#ifndef DXLIB_MAP_H_
#define DXLIB_MAP_H_

/// @brief Reads pixels from GPU resource
class CDxLibMap
{
public:
	CDxLibMap(int iTextureHandle);
	~CDxLibMap();

	bool isAccessible() const;

	int width() const;
	int height() const;
	int stride() const;
	unsigned char* pixels() const;
	/// @brief should be casted to DxLib::COLORDATA*
	void* colorData() const;
private:
	int m_imageHandle = -1;
	bool m_isLocked = false;

	int m_width = 0;
	int m_height = 0;
	int m_stride = 0;
	unsigned char* m_pPixels = nullptr;
	void* m_pColorData = nullptr;

	bool ReadPixels();
	void Unlock() const;
};

#endif // !DXLIB_MAP_H_
