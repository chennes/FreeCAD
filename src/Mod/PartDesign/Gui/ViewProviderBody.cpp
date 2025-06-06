/***************************************************************************
 *   Copyright (c) 2011 Juergen Riegel <FreeCAD@juergen-riegel.net>        *
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
# include <Inventor/actions/SoGetBoundingBoxAction.h>
# include <Inventor/nodes/SoSeparator.h>
# include <Precision.hxx>
# include <QMenu>
#endif

#include <App/Document.h>
#include <App/Origin.h>
#include <App/Part.h>
#include <App/VarSet.h>
#include <Base/Console.h>
#include <Gui/ActionFunction.h>
#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/MDIView.h>
#include <Gui/View3DInventor.h>
#include <Gui/View3DInventorViewer.h>
#include <Gui/ViewProviderCoordinateSystem.h>
#include <Gui/ViewProviderDatum.h>
#include <Mod/PartDesign/App/Body.h>
#include <Mod/PartDesign/App/DatumCS.h>
#include <Mod/PartDesign/App/FeatureSketchBased.h>
#include <Mod/PartDesign/App/FeatureBase.h>

#include "ViewProviderBody.h"
#include "Utils.h"
#include "ViewProvider.h"
#include "ViewProviderDatum.h"


using namespace PartDesignGui;
namespace sp = std::placeholders;

const char* PartDesignGui::ViewProviderBody::BodyModeEnum[] = {"Through","Tip",nullptr};

PROPERTY_SOURCE_WITH_EXTENSIONS(PartDesignGui::ViewProviderBody,PartGui::ViewProviderPart)

ViewProviderBody::ViewProviderBody()
{
    ADD_PROPERTY(DisplayModeBody,((long)0));
    DisplayModeBody.setEnums(BodyModeEnum);

    sPixmap = "PartDesign_Body.svg";

    Gui::ViewProviderOriginGroupExtension::initExtension(this);
}

ViewProviderBody::~ViewProviderBody()
{
}

void ViewProviderBody::attach(App::DocumentObject *pcFeat)
{
    // call parent attach method
    ViewProviderPart::attach(pcFeat);

    //set default display mode
    onChanged(&DisplayModeBody);
}

// TODO on activating the body switch to the "Through" mode (2015-09-05, Fat-Zer)
// TODO different icon in tree if mode is Through (2015-09-05, Fat-Zer)
// TODO drag&drop (2015-09-05, Fat-Zer)
// TODO Add activate () call (2015-09-08, Fat-Zer)

void ViewProviderBody::setDisplayMode(const char* ModeName) {

    //if we show "Through" we must avoid to set the display mask modes, as this would result
    //in going into "tip" mode. When through is chosen the child features are displayed, and all
    //we need to ensure is that the display mode change is propagated to them from within the
    //onChanged() method.
    if(DisplayModeBody.getValue() == 1)
        PartGui::ViewProviderPartExt::setDisplayMode(ModeName);
}

void ViewProviderBody::setOverrideMode(const std::string& mode) {

    //if we are in through mode, we need to ensure that the override mode is not set for the body
    //(as this would result in "tip" mode), it is enough when the children are set to the correct
    //override mode.

    if(DisplayModeBody.getValue() != 0)
        Gui::ViewProvider::setOverrideMode(mode);
    else
        overrideMode = mode;
}

void ViewProviderBody::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    Q_UNUSED(receiver);
    Q_UNUSED(member);
    Gui::ActionFunction* func = new Gui::ActionFunction(menu);

    QAction* act = menu->addAction(tr("Active body"));
    act->setCheckable(true);
    act->setChecked(isActiveBody());
    func->trigger(act, [this]() {
        this->toggleActiveBody();
    });

    Gui::ViewProviderGeometryObject::setupContextMenu(menu, receiver, member); // clazy:exclude=skipped-base-method
}

bool ViewProviderBody::isActiveBody()
{
    auto activeDoc = Gui::Application::Instance->activeDocument();
    if(!activeDoc)
        activeDoc = getDocument();
    auto activeView = activeDoc->setActiveView(this);
    if(!activeView)
        return false;

    if (activeView->isActiveObject(getObject(),PDBODYKEY)){
        return true;
    } else {
        return false;
    }
}

void ViewProviderBody::toggleActiveBody()
{
    if (isActiveBody()) {
        //active body double-clicked. Deactivate.
        Gui::Command::doCommand(Gui::Command::Gui,
                "Gui.ActiveDocument.ActiveView.setActiveObject('%s', None)", PDBODYKEY);
    } else {

        // assure the PartDesign workbench
        if(App::GetApplication().GetUserParameter().GetGroup("BaseApp")->GetGroup("Preferences")->GetGroup("Mod/PartDesign")->GetBool("SwitchToWB", true))
            Gui::Command::assureWorkbench("PartDesignWorkbench");

        // and set correct active objects
        auto* part = App::Part::getPartOfObject ( getObject() );
        if ( part && !isActiveBody() ) {
            Gui::Command::doCommand(Gui::Command::Gui,
                    "Gui.ActiveDocument.ActiveView.setActiveObject('%s',%s)",
                    PARTKEY, Gui::Command::getObjectCmd(part).c_str());
        }

        Gui::Command::doCommand(Gui::Command::Gui,
                "Gui.ActiveDocument.ActiveView.setActiveObject('%s',%s)",
                PDBODYKEY, Gui::Command::getObjectCmd(getObject()).c_str());
    }
}

bool ViewProviderBody::doubleClicked()
{
    toggleActiveBody();
    return true;
}


// TODO To be deleted (2015-09-08, Fat-Zer)
//void ViewProviderBody::updateTree()
//{
//    if (ActiveGuiDoc == NULL) return;
//
//    // Highlight active body and all its features
//    //Base::Console().error("ViewProviderBody::updateTree()\n");
//    PartDesign::Body* body = getObject<PartDesign::Body>();
//    bool active = body->IsActive.getValue();
//    //Base::Console().error("Body is %s\n", active ? "active" : "inactive");
//    ActiveGuiDoc->signalHighlightObject(*this, Gui::Blue, active);
//    std::vector<App::DocumentObject*> features = body->Group.getValues();
//    bool highlight = true;
//    App::DocumentObject* tip = body->Tip.getValue();
//    for (std::vector<App::DocumentObject*>::const_iterator f = features.begin(); f != features.end(); f++) {
//        //Base::Console().error("Highlighting %s: %s\n", (*f)->getNameInDocument(), highlight ? "true" : "false");
//        Gui::ViewProviderDocumentObject* vp = dynamic_cast<Gui::ViewProviderDocumentObject*>(Gui::Application::Instance->getViewProvider(*f));
//        if (vp != NULL)
//            ActiveGuiDoc->signalHighlightObject(*vp, Gui::LightBlue, active ? highlight : false);
//        if (highlight && (tip == *f))
//            highlight = false;
//    }
//}

bool ViewProviderBody::onDelete ( const std::vector<std::string> &) {
    // TODO May be do it conditionally? (2015-09-05, Fat-Zer)
    FCMD_OBJ_CMD(getObject(),"removeObjectsFromDocument()");
    return true;
}

void ViewProviderBody::updateData(const App::Property* prop)
{
    PartDesign::Body* body = getObject<PartDesign::Body>();

    if (prop == &body->Group || prop == &body->BaseFeature) {
        //ensure all model features are in visual body mode
        setVisualBodyMode(true);
    }

    if (prop == &body->Tip) {
        // We changed Tip
        App::DocumentObject* tip = body->Tip.getValue();

        auto features = body->Group.getValues();

        // restore icons
        for (auto feature : features) {
            Gui::ViewProvider* vp = Gui::Application::Instance->getViewProvider(feature);
            if (vp && vp->isDerivedFrom<PartDesignGui::ViewProvider>()) {
                static_cast<PartDesignGui::ViewProvider*>(vp)->setTipIcon(feature == tip);
            }
        }
    }

    PartGui::ViewProviderPart::updateData(prop);
}

void ViewProviderBody::onChanged(const App::Property* prop) {

    if(prop == &DisplayModeBody) {
        auto body = getObject<PartDesign::Body>();

        if ( DisplayModeBody.getValue() == 0 )  {
            //if we are in an override mode we need to make sure to come out, because
            //otherwise the maskmode is blocked and won't go into "through"
            if(getOverrideMode() != "As Is") {
                auto mode = getOverrideMode();
                ViewProvider::setOverrideMode("As Is");
                overrideMode = mode;
            }
            setDisplayMaskMode("Group");
            if(body)
                body->setShowTip(false);
        }
        else {
            if(body)
                body->setShowTip(true);
            if(getOverrideMode() == "As Is")
                setDisplayMaskMode(DisplayMode.getValueAsString());
            else {
                Base::Console().message("Set override mode: %s\n", getOverrideMode().c_str());
                setDisplayMaskMode(getOverrideMode().c_str());
            }
        }

        // #0002559: Body becomes visible upon changing DisplayModeBody
        Visibility.touch();
    }
    else
        unifyVisualProperty(prop);

    PartGui::ViewProviderPartExt::onChanged(prop);
}


void ViewProviderBody::unifyVisualProperty(const App::Property* prop) {

    if (!pcObject || isRestoring()) {
        return;
    }

    if (prop == &Visibility ||
        prop == &Selectable ||
        prop == &DisplayModeBody ||
        prop == &PointColorArray ||
        prop == &ShowPlacement ||
        prop == &LineColorArray) {
        return;
    }

    // Fixes issue 11197. In case of affected projects where the bounding box of a sub-feature
    // is shown allow it to hide it
    if (prop == &BoundingBox) {
        if (BoundingBox.getValue()) {
            return;
        }
    }

    Gui::Document *gdoc = Gui::Application::Instance->getDocument ( pcObject->getDocument() ) ;

    PartDesign::Body *body = static_cast<PartDesign::Body *> ( getObject() );
    auto features = body->Group.getValues();
    for (auto feature : features) {

        if (!feature->isDerivedFrom<PartDesign::Feature>()) {
            continue;
        }

        //copy over the properties data
        if (Gui::ViewProvider* vp = gdoc->getViewProvider(feature)) {
            if (auto fprop = vp->getPropertyByName(prop->getName())) {
                fprop->Paste(*prop);
            }
        }
    }
}

void ViewProviderBody::setVisualBodyMode(bool bodymode) {

    Gui::Document *gdoc = Gui::Application::Instance->getDocument ( pcObject->getDocument() ) ;

    PartDesign::Body *body = static_cast<PartDesign::Body *> ( getObject() );
    auto features = body->Group.getValues();
    for(auto feature : features) {

        if(!feature->isDerivedFrom<PartDesign::Feature>())
            continue;

        auto* vp = static_cast<PartDesignGui::ViewProvider*>(gdoc->getViewProvider(feature));
        if (vp) vp->setBodyMode(bodymode);
    }
}

std::vector< std::string > ViewProviderBody::getDisplayModes() const {

    //we get all display modes and remove the "Group" mode, as this is what we use for "Through"
    //body display mode
    std::vector< std::string > modes = ViewProviderPart::getDisplayModes();
    modes.erase(modes.begin());
    return modes;
}

bool ViewProviderBody::canDropObjects() const
{
    // if the BaseFeature property is marked as hidden or read-only then
    // it's not allowed to modify it.
    auto* body = getObject<PartDesign::Body>();
    if (body->BaseFeature.testStatus(App::Property::Status::Hidden)
        || body->BaseFeature.testStatus(App::Property::Status::ReadOnly)) {
        return false;
    }
    return true;
}

bool ViewProviderBody::canDropObject(App::DocumentObject* obj) const
{
    if (obj->isDerivedFrom<App::VarSet>()) {
        return true;
    }
    else if (obj->isDerivedFrom<App::DatumElement>()) {
        // accept only datums that are not part of a LCS.
        auto* lcs = static_cast<App::DatumElement*>(obj)->getLCS();
        return !lcs;
    }
    else if (obj->isDerivedFrom<App::LocalCoordinateSystem>()) {
        return !obj->isDerivedFrom<App::Origin>();
    }
    else if (!obj->isDerivedFrom<Part::Feature>()) {
        return false;
    }
    else if (PartDesign::Body::findBodyOf(obj)) {
        return false;
    }
    else if (obj->isDerivedFrom (Part::BodyBase::getClassTypeId())) {
        return false;
    }

    App::Part *actPart = PartDesignGui::getActivePart();
    App::Part* partOfBaseFeature = App::Part::getPartOfObject(obj);
    if (partOfBaseFeature && partOfBaseFeature != actPart)
        return false;

    return true;
}

void ViewProviderBody::dropObject(App::DocumentObject* obj)
{
    auto* body = getObject<PartDesign::Body>();
    if (obj->isDerivedFrom<Part::Part2DObject>()
        || obj->isDerivedFrom<App::DatumElement>()
        || obj->isDerivedFrom<App::LocalCoordinateSystem>()) {
        body->addObject(obj);
    }
    else if (PartDesign::Body::isAllowed(obj) && PartDesignGui::isFeatureMovable(obj)) {
        std::vector<App::DocumentObject*> move;
        move.push_back(obj);
        std::vector<App::DocumentObject*> deps = PartDesignGui::collectMovableDependencies(move);
        move.insert(std::end(move), std::begin(deps), std::end(deps));

        PartDesign::Body* source = PartDesign::Body::findBodyOf(obj);
        if (source)
            source->removeObjects(move);
        try {
            body->addObjects(move);
        }
        catch (const Base::Exception& e) {
            e.reportException();
        }
    }
    else if (!body->BaseFeature.getValue()) {
        body->BaseFeature.setValue(obj);
    }

    App::Document* doc  = body->getDocument();
    doc->recompute();

    // check if a proxy object has been created for the base feature
    std::vector<App::DocumentObject*> links = body->Group.getValues();
    for (auto it : links) {
        if (it->isDerivedFrom<PartDesign::FeatureBase>()) {
            PartDesign::FeatureBase* base = static_cast<PartDesign::FeatureBase*>(it);
            if (base && base->BaseFeature.getValue() == obj) {
                Gui::Application::Instance->hideViewProvider(obj);
                break;
            }
        }
    }
}
