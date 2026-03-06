#include "JsonManager.h"
#include "Logger.h"
#include <filesystem>
#include <fstream>

namespace {
std::filesystem::path ResolveJsonPath(const std::string& filename) {
	const std::filesystem::path inputPath(filename);
	const std::filesystem::path fileNameOnly = inputPath.filename();
	return std::filesystem::path("Resources") / "JSON" / fileNameOnly;
}
} // namespace

JsonManager* JsonManager::GetInstance() {
	static JsonManager instance;
	return &instance;
}

bool JsonManager::LoadJson(const std::string& filename) {
	data_.clear();

	const std::filesystem::path jsonPath = ResolveJsonPath(filename);
	std::ifstream file(jsonPath);
	if (!file.is_open()) {
		Logger::Log(std::string("JSON ファイルを開けません: ") + jsonPath.string() + "\n");
		return false;
	}

	try {
		file >> data_;
	} catch (const nlohmann::json::parse_error& e) {
		Logger::Log(std::string("JSON の解析に失敗しました: ") + e.what() + "\n");
		data_.clear();
		return false;
	}

	return true;
}
bool JsonManager::SaveJson(const std::string& filename) const {
	const std::filesystem::path jsonPath = ResolveJsonPath(filename);
	std::filesystem::create_directories(jsonPath.parent_path());

	std::ofstream file(jsonPath);
	if (!file.is_open()) {
		Logger::Log(std::string("JSON ファイルを書き込めません: ") + jsonPath.string() + "\n");
		return false;
	}

	file << data_.dump(2);
	return true;
}
const nlohmann::json* JsonManager::Find(const std::string& key) const {
	if (!data_.is_object()) {
		return nullptr;
	}

	auto it = data_.find(key);
	if (it == data_.end()) {
		return nullptr;
	}

	return &(*it);
}