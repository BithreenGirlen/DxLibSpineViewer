#ifndef DXLIB_HANDLE_H_
#define DXLIB_HANDLE_H_

template <int (*pHandleDeleter)(int)>
class DxLibHandle
{
public:
	DxLibHandle(int iHandle)
		:m_iHandle(iHandle)
	{

	};
	~DxLibHandle()
	{
		Reset();
	}

	int Get()const { return m_iHandle; }
	void Reset()
	{
		if (m_iHandle != -1 && pHandleDeleter != nullptr)
		{
			pHandleDeleter(m_iHandle);
			m_iHandle = -1;
		}
	}

	DxLibHandle(DxLibHandle&& other) noexcept
		: m_iHandle(other.m_iHandle)
	{
		other.m_iHandle = -1;
	}

	DxLibHandle& operator=(DxLibHandle&& other) noexcept
	{
		if (this != &other)
		{
			Reset();

			m_iHandle = other.m_iHandle;
			other.m_iHandle = -1;
		}
		return *this;
	}

	DxLibHandle(const DxLibHandle&) = delete;
	DxLibHandle& operator=(const DxLibHandle&) = delete;
private:
	int m_iHandle = -1;
};

#endif // !DXLIB_HANDLE_H_
