/**
* @author Silmaen
* @date 21/09/2026
* Copyright © 2026 All rights reserved.
* All modification must get authorization from the author.
*/
#include "../TestMainHelper.h"

#include "core/Event.h"
#include "core/StreamRead.h"

#include <cstring>
#include <random>
#include <sstream>

using namespace evl::core;

namespace {

/// An event with enough content to exercise every branch of the reader.
auto makeEvent() -> Event {
	Event event;
	event.setName("Loto de l'école");
	event.setOrganizerName("Amicale");
	event.setLocation("Salle des fêtes");
	event.setRules("Une ligne\nUne autre");
	const GameRound round{GameRound::Type::OneTwoQuineFullCard};
	event.pushGameRound(round);
	GameRound pause{GameRound::Type::Pause};
	pause.setDiapo("images", 5.0);
	event.pushGameRound(pause);
	return event;
}

auto serialize(const Event& iEvent) -> std::string {
	std::ostringstream stream(std::ios::out | std::ios::binary);
	iEvent.write(stream);
	return stream.str();
}

/// Read a buffer and tell whether the reader considered it complete.
auto tryRead(const std::string& iBuffer) -> bool {
	std::istringstream stream(iBuffer, std::ios::in | std::ios::binary);
	Event event;
	event.read(stream, 0);
	return stream.good();
}

}// namespace

TEST(Serialization, RoundTrip) {
	const auto original = makeEvent();
	const auto buffer = serialize(original);
	ASSERT_FALSE(buffer.empty());

	std::istringstream stream(buffer, std::ios::in | std::ios::binary);
	Event restored;
	restored.read(stream, 0);
	EXPECT_TRUE(stream.good());
	EXPECT_EQ(restored.getName(), original.getName());
	EXPECT_EQ(restored.getOrganizerName(), original.getOrganizerName());
	EXPECT_EQ(restored.getLocation(), original.getLocation());
	EXPECT_EQ(restored.getRules(), original.getRules());
	EXPECT_EQ(restored.sizeRounds(), original.sizeRounds());
}

TEST(Serialization, EmptyStreamIsRejected) { EXPECT_FALSE(tryRead("")); }

TEST(Serialization, TruncatedAtEveryOffsetIsRejected) {
	const auto buffer = serialize(makeEvent());
	ASSERT_GT(buffer.size(), 1U);
	// A partial file must never be accepted, and above all never crash the reader.
	for (std::size_t size = 0; size < buffer.size(); ++size) {
		EXPECT_FALSE(tryRead(buffer.substr(0, size))) << "accepté tronqué à " << size << " octets";
	}
}

TEST(Serialization, RandomBytesAreRejected) {
	// NOLINTNEXTLINE(bugprone-random-generator-seed,cert-msc32-c,cert-msc51-cpp): reproducible on purpose
	std::mt19937 generator(987654321U);
	std::uniform_int_distribution<int> byte(0, 255);
	for (std::size_t attempt = 0; attempt < 64U; ++attempt) {
		std::string buffer(128U, '\0');
		for (auto& character: buffer) character = static_cast<char>(byte(generator));
		EXPECT_FALSE(tryRead(buffer)) << "octets aléatoires acceptés à l'essai " << attempt;
	}
}

TEST(Serialization, FutureVersionIsRejected) {
	auto buffer = serialize(makeEvent());
	ASSERT_GE(buffer.size(), sizeof(uint16_t));
	const auto future = static_cast<uint16_t>(getSaveVersion() + 1U);
	std::memcpy(buffer.data(), &future, sizeof(future));
	EXPECT_FALSE(tryRead(buffer));
}

TEST(Serialization, OutOfRangeStatusIsRejected) {
	auto buffer = serialize(makeEvent());
	ASSERT_GT(buffer.size(), sizeof(uint16_t));
	// The status sits right after the version and is not a declared enumerator here.
	buffer[sizeof(uint16_t)] = static_cast<char>(0x7F);
	EXPECT_FALSE(tryRead(buffer));
}

TEST(Serialization, AbsurdLengthIsRejected) {
	auto buffer = serialize(makeEvent());
	constexpr auto statusSize = sizeof(Event::Status);
	constexpr auto lengthOffset = sizeof(uint16_t) + statusSize;
	ASSERT_GT(buffer.size(), lengthOffset + sizeof(std::string::size_type));
	// First length of the stream: the organizer name.
	const auto absurd = static_cast<std::string::size_type>(g_maxSerializedLength + 1U);
	std::memcpy(buffer.data() + lengthOffset, &absurd, sizeof(absurd));
	EXPECT_FALSE(tryRead(buffer));
}
