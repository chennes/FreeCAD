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

TEST(License, spdxIdentifierDerivedFromLicenseName)
{
    EXPECT_STREQ(App::spdxIdentifierForLicenseName("Creative Commons Attribution 4.0"), "CC-BY-4.0");
    EXPECT_STREQ(App::spdxIdentifierForLicenseName("Public Domain"), "CC0-1.0");

    // Licenses that SPDX does not define resolve to nothing rather than to a wrong answer
    EXPECT_STREQ(App::spdxIdentifierForLicenseName("All rights reserved"), "");

    // As do names FreeCAD does not know
    EXPECT_STREQ(App::spdxIdentifierForLicenseName("not a license"), "");
    EXPECT_STREQ(App::spdxIdentifierForLicenseName(""), "");
    EXPECT_STREQ(App::spdxIdentifierForLicenseName(nullptr), "");
}

TEST(License, recognizesNamesWrittenByOlderVersions)
{
    // Versionless spellings meant 4.0, which is what they were introduced alongside
    EXPECT_STREQ(App::spdxIdentifierForLicenseName("CreativeCommons Attribution"), "CC-BY-4.0");
    EXPECT_STREQ(
        App::spdxIdentifierForLicenseName("CreativeCommons Attribution-ShareAlike"),
        "CC-BY-SA-4.0"
    );
    EXPECT_STREQ(App::spdxIdentifierForLicenseName("Creative Commons Attribution"), "CC-BY-4.0");

    // Spellings people have typed by hand
    EXPECT_STREQ(App::spdxIdentifierForLicenseName("CC BY 3.0"), "CC-BY-3.0");
    EXPECT_STREQ(App::spdxIdentifierForLicenseName("CC-BY 3.0"), "CC-BY-3.0");
    EXPECT_STREQ(
        App::spdxIdentifierForLicenseName("Attribution 4.0 International (CC BY 4.0)"),
        "CC-BY-4.0"
    );
    EXPECT_STREQ(App::spdxIdentifierForLicenseName("CC0"), "CC0-1.0");

    // Names that say nothing definite about a license are deliberately not mapped
    EXPECT_STREQ(App::spdxIdentifierForLicenseName("(same license as FreeCAD)"), "");
}

TEST(License, everyAliasNamesAKnownLicense)
{
    for (const auto& alias : App::licenseAliases) {
        EXPECT_GE(App::findLicense(alias.identifier), 0)
            << "alias " << alias.recordedName << " points at " << alias.identifier
            << ", which is not a license FreeCAD lists";
    }
}

TEST(License, aliasesDoNotShadowCurrentNames)
{
    for (const auto& alias : App::licenseAliases) {
        for (const auto& license : App::licenseItems) {
            EXPECT_STRNE(alias.recordedName, license.fullName)
                << alias.recordedName << " is both a current license name and an alias";
        }
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
