/**
 * @file main.cpp
 * @author Silmaen
 * @date 25/09/2022
 * Copyright © 2022 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"
#include <gtest/gtest.h>

int main(int argc, char** argv) {
    startSpdlog();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
