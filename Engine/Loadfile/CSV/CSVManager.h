#pragma once
#include <string>
#include <vector>

class CSVManager {

public:
	static CSVManager* GetInstance();

	// CSV 読み込み
	void LoadCSV(const std::string& filename);

	// 読み込んだデータを返す
	const std::vector<std::vector<int>>& GetData() const { return data_; }

private:
	CSVManager() = default;
	std::vector<std::vector<int>> data_;
};
