/***************************************************************************
 *   Copyright (c) 2021 Chris Hennes <chennes@pioneerlibrarysystem.org>    *
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
#endif

#include "DlgCreateNewThemeImp.h"
#include "ui_DlgCreateNewTheme.h"

using namespace Gui::Dialog;

/* TRANSLATOR Gui::Dialog::DlgCreateNewThemeImp */

/**
 *  Constructs a Gui::Dialog::DlgCreateNewThemeImp as a child of 'parent'
 */
DlgCreateNewThemeImp::DlgCreateNewThemeImp(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui_DlgCreateNewTheme)
{
    ui->setupUi(this);
    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &DlgCreateNewThemeImp::onItemChanged);
}


Gui::Dialog::DlgCreateNewThemeImp::~DlgCreateNewThemeImp()
{
}

void Gui::Dialog::DlgCreateNewThemeImp::addThemeTemplate(const std::string& groupName, const std::string& name, bool checkedByDefault)
{
    QTreeWidgetItem* group;
    if (auto foundGroup = _groups.find(groupName); foundGroup != _groups.end()) {
        group = foundGroup->second;
    }
    else {
        group = new QTreeWidgetItem(ui->treeWidget, QStringList(QString::fromStdString(groupName)));
        group->setCheckState(0,checkedByDefault ? Qt::Checked : Qt::Unchecked);
        _groups.insert(std::make_pair(groupName, group));
    }

    auto newItem = new QTreeWidgetItem(group, QStringList(QString::fromStdString(name)));
    newItem->setCheckState(0, checkedByDefault ? Qt::Checked : Qt::Unchecked);
    if (group->checkState(0) != newItem->checkState(0))
        group->setCheckState(0, Qt::PartiallyChecked);
    group->addChild(newItem);
}

std::vector<std::pair<std::string, std::string>> Gui::Dialog::DlgCreateNewThemeImp::selectedTemplates() const
{
    std::vector<std::pair<std::string, std::string>> results;

    for (const auto& group : _groups) {
        auto groupName = group.first;
        auto treeItem = group.second;
        for (int childIndex = 0; childIndex < treeItem->childCount(); ++childIndex) {
            auto child = treeItem->child(childIndex);
            if (child->checkState(0) == Qt::Checked)
                results.push_back(std::make_pair(groupName, child->text(0).toStdString()));
        }
    }

    return results;
}

std::string Gui::Dialog::DlgCreateNewThemeImp::themeName() const
{
    return ui->lineEdit->text().toStdString();
}

void Gui::Dialog::DlgCreateNewThemeImp::onItemChanged(QTreeWidgetItem* item, int column)
{
    const QSignalBlocker blocker(ui->treeWidget);
    if (auto group = item->parent(); group) {
        // Child clicked
        bool firstItemChecked = false;
        for (int childIndex = 0; childIndex < group->childCount(); ++childIndex) {
            auto child = group->child(childIndex);
            if (childIndex == 0) {
                firstItemChecked = child->checkState(0) == Qt::Checked;
            }
            else {
                bool thisItemChecked = child->checkState(0) == Qt::Checked;
                if (firstItemChecked != thisItemChecked) {
                    group->setCheckState(0, Qt::PartiallyChecked);
                    return;
                }
            }
        }
        group->setCheckState(0, firstItemChecked ? Qt::Checked : Qt::Unchecked);
    }
    else {
        // Group clicked:
        auto groupCheckState = item->checkState(0);
        for (int childIndex = 0; childIndex < item->childCount(); ++childIndex) {
            auto child = item->child(childIndex);
            child->setCheckState(0, groupCheckState);
        }
    }
}


#include "moc_DlgCreateNewThemeImp.cpp"
