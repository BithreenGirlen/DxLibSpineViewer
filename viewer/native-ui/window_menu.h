#ifndef WINDOW_MENU_H_
#define WINDOW_MENU_H_

#include <Windows.h>

namespace window_menu
{
	struct MenuItem
	{
		UINT_PTR id = 0; /* May be 0 if its state never to be changed and click event never to be used. */
		const wchar_t* name = nullptr; /* Menu item name */
		HMENU child = nullptr; /* A child menu; intended to be the return value of MenuBuilder::get() */
	};

	/// @brief Create a menu to be used in menu bar.
	/// @remark The management of menu handle is up to caller on success.
	/// @code
	///		HMENU hMenu = window_menu::MenuBuilder(
	///		{
	///			{0, L"File", window_menu::MenuBuilder(
	///				{
	///					{ Menu::kOpenFile, L"Open file"},
	///				}).get()
	///			}
	///		}).get();
	/// @endcode
	class MenuBuilder
	{
	public:
		template <size_t itemCount>
		MenuBuilder(const MenuItem(&menuItems)[itemCount]) noexcept
		{
			m_hMenu = ::CreateMenu();
			if (m_hMenu == nullptr)return;

			for (const auto& menuItem : menuItems)
			{
				if (!addMenuItem(menuItem))
				{
					destroy();
					break;
				}
			}
		}
		~MenuBuilder() = default;

		HMENU get() const { return m_hMenu; }
	private:
		HMENU m_hMenu = nullptr;

		bool addMenuItem(const MenuItem& menuItem) const noexcept
		{
			BOOL iRet = 0;
			if (menuItem.child == nullptr)
			{
				if (menuItem.name == nullptr)
				{
					iRet = ::AppendMenuW(m_hMenu, MF_SEPARATOR, 0, nullptr);
				}
				else
				{
					iRet = ::AppendMenuW(m_hMenu, MF_STRING, menuItem.id, menuItem.name);
				}
			}
			else
			{
				iRet = ::AppendMenuW(m_hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(menuItem.child), menuItem.name);
			}

			return iRet != 0;
		}

		void destroy() noexcept
		{
			if (m_hMenu != nullptr)
			{
				::DestroyMenu(m_hMenu);
				m_hMenu = nullptr;
			}
		}
	};

	class CContextMenu
	{
	public:
		CContextMenu() noexcept
		{
			m_hPopupMenu = ::CreatePopupMenu();
		}
		~CContextMenu() noexcept
		{
			destroy();
		}

		void addItems(const MenuItem* menuItems, size_t itemCount) noexcept
		{
			if (m_hPopupMenu == nullptr)return;

			for (size_t i = 0; i < itemCount; ++i)
			{
				const auto& menuItem = menuItems[i];

				if (!addMenuItem(menuItem))
				{
					destroy();
					break;
				}
			}
		}

		template <size_t itemCount>
		void addItems(const MenuItem(&menuItems)[itemCount]) noexcept
		{
			addItems(menuItems, itemCount);
		}

		bool addMenuItem(const MenuItem&& menuItem) const noexcept
		{
			return addMenuItem(menuItem);
		}

		/// @return Selected menu item identifier; 0 when cancelled and -1 when failed. 
		BOOL display(HWND hOwnerWindow) const noexcept
		{
			if (!::IsMenu(m_hPopupMenu) || !::IsWindow(hOwnerWindow))return -1;

			POINT point{};
			::GetCursorPos(&point);
			return ::TrackPopupMenu(m_hPopupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, 0, hOwnerWindow, nullptr);
		}

	private:
		HMENU m_hPopupMenu = nullptr;

		bool addMenuItem(const MenuItem& menuItem) const noexcept
		{
			BOOL iRet = 0;
			if (menuItem.child == nullptr)
			{
				if (menuItem.name == nullptr)
				{
					iRet = ::AppendMenuW(m_hPopupMenu, MF_SEPARATOR, 0, nullptr);
				}
				else
				{
					iRet = ::AppendMenuW(m_hPopupMenu, MF_STRING, menuItem.id, menuItem.name);
				}
			}
			else
			{
				iRet = ::AppendMenuW(m_hPopupMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(menuItem.child), menuItem.name);
			}

			return iRet != 0;
		}

		void destroy() noexcept
		{
			if (m_hPopupMenu != nullptr)
			{
				::DestroyMenu(m_hPopupMenu);
				m_hPopupMenu = nullptr;
			}
		}
	};

	/// @brief Get a entry menu handle via index.
	HMENU GetMenuInBar(HWND hOwnerWindow, unsigned int index);
	bool SetMenuCheckState(HMENU hMenu, unsigned int index, bool checked);
	void EnableMenuItems(HMENU hMenu, const unsigned int* itemIndices, size_t indexCount, bool toEnable);
	template<size_t indexCount>
	void EnableMenuItems(HMENU hMenu, const unsigned int(&itemIndices)[indexCount], bool toEnable)
	{
		EnableMenuItems(hMenu, itemIndices, indexCount, toEnable);
	}

} /* namespace window_menu */

#endif // !WINDOW_MENU_H_
