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

#pragma once

#include <QByteArray>
#include <QString>

#include <FCGlobal.h>

class QComboBox;

namespace Gui
{

/**
 * Fills a combo box with the licenses FreeCAD offers, grouped under headings that cannot
 * themselves be selected. Every selectable item carries its App::licenseItems identifier as
 * item data, so callers must read the selection with currentData() rather than by position:
 * the headings and separators mean a combo box index is not a licenseItems index.
 *
 * Superseded licenses are left out unless selectedIdentifier names one of them, so that they
 * stay available to documents already using them without being offered for new work.
 *
 * @param combo the combo box to fill, which is cleared first
 * @param selectedIdentifier identifier to select, empty to select the first entry
 * @param includeOther whether to offer an entry for a license FreeCAD does not list
 */
GuiExport void populateLicenseComboBox(
    QComboBox* combo,
    const QByteArray& selectedIdentifier = {},
    bool includeOther = false
);

/**
 * Adds an entry for a license FreeCAD does not know about and selects it. Used for documents
 * carrying a license written by an older version, a newer version, or by hand.
 */
GuiExport void addUnknownLicenseToComboBox(QComboBox* combo, const QString& licenseName);

/// Identifier carried by the entry standing for a license FreeCAD does not list
GuiExport QByteArray unknownLicenseIdentifier();

}  // namespace Gui
