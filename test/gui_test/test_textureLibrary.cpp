
#include "../TestMainHelper.h"
#include "gui/vulkan/TextureLibrary.h"
#include "gui/vulkan/VulkanContext.h"

TEST(vulkan_TextureLibrary, loadTexture) {
	evl::gui::vulkan::TextureLibrary textureLib;
	textureLib.loadTexture("resources/textures/eiffel_tower.jpg");
	EXPECT_EQ(textureLib.getTextureId("eiffel_tower"),0);
	auto [data, width, height, channels] = textureLib.getRawPixels("eiffel_tower");
	EXPECT_EQ(width, 0);
	EXPECT_EQ(height, 0);
	EXPECT_EQ(channels, 0);
	EXPECT_TRUE(data.empty());

	const auto bob = textureLib.getOrLoadTextureId("eiffel_tower", "resources/textures/eiffel_tower.svg");
	EXPECT_EQ(bob, 0);
}
