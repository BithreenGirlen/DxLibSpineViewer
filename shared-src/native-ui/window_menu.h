#ifndef WINDOW_MENU_H_
#define WINDOW_MENU_H_

#include <Windows.h>

namespace window_menu
{
	struct MenuItem
	{
		UINT_PTR id = 0;
		const wchar_t* name = nullptr;
		HMENU child = nullptr;
	};

	struct Result
	{
		enum { Error = 0, Success };
	};

	class MenuBuilder
	{
	public:
		template <size_t itemCount>
		MenuBuilder(const MenuItem(&menuItems)[itemCount])
		{
			m_hMenu = ::CreateMenu();

			for (const auto& menuItem : menuItems)
			{
				if (IsValid())
				{
					if (menuItem.child == nullptr)
					{
						if (menuItem.name == nullptr)
						{
							m_lastResult = ::AppendMenuW(m_hMenu, MF_SEPARATOR, 0, nullptr);
						}
						else
						{
							m_lastResult = ::AppendMenuW(m_hMenu, MF_STRING, menuItem.id, menuItem.name);
						}
					}
					else
					{
						m_lastResult = ::AppendMenuW(m_hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(menuItem.child), menuItem.name);
					}
				}
				else
				{
					Destroy();
					break;
				}
			}
		}
		~MenuBuilder()
		{
			Destroy();
		}

		HMENU Get() const { return m_hMenu; }
	private:
		HMENU m_hMenu = nullptr;
		BOOL m_lastResult = Result::Success;

		bool IsValid() const { return (m_hMenu != nullptr) && (m_lastResult != Result::Error); }

		void Destroy()
		{
			if (m_hMenu != nullptr && m_lastResult == Result::Error)
			{
				::DestroyMenu(m_hMenu);
				m_hMenu = nullptr;
			}
		}
	};

	HMENU GetMenuInBar(HWND hOwnerWindow, unsigned int index);
	bool SetMenuCheckState(HMENU hMenu, unsigned int index, bool checked);
	void EnableMenuItems(HMENU hMenu, const unsigned int* itemIndices, size_t indexCount, bool toEnable);
	template<size_t indexCount>
	void EnableMenuItems(HMENU hMenu, const unsigned int(&itemIndices)[indexCount], bool toEnable)
	{
		EnableMenuItems(hMenu, itemIndices, indexCount, toEnable);
	}

	class CContextMenu
	{
	public:
		CContextMenu()
		{
			m_hPopupMenu = ::CreatePopupMenu();
		}
		~CContextMenu()
		{
			Destroy();
		}

		void AddItems(const MenuItem* menuItems, size_t itemCount)
		{
			for (size_t i = 0; i < itemCount; ++i)
			{
				const auto& menuItem = menuItems[i];

				if (IsValid())
				{
					if (menuItem.child == nullptr)
					{
						if (menuItem.name == nullptr)
						{
							m_lastResult = ::AppendMenuW(m_hPopupMenu, MF_SEPARATOR, 0, nullptr);
						}
						else
						{
							m_lastResult = ::AppendMenuW(m_hPopupMenu, MF_STRING, menuItem.id, menuItem.name);
						}
					}
					else
					{
						m_lastResult = ::AppendMenuW(m_hPopupMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(menuItem.child), menuItem.name);
					}
				}
				else
				{
					Destroy();
					break;
				}
			}
		}

		template <size_t itemCount>
		void AddItems(const MenuItem(&menuItems)[itemCount])
		{
			AddItems(menuItems, itemCount);
		}

		/// @return Selected menu item identifier; 0 when cancelled and -1 when failed. 
		BOOL Display(HWND hOwnerWindow) const
		{
			if (!::IsMenu(m_hPopupMenu) || !::IsWindow(hOwnerWindow))return - 1;

			POINT point{};
			::GetCursorPos(&point);
			return ::TrackPopupMenu(m_hPopupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, 0, hOwnerWindow, nullptr);
		}

	private:
		HMENU m_hPopupMenu = nullptr;
		BOOL m_lastResult = Result::Success;

		bool IsValid() const { return (m_hPopupMenu != nullptr) && (m_lastResult != Result::Error); }

		void Destroy()
		{
			if (m_hPopupMenu != nullptr)
			{
				::DestroyMenu(m_hPopupMenu);
				m_hPopupMenu = nullptr;
			}
		}
	};
} /* namespace window_menu */

#endif // !WINDOW_MENU_H_

