#include "ToolBar.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

ToolBar::Result ToolBar::Draw(bool isPlaying, bool hasUnsavedChanges, bool canUndo, bool canRedo) {
	Result result{};
#ifdef USE_IMGUI
	if (!canUndo) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Undo")) {
		result.undoRequested = true;
	}
	if (!canUndo) {
		ImGui::EndDisabled();
	}

	ImGui::SameLine();
	if (!canRedo) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Redo")) {
		result.redoRequested = true;
	}
	if (!canRedo) {
		ImGui::EndDisabled();
	}

	ImGui::SameLine();
	ImGui::TextUnformatted("Mode");
	ImGui::SameLine();
	if (!isPlaying) {
		if (ImGui::Button("Play")) {
			result.playRequested = true;
		}
		if (hasUnsavedChanges) {
			ImGui::SameLine();
			ImGui::TextUnformatted("(Unsaved changes)");
		}
	} else {
		if (ImGui::Button("Stop")) {
			result.stopRequested = true;
		}
		ImGui::SameLine();
		ImGui::TextUnformatted("Playing");
	}
#else
	(void)isPlaying;
	(void)hasUnsavedChanges;
	(void)canUndo;
	(void)canRedo;
#endif
	return result;
}