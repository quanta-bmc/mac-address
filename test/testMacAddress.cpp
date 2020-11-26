#include <gtest/gtest.h>
#include "../mac-address.hpp"

TEST (macAddress, macAddressAddOne) {
    std::string testMac = "00:11:22:33:44:55";
    std::string ansMac  = "00:11:22:33:44:56";
    EXPECT_EQ(ansMac,macAddressAddOne(testMac));
}

TEST (macAddressCornerCase, macAddressAddOne) {
    std::string testMac = "00:11:22:33:44:ff";
    std::string ansMac  = "00:11:22:33:45:00";
    EXPECT_EQ(ansMac,macAddressAddOne(testMac));
}

int main(int argc, char *argv[]){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}