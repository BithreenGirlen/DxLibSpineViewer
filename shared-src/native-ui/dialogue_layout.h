#ifndef DIALOGUE_LAYOUT_H_
#define DIALOGUE_LAYOUT_H_

#include <Windows.h>

namespace dialogue_layout
{
	enum class WidthOption : char { Auto, Quarter, Half, Fixed };
	enum class HeightOption : char { Auto, List, Combo, Fixed };
	struct Child
	{
		HWND hWnd;
		WidthOption widthOption = WidthOption::Auto;
		HeightOption heightOption = HeightOption::Auto;
		unsigned short width = 0;
		unsigned short height = 0;
	};

	template <size_t childCount>
	void LayoutControls(HWND hParentWnd, unsigned int fontSize, const Child(&childeren)[childCount])
	{
		RECT rect{};
		::GetClientRect(hParentWnd, &rect);

		int clientWidth = rect.right - rect.left;
		int clientHeight = rect.bottom - rect.top;

		int spaceX = clientWidth / 96;
		int spaceY = clientHeight / 96;

		int fontHeight = static_cast<int>(fontSize * ::GetDpiForWindow(hParentWnd) / 96.f);

		int x = spaceX;
		int y = spaceY;
		int w = clientWidth - spaceX * 2;
		int h = static_cast<int>(fontHeight * 1.5);

		for (const auto& child : childeren)
		{
			switch (child.widthOption)
			{
			case WidthOption::Auto:
				w = clientWidth - spaceX * 2;
				break;
			case WidthOption::Quarter:
				w = clientWidth / 4 - spaceX;
				break;
			case WidthOption::Half:
				w = clientWidth / 2 - spaceX;
				break;
			case WidthOption::Fixed:
				w = child.width;
				break;
			}

			switch (child.heightOption)
			{
			case HeightOption::Auto:
				h = fontHeight + spaceY;
				break;
			case HeightOption::List:
				h = clientHeight / 4 + spaceY;
				break;
			case HeightOption::Combo:
				h = clientHeight / 2 + spaceY;
				break;
			case HeightOption::Fixed:
				h = child.height;
				break;
			}

			if (child.heightOption == HeightOption::Combo)
			{
				y -= spaceY;
			}

			::MoveWindow(child.hWnd, x, y, w, h, TRUE);
			if (child.heightOption == HeightOption::Combo)
			{
				y += fontHeight + spaceY * 2;
			}
			else
			{
				y += h + spaceY;
			}
		}
	}
}

#endif // !DIALOGUE_LAYOUT_H_
