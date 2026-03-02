// SPDX-License-Identifier: LGPL-2.1-or-later

/***************************************************************************
 *   Copyright (c) 2020 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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


#pragma once

#include <App/PropertyStandard.h>
#include <App/PropertyUnits.h>
#include "FeatureSketchBased.h"

class gp_Dir;
class TopLoc_Location;
class TopoDS_Face;
class TopoDS_Shape;

namespace PartDesign
{

/**
 * Controls which extrusion algorithm version is used, for element-map backwards
 * compatibility with files created before FreeCAD 1.1. Each value corresponds to
 * a version of FreeCAD in which the algorithm changed.
 */
enum ExtrusionVersion : long
{
    PreV11 = 0,  ///< Pre-FreeCAD 1.1 algorithm (default for old files)
    V11 = 1,     ///< FreeCAD 1.1+ algorithm (set by setupObject() for new features)
};

class PartDesignExport FeatureExtrude: public ProfileBased
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesign::FeatureExtrude);

public:
    FeatureExtrude();

    App::PropertyEnumeration SideType;
    App::PropertyEnumeration Type;
    App::PropertyEnumeration Type2;
    App::PropertyLength Length;
    App::PropertyLength Length2;
    App::PropertyAngle TaperAngle;
    App::PropertyAngle TaperAngle2;
    App::PropertyBool UseCustomVector;
    App::PropertyVector Direction;
    App::PropertyBool AlongSketchNormal;
    App::PropertyLength Offset;
    App::PropertyLength Offset2;
    App::PropertyLinkSub ReferenceAxis;
    /**
     * Compatibility property that selects which version of the extrusion algorithm is used,
     * to preserve element-map IDs for files created before FreeCAD 1.1. See issue #25720.
     */
    App::PropertyEnumeration ExtrusionVersion;

    static App::PropertyQuantityConstraint::Constraints signedLengthConstraint;
    static double maxAngle;
    static App::PropertyAngle::Constraints floatAngle;

    /** @name methods override feature */
    //@{
    short mustExecute() const override;
    void setupObject() override;

    const char* getViewProviderName() const override
    {
        return "PartDesignGui::ViewProviderExtrude";
    }
    //@}

    static const char* SideTypesEnums[];
    static const char* ExtrusionVersionEnums[];

protected:
    void onDocumentRestored() override;
    Base::Vector3d computeDirection(const Base::Vector3d& sketchVector, bool inverse);
    bool hasTaperedAngle() const;
    void onChanged(const App::Property* prop) override;

    /**
     * Migrates a pre-V11 Symmetric non-Length feature to use an explicit custom direction,
     * so the mirror plane can always use 'dir' without a runtime version check.
     * Returns true if migration succeeded (or was not needed), false if deferred.
     * See issue #25720.
     */
    bool migrateSymmetricDirection();


    /// Options for buildExtrusion()
    enum class ExtrudeOption
    {
        MakeFace = 1,
        MakeFuse = 2,
        LegacyPocket = 4,
        InverseDirection = 8,
    };

    using ExtrudeOptions = Base::Flags<ExtrudeOption>;

    App::DocumentObjectExecReturn* buildExtrusion(ExtrudeOptions options);

    /**
     * generate an open shell from a given shape
     * by removing the farthest face from the sketchshape in the direction
     * if farthest is nearest (circular) then return the initial shape
     */
    TopoShape makeShellFromUpToShape(TopoShape shape, TopoShape sketchshape, gp_Dir& dir);

    /**
     * Disables settings that are not valid for the current method
     */
    void updateProperties();

    TopoShape generateSingleExtrusionSide(
        const TopoShape& sketchShape,  // The base sketch for this side (global CS)
        const std::string& method,
        double length,
        double taperAngleDeg,
        App::PropertyLinkSub& upToFacePropHandle,       // e.g., &UpToFace or &UpToFace2
        App::PropertyLinkSubList& upToShapePropHandle,  // e.g., &UpToShape or &UpToShape2
        gp_Dir dir,
        double offsetVal,
        bool makeFace,
        const TopoShape& base,      // The base shape for context (global CS)
        TopLoc_Location& invObjLoc  // MUST be passed. Cannot be re-accessed, see #26677
    );
};

}  // namespace PartDesign

ENABLE_BITMASK_OPERATORS(PartDesign::FeatureExtrude::ExtrudeOption)
