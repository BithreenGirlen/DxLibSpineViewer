
#include "window_menu.h"

HMENU window_menu::GetMenuInBar(HWND hOwnerWindow, unsigned int index)
{
    HMENU hMenuBar = ::GetMenu(hOwnerWindow);
    if (hMenuBar != nullptr)
    {
        return ::GetSubMenu(hMenuBar, index);
    }

    return nullptr;
}

bool window_menu::SetMenuCheckState(HMENU hMenu, unsigned int index, bool checked)
{
    DWORD ulRet = ::CheckMenuItem(hMenu, index, checked ? MF_CHECKED : MF_UNCHECKED);
    return ulRet != (DWORD)-1;
}

void window_menu::EnableMenuItems(HMENU hMenu, const unsigned int* itemIndices, size_t indexCount, bool toEnable)
{
    for (size_t i = 0; i < indexCount; ++i)
    {
        const unsigned int itemIndex = itemIndices[i];
        ::EnableMenuItem(hMenu, itemIndex, toEnable ? MF_ENABLED : MF_GRAYED);
    }
}
