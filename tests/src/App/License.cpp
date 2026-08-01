// SPDX-License-Identifier: LGPL-2.1-or-later

#include <gtest/gtest.h>

#include <cstring>

#include "App/License.h"

TEST(License, isLicenseEmpty)
{
    EXPECT_EQ(App::findLicense(""), -1);
}

TEST(License, isLicenseNull)
{
    EXPECT_EQ(App::findLicense(nullptr), -1);
}

TEST(License, isLicenseYesStr)
{
    EXPECT_EQ(App::findLicense("CC_BY_40"), 1);
}

TEST(License, UnknownIdentifier)
{
    int index {App::findLicense("junk")};
    EXPECT_EQ(index, -1);
}

TEST(License, direct)
{
    int posn {App::findLicense("CC_BY_40")};
    const App::LicenseInfo expected {
        "CC_BY_40",
        "Creative Commons Attribution 4.0",
        "https://creativecommons.org/licenses/by/4.0/",
        "CC-BY-4.0",
        App::LicenseCategory::GeneralPurpose
    };
    EXPECT_STREQ(App::licenseItems.at(posn).identifier, expected.identifier);
    EXPECT_STREQ(App::licenseItems.at(posn).fullName, expected.fullName);
    EXPECT_STREQ(App::licenseItems.at(posn).url, expected.url);
    EXPECT_STREQ(App::licenseItems.at(posn).spdxIdentifier, expected.spdxIdentifier);
    EXPECT_EQ(App::licenseItems.at(posn).category, expected.category);
}

TEST(License, findLicenseByIdent)
{
    const App::LicenseInfo& license {App::licenseItems.at(App::findLicense("CC_BY_40"))};

    EXPECT_STREQ(license.identifier, "CC_BY_40");
    EXPECT_STREQ(license.fullName, "Creative Commons Attribution 4.0");
    EXPECT_STREQ(license.url, "https://creativecommons.org/licenses/by/4.0/");
}

TEST(License, everySpdxIdentifierIsRecognized)
{
    const auto& spdx = App::SpdxLicenseList::instance();
    ASSERT_FALSE(spdx.entries().empty()) << "the SPDX license list resource failed to load";

    for (const auto& license : App::licenseItems) {
        if (std::strlen(license.spdxIdentifier) == 0) {
            continue;
        }
        EXPECT_TRUE(spdx.contains(license.spdxIdentifier))
            << license.identifier << " claims SPDX identifier " << license.spdxIdentifier
            << ", which the SPDX license list does not define";
    }
}

TEST(License, spdxListIsQueryable)
{
    const auto& spdx = App::SpdxLicenseList::instance();
    ASSERT_FALSE(spdx.entries().empty());

    const auto* entry = spdx.find("CERN-OHL-S-2.0");
    ASSERT_NE(entry, nullptr);
    EXPECT_FALSE(entry->deprecated);
    EXPECT_TRUE(entry->osiApproved);

    EXPECT_EQ(spdx.find("NotAnSpdxIdentifier"), nullptr);
    EXPECT_FALSE(spdx.contains("NotAnSpdxIdentifier"));
}
