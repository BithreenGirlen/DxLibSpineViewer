#ifndef DXLIB_INIT_H_
#define DXLIB_INIT_H_

struct SDxLibInit
{
	SDxLibInit(void* pWindowHandle = nullptr);
	~SDxLibInit();

	int iDxLibInitialised = -1;
};

#endif // !DXLIB_INIT_H_
