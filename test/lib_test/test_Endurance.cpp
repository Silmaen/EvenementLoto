/**
* @author Silmaen
* @date 23/09/2026
* Copyright © 2026 All rights reserved.
* All modification must get authorization from the author.
*/
#include "../TestMainHelper.h"

#include "core/RandomNumberGenerator.h"
#include "core/Rescue.h"

#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace evl::core;

namespace {

/// A temporary data location, wiped on destruction.
class EnduranceArea {
public:
	EnduranceArea() {
		m_path = fs::temp_directory_path() / "evl-endurance";
		remove_all(m_path);
		create_directories(m_path);
		// A string, which is what the YAML backend hands back after a reload. Storing a
		// path here made the fixture test a configuration that never occurs, and that is
		// how the rescue save could stop working entirely without a test noticing.
		getSettings()->setValue("general/data_location", m_path.string());
	}
	~EnduranceArea() { remove_all(m_path); }

	EnduranceArea(const EnduranceArea&) = delete;
	EnduranceArea(EnduranceArea&&) = delete;
	auto operator=(const EnduranceArea&) -> EnduranceArea& = delete;
	auto operator=(EnduranceArea&&) -> EnduranceArea& = delete;

private:
	fs::path m_path;
};

/// Resident set size in kibibytes, or 0 where it cannot be read.
auto residentKiB() -> std::size_t {
#if defined(__linux__)
	std::ifstream statm("/proc/self/statm");
	if (!statm.is_open())
		return 0;
	std::size_t total = 0;
	std::size_t resident = 0;
	statm >> total >> resident;
	return resident * static_cast<std::size_t>(sysconf(_SC_PAGESIZE)) / 1024U;
#else
	return 0;
#endif
}

/// Number of open file descriptors, or 0 where it cannot be counted.
auto openDescriptors() -> std::size_t {
#if defined(__linux__)
	std::error_code error;
	const fs::path fdDir{"/proc/self/fd"};
	if (!is_directory(fdDir, error))
		return 0;
	return static_cast<std::size_t>(std::distance(fs::directory_iterator{fdDir, error}, fs::directory_iterator{}));
#else
	return 0;
#endif
}

/// True when AddressSanitizer is instrumenting this build.
#if defined(__SANITIZE_ADDRESS__)
constexpr bool g_underAddressSanitizer = true;
#elif defined(__has_feature)
#if __has_feature(address_sanitizer)
constexpr bool g_underAddressSanitizer = true;
#else
constexpr bool g_underAddressSanitizer = false;
#endif
#else
constexpr bool g_underAddressSanitizer = false;
#endif

/// How long the run lasts, from EVL_ENDURANCE_SECONDS. Short by default so that an
/// accidental run costs seconds rather than an afternoon.
auto requestedSeconds() -> double {
	const char* const value = std::getenv("EVL_ENDURANCE_SECONDS");
	if (value == nullptr)
		return 10.0;
	try {
		return std::max(1.0, std::stod(value));
	} catch (const std::exception&) { return 10.0; }
}

/// An event in the state it has for a whole afternoon.
auto makeRunningEvent() -> Event {
	Event event;
	event.setName("Loto d'endurance");
	event.setOrganizerName("Amicale");
	event.setLocation("Salle des fêtes");
	for (int round = 0; round < 8; ++round) event.pushGameRound(GameRound{GameRound::Type::OneTwoQuineFullCard});
	event.nextState();
	return event;
}

}// namespace

/// Disabled on purpose: this is minutes to hours of work, not a unit test.
///
/// Run it with:
///   EVL_ENDURANCE_SECONDS=14400 evl_lib_test_unit_test
///       --gtest_also_run_disabled_tests --gtest_filter='*Endurance*'
///
/// What it reproduces is the afternoon that matters: a number drawn, the game saved
/// atomically, and the file read back — over and over, for hours. What it watches is
/// the resident memory and the file descriptors, because a leak of either is what turns
/// a working application into one that dies at 17:00.
TEST(Endurance, DISABLED_DrawAndSaveForHours) {
	const EnduranceArea area;
	auto event = makeRunningEvent();
	RandomNumberGenerator generator;

	const auto deadline = clock::now() + std::chrono::duration<double>(requestedSeconds());
	const auto baselineMemory = residentKiB();
	const auto baselineDescriptors = openDescriptors();
	std::size_t peakMemory = baselineMemory;
	std::size_t peakDescriptors = baselineDescriptors;
	std::size_t draws = 0;
	std::size_t saves = 0;
	std::size_t reloads = 0;

	while (clock::now() < deadline) {
		// A draw, then the save that must follow it: the gap between a number announced
		// and a number on disk is what S5 closed.
		if (const auto number = generator.pick(); number > 0) {
			auto round = event.getCurrentGameRound();
			if (round != event.endRounds()) {
				round->addPickedNumber(static_cast<uint8_t>(number));
				++draws;
			}
		} else {
			// The grid is exhausted: start the afternoon over, as a new round would.
			generator.resetPick();
			event = makeRunningEvent();
		}
		ASSERT_TRUE(saveRescue(event)) << "échec de sauvegarde après " << saves << " écritures";
		++saves;

		// Every so often, read it back: a file written but unreadable is worse than no
		// file, and that is exactly what the recovery path depends on.
		if (saves % 50 == 0) {
			Event restored;
			ASSERT_TRUE(loadRescue(fs::temp_directory_path() / "evl-endurance" / g_rescueFileName, restored))
					<< "relecture impossible après " << saves << " écritures";
			EXPECT_EQ(restored.getName(), event.getName());
			++reloads;
			peakMemory = std::max(peakMemory, residentKiB());
			peakDescriptors = std::max(peakDescriptors, openDescriptors());
		}
	}

	const auto finalMemory = residentKiB();
	const auto finalDescriptors = openDescriptors();
	std::cout << "### endurance : " << draws << " tirages, " << saves << " sauvegardes, " << reloads << " relectures\n"
			  << "### RSS : " << baselineMemory << " -> " << finalMemory << " KiB (crête " << peakMemory << ")\n"
			  << "### descripteurs : " << baselineDescriptors << " -> " << finalDescriptors << " (crête "
			  << peakDescriptors << ")" << std::endl;

	// Descriptors must come back to where they started: every file opened here is
	// closed, and a steady climb is the signature of the leak that kills a long run.
	EXPECT_LE(finalDescriptors, baselineDescriptors + 2);
	// Memory is allowed to settle above the baseline — allocators keep arenas — but not
	// to grow without bound. Twice the baseline plus a margin catches a real leak while
	// tolerating a plateau.
	//
	// Not under AddressSanitizer, whose quarantine holds on to freed blocks by design:
	// it climbed to 355 MiB over 400 000 cycles here while reporting no leak at all, and
	// LeakSanitizer stayed flat over the same run. Resident memory measured through
	// ASan's allocator says nothing about the application.
	if (baselineMemory > 0 && !g_underAddressSanitizer) {
		EXPECT_LT(finalMemory, baselineMemory * 2 + 32768) << "la mémoire résidente a doublé";
	}
}
