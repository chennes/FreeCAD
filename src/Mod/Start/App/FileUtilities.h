// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2025 The FreeCAD Project Association AISBL               *
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/

#ifndef FREECAD_FILEUTILITIES_H
#define FREECAD_FILEUTILITIES_H

#include "../StartGlobal.h"

#include <string>
#include <Base/FileInfo.h>
#include <QString>
#include <QDir>

namespace Start
{

StartExport std::string getThumbnailsImage();

StartExport QString getThumbnailsName();

StartExport QDir getThumbnailsParentDir();

StartExport QString getThumbnailsDir();

StartExport void createThumbnailsDir();

StartExport QString getMD5Hash(const std::string& path);

StartExport QString getUniquePNG(const std::string& path);

StartExport bool useCachedPNG(const std::string& image, const std::string& project);

}  // namespace Start

#endif  // FREECAD_FILEUTILITIES_H
