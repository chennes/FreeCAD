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
#endif

#include <boost/filesystem.hpp>

#include "ThemeManager.h"
#include "Base/Metadata.h"

#include <App/Application.h>

using namespace Gui;
using namespace xercesc;
namespace fs = boost::filesystem;


Theme::Theme(const boost::filesystem::path& path, const Base::Metadata& metadata) : 
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

void Theme::apply() const
{
}



ThemeManager::ThemeManager(const std::vector<boost::filesystem::path> &themePaths) :
	_themePaths (themePaths)
{
	rescan();
}

void ThemeManager::rescan()
{
	for (const auto& path : _themePaths) {
		if (fs::exists(path) && fs::is_directory(path)) {
			for (const auto& mod : fs::directory_iterator(path)) {
				if (fs::is_directory(mod)) {
					// Does this mod have a package.xml file? (required for themes)
					auto packageMetadataFile = mod / "package.xml";
					if (fs::exists(packageMetadataFile) && fs::is_regular_file(packageMetadataFile)) {
						try {
							Base::Metadata metadata(packageMetadataFile);
							auto content = metadata.content();
							for (const auto& item : content) {
								if (item.first == "theme") {
									Theme newTheme(mod, item.second);
									_themes.insert(std::make_pair(newTheme.name(), newTheme));
								}
							}
						}
						catch (...) {
							// Failed to read the metadata, or to create the theme based on it...
						}
					}
				}
			}
		}
	}
}

std::vector<std::string> ThemeManager::themeNames() const
{
	std::vector<std::string> names;
	for (const auto &theme : _themes)
		names.push_back(theme.first);
	return names;
}

std::vector<const Theme *> ThemeManager::themes() const
{
	return std::vector<const Theme *>();
}

void ThemeManager::apply(const std::string& themeName) const
{
	if (auto theme = _themes.find(themeName); theme != _themes.end())
		theme->second.apply();
	else
		throw std::runtime_error("No such theme: " + themeName);
}

void ThemeManager::apply(const Theme& theme) const
{
}

void ThemeManager::save(const std::string& name, const std::string& templateFile, bool compress)
{
}

