/**
 * @file Log.h
 * @author Silmaen
 * @date 02/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include <format>

namespace evl {
/**
 * @brief Get the log file path.
 * @return The log file path.
 */
auto getLogPath() -> std::filesystem::path;

/**
 * @brief Core logging class.
 */
class Log {
public:
	/**
	 * @brief Verbosity levels.
	 */
	enum struct Level : uint8_t {
		Trace,///< TRACE level
		Debug,///< DEBUG level
		Info,///< INFO level
		Warning,///< WARNING level
		Error,///< ERROR level
		Critical,///< CRITICAL level
		Off///< OFF level
	};
	/**
	 * @brief initialize the logging system.
	 * @param[in] iLevel Verbosity level of the logger.
	 */
	static void init(const Level& iLevel = Level::Trace);

	/**
	 * @brief Get the current Verbosity level.
	 * @return The verbosity level.
	 */
	static auto getVerbosityLevel() -> const Level& { return m_verbosity; }

	/**
	 * @brief Defines the Verbosity level
	 * @param[in] iLevel Verbosity level.
	 */
	static void setVerbosityLevel(const Level& iLevel);

	/**
	 * @brief Destroy the logger.
	 */
	static void invalidate();

	/**
	 * @brief Check if logger is initiated.
	 * @return True if initiated.
	 */
	static auto initiated() -> bool;

	/**
	 * @brief Log a message for the core.
	 * @tparam Args Template parameters for format arguments.
	 * @param iLevel Verbosity level.
	 * @param iFmt Format string.
	 * @param iArgs Format arguments.
	 */
	template<typename... Args>
	static void log(const Level& iLevel, std::format_string<Args...> iFmt, Args&&... iArgs) {
		log(iLevel, std::format(iFmt, std::forward<Args>(iArgs)...));
	}

	/**
	 * @brief Log a message for the core.
	 * @param iLevel Verbosity level.
	 * @param iMsg Message to log.
	 */
	static void log(const Level& iLevel, const std::string_view& iMsg);

private:
	/// The level of verbosity.
	static Level m_verbosity;
};


}// namespace evl

#define log_trace(...) ::evl::Log::log(::evl::Log::Level::Trace, __VA_ARGS__)
#define log_debug(...) ::evl::Log::log(::evl::Log::Level::Debug, __VA_ARGS__)
#define log_info(...) ::evl::Log::log(::evl::Log::Level::Info, __VA_ARGS__)
#define log_warn(...) ::evl::Log::log(::evl::Log::Level::Warning, __VA_ARGS__)
#define log_warning(...) ::evl::Log::log(::evl::Log::Level::Warning, __VA_ARGS__)
#define log_error(...) ::evl::Log::log(::evl::Log::Level::Error, __VA_ARGS__)
#define log_critical(...) ::evl::Log::log(::evl::Log::Level::Critical, __VA_ARGS__)
