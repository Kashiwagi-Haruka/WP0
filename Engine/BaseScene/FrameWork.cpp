#include "FrameWork.h"

void FrameWork::Run() {
	Initialize();
	while (true) {

		Update();

		if (IsEndRequest()) {
			break;
		}
		Draw();
	}

	Finalize();
}

void FrameWork::Initialize() {}

void FrameWork::Update() {}

void FrameWork::Finalize() {}
