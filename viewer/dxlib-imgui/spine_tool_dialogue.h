#ifndef SPINE_TOOL_DIALOGUE_H_
#define SPINE_TOOL_DIALOGUE_H_

namespace spine_tool_dialogue
{
	static constexpr int kDefaultImageFps = 30;
	static constexpr int kDefaultVideoFps = 60;

	struct SSpineToolDatum
	{
		void* pSpinePlayer;
		int iTextureWidth;
		int iTextureHeight;
		
		int iImageFps = kDefaultImageFps;
		int iVideoFps = kDefaultVideoFps;
		bool toExportPerAnim = true;

		bool isWindowToBeResized = false;
	};

	void Display(SSpineToolDatum& spineToolDatum, bool* pIsOpen);

	bool HasSlotExclusionFilter();
	bool (*GetSlotExcludeCallback())(const char*, size_t);
}
#endif