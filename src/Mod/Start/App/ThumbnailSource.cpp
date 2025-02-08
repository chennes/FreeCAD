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
#include <QProcess>
#endif

#include "ThumbnailSource.h"
#include "FileUtilities.h"

#include <App/Application.h>

using namespace Start;


ThumbnailSource::ThumbnailSource(QString file)
    : _file(std::move(file))
{}

ThumbnailSourceSignals* ThumbnailSource::signals()
{
    return &_signals;
}

void ThumbnailSource::run()
{
    const QString thumbnailPath = getUniquePNG(_file.toStdString());
    if (!useCachedPNG(thumbnailPath.toStdString(), _file.toStdString())) {
        const ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
            "User parameter:BaseApp/Preferences/Mod/Start");
        const auto f3d = QString::fromUtf8(hGrp->GetASCII("f3d", "f3d").c_str());
        constexpr int resolution = 128;
        QStringList args;
        args << QLatin1String("--config=thumbnail") << QLatin1String("--load-plugins=occt")
             << QLatin1String("--verbose=quiet") << QLatin1String("--output=") + thumbnailPath
             << QLatin1String("--resolution=") + QString::number(resolution) + QLatin1String(",")
                + QString::number(resolution)
             << _file;

        QProcess process;
        process.start(f3d, args);
        process.waitForFinished();
        if (process.exitCode() != 0) {
            return;
        }
    }

    if (QFile thumbnailFile(thumbnailPath); thumbnailFile.exists()) {
        thumbnailFile.open(QIODevice::OpenModeFlag::ReadOnly);
        Q_EMIT _signals.thumbnailAvailable(_file, thumbnailFile.readAll());
    }
}
