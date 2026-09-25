/**
 * @file test_fileDialog.cpp
 * @author Silmaen
 * @date 23/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "../TestMainHelper.h"
#include "core/Settings.h"
#include "gui/Application.h"
#include "gui/utils/FileDialog.h"

#include <imgui.h>

#include <algorithm>
#include <format>
#include <fstream>
#include <string>
#include <vector>

using evl::gui::utils::FileDialog;

namespace {

/// A directory of known content, removed whatever the test does.
class TempTree {
public:
	TempTree() {
		m_root = fs::temp_directory_path() /
				 std::format("evl_filedialog_{}", ::testing::UnitTest::GetInstance()->current_test_info()->name());
		remove_all(m_root);
		create_directories(m_root / "Beta");
		create_directories(m_root / "alpha");
		create_directories(m_root / ".git");
		for (const auto* name: {"partie.lev", "Autre.LEV", "notes.txt", ".cache", "image.png"})
			std::ofstream{m_root / name} << "x";
	}
	~TempTree() {
		std::error_code error;
		remove_all(m_root, error);
	}

	TempTree(const TempTree&) = delete;
	TempTree(TempTree&&) = delete;
	auto operator=(const TempTree&) -> TempTree& = delete;
	auto operator=(TempTree&&) -> TempTree& = delete;

	[[nodiscard]] auto root() const -> const fs::path& { return m_root; }

private:
	fs::path m_root;
};

/// The names a listing returns, in order.
auto namesOf(const std::vector<FileDialog::Entry>& iEntries) -> std::vector<std::string> {
	std::vector<std::string> names;
	names.reserve(iEntries.size());
	for (const auto& entry: iEntries) names.push_back(entry.name);
	return names;
}

}// namespace

TEST(utils_FileDialog, ParsesTheShippedFilters) {
	const auto game = FileDialog::parseFilters(evl::gui::utils::g_gameFilter);
	ASSERT_EQ(game.size(), 1U);
	EXPECT_EQ(game[0].name, "Loto Files");
	EXPECT_EQ(game[0].extensions, std::vector<std::string>{"lev"});

	const auto yaml = FileDialog::parseFilters(evl::gui::utils::g_yamlFilter);
	ASSERT_EQ(yaml.size(), 1U);
	EXPECT_EQ(yaml[0].extensions, (std::vector<std::string>{"yaml", "yml"}));

	// Seven lines, the first of which gathers every extension of the six others.
	const auto images = FileDialog::parseFilters(evl::gui::utils::g_imageFilter);
	ASSERT_EQ(images.size(), 7U);
	EXPECT_EQ(images[0].extensions.size(), 7U);
	EXPECT_EQ(images[2].name, "JPG Files");
	EXPECT_EQ(images[2].extensions, (std::vector<std::string>{"jpg", "jpeg"}));
}

TEST(utils_FileDialog, DropsWhatItCannotRead) {
	// A line without a separator, without a name, without an extension, or empty.
	EXPECT_TRUE(FileDialog::parseFilters("").empty());
	EXPECT_TRUE(FileDialog::parseFilters("no separator").empty());
	EXPECT_TRUE(FileDialog::parseFilters("|lev").empty());
	EXPECT_TRUE(FileDialog::parseFilters("Name|").empty());
	EXPECT_TRUE(FileDialog::parseFilters("\n\n").empty());
	// A usable line still comes through when it sits next to a broken one.
	const auto mixed = FileDialog::parseFilters("broken\nGood|lev");
	ASSERT_EQ(mixed.size(), 1U);
	EXPECT_EQ(mixed[0].name, "Good");
}

TEST(utils_FileDialog, AcceptsBothExtensionSpellings) {
	// The three spellings a filter may use, all reduced to the bare extension.
	for (const auto* spec: {"Name|lev", "Name|*.lev", "Name|.lev"}) {
		const auto filters = FileDialog::parseFilters(spec);
		ASSERT_EQ(filters.size(), 1U) << spec;
		EXPECT_EQ(filters[0].extensions, std::vector<std::string>{"lev"}) << spec;
	}
}

TEST(utils_FileDialog, ReadsACatchAllFilter) {
	// The two spellings of "everything", which an empty extension list carries.
	for (const auto* spec: {"Tous|*", "Tous|*.*"}) {
		const auto filters = FileDialog::parseFilters(spec);
		ASSERT_EQ(filters.size(), 1U) << spec;
		EXPECT_EQ(filters[0].name, "Tous") << spec;
		EXPECT_TRUE(filters[0].extensions.empty()) << spec;
		EXPECT_TRUE(FileDialog::matches("n_importe.quoi", filters[0])) << spec;
	}
	// A catch-all among extensions swallows them: listing everything and listing `lev`
	// at the same time means listing everything.
	const auto mixed = FileDialog::parseFilters("Tous|lev,*");
	ASSERT_EQ(mixed.size(), 1U);
	EXPECT_TRUE(mixed[0].extensions.empty());
}

TEST(utils_FileDialog, MatchesRegardlessOfCase) {
	const auto filters = FileDialog::parseFilters("Loto|lev");
	ASSERT_EQ(filters.size(), 1U);
	EXPECT_TRUE(FileDialog::matches("partie.lev", filters[0]));
	// A file saved on Windows comes back shouting, and is still a game file.
	EXPECT_TRUE(FileDialog::matches("PARTIE.LEV", filters[0]));
	EXPECT_FALSE(FileDialog::matches("partie.txt", filters[0]));
	EXPECT_FALSE(FileDialog::matches("partie", filters[0]));
	// A filter with no extension is the one that accepts everything.
	EXPECT_TRUE(FileDialog::matches("partie.txt", FileDialog::Filter{}));
}

TEST(utils_FileDialog, TellsTheDeclaredFormatsFromTheRest) {
	// What a request for images carries: the seven declared lines, and the catch-all
	// that lists everything else.
	auto filters = FileDialog::parseFilters(evl::gui::utils::g_imageFilter);
	filters.push_back(FileDialog::Filter{.name = std::string{evl::gui::utils::g_allFilesLabel}, .extensions = {}});

	EXPECT_TRUE(FileDialog::isDeclared(filters, "logo.png"));
	EXPECT_TRUE(FileDialog::isDeclared(filters, "LOGO.JPEG"));
	EXPECT_TRUE(FileDialog::isDeclared(filters, "dessin.svg"));
	// Listed by the catch-all, and shown greyed: probably not an image, but the
	// organizer may know better than the extension does.
	EXPECT_FALSE(FileDialog::isDeclared(filters, "partie.lev"));
	EXPECT_FALSE(FileDialog::isDeclared(filters, "photo.heic"));
	EXPECT_FALSE(FileDialog::isDeclared(filters, "sans_extension"));

	// The catch-all on its own declares nothing, so nothing is outside it: a folder
	// request, or a browser opened with no filter at all, greys no row.
	const std::vector<FileDialog::Filter> onlyCatchAll{FileDialog::Filter{.name = "Tous", .extensions = {}}};
	EXPECT_TRUE(FileDialog::isDeclared(onlyCatchAll, "partie.lev"));
	EXPECT_TRUE(FileDialog::isDeclared({}, "partie.lev"));
}

TEST(utils_FileDialog, ListsDirectoriesFirstThenNames) {
	const TempTree tree;
	const auto entries = FileDialog::listEntries(tree.root(), nullptr, false);
	// Directories first, then files, each set ordered without regard to case — so
	// `alpha` precedes `Beta`, which an ASCII sort would have reversed.
	EXPECT_EQ(namesOf(entries),
			  (std::vector<std::string>{"alpha", "Beta", "Autre.LEV", "image.png", "notes.txt", "partie.lev"}));
	EXPECT_TRUE(entries[0].isDirectory);
	EXPECT_FALSE(entries[2].isDirectory);
	EXPECT_EQ(entries[0].path, tree.root() / "alpha");
}

TEST(utils_FileDialog, HidesDottedNamesUntilAsked) {
	const TempTree tree;
	const auto hidden = namesOf(FileDialog::listEntries(tree.root(), nullptr, false));
	EXPECT_EQ(std::ranges::find(hidden, ".git"), hidden.end());
	EXPECT_EQ(std::ranges::find(hidden, ".cache"), hidden.end());

	const auto shown = namesOf(FileDialog::listEntries(tree.root(), nullptr, true));
	EXPECT_NE(std::ranges::find(shown, ".git"), shown.end());
	EXPECT_NE(std::ranges::find(shown, ".cache"), shown.end());
}

TEST(utils_FileDialog, FiltersFilesButNeverDirectories) {
	const TempTree tree;
	const auto filters = FileDialog::parseFilters("Loto|lev");
	ASSERT_EQ(filters.size(), 1U);
	const auto entries = FileDialog::listEntries(tree.root(), &filters[0], false);
	// Both game files, and the two directories, which a filter must not hide: they are
	// the way to the files it is looking for.
	EXPECT_EQ(namesOf(entries), (std::vector<std::string>{"alpha", "Beta", "Autre.LEV", "partie.lev"}));
}

TEST(utils_FileDialog, ComesBackEmptyOnAnUnreadableDirectory) {
	// Nothing to list, and above all nothing thrown in the middle of a frame.
	EXPECT_TRUE(FileDialog::listEntries("/no/such/directory/anywhere", nullptr, false).empty());
	EXPECT_TRUE(FileDialog::listEntries({}, nullptr, false).empty());
	// A file is not a directory.
	const TempTree tree;
	EXPECT_TRUE(FileDialog::listEntries(tree.root() / "notes.txt", nullptr, false).empty());
}

TEST(utils_FileDialog, ResolvesTheTypedName) {
	const auto filters = FileDialog::parseFilters("Loto|lev");
	ASSERT_EQ(filters.size(), 1U);
	const fs::path directory{"/tmp/evl"};

	// A bare name lands in the directory being browsed, and gains the extension of the
	// active filter: nobody should have to type `.lev`.
	EXPECT_EQ(FileDialog::resolveTarget(directory, "partie", &filters[0]), fs::path{"/tmp/evl/partie.lev"});
	// One already there is left alone, even a different one.
	EXPECT_EQ(FileDialog::resolveTarget(directory, "partie.lev", &filters[0]), fs::path{"/tmp/evl/partie.lev"});
	EXPECT_EQ(FileDialog::resolveTarget(directory, "partie.bak", &filters[0]), fs::path{"/tmp/evl/partie.bak"});
	// No filter, no extension added.
	EXPECT_EQ(FileDialog::resolveTarget(directory, "partie", nullptr), fs::path{"/tmp/evl/partie"});
	// An absolute name ignores the directory entirely.
	EXPECT_EQ(FileDialog::resolveTarget(directory, "/ailleurs/partie.lev", &filters[0]),
			  fs::path{"/ailleurs/partie.lev"});
	// Nothing typed, nothing to confirm.
	EXPECT_TRUE(FileDialog::resolveTarget(directory, "", &filters[0]).empty());
}

TEST(utils_FileDialog, StartsSomewhereThatExists) {
	// Whatever the settings hold, browsing must not open on a directory that is not
	// there: there would be no way back to a real one but by typing a path.
	EXPECT_TRUE(is_directory(FileDialog::startDirectory()));
}

TEST(utils_FileDialog, CutsThePathIntoClickableSteps) {
	const auto crumbs = FileDialog::breadcrumb("/data/sources/personnel/EvenementLoto");
	ASSERT_EQ(crumbs.size(), 5U);
	// The root counts as a step, so there is always somewhere to go back to. Its label
	// is spelled with the platform's own separator — `/` here, `\` on Windows — so the
	// expectation comes from the library rather than from a literal.
	EXPECT_EQ(crumbs[0].label, fs::path{"/data/sources"}.lexically_normal().root_path().string());
	EXPECT_EQ(crumbs[0].path, fs::path{"/"});
	EXPECT_EQ(crumbs[2].label, "sources");
	// Each step carries the whole path down to it: clicking the third of five climbs
	// two levels at once, which is the point of a path bar over a Parent button.
	EXPECT_EQ(crumbs[2].path, fs::path{"/data/sources"});
	EXPECT_EQ(crumbs[4].path, fs::path{"/data/sources/personnel/EvenementLoto"});
}

TEST(utils_FileDialog, CutsTheEdgeCasesToo) {
	const auto root = FileDialog::breadcrumb("/");
	ASSERT_EQ(root.size(), 1U);
	EXPECT_EQ(root[0].path, fs::path{"/"});
	EXPECT_TRUE(FileDialog::breadcrumb({}).empty());
	// A trailing separator must not add an empty step at the end.
	EXPECT_EQ(FileDialog::breadcrumb("/data/").size(), 2U);
	// A relative directory has no root, and still gets a step.
	const auto relative = FileDialog::breadcrumb("data");
	ASSERT_EQ(relative.size(), 1U);
	EXPECT_EQ(relative[0].label, "data");
}

TEST(utils_FileDialog, ReadsATypedTextAsAPlaceAndAPrefix) {
	const fs::path base{"/data/sources"};

	// No separator: a few letters narrow the directory being browsed.
	const auto plain = FileDialog::splitQuery(base, "per");
	EXPECT_EQ(plain.directory, base);
	EXPECT_EQ(plain.prefix, "per");
	EXPECT_TRUE(plain.head.empty());

	// Nothing typed: the whole directory, nothing to keep out.
	const auto empty = FileDialog::splitQuery(base, "");
	EXPECT_EQ(empty.directory, base);
	EXPECT_TRUE(empty.prefix.empty());

	// An absolute path looks where it says, whatever is being browsed.
	const auto absolute = FileDialog::splitQuery(base, "/etc/ho");
	EXPECT_EQ(absolute.directory, fs::path{"/etc"});
	EXPECT_EQ(absolute.prefix, "ho");
	// The head is what completion has to put back in front of the name.
	EXPECT_EQ(absolute.head, "/etc/");

	// A relative one is read from the directory being browsed.
	const auto relative = FileDialog::splitQuery(base, "personnel/Even");
	EXPECT_EQ(relative.directory, fs::path{"/data/sources/personnel"});
	EXPECT_EQ(relative.prefix, "Even");
	EXPECT_EQ(relative.head, "personnel/");

	// A path ending on a separator is a whole directory, with nothing to keep out.
	const auto directory = FileDialog::splitQuery(base, "/data/");
	EXPECT_EQ(directory.directory, fs::path{"/data"});
	EXPECT_TRUE(directory.prefix.empty());

	// The root alone.
	const auto root = FileDialog::splitQuery(base, "/da");
	EXPECT_EQ(root.directory, fs::path{"/"});
	EXPECT_EQ(root.prefix, "da");
	EXPECT_EQ(root.head, "/");

	// `..` is resolved, so climbing back does not pile up.
	const auto up = FileDialog::splitQuery(base, "../per");
	EXPECT_EQ(up.directory, fs::path{"/data"});
	EXPECT_EQ(up.prefix, "per");
}

TEST(utils_FileDialog, ReadsABackslashPathToo) {
	// A path pasted from a Windows machine is understood on every platform: reading it
	// costs nothing, and refusing it would look like a bug.
	const auto typed = FileDialog::splitQuery("/data", "C:\\Users\\loto\\par");
	EXPECT_EQ(typed.prefix, "par");
	EXPECT_EQ(typed.head, "C:\\Users\\loto\\");
}

TEST(utils_FileDialog, TellsAParentFromANeighbour) {
	const fs::path branch{"/data/sources"};
	// Itself, and everything below it: those are the branches the tree unfolds.
	EXPECT_TRUE(FileDialog::isAncestorOf(branch, branch));
	EXPECT_TRUE(FileDialog::isAncestorOf(branch, "/data/sources/personnel"));
	EXPECT_TRUE(FileDialog::isAncestorOf(branch, "/data/sources/personnel/EvenementLoto"));
	EXPECT_TRUE(FileDialog::isAncestorOf("/", branch));

	// The neighbour whose name merely begins the same way. A text comparison answered
	// yes here, and the tree unfolded a branch nobody had asked for.
	EXPECT_FALSE(FileDialog::isAncestorOf(branch, "/data/sources2"));
	EXPECT_FALSE(FileDialog::isAncestorOf(branch, "/data/sources2/ailleurs"));
	// And the other way round: a child is not its parent's ancestor.
	EXPECT_FALSE(FileDialog::isAncestorOf("/data/sources/personnel", branch));
	EXPECT_FALSE(FileDialog::isAncestorOf(branch, "/etc"));

	// A trailing separator is the same directory, however it is spelled.
	EXPECT_TRUE(FileDialog::isAncestorOf("/data/sources/", branch));
	EXPECT_TRUE(FileDialog::isAncestorOf(branch, "/data/sources/"));
	// And `..` is resolved before comparing.
	EXPECT_TRUE(FileDialog::isAncestorOf(branch, "/data/sources/personnel/../autre"));
}

TEST(utils_FileDialog, MatchesThePrefixRegardlessOfCase) {
	EXPECT_TRUE(FileDialog::startsWithIgnoringCase("partie.lev", "par"));
	EXPECT_TRUE(FileDialog::startsWithIgnoringCase("Partie.lev", "par"));
	EXPECT_TRUE(FileDialog::startsWithIgnoringCase("partie.lev", "PAR"));
	// Everything matches nothing typed yet, which is how the list starts.
	EXPECT_TRUE(FileDialog::startsWithIgnoringCase("partie.lev", ""));
	EXPECT_FALSE(FileDialog::startsWithIgnoringCase("partie.lev", "art"));
	// More typed than the name is long.
	EXPECT_FALSE(FileDialog::startsWithIgnoringCase("par", "partie"));
}

TEST(utils_FileDialog, CompletesUpToWhatTheNamesShare) {
	const std::vector<FileDialog::Entry> entries{
			{.path = "/a/partie-un.lev", .name = "partie-un.lev", .isDirectory = false},
			{.path = "/a/partie-deux.lev", .name = "partie-deux.lev", .isDirectory = false}};
	EXPECT_EQ(FileDialog::commonPrefix(entries), "partie-");

	// A single entry completes entirely: one press of Tab and the name is typed.
	EXPECT_EQ(FileDialog::commonPrefix({entries[0]}), "partie-un.lev");
	// Nothing in common, nothing to add.
	const std::vector<FileDialog::Entry> apart{{.path = "/a/alpha", .name = "alpha", .isDirectory = true},
											   {.path = "/a/beta", .name = "beta", .isDirectory = true}};
	EXPECT_TRUE(FileDialog::commonPrefix(apart).empty());
	EXPECT_TRUE(FileDialog::commonPrefix({}).empty());

	// Case is ignored to compare, and the first name's case is what comes back.
	const std::vector<FileDialog::Entry> mixed{
			{.path = "/a/Partie-un.lev", .name = "Partie-un.lev", .isDirectory = false},
			{.path = "/a/partie-deux.lev", .name = "partie-deux.lev", .isDirectory = false}};
	EXPECT_EQ(FileDialog::commonPrefix(mixed), "Partie-");
}

TEST(utils_FileDialog, KeepsFiveRecentPathsMostRecentFirst) {
	const auto settings = evl::core::getSettings();
	const auto saved = settings->getValue<std::string>(std::string{evl::gui::utils::g_recentPathsKey}, "");
	settings->setValue(std::string{evl::gui::utils::g_recentPathsKey}, std::string{});

	const TempTree tree;
	// Seven real paths offered, five kept, the newest at the head.
	std::vector<fs::path> pushed;
	for (const auto* name: {"alpha", "Beta", "partie.lev", "Autre.LEV", "notes.txt", "image.png"})
		pushed.push_back(tree.root() / name);
	pushed.push_back(tree.root());
	for (const auto& path: pushed) FileDialog::rememberPath(path);

	auto recents = FileDialog::recentPaths();
	ASSERT_EQ(recents.size(), 5U);
	EXPECT_EQ(recents.front(), pushed.back());

	// Asking again for one already there moves it back to the head instead of
	// appearing twice.
	FileDialog::rememberPath(pushed[2]);
	recents = FileDialog::recentPaths();
	EXPECT_EQ(recents.size(), 5U);
	EXPECT_EQ(recents.front(), pushed[2]);
	EXPECT_EQ(std::ranges::count(recents, pushed[2]), 1);

	// An empty path is not a shortcut.
	FileDialog::rememberPath({});
	EXPECT_EQ(FileDialog::recentPaths().size(), 5U);

	settings->setValue(std::string{evl::gui::utils::g_recentPathsKey}, saved);
}

TEST(utils_FileDialog, ForgetsARecentPathThatIsGone) {
	const auto settings = evl::core::getSettings();
	const auto saved = settings->getValue<std::string>(std::string{evl::gui::utils::g_recentPathsKey}, "");

	fs::path vanished;
	{
		const TempTree tree;
		vanished = tree.root() / "partie.lev";
		settings->setValue(std::string{evl::gui::utils::g_recentPathsKey}, std::string{});
		FileDialog::rememberPath(vanished);
		ASSERT_EQ(FileDialog::recentPaths().size(), 1U);
	}
	// The tree is gone: a shortcut to a file that has been moved or deleted is worse
	// than no shortcut, so it is dropped rather than offered.
	EXPECT_FALSE(exists(vanished));
	EXPECT_TRUE(FileDialog::recentPaths().empty());

	settings->setValue(std::string{evl::gui::utils::g_recentPathsKey}, saved);
}

TEST(utils_FileDialog, HoldsOneRequestAtATime) {
	ASSERT_FALSE(FileDialog::hasRequest());
	bool called = false;
	FileDialog::openFile(evl::gui::utils::g_gameFilter, [&called](const fs::path&) -> void { called = true; });
	EXPECT_TRUE(FileDialog::hasRequest());

	// A second request cannot be drawn on top of the first: it is refused, and the
	// first one is the one that stays.
	bool second = false;
	FileDialog::saveFile(evl::gui::utils::g_yamlFilter, [&second](const fs::path&) -> void { second = true; });
	EXPECT_TRUE(FileDialog::hasRequest());

	// Cancelling answers nobody — a continuation is for a path that was chosen.
	FileDialog::cancel();
	EXPECT_FALSE(FileDialog::hasRequest());
	EXPECT_FALSE(called);
	EXPECT_FALSE(second);
}

TEST(utils_FileDialog, TheOwnerScopeNestsAndRestores) {
	// Two owners on the stack: leaving the inner one gives the browser back to the
	// outer, which is what keeps a request drawn inside the popup that asked for it.
	const int outer = 0;
	const int inner = 0;
	{
		const FileDialog::OwnerScope outerScope{&outer};
		{
			const FileDialog::OwnerScope innerScope{&inner};
			FileDialog::openFile(evl::gui::utils::g_gameFilter, [](const fs::path&) -> void {});
		}
		// Recorded against the inner owner, so the outer one must not draw it.
		EXPECT_TRUE(FileDialog::hasRequest());
		FileDialog::draw(&outer);
		EXPECT_TRUE(FileDialog::hasRequest());
		FileDialog::draw(nullptr);
		EXPECT_TRUE(FileDialog::hasRequest());
	}
	FileDialog::cancel();
	EXPECT_FALSE(FileDialog::hasRequest());
}

namespace {

/// A view that posts one request on its first frame, and nothing after.
class RequestingView final : public evl::gui::views::View {
public:
	RequestingView(const evl::gui::utils::FileDialogMode iMode, const std::string_view iFilter)
		: m_mode{iMode}, m_filter{iFilter} {}

	void onUpdate() override {
		if (m_posted)
			return;
		m_posted = true;
		switch (m_mode) {
			case evl::gui::utils::FileDialogMode::Open:
				FileDialog::openFile(m_filter, [this](const fs::path& iPath) -> void { m_chosen = iPath; });
				break;
			case evl::gui::utils::FileDialogMode::Save:
				FileDialog::saveFile(m_filter, [this](const fs::path& iPath) -> void { m_chosen = iPath; });
				break;
			case evl::gui::utils::FileDialogMode::Folder:
				FileDialog::selectFolder([this](const fs::path& iPath) -> void { m_chosen = iPath; });
				break;
		}
	}

	[[nodiscard]] auto getName() const -> std::string override { return "requesting_view"; }
	[[nodiscard]] auto chosen() const -> const fs::path& { return m_chosen; }

private:
	evl::gui::utils::FileDialogMode m_mode;
	std::string m_filter;
	bool m_posted = false;
	fs::path m_chosen;
};

/// Run a few frames with the browser open, in each of the three modes.
void drawsWithoutBreakingTheFrame(const evl::gui::utils::FileDialogMode iMode, const std::string_view iFilter) {
	// One recent path, so the left pane really draws its shortcuts: an empty list would
	// leave that half of the window untested.
	const auto settings = evl::core::getSettings();
	const auto saved = settings->getValue<std::string>(std::string{evl::gui::utils::g_recentPathsKey}, "");
	FileDialog::rememberPath(evl::core::getExecPath());

	const auto app = evl::gui::createApplication(0, nullptr);
	ASSERT_NE(app, nullptr);
	const auto view = std::make_shared<RequestingView>(iMode, iFilter);
	app->addView(view);
	app->setMaxFrame(4);
	app->run();
	// The window really was drawn, over several frames, and the loop came out of it
	// cleanly: an unbalanced Begin/End inside the browser would have ended in Error.
	EXPECT_EQ(app->getState(), evl::gui::Application::State::Closed);
	// Nobody clicked anything, so nothing was chosen and the continuation never ran.
	EXPECT_TRUE(view->chosen().empty());
	FileDialog::cancel();
	settings->setValue(std::string{evl::gui::utils::g_recentPathsKey}, saved);
}

}// namespace

TEST(utils_FileDialog, DrawsAnOpenRequest) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	drawsWithoutBreakingTheFrame(evl::gui::utils::FileDialogMode::Open, evl::gui::utils::g_imageFilter);
}

TEST(utils_FileDialog, DrawsASaveRequest) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	// Save adds the name field and the overwrite question to what Open draws.
	drawsWithoutBreakingTheFrame(evl::gui::utils::FileDialogMode::Save, evl::gui::utils::g_gameFilter);
}

TEST(utils_FileDialog, DrawsAFolderRequest) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	// Folder drops the filter row entirely.
	drawsWithoutBreakingTheFrame(evl::gui::utils::FileDialogMode::Folder, {});
}

namespace {

/// A popup that opens itself, then asks for a file from inside its own modal.
class RequestingPopup final : public evl::gui::views::Popup {
public:
	RequestingPopup() { open(); }

	void onPopupUpdate() override {
		ImGui::TextUnformatted("test");
		if (m_posted)
			return;
		m_posted = true;
		FileDialog::openFile(evl::gui::utils::g_gameFilter,
							 [this](const fs::path& iPath) -> void { m_chosen = iPath; });
	}

	[[nodiscard]] auto getName() const -> std::string override { return "requesting_popup"; }
	[[nodiscard]] auto getPopupTitle() const -> std::string override { return "Popup de test"; }
	[[nodiscard]] auto chosen() const -> const fs::path& { return m_chosen; }

private:
	bool m_posted = false;
	fs::path m_chosen;
};

}// namespace

TEST(utils_FileDialog, DrawsFromInsideAPopupWithoutDismissingIt) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	const auto app = evl::gui::createApplication(0, nullptr);
	ASSERT_NE(app, nullptr);
	const auto popup = std::make_shared<RequestingPopup>();
	app->addPopup(popup);
	app->setMaxFrame(5);
	app->run();
	// The two modals really were stacked over several frames. Drawing the browser at
	// the root level instead would have dismissed the popup underneath, and an
	// unbalanced Begin/End between the two would have ended in Error.
	EXPECT_EQ(app->getState(), evl::gui::Application::State::Closed);
	EXPECT_TRUE(popup->chosen().empty());
	FileDialog::cancel();
}
