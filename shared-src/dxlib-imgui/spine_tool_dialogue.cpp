
#include <string>
#include <vector>
#include <algorithm>

#include "spine_tool_dialogue.h"
#include "../spine_player_shared.h"

#include <imgui.h>

/* 内部用 */
namespace spine_tool_dialogue
{
	namespace slot_exclusion
	{
		static constexpr int kFilterSize = 32;
		static char s_szFilter[kFilterSize]{};
		static size_t s_nFilterLength = 0;

		static bool IsSlotToBeExcluded(const char* szSlotName, size_t nSlotNameLength)
		{
			if (s_nFilterLength == 0)return false;

			const char* pNameEnd = szSlotName + nSlotNameLength;
			return std::search(szSlotName, pNameEnd, s_szFilter, s_szFilter + s_nFilterLength) != pNameEnd;
		}
	};

	struct ImGuiComboBox
	{
		unsigned int selectedIndex = 0;

		void update(const std::vector<std::string>& itemNames, const char* comboLabel)
		{
			if (selectedIndex >= itemNames.size())
			{
				selectedIndex = 0;
				return;
			}

			if (ImGui::BeginCombo(comboLabel, itemNames[selectedIndex].c_str(), ImGuiComboFlags_HeightLarge))
			{
				for (size_t i = 0; i < itemNames.size(); ++i)
				{
					bool isSelected = (selectedIndex == i);
					if (ImGui::Selectable(itemNames[i].c_str(), isSelected))
					{
						selectedIndex = static_cast<unsigned int>(i);
					}

					if (isSelected)ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
	};

	struct ImGuiListview
	{
		std::basic_string<bool> checks;
		ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_NoAutoSelect | ImGuiMultiSelectFlags_NoAutoClear | ImGuiMultiSelectFlags_ClearOnEscape;

		void update(const std::vector<std::string>& itemNames, const char* windowLabel)
		{
			if (checks.size() != itemNames.size())
			{
				clear(itemNames);
			}

			ImVec2 childWindowSize = { ImGui::GetWindowWidth() * 3 / 4.f, ImGui::GetFontSize() * (checks.size() / 4 + 2LL) };
			if (ImGui::BeginChild(windowLabel, childWindowSize, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY))
			{
				ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags, -1, static_cast<int>(itemNames.size()));
				ImGuiSelectionExternalStorage storage_wrapper;
				storage_wrapper.UserData = (void*)&checks[0];
				storage_wrapper.AdapterSetItemSelected = [](ImGuiSelectionExternalStorage* self, int n, bool selected) { bool* array = (bool*)self->UserData; array[n] = selected; };
				storage_wrapper.ApplyRequests(ms_io);
				for (size_t i = 0; i < itemNames.size(); ++i)
				{
					ImGui::SetNextItemSelectionUserData(i);
					ImGui::Checkbox(itemNames[i].c_str(), &checks[i]);
				}

				ms_io = ImGui::EndMultiSelect();
				storage_wrapper.ApplyRequests(ms_io);
			}

			ImGui::EndChild();
		}

		void pickupCheckedItems(const std::vector<std::string>& itemNames, std::vector<std::string>& selectedItems)
		{
			if (itemNames.size() != checks.size())return;

			for (size_t i = 0; i < checks.size(); ++i)
			{
				if (checks[i])
				{
					selectedItems.emplace_back(itemNames[i]);
				}
			}
		}

		void clear(const std::vector<std::string>& itemNames)
		{
			checks = std::basic_string<bool>(itemNames.size(), false);
		}
	};

	static void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	static void ScrollableSliderInt(const char* label, int* v, int v_min, int v_max)
	{
		ImGui::SliderInt(label, v, v_min, v_max);
		ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
		if (ImGui::IsItemHovered())
		{
			float wheel = ImGui::GetIO().MouseWheel;
			if (wheel > 0 && *v < v_max)
			{
				++(*v);
			}
			else if (wheel < 0 && *v > v_min)
			{
				--(*v);
			}
		}
	}

	static void ScrollableSliderFloat(const char* label, float* v, float v_min, float v_max)
	{
		ImGui::SliderFloat(label, v, v_min, v_max, "%.1f");
		ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
		if (ImGui::IsItemHovered())
		{
			float wheel = ImGui::GetIO().MouseWheel;
			if (wheel > 0 && *v < v_max)
			{
				++(*v);
			}
			else if (wheel < 0 && *v > v_min)
			{
				--(*v);
			}
		}
	}
}

void spine_tool_dialogue::Display(SSpineToolDatum& spineToolDatum, bool* pIsOpen)
{
	if (spineToolDatum.pSpinePlayer == nullptr || pIsOpen == nullptr)return;
	CDxLibSpinePlayer* pDxLibSpinePlayer = static_cast<CDxLibSpinePlayer*>(spineToolDatum.pSpinePlayer);

	ImGui::Begin("Spine tool", pIsOpen);

	if (ImGui::BeginTabBar("Parameter category", ImGuiTabBarFlags_None))
	{
		/* 寸法・座標・尺度 */
		if (ImGui::BeginTabItem("Size/Scale"))
		{
			ImGui::Text("Texture size: (%d, %d)", spineToolDatum.iTextureWidth, spineToolDatum.iTextureHeight);

			const auto& baseSize = pDxLibSpinePlayer->getBaseSize();
			const auto& offset = pDxLibSpinePlayer->getOffset();

			ImGui::Text("Skeleton size: (%.2f, %.2f)", baseSize.x, baseSize.y);
			ImGui::Text("Offset: (%.2f, %.2f)", offset.x, offset.y);
			ImGui::Text("Skeleton scale: %.2f", pDxLibSpinePlayer->getSkeletonScale());
			ImGui::Text("Canvas scale: %.2f", pDxLibSpinePlayer->getCanvasScale());

			if (ImGui::TreeNode("Slot bounding"))
			{
				const std::vector<std::string>& slotNames = pDxLibSpinePlayer->getSlotNames();
				static ImGuiComboBox slotsComboBox;
				slotsComboBox.update(slotNames, "Slot##SlotBounding");
				const auto& slotBounding = pDxLibSpinePlayer->getCurrentBoundingOfSlot(slotNames[slotsComboBox.selectedIndex]);
				if (slotBounding.z == 0.f)
				{
					ImGui::TextColored(ImVec4{1.f, 0.f, 0.f, 1.f}, "Slot not found in this animation.");
				}
				else
				{
					ImGui::Text("Slot bounding: (%.2f, %.2f, %.2f, %.2f)", slotBounding.x, slotBounding.y, slotBounding.x + slotBounding.z, slotBounding.y + slotBounding.w);

					static bool toDrawRect = false;
					ImGui::Checkbox("Draw slot bounding", &toDrawRect);
					if (toDrawRect)
					{
						static constexpr float fMinThickness = 1.f;
						static constexpr float fMaxThickness = 14.f;
						static float fThickness = 2.f;
						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
						ScrollableSliderFloat("Thickness", &fThickness, fMinThickness, fMaxThickness);

						/* 0xf0f0f0 */
						static ImVec4 fRectangleColor = ImVec4(240 / 255.f, 240 / 255.f, 240 / 255.f, 1.00f);
						ImGui::SameLine();
						ImGui::ColorEdit4("Colour", (float*)&fRectangleColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);
						unsigned int rectangleColour = ImGui::ColorConvertFloat4ToU32(fRectangleColor);
						/* ABGR -> ARGB */
						rectangleColour = ((rectangleColour & 0x000000ff) << 16) | ((rectangleColour & 0x00ff0000) >> 16) | ((rectangleColour & 0xff00ff00));

						DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
						DxLib::MATRIX matrix = pDxLibSpinePlayer->calculateTransformMatrix();
						DxLib::SetTransformTo2D(&matrix);
						DxLib::DrawBoxAA(slotBounding.x, slotBounding.y, slotBounding.x + slotBounding.z, slotBounding.y + slotBounding.w, rectangleColour, 0, fThickness);
						DxLib::ResetTransformTo2D();

						if (ImGui::Button("Fit to this slot"))
						{
							pDxLibSpinePlayer->setBaseSize(slotBounding.z, slotBounding.w);
							pDxLibSpinePlayer->update(0.f);
							const auto& updatedSlotBounding = pDxLibSpinePlayer->getCurrentBoundingOfSlot(slotNames[slotsComboBox.selectedIndex]);
							if (updatedSlotBounding.z != 0)
							{
								auto offsetToBe = pDxLibSpinePlayer->getOffset();
								offsetToBe.x += updatedSlotBounding.x;
								offsetToBe.y += updatedSlotBounding.y;
								pDxLibSpinePlayer->setOffset(offsetToBe.x, offsetToBe.y);
								pDxLibSpinePlayer->setBaseSize(slotBounding.z, slotBounding.w);
							}
							spineToolDatum.isWindowToBeResized = true;
						}
					}
				}

				ImGui::TreePop();
			}


			bool bRet = ImGui::TreeNode("Help##Size/Scale");
			ImGui::SameLine();
			/* Always show this help regardless of tree state. */
			HelpMarker("Sizing/scaling can be done by mouse inputs on main window, not via tool window.");
			if (bRet)
			{
				struct HelpText
				{
					enum
					{
						Input,
						Description,
						kMax
					};
				};
				constexpr const char* const mouseHelps[][HelpText::kMax] =
				{
					{"L-drag", "Move view-point"},
					{"Scroll", "Scale up/down."},
					{"Ctrl + scroll", "Zoom in/out"},
					{"L-pressed + scroll", "Speed up/down animation"},
					{"M-click", "Reset zoom, speed, offset."},
					{"R-pressed + M-click", "Show/hide window's frame"},
					{"R-pressed + L-drag", "Move frameless window"},
				};

				float windowWidth = ImGui::GetWindowWidth();
				ImGui::SeparatorText("Mouse functions:");
				for (const auto& mouseHelp : mouseHelps)
				{
					ImGui::BulletText(mouseHelp[HelpText::Input]);
					ImGui::SameLine(windowWidth * 7 / 16);
					ImGui::Text(": %s", mouseHelp[HelpText::Description]);
				}

				ImGui::TreePop();
			}

			ImGui::EndTabItem();
		}
		/* 動作名・動作指定・動作合成 */
		if (ImGui::BeginTabItem("Animation"))
		{
			const std::string& animationName = pDxLibSpinePlayer->getCurrentAnimationName();
			DxLib::FLOAT4 animationWatch{};
			pDxLibSpinePlayer->getCurrentAnimationTime(&animationWatch.x, &animationWatch.y, &animationWatch.z, &animationWatch.w);

			ImGui::SliderFloat(animationName.c_str(), &animationWatch.y, animationWatch.z, animationWatch.w, "%0.2f");
			ImGui::Text("Time scale: %.2f", pDxLibSpinePlayer->getTimeScale());

			const std::vector<std::string>& animationNames = pDxLibSpinePlayer->getAnimationNames();
			/* 動作指定 */
			if (ImGui::TreeNode("Set animation"))
			{
				static ImGuiComboBox animationComboBox;
				animationComboBox.update(animationNames, "##AnimationToSet");

				if (ImGui::Button("Apply##SetAnimation"))
				{
					pDxLibSpinePlayer->setAnimationByIndex(animationComboBox.selectedIndex);
				}

				ImGui::TreePop();
			}
			/* 動作予約 */
			if (ImGui::TreeNode("Add tracks"))
			{
				HelpMarker("Adding tracks will overwrite animation state.\n"
					"Uncheck all the items and then apply to reset the state.");

				static ImGuiListview animationsListView;
				animationsListView.update(animationNames, "Animation to mix##AnimationsToMix");

				static bool looped = false;
				ImGui::Checkbox("Loop", &looped);

				if (ImGui::Button("Add##AddAnimationTracks"))
				{
					std::vector<std::string> checkedItems;
					animationsListView.pickupCheckedItems(animationNames, checkedItems);
					pDxLibSpinePlayer->addAnimationTracks(checkedItems, looped);
				}

				ImGui::TreePop();
			}

			ImGui::EndTabItem();
		}
		/* 装い指定・合成 */
		if (ImGui::BeginTabItem("Skin"))
		{
			const std::vector<std::string>& skinNames = pDxLibSpinePlayer->getSkinNames();
			/* 装い指定 */
			if (ImGui::TreeNode("Set Skin"))
			{
				static ImGuiComboBox skinComboBox;
				skinComboBox.update(skinNames, "##SkinToSet");

				if (ImGui::Button("Apply##SetSkin"))
				{
					pDxLibSpinePlayer->setSkinByIndex(skinComboBox.selectedIndex);
				}

				ImGui::TreePop();
			}
			/* 装い合成 */
			if (ImGui::TreeNode("Mix skin"))
			{
				HelpMarker("Mixing skins will overwrite the current skin.\n"
					"The state cannnot be gotten back to the original state unless reloaded.");

				static ImGuiListview skinListView;
				skinListView.update(skinNames, "Skins to mix##SkinsToMix");

				if (ImGui::Button("Apply##MixSkins"))
				{
					std::vector<std::string> checkedItems;
					skinListView.pickupCheckedItems(skinNames, checkedItems);
					pDxLibSpinePlayer->mixSkins(checkedItems);
				}
				ImGui::TreePop();
			}

			ImGui::EndTabItem();
		}
		/* 槽溝 */
		if (ImGui::BeginTabItem("Slot"))
		{
			const std::vector<std::string>& slotNames = pDxLibSpinePlayer->getSlotNames();
			/* 描画対象から除外 */
			if (ImGui::TreeNode("Exclude slot by items"))
			{
				HelpMarker("Checked slots will be excluded from rendering.");

				static ImGuiListview slotListView;
				slotListView.update(slotNames, "Slots to exclude##SlotsToExclude");

				if (ImGui::Button("Apply##ExcludeSlots"))
				{
					std::vector<std::string> checkedItems;
					slotListView.pickupCheckedItems(slotNames, checkedItems);
					pDxLibSpinePlayer->setSlotsToExclude(checkedItems);
					pDxLibSpinePlayer->setSlotExcludeCallback(nullptr);
				}

				ImGui::SameLine();
				if (ImGui::Button("Clear##ExcludeSlots"))
				{
					slotListView.clear(slotNames);
					pDxLibSpinePlayer->setSlotsToExclude({});
				}

				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Exclude slot by filter"))
			{
				{
					using namespace slot_exclusion;
					ImGui::InputText("Filter", s_szFilter, sizeof(s_szFilter));
					if (ImGui::Button("Apply##ExcludeSlotByFilter"))
					{
						s_nFilterLength = strlen(s_szFilter);
						pDxLibSpinePlayer->setSlotExcludeCallback(s_nFilterLength == 0 ? nullptr : &IsSlotToBeExcluded);
					}
				}

				ImGui::TreePop();
			}
			/* 挿げ替え */
			if (ImGui::TreeNode("Replace attachment"))
			{
				HelpMarker(
					"This feature is available only when there be slot associated with multiple attachments.\n"
					"Even if it is permitted to replace slot, it does not gurantee the consistency in timeline.");

				/* 滅多に利用機会がないので、非効率なのは承知でこのまま。 */
				const auto& slotAttachmentMap = pDxLibSpinePlayer->getSlotNamesWithTheirAttachments();
				if (!slotAttachmentMap.empty())
				{
					std::vector<std::string> slotCandidates;
					slotCandidates.reserve(slotAttachmentMap.size());
					for (const auto& slot : slotAttachmentMap)
					{
						slotCandidates.emplace_back(slot.first);
					}

					static ImGuiComboBox slotsComboBox;
					slotsComboBox.update(slotCandidates, "Slot##SlotCandidates");

					const auto& iter = slotAttachmentMap.find(slotCandidates[slotsComboBox.selectedIndex]);
					if (iter != slotAttachmentMap.cend())
					{
						static ImGuiComboBox attachmentComboBox;
						attachmentComboBox.update(iter->second, "Attachment##AssociatesAttachments");

						if (ImGui::Button("Apply##ReplaceAttachment"))
						{
							pDxLibSpinePlayer->replaceAttachment(
								slotCandidates[slotsComboBox.selectedIndex].c_str(),
								iter->second[attachmentComboBox.selectedIndex].c_str()
							);
						}
					}
				}
				ImGui::TreePop();
			}

			ImGui::EndTabItem();
		}
		/* 描画 */
		if (ImGui::BeginTabItem("Rendering"))
		{
			ImGui::SeparatorText("Premultiplied alpha");

			HelpMarker("For Spine 3.8 and older, PMA should be configured manually.\n"
				"For Spine 4.0 and later, PMA property of atlas page is applied.");

			bool pma = pDxLibSpinePlayer->isAlphaPremultiplied();
			bool pmaChecked = pma;
#if defined(SPINE_4_0) || defined(SPINE_4_1_OR_LATER) || defined(SPINE_4_2_OR_LATER)
			ImGui::BeginDisabled();
#endif
			ImGui::Checkbox("Alpha premultiplied", &pmaChecked);
#if defined(SPINE_4_0) || defined(SPINE_4_1_OR_LATER) || defined(SPINE_4_2_OR_LATER)
			ImGui::EndDisabled();
#endif
			if (pmaChecked != pma)
			{
				pDxLibSpinePlayer->togglePma();
			}
			ImGui::SeparatorText("Blend-mode");

			HelpMarker("Force if blend-mode-multiply is not well rendered.");

			bool toForceBlendModeNormal = pDxLibSpinePlayer->isBlendModeNormalForced();
			bool blendModeChecked = toForceBlendModeNormal;
			ImGui::Checkbox("Force blend-mode-normal", &blendModeChecked);
			if (blendModeChecked != toForceBlendModeNormal)
			{
				pDxLibSpinePlayer->toggleBlendModeAdoption();
			}

			ImGui::SeparatorText("Draw order");
			HelpMarker("Draw order is crutial only when rendering multiple Spines.\n"
				"Be sure to make it appropriate in prior to add animation effect.");

			if (pDxLibSpinePlayer->getNumberOfSpines() > 1)
			{
				bool drawOrder = pDxLibSpinePlayer->isDrawOrderReversed();
				ImGui::Checkbox("Reverse draw order", &drawOrder);
				pDxLibSpinePlayer->setDrawOrder(drawOrder);
			}

			ImGui::EndTabItem();
		}
		/* 書き出し */
		if (ImGui::BeginTabItem("Export"))
		{
			HelpMarker("Export can be done via context menu on main window.");

			ImGui::SeparatorText("Export FPS");
			HelpMarker("GIF delay is defined in 10ms increments.\n Mind that fractional part will be discarded.");

			constexpr int minFps = 15;
			constexpr int maxImageFps = 60;
			constexpr int maxVideoFps = 120;

			auto& imageFps = spineToolDatum.iImageFps;
			auto& videoFps = spineToolDatum.iVideoFps;

			ScrollableSliderInt("Image", &imageFps, minFps, maxImageFps);
			ScrollableSliderInt("Video", &spineToolDatum.iVideoFps, minFps, maxVideoFps);

			ImGui::SeparatorText("Export method");
			HelpMarker("If unchecked, when to start and end recording will be delegated to user.");
			ImGui::Checkbox("Export per animation", &spineToolDatum.toExportPerAnim);

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

bool spine_tool_dialogue::HasSlotExclusionFilter()
{
	return slot_exclusion::s_nFilterLength > 0;
}

bool(*spine_tool_dialogue::GetSlotExcludeCallback())(const char*, size_t)
{
	return &slot_exclusion::IsSlotToBeExcluded;
}