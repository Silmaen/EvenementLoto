/**
 * @file Settings.h
 * @author Silmaen
 * @date 02/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include <any>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace evl::core {

/**
 * @brief Class Settings.
 */
class Settings {
public:
	/**
	 * @brief Default constructor.
	 */
	Settings();

	/**
	 * @brief Default destructor.
	 */
	virtual ~Settings();
	Settings(const Settings&) = default;
	Settings(Settings&&) = default;
	auto operator=(const Settings&) -> Settings& = default;
	auto operator=(Settings&&) -> Settings& = default;

	/**
	 * @brief Load configuration from file.
	 * @param iPath The file path.
	 */
	void fromFile(const std::filesystem::path& iPath);

	/**
	 * @brief Save configuration to file.
	 * @param iPath The file path.
	 */
	void toFile(const std::filesystem::path& iPath) const;

	/**
	 * @brief Get a value from configuration.
	 * @tparam T Type of the value.
	 * @param iKey The key.
	 * @param iDefault The default value.
	 * @return The value.
	 */
	template<typename T>
	auto getValue(const std::string& iKey, const T& iDefault = T{}) const -> T {
		if (const auto it = m_data.find(iKey); it != m_data.end()) {
			try {
				return std::any_cast<T>(it->second);
			} catch (const std::bad_any_cast&) { return iDefault; }
		}
		return iDefault;
	}

	/**
	 * @brief Check if a key exists in configuration.
	 * @param iKey The key.
	 * @return True if the key exists, false otherwise.
	 */
	auto contains(const std::string& iKey) const -> bool { return m_data.contains(iKey); }

	/**
	 * @brief Set a value in configuration.
	 * @param iKey The key.
	 * @param iValue The value.
	 */
	void setValue(const std::string& iKey, const std::any& iValue);

	/**
	 * @brief Clear all settings.
	 */
	void clear();

	/**
	 * @brief remove a setting.
	 * @param iKey The key.
	 */
	void remove(const std::string& iKey);

	/**
	 * @brief Include settings from another Settings object, with an optional prefix.
	 * @param iOther The other Settings object.
	 * @param iPrefix The prefix to add to each key.
	 *
	 * @note If the key already exists, it will be overwritten.
	 */
	void include(const Settings& iOther, std::string_view iPrefix = "");

	/**
	 * @brief Include settings from another Settings object, with an optional prefix.
	 * @param iOther The other Settings object.
	 * @param iPrefix The prefix to add to each key.
	 *
	 * @note If the key already exists, it will NOT be overwritten.
	 */
	void includeMissing(const Settings& iOther, std::string_view iPrefix = "");

	/**
	 * @brief Extract a subset of settings with a given prefix.
	 * @param iPrefix The prefix to filter keys.
	 * @return A new Settings object containing only the filtered settings.
	 */
	auto extract(const std::string_view& iPrefix) const -> Settings;

private:
	/// Data storage.
	std::unordered_map<std::string, std::any> m_data;
};

}// namespace evl::core
