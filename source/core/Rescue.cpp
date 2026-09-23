/**
 * @file Rescue.cpp
 * @author Silmaen
 * @date 21/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Rescue.h"

#include "AtomicFile.h"
#include "Log.h"
#include "timeFunctions.h"
#include "utilities.h"

namespace evl::core {

namespace {

/// The generations tried when recovering, newest first.
auto rescueCandidates() -> std::vector<std::filesystem::path> {
	const auto directory = rescueDirectory();
	if (directory.empty())
		return {};
	const auto current = directory / g_rescueFileName;
	return {current, std::filesystem::path{current}.concat(".1")};
}

auto isResumable(const Event& iEvent) -> bool {
	const auto status = iEvent.getStatus();
	return status != Event::Status::Invalid && status != Event::Status::Finished;
}

auto countDraws(const Event& iEvent) -> std::size_t {
	std::size_t count = 0;
	for (auto round = iEvent.beginRounds(); round != iEvent.endRounds(); ++round) count += round->drawsCount();
	return count;
}

auto ageInSeconds(const std::filesystem::path& iPath) -> double {
	std::error_code error;
	const auto written = last_write_time(iPath, error);
	if (error)
		return 0.0;
	const auto age = std::filesystem::file_time_type::clock::now() - written;
	return std::chrono::duration<double>(age).count();
}

}// namespace

auto rescueDirectory() -> std::filesystem::path {
	return getSettings()->getValue<std::filesystem::path>("general/data_location");
}

auto saveRescue(const Event& iEvent) -> bool {
	const auto directory = rescueDirectory();
	if (directory.empty()) {
		log_warn("Aucun emplacement de données configuré, sauvegarde de secours impossible.");
		return false;
	}
	std::error_code error;
	create_directories(directory, error);
	if (!is_directory(directory, error)) {
		log_warn("L'emplacement de données '{}' n'est pas un dossier.", directory.string());
		return false;
	}
	return writeFileAtomically(
			directory / g_rescueFileName, [&iEvent](std::ostream& oStream) -> void { iEvent.write(oStream); }, true);
}

auto loadRescue(const std::filesystem::path& iPath, Event& oEvent) -> bool {
	std::ifstream file(iPath, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		log_warn("Impossible d'ouvrir '{}'.", iPath.string());
		return false;
	}
	Event candidate;
	candidate.setBasePath(iPath);
	candidate.read(file, {});
	if (!file.good()) {
		log_warn("Le fichier de secours '{}' est incomplet ou corrompu.", iPath.string());
		return false;
	}
	oEvent = candidate;
	return true;
}

auto findRescue() -> std::optional<RescueInfo> {
	std::error_code error;
	for (const auto& candidate: rescueCandidates()) {
		if (!exists(candidate, error))
			continue;
		Event event;
		if (!loadRescue(candidate, event))
			continue;
		if (!isResumable(event)) {
			log_debug("Le fichier de secours '{}' ne contient pas de partie en cours.", candidate.string());
			continue;
		}
		return RescueInfo{.path = candidate,
						  .ageSeconds = ageInSeconds(candidate),
						  .eventName = event.getName(),
						  .drawCount = countDraws(event)};
	}
	return std::nullopt;
}

void archiveRescue() {
	std::error_code error;
	const auto stamp = std::format("{:%Y%m%d-%H%M%S}", std::chrono::floor<std::chrono::seconds>(clock::now()));
	for (const auto& candidate: rescueCandidates()) {
		if (!exists(candidate, error))
			continue;
		auto archived = candidate;
		archived.replace_filename(std::format("rescue-{}-{}", stamp, candidate.filename().string()));
		rename(candidate, archived, error);
		if (error)
			log_warn("Impossible d'archiver '{}' : {}", candidate.string(), error.message());
		else
			log_info("Fichier de secours archivé sous '{}'.", archived.string());
	}
}

}// namespace evl::core
