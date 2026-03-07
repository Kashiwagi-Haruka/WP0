#pragma once
#include <memory>
class HouseHP {

	static std::unique_ptr<HouseHP> instance;

	static constexpr int kMaxHP = 40;
	int HP_;

public:
	static HouseHP* GetInstance();
	void SetHP(int HP) { HP_ = HP; };
	int GetHP() { return HP_; }
	int GetMaxHP() const { return kMaxHP; }
};