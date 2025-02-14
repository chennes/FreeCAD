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

#ifndef FREECAD_THUMBNAILSOURCE_H
#define FREECAD_THUMBNAILSOURCE_H

#include "../StartGlobal.h"

#include <QByteArray>
#include <QRunnable>
#include <QString>
#include <QStringList>
#include <QObject>

#include <App/Application.h>
#include <Base/Parameter.h>

namespace Start
{

class StartExport ThumbnailSourceSignals: public QObject
{
    Q_OBJECT
public:
Q_SIGNALS:

    void thumbnailAvailable(const QString& file, const QByteArray& data);
};

class StartExport ThumbnailSource: public QRunnable
{
public:
    explicit ThumbnailSource(QString file,
                             ParameterGrp::handle = App::GetApplication().GetParameterGroupByPath(
                                 "User parameter:BaseApp/Preferences/Mod/Start"));

    void run() override;

    ThumbnailSourceSignals* signals();

protected:
    std::tuple<QString, QStringList> createF3DCall(const QString& thumbnailPath) const;

private:
    QString _file;
    ThumbnailSourceSignals _signals;
    ParameterGrp::handle _startParameterGrp;
};

}  // namespace Start

#endif  // FREECAD_THUMBNAILSOURCE_H
