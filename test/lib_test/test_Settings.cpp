/**
 * @file test_Settings.cpp
 * @author Silmaen
 * @date 03/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"
#include "core/Settings.h"

using namespace evl::core;

TEST(Settings, instantiate) {
	Settings settings;
	EXPECT_FALSE(settings.contains("non_existing_key"));
	EXPECT_EQ(settings.getValue<int>("non_existing_key", 42), 42);
	settings.setValue("test_key_int", 100);
	settings.setValue("test_key_double", 1.0);
	settings.setValue("test_key_double2", 1.00005);
	settings.setValue("test_key_bool", true);
	settings.setValue("test_key_string", std::string("Hello"));
	// Check values
	EXPECT_TRUE(settings.contains("test_key_int"));
	EXPECT_EQ(settings.getValue<int>("test_key_int", 0), 100);
	EXPECT_TRUE(settings.contains("test_key_double"));
	EXPECT_NEAR(settings.getValue<double>("test_key_double", 0), 1.0, 1e-6);
	EXPECT_TRUE(settings.contains("test_key_double2"));
	EXPECT_NEAR(settings.getValue<double>("test_key_double2", 0), 1.00005, 1e-6);
	EXPECT_TRUE(settings.contains("test_key_bool"));
	EXPECT_EQ(settings.getValue<bool>("test_key_bool", false), true);
	EXPECT_TRUE(settings.contains("test_key_string"));
	EXPECT_STREQ(settings.getValue<std::string>("test_key_string", "").c_str(), "Hello");
	// Bad cast (should return default)
	EXPECT_EQ(settings.getValue<double>("test_key_int", 0.0), 0.0);
}

TEST(Settings, RemoveAndClear) {
	Settings settings;
	settings.setValue("test_key_int", 100);
	settings.setValue("test_key_bool", true);
	EXPECT_TRUE(settings.contains("test_key_int"));
	EXPECT_EQ(settings.getValue<int>("test_key_int", 0), 100);
	// Remove key
	settings.remove("test_key_int");
	EXPECT_FALSE(settings.contains("test_key_int"));
	EXPECT_EQ(settings.getValue<int>("test_key_int", 0), 0);
	// Clear all
	settings.clear();
	EXPECT_FALSE(settings.contains("test_key_bool"));
	EXPECT_EQ(settings.getValue<bool>("test_key_bool", false), false);
}

TEST(core_Settings, FileOperations) {
	Settings settings;
	settings.fromFile("non_existing_file.yaml");
	EXPECT_FALSE(settings.contains("test_key_int"));
	EXPECT_EQ(settings.getValue<int>("test_key_int", 0), 0);
	settings.setValue("test_key_int", 100);
	settings.setValue("test_key_double", 1.0);
	settings.setValue("test_key_double2", 1.00005);
	settings.setValue("test_key_bool", true);
	settings.setValue("test_key_string", std::string("Hello"));

	settings.toFile("test_output.yaml");
	settings.clear();
	settings.fromFile("test_output.yaml");

	EXPECT_TRUE(settings.contains("test_key_int"));
	EXPECT_EQ(settings.getValue<int>("test_key_int", 0), 100);
	EXPECT_TRUE(settings.contains("test_key_double"));
	EXPECT_NEAR(settings.getValue<double>("test_key_double", 0), 1.0, 1e-6);
	EXPECT_TRUE(settings.contains("test_key_double2"));
	EXPECT_NEAR(settings.getValue<double>("test_key_double2", 0), 1.00005, 1e-6);
	EXPECT_TRUE(settings.contains("test_key_bool"));
	EXPECT_EQ(settings.getValue<bool>("test_key_bool", false), true);
	EXPECT_TRUE(settings.contains("test_key_string"));
	EXPECT_STREQ(settings.getValue<std::string>("test_key_string", "").c_str(), "Hello");

	// Clean up
	fs::remove("test_output.yaml");
}

TEST(core_Settings, HierarchicalKeys) {
	Settings settings;
	settings.setValue("graphics/resolution/width", 1920);
	settings.setValue("graphics/resolution/height", 1080);
	settings.setValue("audio/volume/master", 0.8);
	settings.setValue("audio/volume/music", 0.5);
	settings.setValue("audio/volume/sfx", 0.7);
	settings.setValue("audio/volume/sfx", 0.7);
	settings.setValue("flat", 0.7);

	settings.toFile("test_hierarchical_output.yaml");
	settings.clear();
	settings.fromFile("test_hierarchical_output.yaml");

	EXPECT_TRUE(settings.contains("graphics/resolution/width"));
	EXPECT_EQ(settings.getValue<int>("graphics/resolution/width", 0), 1920);
	EXPECT_TRUE(settings.contains("graphics/resolution/height"));
	EXPECT_EQ(settings.getValue<int>("graphics/resolution/height", 0), 1080);
	EXPECT_TRUE(settings.contains("audio/volume/master"));
	EXPECT_NEAR(settings.getValue<double>("audio/volume/master", 0.0), 0.8, 1e-6);
	EXPECT_TRUE(settings.contains("audio/volume/music"));
	EXPECT_NEAR(settings.getValue<double>("audio/volume/music", 0.0), 0.5, 1e-6);
	EXPECT_TRUE(settings.contains("audio/volume/sfx"));
	EXPECT_NEAR(settings.getValue<double>("audio/volume/sfx", 0.0), 0.7, 1e-6);

	// Clean up
	fs::remove("test_hierarchical_output.yaml");
}

TEST(core_Settings, Include) {
	Settings baseSettings;
	baseSettings.setValue("graphics/resolution/width", 1920);
	baseSettings.setValue("graphics/resolution/height", 1080);
	baseSettings.setValue("audio/volume/master", 0.8);

	Settings newSettings;
	newSettings.setValue("audio/volume/music", 0.5);
	newSettings.setValue("audio/volume/sfx", 0.7);

	newSettings.include(baseSettings, "base");

	EXPECT_TRUE(newSettings.contains("base/graphics/resolution/width"));
	EXPECT_EQ(newSettings.getValue<int>("base/graphics/resolution/width", 0), 1920);
	EXPECT_TRUE(newSettings.contains("base/graphics/resolution/height"));
	EXPECT_EQ(newSettings.getValue<int>("base/graphics/resolution/height", 0), 1080);
	EXPECT_TRUE(newSettings.contains("base/audio/volume/master"));
	EXPECT_EQ(newSettings.getValue<double>("base/audio/volume/master", 0.0), 0.8);
	EXPECT_TRUE(newSettings.contains("audio/volume/music"));
	EXPECT_EQ(newSettings.getValue<double>("audio/volume/music", 0.0), 0.5);
	EXPECT_TRUE(newSettings.contains("audio/volume/sfx"));
	EXPECT_EQ(newSettings.getValue<double>("audio/volume/sfx", 0.0), 0.7);
}

TEST(core_Settings, IncludeMissing) {
	Settings baseSettings;
	baseSettings.setValue("graphics/resolution/width", 1920);
	baseSettings.setValue("graphics/resolution/height", 1080);
	baseSettings.setValue("audio/volume/master", 0.8);

	Settings newSettings;
	newSettings.setValue("graphics/resolution/width", 1280);// Existing key
	newSettings.setValue("audio/volume/music", 0.5);

	newSettings.includeMissing(baseSettings, "base");

	EXPECT_TRUE(newSettings.contains("graphics/resolution/width"));
	EXPECT_EQ(newSettings.getValue<int>("graphics/resolution/width", 0), 1280);// Should not be overwritten
	EXPECT_TRUE(newSettings.contains("base/graphics/resolution/height"));
	EXPECT_EQ(newSettings.getValue<int>("base/graphics/resolution/height", 0), 1080);
	EXPECT_TRUE(newSettings.contains("base/audio/volume/master"));
	EXPECT_NEAR(newSettings.getValue<double>("base/audio/volume/master", 0.0), 0.8, 1e-6);
	EXPECT_TRUE(newSettings.contains("audio/volume/music"));
	EXPECT_NEAR(newSettings.getValue<double>("audio/volume/music", 0.0), 0.5, 1e-6);
}

TEST(core_Settings, Extract) {
	Settings settings;
	settings.setValue("graphics/resolution/width", 1920);
	settings.setValue("graphics/resolution/height", 1080);
	settings.setValue("audio/volume/master", 0.8);
	settings.setValue("audio/volume/music", 0.5);
	settings.setValue("audio/volume/sfx", 0.7);
	const Settings audioSettings = settings.extract("audio");
	EXPECT_TRUE(audioSettings.contains("volume/master"));
	EXPECT_NEAR(audioSettings.getValue<double>("volume/master", 0.0), 0.8, 1e-6);
	EXPECT_TRUE(audioSettings.contains("volume/music"));
	EXPECT_NEAR(audioSettings.getValue<double>("volume/music", 0.0), 0.5, 1e-6);
	EXPECT_TRUE(audioSettings.contains("volume/sfx"));
	EXPECT_NEAR(audioSettings.getValue<double>("volume/sfx", 0.0), 0.7, 1e-6);
	EXPECT_NEAR(audioSettings.getValue<float>("volume/sfx", 0.0), 0.7, 1e-6);
	EXPECT_FALSE(audioSettings.contains("graphics/resolution/width"));
	EXPECT_FALSE(audioSettings.contains("graphics/resolution/height"));
}
