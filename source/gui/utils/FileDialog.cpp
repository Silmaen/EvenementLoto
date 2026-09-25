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
#include "core/Settings.h"
#include "core/utilities.h"
#include "gui/Application.h"

#include <imgui.h>
#include <imgui_stdlib.h>

namespace evl::gui::utils {

namespace {

/// Width of the left pane, holding the recent paths and the tree.
constexpr float g_sidePaneWidth = 280.0f;
/// Width of the two closing buttons.
constexpr float g_buttonWidth = 120.0f;

/// What a request asks for, and what to do with the answer.
struct Request {
	/// What is being asked.
	FileDialogMode mode = FileDialogMode::Open;
	/// The filters offered, the catch-all included, never empty outside Folder mode.
	std::vector<FileDialog::Filter> filters;
	/// What to do with the chosen path.
	FileDialogCallback onChosen;
	/// Who asked, so the browser is drawn back inside their scope.
	const void* owner = nullptr;
};

/// What one frame of the browser concluded.
enum struct Outcome : uint8_t {
	Pending,///< Still open, nothing decided.
	Chosen,///< A path was confirmed.
	Cancelled,///< The organizer gave up.
};

/// That conclusion, and the path it concerns.
struct Decision {
	/// What was concluded.
	Outcome outcome = Outcome::Pending;
	/// The path, meaningful only once chosen.
	std::filesystem::path path;
};

/// Everything the browser remembers. A single one: it is a modal.
struct State {
	/// The request being served, if any.
	std::optional<Request> request;
	/// Whether `ImGui::OpenPopup` has already been called for it.
	bool opened = false;
	/// The directory being browsed, which the tree and the path bar show.
	std::filesystem::path directory;
	/// The directory the entries come from: the one above, or the one typed.
	std::filesystem::path listed;
	/// What that directory offers, already sorted and filtered.
	std::vector<FileDialog::Entry> entries;
	/// The entry the organizer clicked, empty if none.
	std::filesystem::path selected;
	/// The name typed in Save mode.
	std::string name;
	/// What has been typed to narrow the list.
	std::string query;
	/// Which filter is active.
	size_t filterIndex = 0;
	/// Whether dotted names are listed.
	bool showHidden = false;
	/// Whether the overwrite question is being asked.
	bool confirmOverwrite = false;
	/// The sub-directories of the tree nodes already opened, listed once each.
	std::map<std::filesystem::path, std::vector<FileDialog::Entry>> treeCache;
	/// Whether the tree still has to unfold down to the current directory.
	bool revealDirectory = true;
	/// Whether the list should take the keyboard focus on the next frame.
	bool focusList = false;
	/// The directory the next request starts from. Outlives the requests.
	std::filesystem::path lastPath;
	/// Who is drawing right now.
	const void* currentOwner = nullptr;
};

auto state() -> State& {
	static State s;
	return s;
}

auto split(const std::string_view iString, const char iDelimiter) -> std::vector<std::string_view> {
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

/// A directory without its trailing separator, so two spellings of it compare equal.
///
/// `lexically_normal` keeps the separator a typed path ends with, and `/data/` then
/// differs from `/data` — enough to make the browser believe it is looking elsewhere.
auto asDirectory(const std::filesystem::path& iPath) -> std::filesystem::path {
	auto normalized = iPath.lexically_normal();
	if (normalized.filename().empty() && normalized != normalized.root_path())
		normalized = normalized.parent_path();
	return normalized;
}

auto lowered(const std::string_view iString) -> std::string {
	std::string result{iString};
	std::ranges::transform(result, result.begin(),
						   [](const unsigned char iChar) -> char { return static_cast<char>(std::tolower(iChar)); });
	return result;
}

/// The title of the window, which is also its ImGui identifier.
auto titleOf(const FileDialogMode iMode) -> const char* {
	switch (iMode) {
		case FileDialogMode::Save:
			return "Enregistrer sous";
		case FileDialogMode::Folder:
			return "Choisir un dossier";
		case FileDialogMode::Open:
			break;
	}
	return "Ouvrir un fichier";
}

/// The active filter, or `nullptr` when there is no list to choose from.
auto activeFilter(const State& iState) -> const FileDialog::Filter* {
	if (!iState.request.has_value() || iState.request->filters.empty())
		return nullptr;
	const auto index = std::min(iState.filterIndex, iState.request->filters.size() - 1);
	return &iState.request->filters[index];
}

/// The entries the typed prefix keeps, in the order they are listed.
auto visibleEntries(const State& iState) -> std::vector<FileDialog::Entry> {
	const auto lookup = FileDialog::splitQuery(iState.directory, iState.query);
	if (lookup.prefix.empty())
		return iState.entries;
	std::vector<FileDialog::Entry> kept;
	for (const auto& entry: iState.entries) {
		if (FileDialog::startsWithIgnoringCase(entry.name, lookup.prefix))
			kept.push_back(entry);
	}
	return kept;
}

/// List again what the browser shows, which a typed path may move elsewhere.
void refresh() {
	auto& s = state();
	s.listed = FileDialog::splitQuery(s.directory, s.query).directory;
	s.entries = FileDialog::listEntries(s.listed, activeFilter(s), s.showHidden);
	s.selected.clear();
}

/// Whether an entry is one of the formats the caller asked for. Folders always are.
auto isSupported(const Request& iRequest, const FileDialog::Entry& iEntry) -> bool {
	return iEntry.isDirectory || FileDialog::isDeclared(iRequest.filters, iEntry.path);
}

/// Enter a directory: the list, the tree and the typed prefix all follow.
void navigateTo(const std::filesystem::path& iDirectory) {
	auto& s = state();
	std::error_code error;
	if (!is_directory(iDirectory, error) || error)
		return;
	s.directory = asDirectory(iDirectory);
	s.query.clear();
	s.revealDirectory = true;
	refresh();
}

/// The roots the tree grows from: the drives on Windows, `/` elsewhere.
auto treeRoots() -> std::vector<std::filesystem::path> {
	std::vector<std::filesystem::path> roots;
#ifdef EVL_PLATFORM_WINDOWS
	for (char letter = 'A'; letter <= 'Z'; ++letter) {
		auto root = std::filesystem::path{std::string{1, letter} + ":\\"};
		std::error_code error;
		if (is_directory(root, error) && !error)
			roots.push_back(std::move(root));
	}
#else
	roots.emplace_back("/");
#endif
	return roots;
}

/// The sub-directories of a node, listed once and kept.
auto subDirectories(const std::filesystem::path& iDirectory) -> const std::vector<FileDialog::Entry>& {
	auto& s = state();
	if (const auto known = s.treeCache.find(iDirectory); known != s.treeCache.end())
		return known->second;
	std::vector<FileDialog::Entry> directories;
	for (auto& entry: FileDialog::listEntries(iDirectory, nullptr, s.showHidden)) {
		if (entry.isDirectory)
			directories.push_back(std::move(entry));
	}
	return s.treeCache.emplace(iDirectory, std::move(directories)).first->second;
}

/// The folder or file icon, when the texture library has one.
void drawIcon(const bool iIsDirectory) {
	if (!Application::instanced())
		return;
	const auto texture = Application::get().getTextureLibrary().getTextureId(iIsDirectory ? "folder_open" : "file");
	if (texture == 0)
		return;
	const float side = ImGui::GetTextLineHeight();
	ImGui::Image(texture, ImVec2(side, side));
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
}

/// A row of the lists: the whole width clickable, the icon included.
///
/// @param[in] iEntry What the row shows.
/// @param[in] iSelected Whether it is the selection.
/// @param[in] iSupported Whether it is one of the formats asked for. It is greyed when
///            it is not, and stays selectable: the organizer takes the risk.
/// @return True when the row was clicked.
auto drawRow(const FileDialog::Entry& iEntry, const bool iSelected, const bool iSupported = true) -> bool {
	// The selectable is drawn first as the background of the row, then the cursor goes
	// back to draw the icon and the name over it. A row where clicking the icon does
	// nothing is a row that looks broken.
	const auto cursor = ImGui::GetCursorPos();
	const bool clicked = ImGui::Selectable(std::format("##row_{}", iEntry.path.string()).c_str(), iSelected,
										   ImGuiSelectableFlags_AllowDoubleClick);
	ImGui::SetCursorPos(cursor);
	drawIcon(iEntry.isDirectory);
	if (iSupported)
		ImGui::TextUnformatted(iEntry.name.c_str());
	else
		ImGui::TextDisabled("%s", iEntry.name.c_str());
	return clicked;
}

/// Record the request, wherever it comes from.
void post(const FileDialogMode iMode, const std::string_view& iFilter, FileDialogCallback iOnChosen) {
	auto& s = state();
	if (s.request.has_value()) {
		// Two requests at once cannot both be drawn: the first one keeps the window,
		// and the second is told so rather than silently dropped.
		log_warn("Une sélection de fichier est déjà en cours, la nouvelle est ignorée.");
		return;
	}
	auto filters = FileDialog::parseFilters(iFilter);
	if (iMode != FileDialogMode::Folder) {
		// Always offered, last: whatever the caller asked for, the organizer must be
		// able to see a file whose extension is not the expected one — a `.lev` saved
		// as `.lev.bak`, an image out of a camera. An empty extension list accepts
		// everything, and adds nothing to a name typed in Save mode.
		filters.push_back(FileDialog::Filter{.name = std::string{g_allFilesLabel}, .extensions = {}});
	}
	s.request = Request{.mode = iMode,
						.filters = std::move(filters),
						.onChosen = std::move(iOnChosen),
						.owner = s.currentOwner};
	s.opened = false;
	s.filterIndex = 0;
	s.confirmOverwrite = false;
	s.name.clear();
	s.query.clear();
	s.treeCache.clear();
	s.revealDirectory = true;
	s.focusList = false;
	s.directory = FileDialog::startDirectory();
	refresh();
}

/// What the closing button offers, empty when there is nothing to confirm.
auto currentTarget(const Request& iRequest) -> std::filesystem::path {
	const auto& s = state();
	switch (iRequest.mode) {
		case FileDialogMode::Folder:
			// A folder request answers with the directory being browsed, unless a
			// sub-directory is selected — which is what a single click means here.
			return s.selected.empty() ? s.directory : s.selected;
		case FileDialogMode::Save:
			return FileDialog::resolveTarget(s.directory, s.name, activeFilter(s));
		case FileDialogMode::Open:
			break;
	}
	return s.selected;
}

/// What a double click, or the Enter key, does to an entry.
auto activate(const Request& iRequest, const FileDialog::Entry& iEntry) -> Decision {
	auto& s = state();
	if (iEntry.isDirectory) {
		navigateTo(iEntry.path);
		return {};
	}
	s.selected = iEntry.path;
	s.name = iEntry.name;
	if (iRequest.mode == FileDialogMode::Open) {
		ImGui::CloseCurrentPopup();
		return {.outcome = Outcome::Chosen, .path = iEntry.path};
	}
	// In Save mode the name is filled in, and confirming stays a deliberate act: it may
	// mean replacing the file that was just double clicked.
	return {};
}

/// The path bar: every level is a button leading straight to it.
void drawBreadcrumb() {
	auto& s = state();
	const auto crumbs = FileDialog::breadcrumb(s.directory);
	for (size_t i = 0; i < crumbs.size(); ++i) {
		if (i > 0) {
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::TextUnformatted("›");
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		}
		// The last one is where we already are: shown, not offered.
		if (i + 1 == crumbs.size()) {
			ImGui::TextUnformatted(crumbs[i].label.c_str());
			continue;
		}
		if (ImGui::SmallButton(std::format("{}##crumb_{}", crumbs[i].label, i).c_str()))
			navigateTo(crumbs[i].path);
	}
	ImGui::Separator();
}

/// The recent paths, as shortcuts at the top of the left pane.
auto drawRecents(const Request& iRequest) -> Decision {
	const auto recents = FileDialog::recentPaths();
	if (recents.empty())
		return {};
	ImGui::TextDisabled("Récents");
	for (const auto& path: recents) {
		std::error_code error;
		const bool isDirectory = is_directory(path, error) && !error;
		// A root has no filename, and would have shown as an empty row.
		auto name = path.filename().string();
		if (name.empty())
			name = path.string();
		const FileDialog::Entry entry{.path = path, .name = std::move(name), .isDirectory = isDirectory};
		// The name alone would be ambiguous between two folders called `data`, hence
		// the whole path on hover.
		if (drawRow(entry, false, isSupported(iRequest, entry))) {
			if (auto decision = activate(iRequest, entry); decision.outcome != Outcome::Pending)
				return decision;
			if (!isDirectory)
				navigateTo(path.parent_path());
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", path.string().c_str());
	}
	ImGui::Separator();
	return {};
}

/// How deep the tree may go. Beyond that, a symbolic link is looping.
constexpr uint32_t g_maxTreeDepth = 32;

/// One node of the tree, and its children when it is open.
///
/// Recursive, and bounded: a tree deeper than `g_maxTreeDepth` is a link pointing back
/// at one of its own parents, and following it would run out of stack in the middle of
/// a frame.
// NOLINTNEXTLINE(misc-no-recursion): a tree is recursive, and the depth is capped
void drawTreeNode(const std::filesystem::path& iDirectory, const std::string& iLabel, const uint32_t iDepth = 0) {
	if (iDepth >= g_maxTreeDepth)
		return;
	auto& s = state();
	const bool isCurrent = iDirectory == s.directory;
	// While the tree unfolds towards the current directory, every ancestor opens.
	const bool isAncestor = FileDialog::isAncestorOf(iDirectory, s.directory);
	if (s.revealDirectory && isAncestor && !isCurrent)
		ImGui::SetNextItemOpen(true);

	auto flags =
			ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (isCurrent)
		flags |= ImGuiTreeNodeFlags_Selected;
	// An empty label: the arrow only, the icon and the name being drawn just after.
	const bool open = ImGui::TreeNodeEx(iDirectory.string().c_str(), flags, "%s", "");
	// Clicking the name goes there; the arrow is left to open and close.
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		navigateTo(iDirectory);
	ImGui::SameLine(0.0f, 0.0f);
	drawIcon(true);
	ImGui::TextUnformatted(iLabel.c_str());
	if (!open)
		return;
	for (const auto& child: subDirectories(iDirectory)) drawTreeNode(child.path, child.name, iDepth + 1);
	ImGui::TreePop();
}

/// Completion on Tab: the typed name grows to what every match shares.
///
/// The directory part of what was typed is put back in front, so completing inside
/// `/data/sou` gives `/data/sources` and not `sources`.
auto completeQuery(ImGuiInputTextCallbackData* ioData) -> int {
	auto& s = state();
	const auto lookup = FileDialog::splitQuery(s.directory, s.query);
	const auto visible = visibleEntries(s);
	auto completed = FileDialog::commonPrefix(visible);
	if (completed.empty() || completed.size() <= lookup.prefix.size())
		return 0;
	// A single directory left completes to its own separator, so the next Tab looks
	// inside it instead of offering the same name again.
	if (visible.size() == 1 && visible.front().isDirectory)
		completed += static_cast<char>(std::filesystem::path::preferred_separator);
	const auto replaced = lookup.head + completed;
	ioData->DeleteChars(0, ioData->BufTextLen);
	ioData->InsertChars(0, replaced.c_str());
	s.query = replaced;
	refresh();
	return 0;
}

/// The right pane: what has been typed, and what it keeps.
auto drawEntryPane(const Request& iRequest) -> Decision {
	auto& s = state();
	Decision decision;

	ImGui::TextDisabled("Chemin");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(-1);
	if (s.focusList) {
		ImGui::SetKeyboardFocusHere();
		s.focusList = false;
	}
	// One box for both: a few letters narrow the directory shown, a path reaches any
	// other one. Enter acts on what is left, Tab completes what has been typed.
	const bool submitted = ImGui::InputText(
			"##Query", &s.query, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion,
			[](ImGuiInputTextCallbackData* ioData) -> int { return completeQuery(ioData); });
	// The listing follows what is typed, which may be a directory other than the one
	// being browsed.
	if (ImGui::IsItemEdited())
		refresh();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Un préfixe filtre ce dossier, un chemin mène ailleurs.\n"
						  "Tabulation pour compléter, Entrée pour ouvrir.");
	// Where the entries come from, when it is not the directory being browsed.
	if (s.listed != s.directory)
		ImGui::TextDisabled("dans %s", s.listed.string().c_str());

	const auto visible = visibleEntries(s);
	if (ImGui::BeginChild("Entries", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
		for (const auto& entry: visible) {
			if (drawRow(entry, entry.path == s.selected, isSupported(iRequest, entry))) {
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					decision = activate(iRequest, entry);
				} else {
					s.selected = entry.path;
					if (!entry.isDirectory)
						s.name = entry.name;
				}
			}
			if (decision.outcome != Outcome::Pending)
				break;
		}
	}
	ImGui::EndChild();

	// Enter, whether from the filter box or from the list, opens what is pointed at:
	// the selection, or the only entry left once the prefix has narrowed it to one.
	const bool listEnter = !ImGui::IsAnyItemActive() &&
						   (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter));
	if (decision.outcome == Outcome::Pending && (submitted || listEnter)) {
		// A whole directory typed out goes there, without needing a match in the list.
		const auto typed = FileDialog::splitQuery(s.directory, s.query);
		std::error_code error;
		if (const auto whole = typed.directory / typed.prefix;
			!typed.prefix.empty() && is_directory(whole, error) && !error) {
			navigateTo(whole);
		} else if (const auto known = std::ranges::find_if(
						   visible,
						   [&s](const FileDialog::Entry& iEntry) -> bool { return iEntry.path == s.selected; });
				   known != visible.end()) {
			decision = activate(iRequest, *known);
		} else if (visible.size() == 1) {
			decision = activate(iRequest, visible.front());
		}
	}
	return decision;
}

/// The filter combo and the hidden files switch.
void drawFilterRow(const Request& iRequest) {
	auto& s = state();
	const auto& filters = iRequest.filters;
	if (!filters.empty()) {
		const auto index = std::min(s.filterIndex, filters.size() - 1);
		ImGui::TextDisabled("Type");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(280);
		if (ImGui::BeginCombo("##Filter", filters[index].name.c_str())) {
			for (size_t i = 0; i < filters.size(); ++i) {
				if (ImGui::Selectable(filters[i].name.c_str(), i == index)) {
					s.filterIndex = i;
					refresh();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
	}
	if (ImGui::Checkbox("Fichiers cachés", &s.showHidden)) {
		s.treeCache.clear();
		refresh();
	}
	// Only worth saying when something greyed can actually be on screen: the catch-all
	// is what lists a file no declared filter accepts.
	const bool declaresAnything = std::ranges::any_of(
			iRequest.filters, [](const FileDialog::Filter& iOne) -> bool { return !iOne.extensions.empty(); });
	if (const auto* filter = activeFilter(s); declaresAnything && filter != nullptr && filter->extensions.empty()) {
		ImGui::SameLine();
		ImGui::TextDisabled("(les formats non pris en charge sont grisés)");
	}
}

/// The buttons, or the overwrite question that replaces them.
auto drawDecision(const Request& iRequest) -> Decision {
	auto& s = state();
	const auto target = currentTarget(iRequest);
	if (s.confirmOverwrite) {
		ImGui::TextUnformatted("Ce fichier existe déjà. Le remplacer ?");
		ImGui::SameLine();
		if (ImGui::Button("Remplacer", ImVec2(g_buttonWidth, 0))) {
			ImGui::CloseCurrentPopup();
			return {.outcome = Outcome::Chosen, .path = target};
		}
		ImGui::SameLine();
		if (ImGui::Button("Retour", ImVec2(g_buttonWidth, 0)))
			s.confirmOverwrite = false;
		return {};
	}

	// The two buttons sit at the right end of the row, where a dialog ends.
	const float offset = ImGui::GetContentRegionAvail().x - 2.0f * g_buttonWidth - ImGui::GetStyle().ItemSpacing.x;
	if (offset > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
	const char* accept = iRequest.mode == FileDialogMode::Save ? "Enregistrer" : "Choisir";
	ImGui::BeginDisabled(target.empty());
	const bool accepted = ImGui::Button(accept, ImVec2(g_buttonWidth, 0));
	ImGui::EndDisabled();
	ImGui::SameLine();
	const bool cancelled = ImGui::Button("Annuler", ImVec2(g_buttonWidth, 0));
	if (accepted) {
		std::error_code error;
		// Replacing a file is asked once, on this same window, rather than in a third
		// modal on top of the two already stacked.
		if (iRequest.mode == FileDialogMode::Save && exists(target, error) && !error) {
			s.confirmOverwrite = true;
			return {};
		}
		ImGui::CloseCurrentPopup();
		return {.outcome = Outcome::Chosen, .path = target};
	}
	if (cancelled) {
		ImGui::CloseCurrentPopup();
		return {.outcome = Outcome::Cancelled, .path = {}};
	}
	return {};
}

auto drawContent(const Request& iRequest) -> Decision {
	auto& s = state();
	Decision decision;
	drawBreadcrumb();

	// Two panes: where to go on the left, what is there on the right.
	const float rows = iRequest.mode == FileDialogMode::Save ? 3.0f : 2.0f;
	const float panes = ImGui::GetContentRegionAvail().y - rows * ImGui::GetFrameHeightWithSpacing();
	if (ImGui::BeginChild("Tree", ImVec2(g_sidePaneWidth, panes), ImGuiChildFlags_Borders)) {
		decision = drawRecents(iRequest);
		if (decision.outcome == Outcome::Pending) {
			for (const auto& root: treeRoots()) drawTreeNode(root, root.string());
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("Pane", ImVec2(0, panes)) && decision.outcome == Outcome::Pending)
		decision = drawEntryPane(iRequest);
	ImGui::EndChild();
	// The tree has unfolded: it must not fight the organizer closing a node again.
	s.revealDirectory = false;

	if (decision.outcome != Outcome::Pending)
		return decision;

	drawFilterRow(iRequest);
	if (iRequest.mode == FileDialogMode::Save) {
		ImGui::TextDisabled("Nom");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-1);
		ImGui::InputText("##FileName", &s.name);
	}
	decision = drawDecision(iRequest);
	// Escape answers nothing, like the Cancel button. A modal opened without a close
	// flag does not do it on its own.
	if (decision.outcome == Outcome::Pending && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
		ImGui::CloseCurrentPopup();
		decision = Decision{.outcome = Outcome::Cancelled, .path = {}};
	}
	return decision;
}

}// namespace
FileDialog::OwnerScope::OwnerScope(const void* iOwner) : m_previous{state().currentOwner} {
	state().currentOwner = iOwner;
}

FileDialog::OwnerScope::~OwnerScope() { state().currentOwner = m_previous; }

void FileDialog::openFile(const std::string_view& iFilter, FileDialogCallback iOnChosen) {
	post(FileDialogMode::Open, iFilter, std::move(iOnChosen));
}

void FileDialog::saveFile(const std::string_view& iFilter, FileDialogCallback iOnChosen) {
	post(FileDialogMode::Save, iFilter, std::move(iOnChosen));
}

void FileDialog::selectFolder(FileDialogCallback iOnChosen) { post(FileDialogMode::Folder, {}, std::move(iOnChosen)); }

auto FileDialog::hasRequest() -> bool { return state().request.has_value(); }

void FileDialog::cancel() {
	auto& s = state();
	s.request.reset();
	s.opened = false;
}

void FileDialog::draw(const void* iOwner) {
	auto& s = state();
	if (!s.request.has_value() || s.request->owner != iOwner)
		return;

	const char* title = titleOf(s.request->mode);
	if (!s.opened) {
		ImGui::SetNextWindowSize(ImVec2(760, 560), ImGuiCond_FirstUseEver);
		ImGui::OpenPopup(title);
		s.opened = true;
	}
	Decision decision;
	if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_None)) {
		decision = drawContent(*s.request);
		ImGui::EndPopup();
	}
	if (decision.outcome == Outcome::Pending)
		return;

	// Decided, and the window is closed. The request is dropped first, so the
	// continuation runs outside the browser's scope and may open a popup of its own —
	// and so a cancellation drops the continuation rather than calling it with nothing.
	auto callback = std::move(s.request->onChosen);
	s.request.reset();
	s.opened = false;
	if (decision.outcome == Outcome::Cancelled)
		return;
	std::error_code error;
	s.lastPath = is_directory(decision.path, error) && !error ? decision.path : decision.path.parent_path();
	rememberPath(decision.path);
	if (callback)
		callback(decision.path);
}

auto FileDialog::parseFilters(const std::string_view& iFilter) -> std::vector<Filter> {
	std::vector<Filter> filters;
	for (const auto& line: split(iFilter, '\n')) {
		const auto items = split(line, '|');
		if (items.size() != 2 || items[0].empty() || items[1].empty())
			continue;
		Filter filter{.name = std::string{items[0]}, .extensions = {}};
		bool acceptsEverything = false;
		for (auto spec: split(items[1], ',')) {
			// `*` and `*.*` are the two spellings of a catch-all, and an extension
			// list left empty is how one is carried.
			if (spec == "*" || spec == "*.*") {
				acceptsEverything = true;
				continue;
			}
			if (spec.starts_with("*."))
				spec.remove_prefix(2);
			else if (spec.starts_with("."))
				spec.remove_prefix(1);
			if (!spec.empty())
				filter.extensions.push_back(lowered(spec));
		}
		if (acceptsEverything)
			filter.extensions.clear();
		if (acceptsEverything || !filter.extensions.empty())
			filters.push_back(std::move(filter));
	}
	return filters;
}

auto FileDialog::isDeclared(const std::vector<Filter>& iFilters, const std::filesystem::path& iPath) -> bool {
	bool anyDeclared = false;
	for (const auto& filter: iFilters) {
		// The catch-all declares nothing, and must not make everything supported.
		if (filter.extensions.empty())
			continue;
		anyDeclared = true;
		if (matches(iPath, filter))
			return true;
	}
	return !anyDeclared;
}

auto FileDialog::matches(const std::filesystem::path& iPath, const Filter& iFilter) -> bool {
	if (iFilter.extensions.empty())
		return true;
	auto extension = lowered(iPath.extension().string());
	if (extension.starts_with("."))
		extension.erase(0, 1);
	return std::ranges::find(iFilter.extensions, extension) != iFilter.extensions.end();
}

auto FileDialog::listEntries(const std::filesystem::path& iDirectory, const Filter* iFilter, const bool iShowHidden)
		-> std::vector<Entry> {
	std::vector<Entry> entries;
	std::error_code error;
	// The non-throwing iterator: a directory the organizer may not read must give an
	// empty list, never an exception in the middle of a frame.
	for (const auto& item: std::filesystem::directory_iterator{
				 iDirectory, std::filesystem::directory_options::skip_permission_denied, error}) {
		auto name = item.path().filename().string();
		if (name.empty() || (!iShowHidden && name.starts_with(".")))
			continue;
		std::error_code itemError;
		const bool isDirectory = item.is_directory(itemError);
		if (itemError)
			continue;
		if (!isDirectory && iFilter != nullptr && !matches(item.path(), *iFilter))
			continue;
		entries.push_back(Entry{.path = item.path(), .name = std::move(name), .isDirectory = isDirectory});
	}
	if (error)
		log_warn("Impossible de lister '{}' : {}", iDirectory.string(), error.message());
	std::ranges::sort(entries, [](const Entry& iLeft, const Entry& iRight) -> bool {
		if (iLeft.isDirectory != iRight.isDirectory)
			return iLeft.isDirectory;
		return lowered(iLeft.name) < lowered(iRight.name);
	});
	return entries;
}

auto FileDialog::resolveTarget(const std::filesystem::path& iDirectory, const std::string_view& iName,
							   const Filter* iFilter) -> std::filesystem::path {
	if (iName.empty())
		return {};
	std::filesystem::path target{iName};
	if (!target.is_absolute())
		target = iDirectory / target;
	if (!target.has_extension() && iFilter != nullptr && !iFilter->extensions.empty())
		target.replace_extension(iFilter->extensions.front());
	return target.lexically_normal();
}

auto FileDialog::breadcrumb(const std::filesystem::path& iDirectory) -> std::vector<Crumb> {
	std::vector<Crumb> crumbs;
	const auto normalized = iDirectory.lexically_normal();
	// The root first, under its own name: `/` on Linux, `C:\` on Windows.
	if (normalized.has_root_path()) {
		crumbs.push_back(Crumb{.label = normalized.root_path().string(), .path = normalized.root_path()});
	}
	auto walked = normalized.root_path();
	for (const auto& part: normalized.relative_path()) {
		if (part.empty() || part == ".")
			continue;
		walked /= part;
		crumbs.push_back(Crumb{.label = part.string(), .path = walked});
	}
	// A relative directory has no root, and still deserves a first step.
	if (crumbs.empty() && !normalized.empty())
		crumbs.push_back(Crumb{.label = normalized.string(), .path = normalized});
	return crumbs;
}

auto FileDialog::splitQuery(const std::filesystem::path& iBase, const std::string_view& iQuery) -> Lookup {
	// Both separators are read on every platform: a path pasted from elsewhere must
	// still be understood.
	const auto cut = iQuery.find_last_of("/\\");
	if (cut == std::string_view::npos)
		return Lookup{.directory = iBase, .prefix = std::string{iQuery}, .head = {}};
	const auto head = iQuery.substr(0, cut + 1);
	std::filesystem::path directory{head};
	// A relative path is read from the directory being browsed, which is what makes
	// `sources/per` work without typing the whole thing.
	if (!directory.is_absolute())
		directory = iBase / directory;
	return Lookup{.directory = asDirectory(directory),
				  .prefix = std::string{iQuery.substr(cut + 1)},
				  .head = std::string{head}};
}

auto FileDialog::isAncestorOf(const std::filesystem::path& iParent, const std::filesystem::path& iChild) -> bool {
	const auto parent = asDirectory(iParent);
	const auto child = asDirectory(iChild);
	auto parentPart = parent.begin();
	auto childPart = child.begin();
	for (; parentPart != parent.end() && childPart != child.end(); ++parentPart, ++childPart) {
		if (*parentPart != *childPart)
			return false;
	}
	// Every component of the parent was matched: what is left of the child is below it.
	return parentPart == parent.end();
}

auto FileDialog::startsWithIgnoringCase(const std::string_view& iName, const std::string_view& iPrefix) -> bool {
	if (iPrefix.empty())
		return true;
	if (iName.size() < iPrefix.size())
		return false;
	return lowered(iName.substr(0, iPrefix.size())) == lowered(iPrefix);
}

auto FileDialog::commonPrefix(const std::vector<Entry>& iEntries) -> std::string {
	if (iEntries.empty())
		return {};
	// The first name, cut back until every other one begins with it. Comparison
	// ignores case, the answer keeps the case of the first: completing `PART` into
	// `partie.lev` must not shout back.
	std::string prefix = iEntries.front().name;
	for (const auto& entry: iEntries) {
		while (!prefix.empty() && !startsWithIgnoringCase(entry.name, prefix)) prefix.pop_back();
		if (prefix.empty())
			break;
	}
	return prefix;
}

auto FileDialog::recentPaths() -> std::vector<std::filesystem::path> {
	const auto settings = core::getSettings();
	if (settings == nullptr)
		return {};
	std::vector<std::filesystem::path> paths;
	// Named, not inlined into the loop: `split` returns views into it, and a temporary
	// would be gone before the first one is read.
	const auto stored = settings->getValue<std::string>(std::string{g_recentPathsKey}, "");
	for (const auto& line: split(stored, '\n')) {
		if (line.empty())
			continue;
		std::filesystem::path path{line};
		std::error_code error;
		// A shortcut to something that has been moved or deleted is worse than no
		// shortcut: it is dropped rather than offered.
		if (!exists(path, error) || error)
			continue;
		if (std::ranges::find(paths, path) == paths.end())
			paths.push_back(std::move(path));
		if (paths.size() >= g_maxRecentPaths)
			break;
	}
	return paths;
}

void FileDialog::rememberPath(const std::filesystem::path& iPath) {
	if (iPath.empty())
		return;
	const auto settings = core::getSettings();
	if (settings == nullptr)
		return;
	auto paths = recentPaths();
	// Already known: it moves back to the head rather than appearing twice.
	std::erase(paths, iPath);
	paths.insert(paths.begin(), iPath);
	if (paths.size() > g_maxRecentPaths)
		paths.resize(g_maxRecentPaths);
	std::string joined;
	for (const auto& path: paths) {
		if (!joined.empty())
			joined += '\n';
		joined += path.string();
	}
	settings->setValue(std::string{g_recentPathsKey}, joined);
}

auto FileDialog::startDirectory() -> std::filesystem::path {
	const auto& s = state();
	std::error_code error;
	if (!s.lastPath.empty() && is_directory(s.lastPath, error) && !error)
		return s.lastPath;
	if (const auto data = core::getDataLocation(); is_directory(data, error) && !error)
		return data;
	return core::getExecPath();
}

}// namespace evl::gui::utils
