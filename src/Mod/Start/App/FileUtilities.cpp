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

#include "PreCompiled.h"
#ifndef _PreComp_
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QUrl>
#endif

#include "FileUtilities.h"

using namespace Start;

std::string Start::getThumbnailsImage()
{
    return "thumbnails/Thumbnail.png";
}

QString Start::getThumbnailsName()
{
#if defined(Q_OS_LINUX)
    return QLatin1String("thumbnails/normal");
#else
    return QLatin1String("FreeCADStartThumbnails");
#endif
}

QDir Start::getThumbnailsParentDir()
{
    return {QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)};
}

QString Start::getThumbnailsDir()
{
    const QDir dir = getThumbnailsParentDir();
    return dir.absoluteFilePath(getThumbnailsName());
}

void Start::createThumbnailsDir()
{
    const QString name = getThumbnailsName();
    const QDir dir(getThumbnailsParentDir());
    if (!dir.exists(name)) {
        dir.mkpath(name);
    }
}

QString Start::getMD5Hash(const std::string& path)
{
    // Use MD5 hash as specified here:
    // https://specifications.freedesktop.org/thumbnail-spec/0.8.0/thumbsave.html
    QUrl url(QString::fromStdString(path));
    url.setScheme(QString::fromLatin1("file"));
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(url.toEncoded());
    QByteArray ba = hash.result().toHex();
    return QString::fromLatin1(ba);
}

QString Start::getUniquePNG(const std::string& path)
{
    QDir dir = getThumbnailsDir();
    QString md5 = getMD5Hash(path) + QLatin1String(".png");
    return dir.absoluteFilePath(md5);
}

bool Start::useCachedPNG(const std::string& image, const std::string& project)
{
    Base::FileInfo f1(image);
    Base::FileInfo f2(project);
    if (!f1.exists()) {
        return false;
    }
    if (!f2.exists()) {
        return false;
    }

    return f1.lastModified() > f2.lastModified();
}
