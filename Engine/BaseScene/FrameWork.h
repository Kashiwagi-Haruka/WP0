#pragma once
#include "AbstractSceneFactory.h"
#include <memory>
class FrameWork {

	std::unique_ptr<AbstractSceneFactory> sceneFactory_ = nullptr;

protected:
	bool endRequest_ = false;

public:
	virtual ~FrameWork() = default;
	virtual void Initialize();
	virtual void Finalize();
	virtual void Update();
	virtual void Draw() = 0;
	virtual bool IsEndRequest() { return endRequest_; }

	void Run();
};
