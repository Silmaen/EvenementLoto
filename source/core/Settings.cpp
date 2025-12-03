/**
 * @file Settings.cpp
 * @author Silmaen
 * @date 02/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Settings.h"
#include "core/Log.h"
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace evl::core {

Settings::Settings() = default;

Settings::~Settings() = default;

void Settings::fromFile(const std::filesystem::path& iPath) {
	try {
		const YAML::Node root = YAML::LoadFile(iPath.string());

		// Recursive lambda to flatten YAML hierarchy
		std::function<void(const YAML::Node&, const std::string&)> flattenNode;
		flattenNode = [&](const YAML::Node& iNode, const std::string& iPrefix) -> void {
			if (iNode.IsMap()) {
				for (const auto& pair: iNode) {
					const std::string key = iPrefix.empty()
													? pair.first.as<std::string>()
													: std::format("{}/{}", iPrefix, pair.first.as<std::string>());
					flattenNode(pair.second, key);
				}
			} else if (iNode.IsScalar()) {
				// Try to parse as different types
				try {
					m_data[iPrefix] = iNode.as<bool>();
				} catch (...) {
					try {
						m_data[iPrefix] = iNode.as<int>();
					} catch (...) {
						try {
							m_data[iPrefix] = iNode.as<double>();
						} catch (...) { m_data[iPrefix] = iNode.as<std::string>(); }
					}
				}
			}
		};

		flattenNode(root, "");

	} catch (const YAML::Exception& e) { log_error("Failed to load settings from '{}': {}", iPath.string(), e.what()); }
}

void Settings::toFile(const std::filesystem::path& iPath) const {
	// Group keys by hierarchy
	std::map<std::string, std::map<std::string, std::any>> hierarchy;

	for (const auto& [key, value]: m_data) {
		if (const size_t lastSlash = key.rfind('/'); lastSlash == std::string::npos) {
			hierarchy[""][key] = value;
		} else {
			const std::string prefix = key.substr(0, lastSlash);
			const std::string finalKey = key.substr(lastSlash + 1);
			hierarchy[prefix][finalKey] = value;

			// Add intermediate levels
			std::string currentPrefix;
			size_t pos = 0;
			while ((pos = prefix.find('/', pos)) != std::string::npos) {
				const std::string parentPrefix = currentPrefix;
				const std::string levelKey =
						prefix.substr(currentPrefix.empty() ? 0 : currentPrefix.length() + 1,
									  pos - (currentPrefix.empty() ? 0 : currentPrefix.length() + 1));
				if (!hierarchy[parentPrefix].contains(levelKey)) {
					hierarchy[parentPrefix][levelKey] = std::any{};// Empty placeholder
				}

				currentPrefix = prefix.substr(0, pos);
				++pos;
			}

			// Add final intermediate level
			const std::string parentPrefix = currentPrefix;
			if (const std::string levelKey = prefix.substr(currentPrefix.empty() ? 0 : currentPrefix.length() + 1);
				!hierarchy[parentPrefix].contains(levelKey)) {
				hierarchy[parentPrefix][levelKey] = std::any{};
			}
		}
	}

	// Build YAML output
	YAML::Emitter out;
	out << YAML::BeginMap;

	std::function<void(const std::string&)> emitLevel;
	emitLevel = [&](const std::string& iPrefix) -> void {
		for (const auto& [key, value]: hierarchy[iPrefix]) {
			out << YAML::Key << key;

			const std::string fullKey = iPrefix.empty() ? key : std::format("{}/{}", iPrefix, key);

			// Check if this key has children by looking in m_data
			const std::string childPrefix = std::format("{}/", fullKey);
			const bool hasChildren = std::ranges::any_of(
					m_data, [&childPrefix](const auto& iPair) -> bool { return iPair.first.starts_with(childPrefix); });

			if (hasChildren) {
				out << YAML::Value << YAML::BeginMap;
				emitLevel(fullKey);
				out << YAML::EndMap;
			} else {
				out << YAML::Value;
				if (value.type() == typeid(bool)) {
					out << std::any_cast<bool>(value);
				} else if (value.type() == typeid(int)) {
					out << std::any_cast<int>(value);
				} else if (value.type() == typeid(double)) {
					const auto dValue = std::any_cast<double>(value);
					std::string formatted = std::format("{}", dValue);
					if (!formatted.contains('.') && !formatted.contains('e')) {
						formatted = std::format("{}.0", formatted);
					}
					out << formatted;
				} else if (value.type() == typeid(std::string)) {
					out << std::any_cast<std::string>(value);
				}
			}
		}
	};

	emitLevel("");
	out << YAML::EndMap;

	try {
		std::ofstream file(iPath);
		file << out.c_str();
	} catch (const std::exception& e) { log_error("Failed to save settings to '{}': {}", iPath.string(), e.what()); }
}

void Settings::setValue(const std::string& iKey, const std::any& iValue) { m_data[iKey] = iValue; }

void Settings::clear() { m_data.clear(); }

void Settings::remove(const std::string& iKey) { m_data.erase(iKey); }

void Settings::include(const Settings& iOther, const std::string_view iPrefix) {
	for (const auto& [key, value]: iOther.m_data) {
		const std::string newKey = iPrefix.empty() ? key : std::format("{}/{}", iPrefix, key);
		m_data[newKey] = value;
	}
}

void Settings::includeMissing(const Settings& iOther, std::string_view iPrefix) {
	for (const auto& [key, value]: iOther.m_data) {
		if (const std::string newKey = iPrefix.empty() ? key : std::format("{}/{}", iPrefix, key);
			!m_data.contains(newKey)) {
			m_data[newKey] = value;
		}
	}
}

auto Settings::extract(const std::string_view& iPrefix) const -> Settings {
	Settings result;
	const std::string prefixWithSlash = std::format("{}/", iPrefix);
	for (const auto& [key, value]: m_data) {
		if (key.starts_with(prefixWithSlash)) {// key starts with prefixWithSlash
			const std::string newKey = key.substr(prefixWithSlash.length());
			result.setValue(newKey, value);
		}
	}
	return result;
}

}// namespace evl::core
