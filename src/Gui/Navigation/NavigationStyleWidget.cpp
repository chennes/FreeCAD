// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2026 The FreeCAD project association AISBL
// SPDX-FileNotice: Part of the FreeCAD project.

/******************************************************************************
 *                                                                            *
 *   FreeCAD is free software: you can redistribute it and/or modify          *
 *   it under the terms of the GNU Lesser General Public License as           *
 *   published by the Free Software Foundation, either version 2.1            *
 *   of the License, or (at your option) any later version.                   *
 *                                                                            *
 *   FreeCAD is distributed in the hope that it will be useful,               *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty              *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *   See the GNU Lesser General Public License for more details.              *
 *                                                                            *
 *   You should have received a copy of the GNU Lesser General Public         *
 *   License along with FreeCAD. If not, see https://www.gnu.org/licenses     *
 *                                                                            *
 ******************************************************************************/

// This widget's interface is based on original work by triplus in 2016-2018. While
// essentially none of the original code survived intact (it was written in Python),
// the interface implemented here is from Triplus's work on the Tux mod, originally in
// NavigationIndicatorGui.py


#include "NavigationStyleWidget.h"

#include "NavigationStyle.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QMenu>

using namespace Gui;

NavigationStyleWidget::NavigationStyleWidget(QWidget* parent)
    : QPushButton(parent)
    , WindowParameter("NavigationStyle")
{
    setFlat(true);
    setText(qApp->translate("Gui::MainWindow", "Navigation Style"));
    setMinimumWidth(120);

    auto* menu = new QMenu(this);
    auto* actionGrp = new QActionGroup(menu);

    auto setAction = [&](const std::string& navStyleMapEntry) mutable {
        QAction* action = menu->addAction(QString::fromStdString(navStyleMapEntry));
        actionGrp->addAction(action);
        action->setCheckable(true);
    };

    auto navStyleEntries = UserNavigationStyle::getUserFriendlyNames();
    std::ranges::for_each(navStyleEntries, setAction);
}
