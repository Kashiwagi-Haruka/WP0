#include "HouseHP.h"

std::unique_ptr<HouseHP> HouseHP::instance = nullptr;

HouseHP* HouseHP::GetInstance(){
	if (instance == nullptr) {
		instance = std::make_unique<HouseHP>();
	}
	return instance.get();
}