/**
 * @file AtomicFile.cpp
 * @author Silmaen
 * @date 21/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "AtomicFile.h"

#include "Log.h"

namespace evl::core {

namespace {

auto temporaryPath(const std::filesystem::path& iPath) -> std::filesystem::path {
	return std::filesystem::path{iPath}.concat(".tmp");
}

auto previousPath(const std::filesystem::path& iPath) -> std::filesystem::path {
	return std::filesystem::path{iPath}.concat(".1");
}

auto fillTemporary(const std::filesystem::path& iTemporary, const std::function<void(std::ostream&)>& iWriter) -> bool {
	std::ofstream file(iTemporary, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!file.is_open()) {
		log_error("Impossible d'ouvrir '{}' en écriture.", iTemporary.string());
		return false;
	}
	iWriter(file);
	file.flush();
	if (!file.good()) {
		log_error("Échec de l'écriture de '{}'.", iTemporary.string());
		return false;
	}
	return true;
}

auto writeChecked(const std::filesystem::path& iPath, const std::function<void(std::ostream&)>& iWriter,
				  const bool iKeepPrevious) -> bool {
	const auto temporary = temporaryPath(iPath);
	std::error_code error;
	if (!fillTemporary(temporary, iWriter)) {
		remove(temporary, error);
		return false;
	}
	if (iKeepPrevious && exists(iPath, error)) {
		// Best effort: losing the previous generation must not prevent the new save.
		rename(iPath, previousPath(iPath), error);
		if (error)
			log_warn("Impossible de conserver la sauvegarde précédente de '{}' : {}", iPath.string(), error.message());
	}
	rename(temporary, iPath, error);
	if (error) {
		log_error("Impossible de remplacer '{}' : {}", iPath.string(), error.message());
		remove(temporary, error);
		return false;
	}
	return true;
}

}// namespace

auto writeFileAtomically(const std::filesystem::path& iPath, const std::function<void(std::ostream&)>& iWriter,
						 const bool iKeepPrevious) noexcept -> bool {
	try {
		return writeChecked(iPath, iWriter, iKeepPrevious);
	} catch (...) {
		// The caller is often on a shutdown path: report the failure, never propagate.
		std::error_code error;
		remove(temporaryPath(iPath), error);
		return false;
	}
}

}// namespace evl::core
