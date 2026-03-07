#pragma once
#include "PlayableParameter.h"
class Playable {

	PlayableParameter parametersNow_;
	PlayableParameter parametersLv1_;
	PlayableParameter parametersLv80_;

	bool isHave_;
	bool newHave_;

	public:
	void Initialize();
	void Update();
	void Draw();

};
