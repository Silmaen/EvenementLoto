/**
* @author Silmaen
* @date 19/10/2021
*/
#include "core/PaquetCartons.h"
#include <gtest/gtest.h>

#include <fstream>

TEST(PaquetCartons, generation) {
    evl::core::PaquetCartons p;
    p.getNom()= "Mon Super Loto";
    p.generate(200);
    std::ofstream filesave;
    filesave.open("test.sdeg");
    p.write(filesave);
    filesave.close();

    evl::core::PaquetCartons p2;
    std::ifstream fileRead;
    fileRead.open("test.sdeg", std::ios::in);
    p2.read(fileRead);

    EXPECT_STREQ(p.getNom().c_str(), p2.getNom().c_str());
    EXPECT_EQ(p.getNumber(), p2.getNumber());
    EXPECT_STREQ(p.getCartons()[50].asString().c_str(), p2.getCartons()[50].asString().c_str());
}
