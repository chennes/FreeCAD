// SPDX-License-Identifier: LGPL-2.1-or-later

#include <gtest/gtest.h>
#include "src/App/InitApplication.h"

#include <memory>
#include <QString>

#include <Mod/Start/App/FileUtilities.h>

class FileUtilitiesTest: public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        tests::initApplication();
    }

    void SetUp() override
    {}

    void TearDown() override
    {}


private:
};

TEST_F(FileUtilitiesTest, getMD5Hash)
{
    // Arrange
    std::string baseFile {"/foo/bar/baz"};

    // The expected result is the MD5 sum of the encoded string "file:///foo/bar/baz":
    std::string expectedResult {"5860a733ec3759be22a3473cc26f71d5"};

    // Act - cnvert to a std::string so that the error output on non-match is readable
    auto actualResult = Start::getMD5Hash(baseFile).toStdString();

    // Assert
    EXPECT_EQ(expectedResult, actualResult);
}

TEST_F(FileUtilitiesTest, getUniquePNG)
{
    // Arrange
    std::string baseFile {"/foo/bar/baz"};

    // Act
    auto actualResult = Start::getUniquePNG(baseFile);

    // Assert
    EXPECT_TRUE(actualResult.endsWith("5860a733ec3759be22a3473cc26f71d5.png"));
}
