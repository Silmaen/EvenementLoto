/**
* @author Silmaen
* @date 19/10/2021
*/
#include "core/PaquetCartons.h"
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
namespace fs= std::filesystem;
using namespace evl;
using namespace evl::core;

TEST(PaquetCartons, generation) {
    PaquetCartons p;
    p.getNom()= "Mon  Loto";

    RandomNumberGenerator rng;

    p.generate(200, &rng);

    fs::path tmp= fs::temp_directory_path() / "test";
    fs::create_directories(tmp);
    fs::path file= tmp / "test.sdeg";

    std::ofstream filesave;
    filesave.open(file, std::ios::out | std::ios::binary);
    p.write(filesave);
    filesave.close();

    evl::core::PaquetCartons p2;
    std::ifstream fileRead;
    fileRead.open(file, std::ios::in | std::ios::binary);
    p2.read(fileRead);
    fileRead.close();

    EXPECT_STREQ(p.getNom().c_str(), p2.getNom().c_str());
    EXPECT_EQ(p.getNumber(), p2.getNumber());
    EXPECT_STREQ(p.getCartons()[5].asString().c_str(), p2.getCartons()[5].asString().c_str());
    fs::remove_all(tmp);
}
