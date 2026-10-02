/**
 * @file test_ErrorPaths.cpp
 * @author Silmaen
 * @date 28/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"

#include "core/AtomicFile.h"
#include "core/Event.h"
#include "core/Rescue.h"
#include "core/Settings.h"
#include "core/YamlRead.h"
#include "core/utilities.h"

#include <filesystem>
#include <fstream>

using namespace evl::core;

namespace fs = std::filesystem;

namespace {

/// A directory of its own, removed at the end of the test.
class Area {
public:
	explicit Area(const std::string& iName) : m_path{fs::temp_directory_path() / ("evl-errors-" + iName)} {
		remove_all(m_path);
		create_directories(m_path);
	}
	~Area() { remove_all(m_path); }

	Area(const Area&) = delete;
	Area(Area&&) = delete;
	auto operator=(const Area&) -> Area& = delete;
	auto operator=(Area&&) -> Area& = delete;

	[[nodiscard]] auto path() const -> const fs::path& { return m_path; }
	[[nodiscard]] auto file(const std::string& iName) const -> fs::path { return m_path / iName; }

	/// Write a file with the given content.
	void write(const std::string& iName, const std::string& iContent) const {
		std::ofstream out(file(iName), std::ios::out | std::ios::binary);
		out << iContent;
	}

private:
	fs::path m_path;
};

/// Bytes that are not text and mean nothing in any format.
auto rubbish(const std::size_t iSize) -> std::string {
	std::string bytes;
	for (std::size_t i = 0; i < iSize; ++i) bytes += static_cast<char>((i * 61 + 7) % 256);
	return bytes;
}

}// namespace

// ---------------------------------------------------------------------------- Settings

TEST(ErrorPaths, SettingsFromAMissingFileKeepsTheDefaults) {
	Settings settings;
	settings.setValue("gui/answer", 42);
	// Aucun réglage enregistré est l'état normal d'un premier démarrage : ce n'est pas
	// une erreur, et ce qui est déjà en mémoire ne doit pas disparaître.
	settings.fromFile("/n/existe/pas/config.yml");
	EXPECT_EQ(settings.getValue<int>("gui/answer"), 42);
}

TEST(ErrorPaths, SettingsFromADirectory) {
	const Area area{"settings-dir"};
	Settings settings;
	settings.fromFile(area.path());
	// Rien lu, rien planté.
	EXPECT_FALSE(settings.contains("gui/answer"));
}

TEST(ErrorPaths, SettingsFromMalformedYaml) {
	const Area area{"settings-yaml"};
	area.write("config.yml", "general:\n  log_level: info\n [[[ ceci ne ferme jamais\n");
	Settings settings;
	settings.setValue("survivant", std::string{"oui"});
	settings.fromFile(area.file("config.yml"));
	// L'exception de l'analyseur est rattrapée, et ce qui était là reste là.
	EXPECT_EQ(settings.getValue<std::string>("survivant"), "oui");
}

TEST(ErrorPaths, SettingsFromBinaryRubbish) {
	const Area area{"settings-bin"};
	area.write("config.yml", rubbish(1024));
	Settings settings;
	settings.fromFile(area.file("config.yml"));
	EXPECT_FALSE(settings.contains("general/log_level"));
}

TEST(ErrorPaths, SettingsToAnUnwritableDestination) {
	Settings settings;
	settings.setValue("general/log_level", std::string{"info"});
	// Un répertoire qui n'existe pas : l'échec est journalisé, pas propagé.
	settings.toFile("/n/existe/pas/config.yml");
	// Et l'objet est intact, donc réutilisable.
	EXPECT_EQ(settings.getValue<std::string>("general/log_level"), "info");
}

TEST(ErrorPaths, SettingsValueOfTheWrongType) {
	Settings settings;
	settings.setValue("gui/scale", std::string{"ce n'est pas un nombre"});
	// Demander un type que la valeur n'a pas rend le défaut, sans lever.
	EXPECT_NEAR(settings.getValue<float>("gui/scale", 2.5f), 2.5f, 0.001f);
	EXPECT_EQ(settings.getValue<int>("gui/scale", 7), 7);
	// Une clé absente aussi.
	EXPECT_EQ(settings.getValue<int>("gui/absent", 3), 3);
	// Float et double sont interchangeables, c'est voulu.
	settings.setValue("gui/ratio", 1.5);
	EXPECT_NEAR(settings.getValue<float>("gui/ratio"), 1.5f, 0.001f);
	settings.setValue("gui/other", 2.5f);
	EXPECT_NEAR(settings.getValue<double>("gui/other"), 2.5, 0.001);
}

TEST(ErrorPaths, SettingsSurviveARoundTripThroughRubbishValues) {
	const Area area{"settings-roundtrip"};
	Settings settings;
	// Une chaîne avec des caractères que YAML traite à part.
	settings.setValue("general/path", std::string{"C:\\Program Files\\loto: le vrai"});
	settings.setValue("general/empty", std::string{});
	settings.toFile(area.file("config.yml"));

	Settings reloaded;
	reloaded.fromFile(area.file("config.yml"));
	EXPECT_EQ(reloaded.getValue<std::string>("general/path"), "C:\\Program Files\\loto: le vrai");
}

// -------------------------------------------------------------------------- AtomicFile

TEST(ErrorPaths, AtomicWriteToADirectoryPath) {
	const Area area{"atomic-dir"};
	// La destination est un répertoire : impossible de le remplacer par un fichier.
	EXPECT_FALSE(writeFileAtomically(area.path(), [](std::ostream& oBs) -> void { oBs << "contenu"; }));
}

TEST(ErrorPaths, AtomicWriteLeavesNoTemporaryBehind) {
	const Area area{"atomic-tmp"};
	const auto target = area.file("fichier.lev");
	EXPECT_FALSE(writeFileAtomically(target, [](std::ostream& oBs) -> void {
		oBs << "un début";
		throw std::runtime_error("l'écriture échoue en plein milieu");
	}));
	// Le fichier temporaire ne traîne pas : le prochain essai part propre.
	EXPECT_FALSE(exists(fs::path{target}.concat(".tmp")));
	EXPECT_FALSE(exists(target));
}

TEST(ErrorPaths, AtomicWriteOfNothingIsStillAFile) {
	const Area area{"atomic-empty"};
	const auto target = area.file("vide.lev");
	EXPECT_TRUE(writeFileAtomically(target, [](std::ostream&) -> void {}));
	ASSERT_TRUE(exists(target));
	EXPECT_EQ(file_size(target), 0U);
}

// ------------------------------------------------------------------------------ Rescue

TEST(ErrorPaths, LoadingAMissingRescue) {
	Event event;
	EXPECT_FALSE(loadRescue("/n/existe/pas/rescue.lev", event));
}

TEST(ErrorPaths, LoadingARescueThatIsRubbish) {
	const Area area{"rescue-rubbish"};
	area.write("rescue.lev", rubbish(2048));
	Event event;
	event.setName("intact");
	EXPECT_FALSE(loadRescue(area.file("rescue.lev"), event));
}

TEST(ErrorPaths, LoadingARescueThatIsADirectory) {
	const Area area{"rescue-dir"};
	Event event;
	EXPECT_FALSE(loadRescue(area.path(), event));
}

TEST(ErrorPaths, BothRescueGenerationsCorruptedFindsNothing) {
	const Area area{"rescue-both"};
	const auto settings = getSettings();
	const auto saved = settings->getValue<std::string>("general/data_location", std::string{});
	settings->setValue("general/data_location", area.path().string());

	// Les deux générations illisibles : rien à proposer, plutôt qu'une reprise sur des
	// octets au hasard.
	area.write("rescue.lev", rubbish(300));
	area.write("rescue.lev.1", rubbish(300));
	EXPECT_FALSE(findRescue().has_value());

	settings->setValue("general/data_location", saved);
}

TEST(ErrorPaths, ARescueDirectoryThatIsAFile) {
	const Area area{"rescue-notdir"};
	area.write("occupé", "je ne suis pas un répertoire");
	const auto settings = getSettings();
	const auto saved = settings->getValue<std::string>("general/data_location", std::string{});
	settings->setValue("general/data_location", area.file("occupé").string());

	// L'emplacement de secours n'est pas un répertoire : rien à trouver, et la
	// sauvegarde échoue sans emporter l'application.
	EXPECT_FALSE(findRescue().has_value());
	Event event;
	event.setName("loto");
	event.setOrganizerName("amicale");
	event.pushGameRound(GameRound());
	event.nextState();
	static_cast<void>(saveRescue(event));

	settings->setValue("general/data_location", saved);
}

// ------------------------------------------------------------------------------- Event

TEST(ErrorPaths, ReadingAnEventFromAnEmptyStream) {
	std::istringstream empty;
	Event event;
	event.read(empty, {});
	EXPECT_FALSE(empty.good());
}

TEST(ErrorPaths, ReadingAnEventFromRubbish) {
	std::istringstream stream(rubbish(4096), std::ios::in | std::ios::binary);
	Event event;
	event.setName("intact");
	event.read(stream, {});
	EXPECT_FALSE(stream.good());
}

TEST(ErrorPaths, AnEventWithoutNameOrOrganizerIsInvalid) {
	Event event;
	EXPECT_EQ(event.getStatus(), Event::Status::Invalid);
	event.setName("loto");
	EXPECT_EQ(event.getStatus(), Event::Status::Invalid);
	event.setOrganizerName("amicale");
	// Nommé mais sans partie : il manque encore quelque chose, et c'est dit.
	EXPECT_EQ(event.getStatus(), Event::Status::MissingParties);
	event.pushGameRound(GameRound());
	EXPECT_EQ(event.getStatus(), Event::Status::Ready);
}

TEST(ErrorPaths, AFinishedEventRefusesEverything) {
	Event event;
	event.setName("loto");
	event.setOrganizerName("amicale");
	event.pushGameRound(GameRound());
	while (event.getStatus() != Event::Status::Finished) {
		const auto before = event.getStateString();
		event.nextState();
		if (event.getStatus() == Event::Status::GameRunning &&
			event.getCurrentCGameRound()->getCurrentSubRound()->getStatus() == SubGameRound::Status::Running) {
			event.addWinnerToCurrentRound("carton 1");
		}
		// Sécurité : la boucle doit avancer, sinon le test tournerait sans fin.
		ASSERT_NE(before + event.getStateString(), before + before) << before;
	}

	// Terminé, plus rien ne bouge, et rien ne lève.
	event.setName("autre nom");
	EXPECT_STRNE(event.getName().c_str(), "autre nom");
	event.pushGameRound(GameRound());
	EXPECT_EQ(event.sizeRounds(), 1);
	event.setCatalogue({Prize{"un lot", 10.0}});
	EXPECT_TRUE(event.getCatalogue().empty());
	EXPECT_FALSE(event.insertGameRound(0, GameRound()).has_value());
	EXPECT_FALSE(event.canDraw());
}

TEST(ErrorPaths, IndexesOutOfRangeAreRefusedNotDereferenced) {
	Event event;
	event.setName("loto");
	event.setOrganizerName("amicale");
	event.pushGameRound(GameRound());
	// `std::next` au-delà de la fin est un comportement indéfini : les accès sont bornés.
	EXPECT_EQ(event.getGameRound(42), event.endRounds());
	EXPECT_EQ(event.getGameRound(1), event.endRounds());
	const auto round = event.getGameRound(0);
	EXPECT_EQ(round->getSubRound(42), round->endSubRound());
	// Supprimer et permuter hors limites ne fait rien de fâcheux.
	event.deleteRoundByIndex(42);
	event.swapRoundByIndex(0, 42);
	EXPECT_EQ(event.sizeRounds(), 1);
}

// ---------------------------------------------------------------------------- YamlRead

TEST(ErrorPaths, YamlAccessorsNeverThrow) {
	const YAML::Node root = YAML::Load("clé: valeur\nnulle: ~\nliste: [1, 2]\n");
	EXPECT_TRUE(yamlHas(root, "clé"));
	// Une clé écrite sans valeur est présente : c'est là que `IsDefined()` se trompe.
	EXPECT_TRUE(yamlHas(root, "nulle"));
	EXPECT_FALSE(yamlHas(root, "absente"));

	// Un nœud rendu pour une clé absente répond non à tout, sans lever.
	const auto missing = yamlChild(root, "absente");
	EXPECT_FALSE(missing.IsSequence());
	EXPECT_FALSE(missing.IsMap());
	EXPECT_TRUE(yamlChild(root, "liste").IsSequence());

	// Les valeurs passent par `yamlValue` et non par `as` sur le nœud rendu : un nœud
	// nul se convertit sans se plaindre en la chaine « null », et un lot sans
	// désignation s'appelait donc « null ».
	EXPECT_EQ(missing.as<std::string>("défaut"), "null");
	EXPECT_EQ(yamlValue(root, "absente", std::string{"défaut"}), "défaut");
	EXPECT_EQ(yamlValue(root, "nulle", std::string{"défaut"}), "défaut");
	EXPECT_EQ(yamlValue(root, "clé", std::string{"défaut"}), "valeur");
	EXPECT_EQ(yamlValue(root, "absente", 7), 7);
	// Une valeur du mauvais type rend le défaut.
	EXPECT_EQ(yamlValue(root, "clé", 7), 7);

	// Un parent qui n'est pas une table, et un nœud invalide en entrée.
	const YAML::Node scalar = YAML::Load("juste un scalaire");
	EXPECT_FALSE(yamlHas(scalar, "clé"));
	EXPECT_FALSE(yamlChild(scalar, "clé").IsMap());
	const YAML::Node sequence = YAML::Load("[1, 2, 3]");
	EXPECT_FALSE(yamlHas(sequence, "clé"));
}

TEST(ErrorPaths, AYamlPrizeWithoutADesignationIsNotCalledNull) {
	// Le piège attrapé par le test ci-dessus, vu depuis un import réel.
	const Area area{"yaml-null"};
	area.write("evt.yml", "rounds:\n"
						  "  - type: normale\n"
						  "    Id: 1\n"
						  "    subGames:\n"
						  "      - type: simple quine\n"
						  "        prizes:\n"
						  "          - value: 25.0\n");
	Event event;
	ASSERT_TRUE(event.importYaml(area.file("evt.yml")));
	ASSERT_EQ(event.sizeRounds(), 1);
	const auto sub = event.getGameRound(0)->getSubRound(0);
	ASSERT_EQ(sub->getPrizes().size(), 1);
	EXPECT_TRUE(sub->getPrizes().front().getDesignation().empty());
	EXPECT_NEAR(sub->getPrizes().front().getValue(), 25.0, 0.001);
}
