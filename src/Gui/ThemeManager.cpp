/***************************************************************************
 *   Copyright (c) 2021 Chris Hennes <chennes@pioneerlibrarysystem.org>    *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"

#ifndef _PreComp_
# include <memory>
# include <string_view>
# include <mutex>
#endif

#include <boost/filesystem.hpp>

#include "ThemeManager.h"
#include "Base/Metadata.h"
#include "Base/Parameter.h"
#include "Base/Interpreter.h"
#include "Base/Console.h"

#include <App/Application.h>

#include <ctime> // For generating a timestamped filename


using namespace Gui;
using namespace xercesc;
namespace fs = boost::filesystem;


Theme::Theme(const fs::path& path, const Base::Metadata& metadata) : 
    _path(path), _metadata (metadata)
{
    if (!fs::exists(_path)) {
        throw std::runtime_error{ "Cannot access " + path.string() };
    }
}

std::string Theme::name() const
{
    return _metadata.name();
}

bool Theme::apply() const
{
    // Run the pre.FCMacro, if it exists: if it raises an exception, abort the process
    auto preMacroPath = _path / "pre.FCMacro";
    if (fs::exists(preMacroPath)) {
        try {
            Base::Interpreter().runFile(preMacroPath.string().c_str(), false);
        }
        catch (...) {
            Base::Console().Message("Theme application aborted by the theme's pre.FCMacro");
            return false;
        }
    }

    // Back up the old config file
    auto originalConfig = fs::path(App::Application::getUserAppDataDir()) / "user.cfg";
    auto savedThemesDirectory = fs::path(App::Application::getUserAppDataDir()) / "SavedThemes";
    auto backupFile = savedThemesDirectory / "user.cfg.backup";
    try {
        fs::remove(backupFile);
    }
    catch (...) {}
    App::GetApplication().GetUserParameter().SaveDocument(backupFile.string().c_str());

    // Apply the config settings
    applyConfigChanges();

    // Run the Post.FCMacro, if it exists
    auto postMacroPath = _path / "post.FCMacro";
    if (fs::exists(postMacroPath)) {
        try {
            Base::Interpreter().runFile(postMacroPath.string().c_str(), false);
        }
        catch (...) {
            Base::Console().Message("Theme application reverted by the theme's post.FCMacro");
            App::GetApplication().GetUserParameter().LoadDocument(backupFile.string().c_str());
            return false;
        }
    }

    return true;
}

void Theme::applyConfigChanges() const
{
    auto configFile = _path / (_metadata.name() + ".cfg");
    if (fs::exists(configFile)) {
        ParameterManager newParameters;
        newParameters.LoadDocument(configFile.string().c_str());
        auto baseAppGroup = App::GetApplication().GetUserParameter().GetGroup("BaseApp");
        newParameters.GetGroup("BaseApp")->copyTo(baseAppGroup);
    }
}



ThemeManager::ThemeManager()
{
    auto modPath = fs::path(App::Application::getUserAppDataDir()) / "Mod";
    auto savedPath = fs::path(App::Application::getUserAppDataDir()) / "SavedThemes";
    auto resourcePath = fs::path(App::Application::getResourceDir()) / "Themes";
    _themePaths.push_back(resourcePath);
    _themePaths.push_back(modPath);
    _themePaths.push_back(savedPath);
    rescan();

    // Housekeeping:
    DeleteOldBackups();
}

void ThemeManager::rescan()
{
    std::lock_guard<std::mutex> lock(_mutex);
    for (const auto& path : _themePaths) {
        if (fs::exists(path) && fs::is_directory(path)) {
            FindThemesInPackage(path);
            for (const auto& mod : fs::directory_iterator(path)) {
                if (fs::is_directory(mod)) {
                    FindThemesInPackage(mod);
                }
            }
        }
    }
}

void Gui::ThemeManager::FindThemesInPackage(const fs::path& mod)
{
    auto packageMetadataFile = mod / "package.xml";
    if (fs::exists(packageMetadataFile) && fs::is_regular_file(packageMetadataFile)) {
        try {
            Base::Metadata metadata(packageMetadataFile);
            auto content = metadata.content();
            for (const auto& item : content) {
                if (item.first == "theme") {
                    Theme newTheme(mod / item.second.name(), item.second);
                    _themes.insert(std::make_pair(newTheme.name(), newTheme));
                }
            }
        }
        catch (...) {
            // Failed to read the metadata, or to create the theme based on it...
            Base::Console().Error(("Failed to read " + packageMetadataFile.string()).c_str());
        }
    }
}

std::vector<std::string> ThemeManager::themeNames() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::vector<std::string> names;
    for (const auto &theme : _themes)
        names.push_back(theme.first);
    return names;
}

bool ThemeManager::apply(const std::string& themeName) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (auto theme = _themes.find(themeName); theme != _themes.end()) {
        BackupCurrentConfig();
        return theme->second.apply();
    }
    else {
        throw std::runtime_error("No such theme: " + themeName);
    }
}

void copyTemplateParameters(Base::Reference<ParameterGrp> templateGroup, const std::string &path, Base::Reference<ParameterGrp> outputGroup)
{
    auto userParameterHandle = App::GetApplication().GetParameterGroupByPath(path.c_str());

    auto boolMap = templateGroup->GetBoolMap();
    for (const auto& kv : boolMap) {
        auto currentValue = userParameterHandle->GetBool(kv.first.c_str(), kv.second);
        outputGroup->SetBool(kv.first.c_str(), currentValue);
    }

    auto intMap = templateGroup->GetIntMap();
    for (const auto& kv : intMap) {
        auto currentValue = userParameterHandle->GetInt(kv.first.c_str(), kv.second);
        outputGroup->SetInt(kv.first.c_str(), currentValue);
    }

    auto uintMap = templateGroup->GetUnsignedMap();
    for (const auto& kv : uintMap) {
        auto currentValue = userParameterHandle->GetUnsigned(kv.first.c_str(), kv.second);
        outputGroup->SetUnsigned(kv.first.c_str(), currentValue);
    }

    auto floatMap = templateGroup->GetFloatMap();
    for (const auto& kv : floatMap) {
        auto currentValue = userParameterHandle->GetFloat(kv.first.c_str(), kv.second);
        outputGroup->SetFloat(kv.first.c_str(), currentValue);
    }

    auto asciiMap = templateGroup->GetASCIIMap();
    for (const auto& kv : asciiMap) {
        auto currentValue = userParameterHandle->GetASCII(kv.first.c_str(), kv.second.c_str());
        outputGroup->SetASCII(kv.first.c_str(), currentValue.c_str());
    }

    // Recurse...
    auto templateSubgroups = templateGroup->GetGroups();
    for (auto& templateSubgroup : templateSubgroups) {
        std::string sgName = templateSubgroup->GetGroupName();
        auto outputSubgroupHandle = outputGroup->GetGroup(sgName.c_str());
        copyTemplateParameters(templateSubgroup, path + "/" + sgName, outputSubgroupHandle);
    }
}

void copyTemplateParameters(/*const*/ ParameterManager &templateParameterManager, ParameterManager &outputParameterManager)
{
    auto groups = templateParameterManager.GetGroups();
    for (auto& group : groups) {
        std::string name = group->GetGroupName();
        auto groupHandle = outputParameterManager.GetGroup(name.c_str());
        copyTemplateParameters(group, "User parameter:" + name, groupHandle);
    }
}

void ThemeManager::save(const std::string& name, const std::vector<TemplateFile> &templates)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto savedThemesDirectory = fs::path(App::Application::getUserAppDataDir()) / "SavedThemes";
    fs::path themeDirectory(savedThemesDirectory / name);
    if (fs::exists(themeDirectory) && !fs::is_directory(themeDirectory))
        throw std::runtime_error("Cannot create " + savedThemesDirectory.string() + ": file with that name exists already");

    if (!fs::exists(themeDirectory))
        fs::create_directories(themeDirectory);

    // Create or update the saved user themes package.xml metadata file
    std::unique_ptr<Base::Metadata> metadata;
    if (fs::exists(savedThemesDirectory / "package.xml")) {
        metadata = std::make_unique<Base::Metadata>(savedThemesDirectory / "package.xml");
    }
    else {
        // Create and set all of the required metadata to make it easier for Theme authors to copy this
        // file into their theme distributions.
        metadata = std::make_unique<Base::Metadata>();
        metadata->setName("User-Saved Themes");
        metadata->setDescription("Generated automatically -- edits may be lost when saving new themes");
        metadata->setVersion(1);
        Base::Meta::Contact c;
        c.name = "No Maintainer";
        c.email = "email@freecadweb.org";
        metadata->addMaintainer(c);
        Base::Meta::License l;
        l.file = "(Unspecified)";
        l.name = "(Unspecified)";
        metadata->addLicense(l);
        Base::Meta::Url u;
        u.location = "https://github.com/FreeCAD/FreeCAD";
        u.type = Base::Meta::UrlType::repository;
        metadata->addUrl(u);
    }
    Base::Metadata newThemeMetadata;
    newThemeMetadata.setName(name);
    metadata->addContentItem("theme", newThemeMetadata);
    metadata->write(savedThemesDirectory / "package.xml");

    // Create the config file
    ParameterManager outputParameterManager;
    outputParameterManager.CreateDocument();
    for (const auto& t : templates) {
        ParameterManager templateParameterManager;
        templateParameterManager.LoadDocument(t.path.string().c_str());
        copyTemplateParameters(templateParameterManager, outputParameterManager);
    }
    auto cfgFilename = savedThemesDirectory / name / (name + ".cfg");
    outputParameterManager.SaveDocument(cfgFilename.string().c_str());

    // Future work: copy any referenced auxilliary files that we recognize
}

// Needed until we support only C++20 and above and can use std::string's built-in ends_with()
bool fc_ends_with (std::string_view str, std::string_view suffix)
{
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::vector<ThemeManager::TemplateFile> scanForTemplateFiles(const std::string &groupName, const fs::path& entry)
{
    std::vector<ThemeManager::TemplateFile> templateFiles;
    if (fs::exists(entry)) {
        if (fs::is_directory(entry)) {
            std::string subgroupName = groupName + "/" + entry.filename().string();
            for (const auto& subentry : fs::directory_iterator(entry)) {
                auto contents = scanForTemplateFiles(subgroupName, subentry);
                std::copy(contents.begin(), contents.end(), std::back_inserter(templateFiles));
            }
        }
        else if (fs::is_regular(entry)) {
            auto filename = entry.filename().string();
            if (fc_ends_with(filename, "_theme_template.cfg")) { //19 chars
                auto name = filename.substr(0,filename.length() - 19);
                std::replace(name.begin(), name.end(), '_', ' ');
                templateFiles.emplace_back(ThemeManager::TemplateFile{ groupName, name, entry });
            }
        }
    }
    return templateFiles;
}

std::vector<ThemeManager::TemplateFile> ThemeManager::templateFiles(bool rescan)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_templateFiles.empty() && !rescan)
        return _templateFiles;

    // Locate all of the template files available on this system
    // Template files end in "_theme_template.cfg" -- They are located in:
    // * $INSTALL_DIR/data/Gui/ThemeTemplates/*
    // * $DATA_DIR/Mod/*

    auto resourcePath = fs::path(App::Application::getResourceDir()) / "Gui" / "ThemeTemplates";
    auto modPath = fs::path(App::Application::getUserAppDataDir()) / "Mod";

    std::string group = "Built-In";
    if (fs::exists(resourcePath) && fs::is_directory(resourcePath)) {
        const auto localFiles = scanForTemplateFiles(group, resourcePath);
        std::copy(localFiles.begin(), localFiles.end(), std::back_inserter(_templateFiles));
    }

    if (fs::exists(modPath) && fs::is_directory(modPath)) {
        for (const auto& mod : fs::directory_iterator(modPath)) {
            group = mod.path().filename().string();
            const auto localFiles = scanForTemplateFiles(group, mod);
            std::copy(localFiles.begin(), localFiles.end(), std::back_inserter(_templateFiles));
        }
    }
    
    return _templateFiles;
}

void Gui::ThemeManager::BackupCurrentConfig() const
{
    auto backupDirectory = fs::path(App::Application::getUserAppDataDir()) / "SavedThemes" / "Backups";
    fs::create_directories(backupDirectory);

    // Create a timestamped filename:
    auto time = std::time(nullptr);
    std::ostringstream timestampStream;
    timestampStream << "user." << time << ".cfg";
    auto filename = backupDirectory / timestampStream.str();

    // Save the current config:
    App::GetApplication().GetUserParameter().SaveDocument(filename.string().c_str());
}

void Gui::ThemeManager::DeleteOldBackups() const
{
    constexpr auto oneWeek = 60.0 * 60.0 * 24.0 * 7.0;
    const auto now = std::time(nullptr);
    auto backupDirectory = fs::path(App::Application::getUserAppDataDir()) / "SavedThemes" / "Backups";
    if (fs::exists(backupDirectory) && fs::is_directory(backupDirectory)) {
        for (const auto& backup : fs::directory_iterator(backupDirectory)) {
            if (std::difftime(now, fs::last_write_time(backup)) > oneWeek) {
                try {
                    fs::remove(backup);
                }
                catch (...) {}
            }
        }
    }
}