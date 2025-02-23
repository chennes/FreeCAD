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
#include <Base/Console.h>

using namespace Start;


ThumbnailSource::ThumbnailSource(QString file, ParameterGrp::handle hGrp)
    : _file(std::move(file))
    , _startParameterGrp(hGrp)
{}

ThumbnailSourceSignals* ThumbnailSource::signals()
{
    return &_signals;
}

void ThumbnailSource::run()
{
    const QString thumbnailPath = getUniquePNG(_file.toStdString());
    if (!useCachedPNG(thumbnailPath.toStdString(), _file.toStdString())) {
        Base::Console().Log("Running ThumbnailSource f3d cache update for %s\n",
                            _file.toStdString().c_str());
        auto [f3d, args] = createF3DCall(thumbnailPath);
        QProcess process;
        process.start(f3d, args);
        process.waitForFinished();
        if (process.exitCode() != 0) {
            Base::Console().Log("  -> f3d call failed for %s\n", _file.toStdString().c_str());
            return;
        }
        Base::Console().Log("  -> f3d call completed for %s\n", _file.toStdString().c_str());
    }

    if (QFile thumbnailFile(thumbnailPath); thumbnailFile.exists()) {
        thumbnailFile.open(QIODevice::OpenModeFlag::ReadOnly);
        Q_EMIT _signals.thumbnailAvailable(_file, thumbnailFile.readAll());
    }
}

std::tuple<QString, QStringList> ThumbnailSource::createF3DCall(const QString& thumbnailPath) const
{
    const auto f3d = QString::fromStdString(_startParameterGrp->GetASCII("f3d", "f3d"));
    constexpr int resolution = 128;
    QStringList args;
    args << QLatin1String("--load-plugins=occt") << QLatin1String("--output=") + thumbnailPath
         << QLatin1String("--filename=0") << QLatin1String("--grid=0")
         << QLatin1String("--no-background") << QLatin1String("--max-size=100")
         << QLatin1String("--resolution=") + QString::number(resolution) + QLatin1String(",")
            + QString::number(resolution)
         << _file;
    return std::make_tuple(f3d, args);
}
