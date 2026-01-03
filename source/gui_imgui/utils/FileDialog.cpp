/**
 * @file FileDialog.cpp
 * @author Silmaen
 * @date 20/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "FileDialog.h"

#include "core/Log.h"
#include "core/utilities.h"
#include "gui_imgui/Application.h"

#include <nfd.hpp>

namespace evl::gui_imgui::utils {

namespace {

auto split(const std::string_view iString, const char iDelimiter = '\n') -> std::vector<std::string_view> {
	std::vector<std::string_view> result;
	size_t start = 0;
	for (size_t i = 0; i < iString.size(); ++i) {
		if (iString[i] == iDelimiter) {
			result.emplace_back(iString.substr(start, i - start));
			start = i + 1;
		}
	}
	if (start < iString.size())
		result.emplace_back(iString.substr(start));
	return result;
}

auto parseFilter(const std::string& iFilter) -> std::vector<nfdu8filteritem_t> {
	std::vector<nfdu8filteritem_t> filters;
	for (const auto filterLines = split(iFilter); const auto& line: filterLines) {
		if (line.empty())
			continue;
		const auto items = split(line, '|');
		if (items.size() != 2 || items[0].empty() || items[1].empty())
			continue;
		std::string_view spec = items[1];
		if (spec.starts_with("*.")) {
			spec.remove_prefix(2);
		}
		auto name = std::make_unique<nfdu8char_t[]>(items[0].size() + 1);
		auto specStr = std::make_unique<nfdu8char_t[]>(spec.size() + 1);
		std::memcpy(name.get(), items[0].data(), items[0].size());
		name[items[0].size()] = '\0';
		std::memcpy(specStr.get(), spec.data(), spec.size());
		specStr[spec.size()] = '\0';
		filters.push_back(nfdu8filteritem_t{name.release(), specStr.release()});
	}

	return filters;
}

}// namespace

std::filesystem::path FileDialog::m_lastPath = std::filesystem::path{};

auto FileDialog::openFile(const std::string& iFilter) -> std::filesystem::path {
	NFD::Init();
	nfdu8char_t* outPath = nullptr;
	std::filesystem::path resultPath;
	const std::string& filters{iFilter};
	auto ff = parseFilter(filters);
	if (m_lastPath.empty() || !exists(m_lastPath))
		m_lastPath = core::getSettings()->getValue<std::filesystem::path>("general/data_location", core::getExecPath());

	// Set parent window to inherit icon
	nfdwindowhandle_t parentWindow = {.type = NFD_WINDOW_HANDLE_TYPE_UNSET, .handle = nullptr};
	if (auto* window = Application::get().getMainWindow().getNativeWindow()) {
#if defined(_WIN32)
		parentWindow.type = NFD_WINDOW_HANDLE_TYPE_WINDOWS;
#elif defined(__APPLE__)
		parentWindow.type = NFD_WINDOW_HANDLE_TYPE_COCOA;
#elif defined(__linux__)
		parentWindow.type = NFD_WINDOW_HANDLE_TYPE_X11;
#endif
		parentWindow.handle = window;
	}
	const std::string mLast = m_lastPath.string();
	const nfdopendialogu8args_t args{.filterList = ff.data(),
									 .filterCount = static_cast<nfdfiltersize_t>(ff.size()),
									 .defaultPath = mLast.c_str(),
									 .parentWindow = parentWindow};
	if (const auto result = NFD_OpenDialogU8_With(&outPath, &args); result == NFD_CANCEL) {
		resultPath = std::filesystem::path{};
	} else if (result == NFD_OKAY) {
		resultPath = std::filesystem::path{outPath};
		if (is_directory(resultPath))
			m_lastPath = resultPath;
		else
			m_lastPath = resultPath.parent_path();
		NFD_FreePath(outPath);
	} else {
		log_error("while opening file: {}", NFD::GetError());
	}
	ff.clear();
	NFD::Quit();
	return resultPath;
}

auto FileDialog::saveFile([[maybe_unused]] const std::string& iFilter) -> std::filesystem::path {
	NFD::Init();
	nfdu8char_t* outPath = nullptr;
	std::filesystem::path resultPath;
	const std::string& filters{iFilter};
	auto ff = parseFilter(filters);
	if (m_lastPath.empty() || !exists(m_lastPath))
		m_lastPath = core::getSettings()->getValue<std::filesystem::path>("general/data_location", core::getExecPath());
	// Set parent window to inherit icon
	nfdwindowhandle_t parentWindow = {.type = NFD_WINDOW_HANDLE_TYPE_UNSET, .handle = nullptr};
	if (auto* window = Application::get().getMainWindow().getNativeWindow()) {
#if defined(_WIN32)
		parentWindow.type = NFD_WINDOW_HANDLE_TYPE_WINDOWS;
#elif defined(__APPLE__)
		parentWindow.type = NFD_WINDOW_HANDLE_TYPE_COCOA;
#elif defined(__linux__)
		parentWindow.type = NFD_WINDOW_HANDLE_TYPE_X11;
#endif
		parentWindow.handle = window;
	}
	const std::string mLast = m_lastPath.string();
	const nfdsavedialogu8args_t args{.filterList = ff.data(),
									 .filterCount = static_cast<nfdfiltersize_t>(ff.size()),
									 .defaultPath = mLast.c_str(),
									 .defaultName = nullptr,
									 .parentWindow = parentWindow};
	if (const auto result = NFD_SaveDialogU8_With(&outPath, &args); result == NFD_CANCEL) {
		resultPath = std::filesystem::path{};
	} else if (result == NFD_OKAY) {
		resultPath = std::filesystem::path{outPath};
		if (is_directory(resultPath))
			m_lastPath = resultPath;
		else
			m_lastPath = resultPath.parent_path();
	} else {
		log_error("while opening file: {}", NFD::GetError());
	}
	ff.clear();
	NFD::Quit();
	return resultPath;
}

auto FileDialog::selectFolder() -> std::filesystem::path {
	NFD::Init();
	nfdu8char_t* outPath = nullptr;
	std::filesystem::path resultPath;
	if (m_lastPath.empty() || !exists(m_lastPath))
		m_lastPath = core::getSettings()->getValue<std::filesystem::path>("general/data_location", core::getExecPath());

	// Set parent window to inherit icon
	nfdwindowhandle_t parentWindow = {.type = NFD_WINDOW_HANDLE_TYPE_UNSET, .handle = nullptr};
	if (auto* window = Application::get().getMainWindow().getNativeWindow()) {
#if defined(_WIN32)
		parentWindow.type = NFD_WINDOW_HANDLE_TYPE_WINDOWS;
#elif defined(__APPLE__)
		parentWindow.type = NFD_WINDOW_HANDLE_TYPE_COCOA;
#elif defined(__linux__)
		parentWindow.type = NFD_WINDOW_HANDLE_TYPE_X11;
#endif
		parentWindow.handle = window;
	}
	const std::string mLast = m_lastPath.string();
	const nfdpickfolderu8args_t args{.defaultPath = mLast.c_str(), .parentWindow = parentWindow};
	if (const auto result = NFD_PickFolderU8_With(&outPath, &args); result == NFD_CANCEL) {
		resultPath = std::filesystem::path{};
	} else if (result == NFD_OKAY) {
		resultPath = std::filesystem::path{outPath};
		m_lastPath = resultPath;
		NFD_FreePath(outPath);
	} else {
		log_error("while selecting folder: {}", NFD::GetError());
	}
	NFD::Quit();
	return resultPath;
}

}// namespace evl::gui_imgui::utils
