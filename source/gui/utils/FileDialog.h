/**
 * @file FileDialog.h
 * @author Silmaen
 * @date 20/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

namespace evl::gui::utils {

constexpr std::string_view g_gameFilter = "Loto Files|lev";
constexpr std::string_view g_imageFilter = "Image Files|png,jpg,jpeg,bmp,tga,gif,svg\n"
										   "PNG Files|png\n"
										   "JPG Files|jpg,jpeg\n"
										   "BMP Files|bmp\n"
										   "TGA Files|tga\n"
										   "GIF Files|gif\n"
										   "SVG Files|svg";
constexpr std::string_view g_yamlFilter = "YAML Files|yaml,yml";

/// The catch-all offered with every filter list, always last.
constexpr std::string_view g_allFilesLabel = "Tous les fichiers (*.*)";
/// How many recently used paths are offered. Five is a shortcut, not a history.
constexpr size_t g_maxRecentPaths = 5;
/// Where those paths are kept between sessions.
constexpr std::string_view g_recentPathsKey = "gui/recent_paths";

/**
 * @brief What the browser is asked to produce.
 */
enum struct FileDialogMode : uint8_t {
	Open,///< An existing file.
	Save,///< A file to create or to replace.
	Folder,///< A directory.
};

/**
 * @brief Called with the chosen path, never when the request is cancelled.
 *
 * It runs on the frame the organizer confirms, once the browser is closed and outside
 * its window scope — so it is free to open another popup in turn.
 */
using FileDialogCallback = std::function<void(const std::filesystem::path&)>;

/**
 * @brief The file browser, drawn in ImGui.
 *
 * Replaces the native dialogs: those were a system dependency (GTK, dbus) in an
 * application shipped as an archive, and a native modal has a habit of opening behind
 * the fullscreen display window, or stealing its focus.
 *
 * Being drawn in ImGui makes it **asynchronous**: it spreads over frames, so a request
 * cannot return the path it obtained. Each request carries a continuation instead, which
 * is what the caller used to write in the body of `if (!path.empty())`.
 */
class FileDialog {
public:
	/**
	 * @brief One line of a filter string: a label, and the extensions it accepts.
	 */
	struct Filter {
		/// What the combo box shows.
		std::string name;
		/// Lower case, without the dot. Empty accepts everything.
		std::vector<std::string> extensions;
	};

	/**
	 * @brief One step of the path bar: what it reads, and where it leads.
	 */
	struct Crumb {
		/// The directory name, or the root itself for the first one.
		std::string label;
		/// Where clicking it goes, which is several levels up at once.
		std::filesystem::path path;
	};

	/**
	 * @brief What a typed text points at: somewhere to list, and what to keep in it.
	 */
	struct Lookup {
		/// The directory to list. The one being browsed when no path was typed.
		std::filesystem::path directory;
		/// The beginning the names must have. Empty keeps the whole directory.
		std::string prefix;
		/// What precedes the prefix in the typed text, completion putting it back.
		std::string head;
	};

	/**
	 * @brief One line of the list.
	 */
	struct Entry {
		/// The full path.
		std::filesystem::path path;
		/// What the list shows.
		std::string name;
		/// Directories are sorted first, and are navigated into rather than chosen.
		bool isDirectory = false;
	};

	/**
	 * @brief Ask for an existing file.
	 * @param[in] iFilter The filter, as `"Name|ext1,ext2\nOther|ext"`.
	 * @param[in] iOnChosen What to do with the file.
	 */
	static void openFile(const std::string_view& iFilter, FileDialogCallback iOnChosen);

	/**
	 * @brief Ask for a file to write, existing or not.
	 * @param[in] iFilter The filter, as `"Name|ext1,ext2\nOther|ext"`.
	 * @param[in] iOnChosen What to do with the file. Replacing an existing one has
	 *            already been confirmed by then.
	 */
	static void saveFile(const std::string_view& iFilter, FileDialogCallback iOnChosen);

	/**
	 * @brief Ask for a directory.
	 * @param[in] iOnChosen What to do with the directory.
	 */
	static void selectFolder(FileDialogCallback iOnChosen);

	/**
	 * @brief Draw the browser, if the pending request belongs to @p iOwner.
	 *
	 * Called once per frame for each possible owner. The browser has to be drawn inside
	 * the window scope of whoever asked for it: a modal opened at the root level while
	 * a configuration popup is open would dismiss that popup, and the organizer would
	 * lose the page they were editing. Opened from inside it, the two stack.
	 *
	 * @param[in] iOwner Who is drawing, `nullptr` for the root level. Only ever
	 *            compared for identity.
	 */
	static void draw(const void* iOwner);

	/**
	 * @brief Marks who is drawing, so a request made now is drawn back in their scope.
	 */
	class OwnerScope final {
	public:
		/**
		 * @brief Takes ownership for the lifetime of the object.
		 * @param[in] iOwner Who is drawing.
		 */
		explicit OwnerScope(const void* iOwner);
		/// Gives it back to the previous owner.
		~OwnerScope();

		OwnerScope(const OwnerScope&) = delete;
		OwnerScope(OwnerScope&&) = delete;
		auto operator=(const OwnerScope&) -> OwnerScope& = delete;
		auto operator=(OwnerScope&&) -> OwnerScope& = delete;

	private:
		/// Who owned it before.
		const void* m_previous = nullptr;
	};

	/**
	 * @brief Whether a request is waiting to be drawn or answered.
	 * @return True if a request is pending.
	 */
	[[nodiscard]] static auto hasRequest() -> bool;

	/**
	 * @brief Drop the pending request without calling its continuation.
	 *
	 * What cancelling does, and what closing the application has to do: a continuation
	 * left pending holds captures that must not outlive the frame loop.
	 */
	static void cancel();

	/**
	 * @brief Split a filter string into its lines.
	 *
	 * A line's extensions may be spelled `lev`, `*.lev` or `.lev` indifferently, and
	 * `*` or `*.*` makes that line a catch-all. One is appended to every request
	 * anyway, so a caller only spells it out to place it somewhere other than last.
	 *
	 * @param[in] iFilter The filter, as `"Name|ext1,ext2\nOther|ext"`.
	 * @return One filter per usable line, malformed lines dropped.
	 */
	[[nodiscard]] static auto parseFilters(const std::string_view& iFilter) -> std::vector<Filter>;

	/**
	 * @brief Whether a file passes a filter.
	 * @param[in] iPath The file to test.
	 * @param[in] iFilter The filter to pass. One without extension accepts everything.
	 * @return True if the file is shown.
	 */
	[[nodiscard]] static auto matches(const std::filesystem::path& iPath, const Filter& iFilter) -> bool;

	/**
	 * @brief Whether a path is one of the formats a filter list declares.
	 *
	 * What answers false is shown greyed rather than hidden, the catch-all being always
	 * available: an organizer who knows their file is an image whatever its extension
	 * says must be able to pick it, and take the risk.
	 *
	 * @param[in] iFilters The filters of the request, catch-all included.
	 * @param[in] iPath The path to judge.
	 * @return True when a filter declares that extension, and true as well when none
	 *         of them declares anything — there is then nothing to be outside of.
	 */
	[[nodiscard]] static auto isDeclared(const std::vector<Filter>& iFilters, const std::filesystem::path& iPath)
			-> bool;

	/**
	 * @brief List what a directory offers, directories first then files, by name.
	 *
	 * Never throws and never reports: a directory that cannot be read comes back empty,
	 * which is what the organizer has to see rather than a dialog that vanishes.
	 *
	 * @param[in] iDirectory The directory to list.
	 * @param[in] iFilter The filter applied to files, `nullptr` to keep them all.
	 * @param[in] iShowHidden Whether names starting with a dot are listed.
	 * @return The entries, sorted.
	 */
	[[nodiscard]] static auto listEntries(const std::filesystem::path& iDirectory, const Filter* iFilter,
										  bool iShowHidden) -> std::vector<Entry>;

	/**
	 * @brief Turn a typed name into the path it means.
	 * @param[in] iDirectory The directory being browsed.
	 * @param[in] iName What was typed. An absolute path is taken as it is.
	 * @param[in] iFilter The active filter, whose first extension is appended when the
	 *            name carries none. `nullptr` appends nothing.
	 * @return The path, empty if the name is empty.
	 */
	[[nodiscard]] static auto resolveTarget(const std::filesystem::path& iDirectory, const std::string_view& iName,
											const Filter* iFilter) -> std::filesystem::path;

	/**
	 * @brief Where a request starts browsing.
	 * @return The last directory used, else the data location, else the executable's.
	 */
	[[nodiscard]] static auto startDirectory() -> std::filesystem::path;

	/**
	 * @brief Cut a directory into the steps of the path bar, root first.
	 *
	 * Each step carries the whole path down to it, so clicking the third of six goes up
	 * three levels in one move rather than three presses of a Parent button.
	 *
	 * @param[in] iDirectory The directory shown.
	 * @return One crumb per level, root first, the directory itself last.
	 */
	[[nodiscard]] static auto breadcrumb(const std::filesystem::path& iDirectory) -> std::vector<Crumb>;

	/**
	 * @brief Read a typed text as a place to look and a beginning to look for.
	 *
	 * The same box narrows the current directory and reaches any other one: with no
	 * separator the text is a prefix, and with one it is a path whose last component is
	 * the prefix. `/data/sou` lists `/data` and keeps `sources`, which is what makes
	 * completion worth a key.
	 *
	 * @param[in] iBase The directory being browsed, used when no path was typed.
	 * @param[in] iQuery What was typed.
	 * @return Where to look, what to keep, and what completion must put back.
	 */
	[[nodiscard]] static auto splitQuery(const std::filesystem::path& iBase, const std::string_view& iQuery) -> Lookup;

	/**
	 * @brief Whether a directory is another one, or one of its parents.
	 *
	 * Compared component by component, never as text: `/data/sources` is not a parent
	 * of `/data/sources2`, though one string does begin with the other. That is what
	 * decides which branches of the tree unfold, and a text comparison unfolded the
	 * neighbours too.
	 *
	 * @param[in] iParent The directory that may be the ancestor.
	 * @param[in] iChild The directory that may descend from it.
	 * @return True when @p iChild is @p iParent or sits below it.
	 */
	[[nodiscard]] static auto isAncestorOf(const std::filesystem::path& iParent, const std::filesystem::path& iChild)
			-> bool;

	/**
	 * @brief Whether a name begins with what has been typed, whatever the case.
	 * @param[in] iName The name listed.
	 * @param[in] iPrefix What was typed. Empty matches everything.
	 * @return True if the name is kept.
	 */
	[[nodiscard]] static auto startsWithIgnoringCase(const std::string_view& iName, const std::string_view& iPrefix)
			-> bool;

	/**
	 * @brief The longest beginning every entry shares, which is what completion adds.
	 * @param[in] iEntries The entries the typed prefix kept.
	 * @return Their common beginning, in the case of the first one. Empty if they share
	 *         nothing, or if there is nothing to complete.
	 */
	[[nodiscard]] static auto commonPrefix(const std::vector<Entry>& iEntries) -> std::string;

	/**
	 * @brief The paths used lately, most recent first.
	 * @return At most `g_maxRecentPaths` paths, those that no longer exist dropped.
	 */
	[[nodiscard]] static auto recentPaths() -> std::vector<std::filesystem::path>;

	/**
	 * @brief Put a path at the head of the recent ones, and persist the list.
	 * @param[in] iPath The path just used. An empty one is ignored.
	 */
	static void rememberPath(const std::filesystem::path& iPath);
};

}// namespace evl::gui::utils
