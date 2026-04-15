

#include "dxlib_map.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>


CDxLibMap::CDxLibMap(int iTextureHandle)
	:m_imageHandle(iTextureHandle)
{
	ReadPixels();
}

CDxLibMap::~CDxLibMap()
{
	Unlock();
}

bool CDxLibMap::isAccessible() const
{
	return m_isLocked;
}

int CDxLibMap::width() const
{
	return m_width;
}

int CDxLibMap::height() const
{
	return m_height;
}

int CDxLibMap::stride() const
{
	return m_stride;
}

unsigned char* CDxLibMap::pixels() const
{
	return m_pPixels;
}

void* CDxLibMap::colorData() const
{
	return m_pColorData;
}

bool CDxLibMap::ReadPixels()
{
	int iRet = DxLib::GetGraphSize(m_imageHandle, &m_width, &m_height);
	if (iRet == -1)return false;

	void* pData = nullptr;
	DxLib::COLORDATA* pFormat = nullptr;
	iRet = DxLib::GraphLock(m_imageHandle, &m_stride, &pData, &pFormat);
	if (iRet != -1)
	{
		m_isLocked = true;
		m_pPixels = static_cast<unsigned char*>(pData);
		m_pColorData = pFormat;
	}

	return m_isLocked;
}

void CDxLibMap::Unlock() const
{
	if (m_imageHandle != -1 && m_isLocked)
	{
		DxLib::GraphUnLock(m_imageHandle);
	}
}
