
#include "../TestMainHelper.h"
#include "gui/vulkan/VulkanContext.h"


TEST(vulkan_VulkanContext, instantiate) {
	auto& vkContext = evl::gui::vulkan::VulkanContext::get();
	vkContext.init({});
	const auto& data = vkContext.getVkData();
	EXPECT_NE(data.instance, VK_NULL_HANDLE);
	EXPECT_NE(data.physicalDevice, VK_NULL_HANDLE);
	EXPECT_NE(data.device, VK_NULL_HANDLE);
	EXPECT_NE(data.commandPool, VK_NULL_HANDLE);
	vkContext.reset();
}
