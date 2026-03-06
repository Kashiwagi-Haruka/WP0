#pragma once

class ToolBar final {
public:
	struct Result final {
		bool undoRequested = false;
		bool redoRequested = false;
		bool playRequested = false;
		bool stopRequested = false;
	};

	static Result Draw(bool isPlaying, bool hasUnsavedChanges, bool canUndo, bool canRedo);
};