/**
* @author Silmaen
* @date 22/09/2026
* Copyright © 2026 All rights reserved.
* All modification must get authorization from the author.
*/
#include "../TestMainHelper.h"

#include "core/AtomicFile.h"

#include <fstream>

using namespace evl::core;

namespace {

/// A temporary directory, wiped on destruction.
class Area {
public:
	Area() {
		m_path = fs::temp_directory_path() / fs::path{std::format("evl-atomic-{}", ++s_counter)};
		remove_all(m_path);
		create_directories(m_path);
	}
	~Area() { remove_all(m_path); }

	Area(const Area&) = delete;
	Area(Area&&) = delete;
	auto operator=(const Area&) -> Area& = delete;
	auto operator=(Area&&) -> Area& = delete;

	[[nodiscard]] auto file(const std::string& iName) const -> fs::path { return m_path / iName; }

private:
	fs::path m_path;
	static int s_counter;
};

int Area::s_counter = 0;

auto readAll(const fs::path& iPath) -> std::string {
	std::ifstream file(iPath, std::ios::in | std::ios::binary);
	return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

auto writeText(const fs::path& iPath, const std::string& iContent) -> bool {
	return writeFileAtomically(iPath, [&iContent](std::ostream& oStream) -> void { oStream << iContent; });
}

}// namespace

TEST(AtomicFile, WritesTheContent) {
	const Area area;
	const auto target = area.file("data.bin");
	EXPECT_TRUE(writeText(target, "contenu"));
	EXPECT_EQ(readAll(target), "contenu");
	// The temporary must not be left behind.
	EXPECT_FALSE(exists(fs::path{target}.concat(".tmp")));
}

TEST(AtomicFile, AThrowingWriterLeavesThePreviousVersionIntact) {
	const Area area;
	const auto target = area.file("data.bin");
	ASSERT_TRUE(writeText(target, "version-1"));

	const auto written = writeFileAtomically(target, [](std::ostream& oStream) -> void {
		oStream << "moitie";
		throw std::runtime_error("écriture interrompue");
	});
	EXPECT_FALSE(written);
	EXPECT_EQ(readAll(target), "version-1");
	EXPECT_FALSE(exists(fs::path{target}.concat(".tmp")));
}

TEST(AtomicFile, AnUnwritableDestinationFailsWithoutCrashing) {
	const Area area;
	// A directory where the target name already exists as a directory: the temporary
	// cannot be created, and nothing must throw.
	const auto target = area.file("as-a-directory");
	create_directories(fs::path{target}.concat(".tmp"));
	EXPECT_FALSE(writeText(target, "contenu"));
	EXPECT_FALSE(exists(target));
}

TEST(AtomicFile, KeepsOnePreviousGeneration) {
	const Area area;
	const auto target = area.file("data.bin");
	const auto previous = fs::path{target}.concat(".1");

	EXPECT_TRUE(writeFileAtomically(target, [](std::ostream& oStream) -> void { oStream << "version-1"; }, true));
	EXPECT_FALSE(exists(previous));

	EXPECT_TRUE(writeFileAtomically(target, [](std::ostream& oStream) -> void { oStream << "version-2"; }, true));
	ASSERT_TRUE(exists(previous));
	EXPECT_EQ(readAll(target), "version-2");
	EXPECT_EQ(readAll(previous), "version-1");
}
