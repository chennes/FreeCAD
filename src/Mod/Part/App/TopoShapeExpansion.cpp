// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2018 Zheng Lei (realthunder) <realthunder.dev@gmail.com> *
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

#include "TopoShape.h"
#include <App/ElementMap.h>

FC_LOG_LEVEL_INIT("TopoShape", true, 2);  // NOLINT

#if OCC_VERSION_HEX >= 0x070600
using Adaptor3d_HCurve = Adaptor3d_Curve;
using BRepAdaptor_HCurve = BRepAdaptor_Curve;
using BRepAdaptor_HCompCurve = BRepAdaptor_CompCurve;
#endif

using namespace Part;
using namespace Data;

namespace
{
inline void WarnNullInput()
{
    FC_WARN("Null input shape");  // NOLINT
}

inline void HandleNullInput()
{
    FC_THROWM(NullShapeException, "Null input shape");  // NOLINT
}

inline void HandleNullShape()
{
    FC_THROWM(NullShapeException, "Null shape");  // NOLINT
}

#define HANDLE_NULL_SHAPE Error "Replaced by HandleNullShape()"
#define HANDLE_NULL_INPUT Error "Replaced by HandleNullInput()"
#define WARN_NULL_INPUT Error "Replaced by WarnNullInput()"
}


bool TopoShape::hasPendingElementMap() const
{
    return false;
    // Cache not implemented in TNP integration Phase 3 - CCH
    //return !elementMap(false)
    //    && this->_Cache
    //    && (this->_ParentCache || this->_Cache->cachedElementMap);
}


bool TopoShape::canMapElement(const TopoShape &other) const {
    if(isNull() || other.isNull() || this == &other || other.Tag == -1 || Tag == -1)
        return false;
    if(!other.Tag
        && !other.elementMap(false)
        && !other.hasPendingElementMap())
        return false;

    // Cache not implemented in TNP integration Phase 3 - CCH
    //INIT_SHAPE_CACHE();
    //other.INIT_SHAPE_CACHE();
    //_Cache->relations.clear();

    return true;
}


void TopoShape::mapSubElement(const std::vector<TopoShape> &shapes, const char *op)
{
    if (shapes.empty())
        return;

    if (shapeType(true) == TopAbs_COMPOUND) {
        int count = 0;
        for (auto & s : shapes) {
            if (s.isNull())
                continue;
            if (!getSubShape(TopAbs_SHAPE, ++count, true).IsPartner(s._Shape)) {
                count = 0;
                break;
            }
        }
        if (count) {
            std::vector<Data::ElementMap::MappedChildElements> children;
            children.reserve(count*3);
            TopAbs_ShapeEnum types[] = {TopAbs_VERTEX, TopAbs_EDGE, TopAbs_FACE};
            for (unsigned i=0; i<sizeof(types)/sizeof(types[0]); ++i) {
                int offset = 0;
                for (auto & s : shapes) {
                    if (s.isNull())
                        continue;
                    int count = s.countSubShapes(types[i]);
                    if (!count)
                        continue;
                    children.emplace_back();
                    auto & child = children.back();
                    child.indexedName = Data::IndexedName::fromConst(shapeName(types[i]).c_str(), 1);
                    child.offset = offset;
                    offset += count;
                    child.count = count;
                    child.elementMap = s.elementMap();
                    child.tag = s.Tag;
                    if (op)
                        child.postfix = op;
                }
            }
            setMappedChildElements(children);
            return;
        }
    }

    for(auto &shape : shapes)
        mapSubElement(shape,op);
}


void TopoShape::mapSubElement(const TopoShape &other, const char *op, bool forceHasher)
{
    if(!canMapElement(other))
        return;

    if (!getElementMapSize(false) && this->_Shape.IsPartner(other._Shape)) {
        if (!this->Hasher)
            this->Hasher = other.Hasher;
        copyElementMap(other, op);
        return;
    }

    bool warned = false;
    static const std::array<TopAbs_ShapeEnum, 3> types = {TopAbs_VERTEX, TopAbs_EDGE, TopAbs_FACE};

    auto checkHasher = [this](const TopoShape &other) {
        if(Hasher) {
            if(other.Hasher!=Hasher) {
                if(!getElementMapSize(false)) {
                    if(FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
                        FC_WARN("hasher mismatch");
                }else {
                    // FC_THROWM(Base::RuntimeError, "hasher mismatch");
                    FC_ERR("hasher mismatch");
                }
                Hasher = other.Hasher;
            }
        }else
            Hasher = other.Hasher;
    };

    for(auto type : types) {
        auto &shapeMap = _Cache->getInfo(type);
        auto &otherMap = other._Cache->getInfo(type);
        if(!shapeMap.count() || !otherMap.count())
            continue;
        if(!forceHasher && other.Hasher) {
            forceHasher = true;
            checkHasher(other);
        }
        const char *shapetype = shapeName(type).c_str();
        std::ostringstream ss;

        bool forward;
        int count;
        if(otherMap.count()<=shapeMap.count()) {
            forward = true;
            count = otherMap.count();
        }else{
            forward = false;
            count = shapeMap.count();
        }
        for(int k=1;k<=count;++k) {
            int i,idx;
            if(forward) {
                i = k;
                idx = shapeMap.find(_Shape,otherMap.find(other._Shape,k));
                if(!idx) continue;
            } else {
                idx = k;
                i = otherMap.find(other._Shape,shapeMap.find(_Shape,k));
                if(!i) continue;
            }
            Data::IndexedName element = Data::IndexedName::fromConst(shapetype, idx);
            for(auto &v : other.getElementMappedNames(
                     Data::IndexedName::fromConst(shapetype,i),true))
            {
                auto &name = v.first;
                auto &sids = v.second;
                if(sids.size()) {
                    if (!Hasher)
                        Hasher = sids[0].getHasher();
                    else if (!sids[0].isFromSameHasher(Hasher)) {
                        if (!warned) {
                            warned = true;
                            FC_WARN("hasher mismatch");
                        }
                        sids.clear();
                    }
                }
                ss.str("");
                encodeElementName(shapetype[0],name,ss,&sids,op,other.Tag);
                setElementName(element,name,&sids);
            }
        }
    }
}


TopoShape &TopoShape::makeElementCompound(const std::vector<TopoShape>& shapes, const char* op, bool force)
{
    if(!force && shapes.size()==1) {
        *this = shapes[0];
        return *this;
    }

    BRep_Builder builder;
    TopoDS_Compound comp;
    builder.MakeCompound(comp);

    if(shapes.empty()) {
        setShape(comp);
        return *this;
    }

    int count = 0;
    for(auto &s : shapes) {
        if(s.isNull()) {
            WarnNullInput();
            continue;
        }
        builder.Add(comp,s.getShape());
        ++count;
    }
    if(!count)
        HandleNullShape();
    setShape(comp);
    // INIT_SHAPE_CACHE(); // Cache not implemented in TNP integration Phase 3 - CCH

    mapSubElement(shapes,op);
    return *this;
}