/**
* @author Silmaen
* @date 21/09/2026
* Copyright © 2026 All rights reserved.
* All modification must get authorization from the author.
*/
#include "../TestMainHelper.h"

#include "core/Event.h"
#include "core/FileFormat.h"
#include "core/StreamRead.h"

#include <array>
#include <cstring>
#include <fstream>
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

/// Offset of the first byte of the body in a framed buffer.
constexpr std::size_t g_bodyOffset = 4 + sizeof(uint16_t);

/// Re-sign a buffer whose body was altered on purpose.
///
/// Without this, every patched byte would be caught by the checksum and the tests below
/// would all pass for that one reason, never exercising the check they are named after.
void reSign(std::string& ioBuffer) {
	const auto payload = std::string_view{ioBuffer}.substr(0, ioBuffer.size() - sizeof(uint32_t));
	const auto sum = crc32(payload);
	std::memcpy(ioBuffer.data() + ioBuffer.size() - sizeof(uint32_t), &sum, sizeof(sum));
}

/// Read a buffer and tell whether the reader considered it complete.
auto tryRead(const std::string& iBuffer) -> bool {
	std::istringstream stream(iBuffer, std::ios::in | std::ios::binary);
	Event event;
	event.read(stream, {});
	return stream.good();
}

}// namespace

TEST(Serialization, RoundTrip) {
	const auto original = makeEvent();
	const auto buffer = serialize(original);
	ASSERT_FALSE(buffer.empty());

	std::istringstream stream(buffer, std::ios::in | std::ios::binary);
	Event restored;
	restored.read(stream, {});
	EXPECT_TRUE(stream.good());
	EXPECT_EQ(restored.getName(), original.getName());
	EXPECT_EQ(restored.getOrganizerName(), original.getOrganizerName());
	EXPECT_EQ(restored.getLocation(), original.getLocation());
	EXPECT_EQ(restored.getRules(), original.getRules());
	EXPECT_EQ(restored.sizeRounds(), original.sizeRounds());
}

TEST(Serialization, PreviousVersionStillReads) {
	// A version 6 file: no magic, no checksum, the version as the first field. The body
	// itself is unchanged — lengths were already eight bytes and dates already counted
	// nanoseconds on this platform, which is what makes the fixed widths a spelling out
	// rather than a migration.
	const auto framed = serialize(makeEvent());
	ASSERT_GT(framed.size(), g_bodyOffset + sizeof(uint32_t));
	const auto body = framed.substr(g_bodyOffset, framed.size() - g_bodyOffset - sizeof(uint32_t));
	constexpr uint16_t legacyVersion = 6;
	std::string legacy(sizeof(legacyVersion), '\0');
	std::memcpy(legacy.data(), &legacyVersion, sizeof(legacyVersion));
	legacy += body;

	std::istringstream stream(legacy, std::ios::in | std::ios::binary);
	Event restored;
	restored.read(stream, {});
	EXPECT_TRUE(stream.good());
	EXPECT_EQ(restored.getName(), makeEvent().getName());
	EXPECT_EQ(restored.sizeRounds(), makeEvent().sizeRounds());
}

/// What each file delivered in data/ is expected to contain.
struct ShippedFile {
	std::string_view file;///< name inside data/
	std::string_view name;///< event name
	std::string_view organizer;///< organizer name
	std::size_t rounds;///< number of game rounds
};

/// The four files, in versions 3, 4 and 6 — and one of the two version 6 files writes
/// its enumerations on four bytes while the other writes them on one.
constexpr std::array<ShippedFile, 4> g_shippedFiles{{
		{"loto_sou.lev", "Le loto du Sou des écoles", "Sou des écoles de Genay", 9},
		{"loto_sou_2.lev", "Le loto du Sou des écoles", "Sou des écoles de Genay", 11},
		{"super_loto.lev", "Bingo des Familles", "Le Comité des Cons", 5},
		{"test_sou.lev", "loto du sou", "Sou des écoles", 1},
}};

TEST(Serialization, ShippedFilesStillRead) {
	// The real thing, not a hand-made buffer. Checking the content and not merely that
	// the read reported success: a misparse can walk a file to its end and hand back
	// nonsense, which is exactly what reading an old event must never do.
	const fs::path dataDir{EVL_TEST_DATA_DIR};
	ASSERT_TRUE(is_directory(dataDir)) << dataDir.string();
	for (const auto& expected: g_shippedFiles) {
		const auto path = dataDir / expected.file;
		ASSERT_TRUE(exists(path)) << path.string();
		std::ifstream file(path, std::ios::in | std::ios::binary);
		ASSERT_TRUE(file.is_open()) << path.string();
		Event event;
		event.setBasePath(path);
		event.read(file, {});
		EXPECT_TRUE(file.good()) << expected.file;
		EXPECT_EQ(event.getName(), expected.name) << expected.file;
		EXPECT_EQ(event.getOrganizerName(), expected.organizer) << expected.file;
		EXPECT_EQ(event.sizeRounds(), expected.rounds) << expected.file;
	}
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

TEST(Serialization, WritingIsDeterministic) {
	// Same event, same bytes. This is what makes a reference file possible, and it is
	// also the cheapest way to notice that something uncontrolled — a clock, a hash
	// order, an uninitialised field — slipped into the format.
	EXPECT_EQ(serialize(makeEvent()), serialize(makeEvent()));
}

TEST(Serialization, MatchesTheReferenceFile) {
	// `test/lib_test/reference-v7.lev` was produced once by the GCC x64 build and is
	// checked in. Comparing against it is what makes the format verifiably portable:
	// every toolchain the CI runs — Clang, MinGW GCC, MinGW Clang, and an arm64 build
	// the day there is one — checks the very same bytes, with nothing to orchestrate.
	//
	// Should this fail after a deliberate format change, bump the save version and
	// regenerate the file, keeping the old one as a compatibility fixture.
	std::ifstream reference(EVL_TEST_REFERENCE_FILE, std::ios::in | std::ios::binary);
	ASSERT_TRUE(reference.is_open()) << EVL_TEST_REFERENCE_FILE;
	const std::string expected{std::istreambuf_iterator<char>(reference), std::istreambuf_iterator<char>()};
	EXPECT_EQ(serialize(makeEvent()), expected);

	// And it reads back, which is the other half of the promise.
	std::istringstream stream(expected, std::ios::in | std::ios::binary);
	Event restored;
	restored.read(stream, {});
	EXPECT_TRUE(stream.good());
	EXPECT_EQ(restored.getName(), makeEvent().getName());
}

TEST(Serialization, MagicIsPresent) {
	const auto buffer = serialize(makeEvent());
	ASSERT_GT(buffer.size(), g_bodyOffset);
	EXPECT_TRUE(buffer.starts_with(g_fileMagic));
	uint16_t version = 0;
	std::memcpy(&version, buffer.data() + g_fileMagic.size(), sizeof(version));
	EXPECT_EQ(version, getSaveVersion());
}

TEST(Serialization, ForeignFileIsRejectedOnItsMagic) {
	// Long enough to look plausible, and not a save file.
	EXPECT_FALSE(tryRead(std::string(512, 'x')));
}

TEST(Serialization, AlteredByteIsCaughtByTheChecksum) {
	auto buffer = serialize(makeEvent());
	ASSERT_GT(buffer.size(), g_bodyOffset + 1);
	// Not re-signed: this is exactly what the checksum is for.
	buffer[g_bodyOffset + 1] = static_cast<char>(buffer[g_bodyOffset + 1] ^ 0x01);
	EXPECT_FALSE(tryRead(buffer));
}

TEST(Serialization, FutureVersionIsRejected) {
	auto buffer = serialize(makeEvent());
	ASSERT_GT(buffer.size(), g_bodyOffset);
	const auto future = static_cast<uint16_t>(getSaveVersion() + 1U);
	std::memcpy(buffer.data() + g_fileMagic.size(), &future, sizeof(future));
	reSign(buffer);
	EXPECT_FALSE(tryRead(buffer));
}

TEST(Serialization, OutOfRangeStatusIsRejected) {
	auto buffer = serialize(makeEvent());
	ASSERT_GT(buffer.size(), g_bodyOffset);
	// The status opens the body and is not a declared enumerator here.
	buffer[g_bodyOffset] = static_cast<char>(0x7F);
	reSign(buffer);
	EXPECT_FALSE(tryRead(buffer));
}

TEST(Serialization, AbsurdLengthIsRejected) {
	auto buffer = serialize(makeEvent());
	constexpr auto lengthOffset = g_bodyOffset + sizeof(Event::Status);
	ASSERT_GT(buffer.size(), lengthOffset + sizeof(uint64_t));
	// First length of the body: the organizer name.
	const auto absurd = static_cast<uint64_t>(g_maxSerializedLength + 1U);
	std::memcpy(buffer.data() + lengthOffset, &absurd, sizeof(absurd));
	reSign(buffer);
	EXPECT_FALSE(tryRead(buffer));
}
