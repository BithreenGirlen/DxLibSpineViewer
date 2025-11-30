
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

		void Update(const std::vector<std::string>& itemNames, const char* comboLabel)
		{
			if (selectedIndex >= itemNames.size())
			{
				selectedIndex = 0;
				return;
			}

			if (ImGui::BeginCombo(comboLabel, itemNames[selectedIndex].c_str()))
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

		void Update(const std::vector<std::string>& itemNames, const char* windowLabel)
		{
			if (checks.size() != itemNames.size())
			{
				Clear(itemNames);
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

		void PickupCheckedItems(const std::vector<std::string>& itemNames, std::vector<std::string>& selectedItems)
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

		void Clear(const std::vector<std::string>& itemNames)
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

			const auto& baseSize = pDxLibSpinePlayer->GetBaseSize();
			const auto& offset = pDxLibSpinePlayer->GetOffset();

			ImGui::Text("Skeleton size: (%.2f, %.2f)", baseSize.x, baseSize.y);
			ImGui::Text("Offset: (%.2f, %.2f)", offset.x, offset.y);
			ImGui::Text("Skeleton scale: %.2f", pDxLibSpinePlayer->GetSkeletonScale());
			ImGui::Text("Canvas scale: %.2f", pDxLibSpinePlayer->GetCanvasScale());

			if (ImGui::TreeNode("Slot bounding"))
			{
				const std::vector<std::string>& slotNames = pDxLibSpinePlayer->GetSlotNames();
				static ImGuiComboBox slotsComboBox;
				slotsComboBox.Update(slotNames, "Slot##SlotBounding");
				const auto& slotBounding = pDxLibSpinePlayer->GetCurrentBoundingOfSlot(slotNames[slotsComboBox.selectedIndex]);
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
						static constexpr float fMaxThickness = 10.f;
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
						DxLib::MATRIX matrix = pDxLibSpinePlayer->CalculateTransformMatrix();
						DxLib::SetTransformTo2D(&matrix);
						DxLib::DrawBoxAA(slotBounding.x, slotBounding.y, slotBounding.x + slotBounding.z, slotBounding.y + slotBounding.w, rectangleColour, 0, fThickness);
						DxLib::ResetTransformTo2D();
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
			const std::string& animationName = pDxLibSpinePlayer->GetCurrentAnimationName();
			DxLib::FLOAT4 animationWatch{};
			pDxLibSpinePlayer->GetCurrentAnimationTime(&animationWatch.x, &animationWatch.y, &animationWatch.z, &animationWatch.w);

			ImGui::SliderFloat(animationName.c_str(), &animationWatch.y, animationWatch.z, animationWatch.w, "%0.2f");
			ImGui::Text("Time scale: %.2f", pDxLibSpinePlayer->GetTimeScale());

			const std::vector<std::string>& animationNames = pDxLibSpinePlayer->GetAnimationNames();
			/* 動作指定 */
			if (ImGui::TreeNode("Set animation"))
			{
				static ImGuiComboBox animationComboBox;
				animationComboBox.Update(animationNames, "##AnimationToSet");

				if (ImGui::Button("Apply##SetAnimation"))
				{
					pDxLibSpinePlayer->SetAnimationByIndex(animationComboBox.selectedIndex);
				}

				ImGui::TreePop();
			}
			/* 動作合成 */
			if (ImGui::TreeNode("Mix animation"))
			{
				HelpMarker("Mixing animations will overwrite animation state.\n"
					"Uncheck all the items and then apply to reset the state.");

				static ImGuiListview animationsListView;
				animationsListView.Update(animationNames, "Animation to mix##AnimationsToMix");

				static bool toBeLooped = false;
				ImGui::Checkbox("Loop", &toBeLooped);

				if (ImGui::Button("Apply##MixAnimations"))
				{
					std::vector<std::string> checkedItems;
					animationsListView.PickupCheckedItems(animationNames, checkedItems);
					pDxLibSpinePlayer->MixAnimations(checkedItems, toBeLooped);
				}

				ImGui::TreePop();
			}

			ImGui::EndTabItem();
		}
		/* 装い指定・合成 */
		if (ImGui::BeginTabItem("Skin"))
		{
			const std::vector<std::string>& skinNames = pDxLibSpinePlayer->GetSkinNames();
			/* 装い指定 */
			if (ImGui::TreeNode("Set Skin"))
			{
				static ImGuiComboBox skinComboBox;
				skinComboBox.Update(skinNames, "##SkinToSet");

				if (ImGui::Button("Apply##SetSkin"))
				{
					pDxLibSpinePlayer->SetSkinByIndex(skinComboBox.selectedIndex);
				}

				ImGui::TreePop();
			}
			/* 装い合成 */
			if (ImGui::TreeNode("Mix skin"))
			{
				HelpMarker("Mixing skins will overwrite the current skin.\n"
					"The state cannnot be gotten back to the original state unless reloaded.");

				static ImGuiListview skinListView;
				skinListView.Update(skinNames, "Skins to mix##SkinsToMix");

				if (ImGui::Button("Apply##MixSkins"))
				{
					std::vector<std::string> checkedItems;
					skinListView.PickupCheckedItems(skinNames, checkedItems);
					pDxLibSpinePlayer->MixSkins(checkedItems);
				}
				ImGui::TreePop();
			}

			ImGui::EndTabItem();
		}
		/* 槽溝 */
		if (ImGui::BeginTabItem("Slot"))
		{
			const std::vector<std::string>& slotNames = pDxLibSpinePlayer->GetSlotNames();
			/* 描画対象から除外 */
			if (ImGui::TreeNode("Exclude slot by items"))
			{
				HelpMarker("Checked slots will be excluded from rendering.");

				static ImGuiListview slotListView;
				slotListView.Update(slotNames, "Slots to exclude##SlotsToExclude");

				if (ImGui::Button("Apply##ExcludeSlots"))
				{
					std::vector<std::string> checkedItems;
					slotListView.PickupCheckedItems(slotNames, checkedItems);
					pDxLibSpinePlayer->SetSlotsToExclude(checkedItems);
					pDxLibSpinePlayer->SetSlotExcludeCallback(nullptr);
				}

				ImGui::SameLine();
				if (ImGui::Button("Clear##ExcludeSlots"))
				{
					slotListView.Clear(slotNames);
					pDxLibSpinePlayer->SetSlotsToExclude({});
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
						pDxLibSpinePlayer->SetSlotExcludeCallback(s_nFilterLength == 0 ? nullptr : &IsSlotToBeExcluded);
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
				const auto& slotAttachmentMap = pDxLibSpinePlayer->GetSlotNamesWithTheirAttachments();
				if (!slotAttachmentMap.empty())
				{
					std::vector<std::string> slotCandidates;
					slotCandidates.reserve(slotAttachmentMap.size());
					for (const auto& slot : slotAttachmentMap)
					{
						slotCandidates.emplace_back(slot.first);
					}

					static ImGuiComboBox slotsComboBox;
					slotsComboBox.Update(slotCandidates, "Slot##SlotCandidates");

					const auto& iter = slotAttachmentMap.find(slotCandidates[slotsComboBox.selectedIndex]);
					if (iter != slotAttachmentMap.cend())
					{
						static ImGuiComboBox attachmentComboBox;
						attachmentComboBox.Update(iter->second, "Attachment##AssociatesAttachments");

						if (ImGui::Button("Apply##ReplaceAttachment"))
						{
							pDxLibSpinePlayer->ReplaceAttachment(
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

			bool pma = pDxLibSpinePlayer->IsAlphaPremultiplied();
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
				pDxLibSpinePlayer->TogglePma();
			}
			ImGui::SeparatorText("Blend-mode");

			HelpMarker("Force if blend-mode-multiply is not well rendered.");

			bool toForceBlendModeNormal = pDxLibSpinePlayer->IsBlendModeNormalForced();
			bool blendModeChecked = toForceBlendModeNormal;
			ImGui::Checkbox("Force blend-mode-normal", &blendModeChecked);
			if (blendModeChecked != toForceBlendModeNormal)
			{
				pDxLibSpinePlayer->ToggleBlendModeAdoption();
			}

			ImGui::SeparatorText("Draw order");
			HelpMarker("Draw order is crutial only when rendering multiple Spines.\n"
				"Be sure to make it appropriate in prior to add animation effect.");

			if (pDxLibSpinePlayer->GetNumberOfSpines() > 1)
			{
				bool drawOrder = pDxLibSpinePlayer->IsDrawOrderReversed();
				ImGui::Checkbox("Reverse draw order", &drawOrder);
				pDxLibSpinePlayer->SetDrawOrder(drawOrder);
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