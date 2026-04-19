// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2026 The FreeCAD project association AISBL
// SPDX-FileNotice: Part of the FreeCAD project.

#include <gtest/gtest.h>

#include "Gui/Navigation/PythonNavigationStyle.h"

#include <src/App/InitApplication.h>

using namespace Gui;

class NavigationStyleRegistryTest: public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        tests::initApplication();
    }
};

TEST_F(NavigationStyleRegistryTest, RegistryStartsEmpty)
{
    PythonNavigationStyleRegistry registry;
    auto styles = registry.getStyles();
    EXPECT_TRUE(styles.empty());
}

TEST_F(NavigationStyleRegistryTest, AddStyleAddsAStyle)
{
    // Arrange
    PythonNavigationStyleRegistry registry;
    auto styles = registry.getStyles();
    EXPECT_TRUE(styles.empty());
    Py::Object obj {Py::String("My great style's class")};

    // Act
    registry.addStyle("My great style", obj);

    // Assert
    styles = registry.getStyles();
    EXPECT_FALSE(styles.empty());
    auto style = registry.getStyle("My great style");
    EXPECT_EQ(style.ptr(), obj.ptr());
}

TEST_F(NavigationStyleRegistryTest, AddDuplicateStyleNameThrows)
{
    // Arrange
    PythonNavigationStyleRegistry registry;
    auto styles = registry.getStyles();
    EXPECT_TRUE(styles.empty());
    Py::Object class1 {Py::String("My great style's class")};
    registry.addStyle("My great style", class1);
    Py::Object class2 {Py::String("My other great style's class")};

    // Act and assert
    EXPECT_ANY_THROW(
        registry.addStyle("My great style", class2)  // Same name, though...
    );
}

TEST_F(NavigationStyleRegistryTest, UnknownStyleReturnsNone)
{
    // Arrange
    PythonNavigationStyleRegistry registry;
    Py::Object obj {Py::String("My great style's class")};
    registry.addStyle("My great style", obj);

    // Act
    auto style = registry.getStyle("My not-so-great style");

    // Assert
    EXPECT_EQ(style.ptr(), Py::None().ptr());
}

TEST_F(NavigationStyleRegistryTest, RemoveStyleRemovesAStyle)
{
    // Arrange
    PythonNavigationStyleRegistry registry;
    Py::Object class1 {Py::String("My great style's class")};
    registry.addStyle("My great style", class1);
    Py::Object class2 {Py::String("My other great style's class")};
    registry.addStyle("My other great style", class2);

    // Act
    registry.removeStyle("My great style");

    // Assert
    auto style = registry.getStyle("My great style");
    EXPECT_EQ(style.ptr(), Py::None().ptr());
    auto styles = registry.getStyles();
    EXPECT_EQ(styles.size(), 1);
    auto remainingStyle = registry.getStyle("My other great style");
    EXPECT_NE(remainingStyle.ptr(), Py::None().ptr());
}

TEST_F(NavigationStyleRegistryTest, GetUserFriendlyNamesIncludesPythonStyles)
{
    // Arrange
    auto originalStyles = Gui::UserNavigationStyle::getUserFriendlyNames();
    Py::Object class1 {Py::String("Glitter rainbows class")};

    // Act -- Add a new style and get the list of names
    Gui::PythonNavigationStyle::registry().addStyle("Glitter rainbows", class1);
    auto newStyles = Gui::UserNavigationStyle::getUserFriendlyNames();

    // Assert
    EXPECT_EQ(originalStyles.size() + 1, newStyles.size());
    EXPECT_NE(std::ranges::find(newStyles, "Glitter rainbows"), originalStyles.end());

    // Act -- Remove that style and get the list of names
    Gui::PythonNavigationStyle::registry().removeStyle("Glitter rainbows");
    auto revertedStyles = Gui::UserNavigationStyle::getUserFriendlyNames();

    // Assert
    EXPECT_EQ(originalStyles.size(), revertedStyles.size());
    EXPECT_EQ(std::ranges::find(revertedStyles, "Glitter rainbows"), originalStyles.end());
}
