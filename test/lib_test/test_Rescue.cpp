/**
* @author Silmaen
* @date 22/09/2026
* Copyright © 2026 All rights reserved.
* All modification must get authorization from the author.
*/
#include "../TestMainHelper.h"

#include "core/Rescue.h"

#include <fstream>

using namespace evl::core;

namespace {

/// A temporary data location, wiped on destruction.
class RescueArea {
public:
	RescueArea() {
		m_path = fs::temp_directory_path() / fs::path{std::format("evl-rescue-{}", ++s_counter)};
		remove_all(m_path);
		create_directories(m_path);
		getSettings()->setValue("general/data_location", m_path);
	}
	~RescueArea() { remove_all(m_path); }

	RescueArea(const RescueArea&) = delete;
	RescueArea(RescueArea&&) = delete;
	auto operator=(const RescueArea&) -> RescueArea& = delete;
	auto operator=(RescueArea&&) -> RescueArea& = delete;

	[[nodiscard]] auto path() const -> const fs::path& { return m_path; }

private:
	fs::path m_path;
	static int s_counter;
};

int RescueArea::s_counter = 0;

/// An event in a state worth resuming.
auto makeRunningEvent() -> Event {
	Event event;
	event.setName("Loto du dimanche");
	event.setOrganizerName("Amicale");
	event.setLocation("Salle des fêtes");
	event.pushGameRound(GameRound{GameRound::Type::OneTwoQuineFullCard});
	event.nextState();
	return event;
}

}// namespace

TEST(Rescue, SaveThenFindAndLoad) {
	const RescueArea area;
	const auto event = makeRunningEvent();
	ASSERT_TRUE(saveRescue(event));
	ASSERT_TRUE(exists(area.path() / g_rescueFileName));

	const auto found = findRescue();
	ASSERT_TRUE(found.has_value());
	const auto info = found.value_or(RescueInfo{});
	EXPECT_EQ(info.eventName, event.getName());
	EXPECT_EQ(info.path, area.path() / g_rescueFileName);

	Event restored;
	ASSERT_TRUE(loadRescue(info.path, restored));
	EXPECT_EQ(restored.getName(), event.getName());
	EXPECT_EQ(restored.getStatus(), event.getStatus());
}

TEST(Rescue, NothingToFindOnAnEmptyArea) {
	const RescueArea area;
	EXPECT_FALSE(findRescue().has_value());
}

TEST(Rescue, SecondSaveKeepsThePreviousGeneration) {
	const RescueArea area;
	ASSERT_TRUE(saveRescue(makeRunningEvent()));
	auto second = makeRunningEvent();
	second.setName("Deuxième");
	ASSERT_TRUE(saveRescue(second));

	const auto previous = fs::path{area.path() / g_rescueFileName}.concat(".1");
	ASSERT_TRUE(exists(previous));
	Event restored;
	ASSERT_TRUE(loadRescue(previous, restored));
	EXPECT_EQ(restored.getName(), "Loto du dimanche");
}

TEST(Rescue, CorruptedCurrentFallsBackToThePreviousGeneration) {
	const RescueArea area;
	ASSERT_TRUE(saveRescue(makeRunningEvent()));
	auto second = makeRunningEvent();
	second.setName("Deuxième");
	ASSERT_TRUE(saveRescue(second));

	// Truncate the newest generation, as a crash during the write would.
	const auto current = area.path() / g_rescueFileName;
	{
		std::ofstream truncated(current, std::ios::out | std::ios::binary | std::ios::trunc);
		truncated << "abc";
	}
	const auto found = findRescue();
	ASSERT_TRUE(found.has_value());
	const auto info = found.value_or(RescueInfo{});
	EXPECT_EQ(info.eventName, "Loto du dimanche");
	EXPECT_EQ(info.path, fs::path{current}.concat(".1"));
}

TEST(Rescue, FinishedEventIsNotProposed) {
	const RescueArea area;
	Event event;
	event.setName("Terminé");
	ASSERT_TRUE(saveRescue(event));// a default event is Invalid
	EXPECT_FALSE(findRescue().has_value());
}

TEST(Rescue, ArchiveKeepsTheFiles) {
	const RescueArea area;
	ASSERT_TRUE(saveRescue(makeRunningEvent()));
	archiveRescue();
	EXPECT_FALSE(exists(area.path() / g_rescueFileName));
	EXPECT_FALSE(findRescue().has_value());
	const auto archived = std::ranges::count_if(fs::directory_iterator{area.path()}, [](const auto& iEntry) -> bool {
		return iEntry.path().filename().string().starts_with("rescue-");
	});
	EXPECT_EQ(archived, 1);
}
