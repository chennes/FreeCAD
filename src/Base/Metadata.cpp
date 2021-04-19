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
 *   License along with this library; see the file LICENSE.html. If not,   *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#include "PreCompiled.h"

#ifndef _PreComp_
# include <memory>
#endif

#include "Metadata.h"

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "XMLTools.h"

using namespace Base;
namespace fs = boost::filesystem;
XERCES_CPP_NAMESPACE_USE


Metadata::Metadata(const boost::filesystem::path& metadataFile)
{
    // Any exception thrown by the XML code propagates out and prevents object creation
    XMLPlatformUtils::Initialize();

    _parser = std::make_shared<XercesDOMParser> ();
    _parser->setValidationScheme(XercesDOMParser::Val_Never);
    _parser->setDoNamespaces(true);

    auto errHandler = std::make_unique<HandlerBase>();
    _parser->setErrorHandler(errHandler.get());

    _parser->parse(metadataFile.string().c_str());

    auto doc = _parser->getDocument();
    _dom = doc->getDocumentElement();

    auto rootTagName = StrXUTF8(_dom->getTagName()).str;
    if (rootTagName != "package")
        throw std::runtime_error("package.xml must contain one, and only one, <package> element.");

    auto formatVersion = XMLString::parseInt(_dom->getAttribute(XUTF8Str("format").unicodeForm()));    
    switch (formatVersion) {
    case 3:
        parseVersion3(_dom);
        break;
    default:
        throw std::exception("pacakge.xml format version is not supported by this version of FreeCAD");
    }
}

Base::Metadata::Metadata(const DOMNode* domNode, int format) : _dom(nullptr)
{
    auto element = dynamic_cast<const DOMElement*>(domNode);
    if (element) {
        switch (format) {
        case 3:
            parseVersion3(element);
            break;
        default:
            throw std::exception("Requested format version is not supported by this version of FreeCAD");
        }
    }
}

Metadata::~Metadata()
{
}

std::string Metadata::name() const
{
    return _name;
}

std::string Metadata::version() const
{
    return _version;
}

std::string Metadata::description() const
{
    return _description;
}

std::vector<Meta::Contact> Metadata::maintainer() const
{
    return _maintainer;
}

std::vector<Meta::License> Metadata::license() const
{
    return _license;
}

std::vector<Meta::Url> Metadata::url() const
{
    return _url;
}

std::vector<Meta::Contact> Metadata::author() const
{
    return _author;
}

std::vector<Meta::Dependency> Metadata::depend() const
{
    return _depend;
}

std::vector<Meta::Dependency> Metadata::conflict() const
{
    return _conflict;
}

std::vector<Meta::Dependency> Metadata::replace() const
{
    return _replace;
}

std::vector<std::string> Metadata::tag() const
{
    return _tag;
}

boost::filesystem::path Metadata::icon() const
{
    return _icon;
}

std::string Metadata::classname() const
{
    return _classname;
}

std::vector<boost::filesystem::path> Metadata::file() const
{
    return _file;
}

std::multimap<std::string, Metadata> Metadata::content() const
{
    return _content;
}

std::vector<Meta::GenericMetadata> Metadata::operator[](const std::string& tag) const
{
    std::vector<Meta::GenericMetadata> returnValue;
    auto range = _genericMetadata.equal_range(tag);
    for (auto item = range.first; item != range.second; ++item)
        returnValue.push_back(item->second);
    return returnValue;
}

XERCES_CPP_NAMESPACE::DOMElement* Metadata::dom() const
{
    return _dom;
}

void Metadata::parseVersion3(const DOMNode *startNode)
{
    auto children = startNode->getChildNodes();

    for (int i = 0; i < children->getLength(); ++i) {
        auto child = children->item(i);
        auto element = dynamic_cast<const DOMElement*>(child);
        if (!element)
            continue;

        auto tag = element->getNodeName();
        auto tagString = StrXUTF8(tag).str;

        if (tagString == "name")
            _name = StrXUTF8(element->getTextContent()).str;
        else if (tagString == "version")
            _version = StrXUTF8(element->getTextContent()).str;
        else if (tagString == "description")
            _description = StrXUTF8(element->getTextContent()).str;
        else if (tagString == "maintainer")
            _maintainer.emplace_back(element);
        else if (tagString == "license")
            _license.emplace_back(element);
        else if (tagString == "url")
            _url.emplace_back(element);
        else if (tagString == "author")
            _author.emplace_back(element);
        else if (tagString == "depend")
            _depend.emplace_back(element);
        else if (tagString == "conflict")
            _conflict.emplace_back(element);
        else if (tagString == "replace")
            _replace.emplace_back(element);
        else if (tagString == "tag")
            _tag.emplace_back(StrXUTF8(element->getTextContent()).str);
        else if (tagString == "file")
            _file.emplace_back(StrXUTF8(element->getTextContent()).str);
        else if (tagString == "classname")
            _classname = StrXUTF8(element->getTextContent()).str;
        else if (tagString == "icon")
            _icon = fs::path(StrXUTF8(element->getTextContent()).str);
        else if (tagString == "content")
            parseContentNodeVersion3(element); // Recursive call
        else if (child->getChildNodes()->getLength() == 0)
            _genericMetadata.insert(std::make_pair(tagString, Meta::GenericMetadata(element)));
        // else we don't recognize this tag, just ignore it, but leave it in the DOM tree
    }
}

void Metadata::parseContentNodeVersion3(const DOMElement* contentNode)
{
    auto children = contentNode->getChildNodes();
    for (int i = 0; i < children->getLength(); ++i) {
        auto child = dynamic_cast<const DOMElement*>(children->item(i));
        if (child) {
            auto tag = StrXUTF8(child->getTagName()).str;
            _content.insert(std::make_pair(tag, Metadata(child, 3)));
        }
    }
}

Meta::Contact::Contact(const XERCES_CPP_NAMESPACE::DOMElement* e)
{
    auto emailAttribute = e->getAttribute(XUTF8Str("email").unicodeForm());
    name = StrXUTF8(e->getTextContent()).str;
    email = StrXUTF8(emailAttribute).str;
}

Meta::License::License(const XERCES_CPP_NAMESPACE::DOMElement* e)
{
    auto fileAttribute = e->getAttribute(XUTF8Str("file").unicodeForm());
    if (XMLString::stringLen(fileAttribute) > 0) {
        file = fs::path(StrXUTF8(fileAttribute).str);
    }
    name = StrXUTF8(e->getTextContent()).str;
}

Meta::Url::Url(const XERCES_CPP_NAMESPACE::DOMElement* e)
{
    auto typeAttribute = StrXUTF8(e->getAttribute(XUTF8Str("type").unicodeForm())).str;
    if (typeAttribute.empty() || typeAttribute == "website")
        type = UrlType::website;
    else if (typeAttribute == "bugtracker")
        type = UrlType::bugtracker;
    else if (typeAttribute == "repository")
        type = UrlType::repository;
    else if (typeAttribute == "readme")
        type = UrlType::readme;
    else if (typeAttribute == "documentation")
        type = UrlType::documentation;
    location = StrXUTF8(e->getTextContent()).str;
}

Meta::Dependency::Dependency(const XERCES_CPP_NAMESPACE::DOMElement* e)
{
    version_lt = StrXUTF8(e->getAttribute(XUTF8Str("version_lt").unicodeForm())).str;
    version_lte = StrXUTF8(e->getAttribute(XUTF8Str("version_lte").unicodeForm())).str;
    version_eq = StrXUTF8(e->getAttribute(XUTF8Str("version_eq").unicodeForm())).str;
    version_gte = StrXUTF8(e->getAttribute(XUTF8Str("version_gte").unicodeForm())).str;
    version_gt = StrXUTF8(e->getAttribute(XUTF8Str("version_gt").unicodeForm())).str;
    condition = StrXUTF8(e->getAttribute(XUTF8Str("condition").unicodeForm())).str;

    package = StrXUTF8(e->getTextContent()).str;
}

bool Meta::Dependency::matchesDependency(const std::string &version) const
{
    return false;
}

Meta::GenericMetadata::GenericMetadata(const XERCES_CPP_NAMESPACE::DOMElement* e)
{
    contents = StrXUTF8(e->getTextContent()).str;
    for (int i = 0; i < e->getAttributes()->getLength(); ++i) {
        auto a = e->getAttributes()->item(i);
        attributes.insert(std::make_pair(StrXUTF8(a->getNodeName()).str,
                                         StrXUTF8(a->getTextContent()).str));
    }
}
