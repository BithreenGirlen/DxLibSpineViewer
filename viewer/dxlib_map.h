#ifndef DXLIB_MAP_H_
#define DXLIB_MAP_H_

/// @brief Reads pixels from GPU resource
class CDxLibMap
{
public:
	CDxLibMap(int iTextureHandle);
	~CDxLibMap();

	bool IsAccessible() const;

	int width = 0;
	int height = 0;
	int stride = 0;
	unsigned char* pPixels = nullptr;

	/// @brief should be casted to DxLib::COLORDATA*
	void* pColorData = nullptr;
private:
	int m_imageHandle = -1;
	bool m_isLocked = false;

	bool ReadPixels();
	void Unlock() const;
};

#endif // !DXLIB_MAP_H_
