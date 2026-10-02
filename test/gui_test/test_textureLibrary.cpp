
#include "../TestMainHelper.h"
#include "gui/vulkan/TextureLibrary.h"
#include "gui/vulkan/VulkanContext.h"

#include <filesystem>
#include <fstream>

TEST(vulkan_TextureLibrary, loadTexture) {
	evl::gui::vulkan::TextureLibrary textureLib;
	textureLib.loadTexture("resources/textures/eiffel_tower.jpg");
	EXPECT_EQ(textureLib.getTextureId("eiffel_tower"), 0);
	auto [data, width, height, channels] = textureLib.getRawPixels("eiffel_tower");
	EXPECT_EQ(width, 0);
	EXPECT_EQ(height, 0);
	EXPECT_EQ(channels, 0);
	EXPECT_TRUE(data.empty());

	const auto bob = textureLib.getOrLoadTextureId("eiffel_tower", "resources/textures/eiffel_tower.svg");
	EXPECT_EQ(bob, 0);
}

TEST(vulkan_TextureLibrary, refusesWhatIsNotAnImage) {
	// Un logo mal saisi dans la configuration d'un événement, c'est-à-dire un chemin
	// quelconque : il ne doit rien charger et surtout rien emporter.
	const auto tmp = std::filesystem::temp_directory_path() / "evl-textures";
	create_directories(tmp);
	evl::gui::vulkan::TextureLibrary library;

	// Un fichier qui n'existe pas.
	library.loadTexture("logo", tmp / "absent.png");
	EXPECT_EQ(library.getTextureId("logo"), 0U);

	// Un fichier qui existe et n'est pas une image, sous une extension d'image.
	const auto fake = tmp / "faux.png";
	{
		std::ofstream out(fake, std::ios::out | std::ios::binary);
		out << "ceci n'est pas un PNG, juste du texte";
	}
	library.loadTexture("logo", fake);
	EXPECT_EQ(library.getTextureId("logo"), 0U);

	// Un SVG qui n'en est pas un.
	const auto fakeSvg = tmp / "faux.svg";
	{
		std::ofstream out(fakeSvg);
		out << "<html>pas du svg</html>";
	}
	library.loadTexture("logo", fakeSvg);
	EXPECT_EQ(library.getTextureId("logo"), 0U);

	// Un fichier vide.
	const auto empty = tmp / "vide.png";
	{ const std::ofstream out(empty); }
	library.loadTexture("logo", empty);
	EXPECT_EQ(library.getTextureId("logo"), 0U);

	// Une extension inconnue : rien n'est tenté.
	const auto odd = tmp / "lot.xyz";
	{
		std::ofstream out(odd);
		out << "peu importe";
	}
	library.loadTexture("logo", odd);
	EXPECT_EQ(library.getTextureId("logo"), 0U);

	// Un répertoire.
	library.loadTexture("logo", tmp);
	EXPECT_EQ(library.getTextureId("logo"), 0U);

	remove_all(tmp);
}

TEST(vulkan_TextureLibrary, askingForWhatWasNeverLoaded) {
	const evl::gui::vulkan::TextureLibrary library;
	EXPECT_EQ(library.getTextureId("jamais_chargé"), 0U);
	const auto pixels = library.getRawPixels("jamais_chargé");
	EXPECT_TRUE(pixels.data.empty());
	EXPECT_EQ(pixels.width, 0U);
}

TEST(vulkan_TextureLibrary, reloadingAnEmptyLibraryIsHarmless) {
	evl::gui::vulkan::TextureLibrary library;
	// Rien à recharger après la perte du périphérique : ce n'est pas une erreur.
	library.reload();
	EXPECT_EQ(library.getTextureId("quoi que ce soit"), 0U);
}

TEST(vulkan_TextureLibrary, loadingAFolderThatIsNotOne) {
	evl::gui::vulkan::TextureLibrary library;
	library.loadFolder("/n/existe/pas/du/tout");
	EXPECT_EQ(library.getTextureId("quoi que ce soit"), 0U);
}
