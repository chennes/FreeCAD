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
    case 1:
        parseVersion1(_dom);
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
        case 1:
            parseVersion1(element);
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

void Metadata::setName(const std::string& name)
{
    _name = name;
}

void Metadata::setVersion(int major, int minor, int patch, const std::string& suffix)
{
    std::stringstream s;
    s << major << "." << minor << "." << patch << suffix;
    _version = s.str();
}

void Metadata::setDescription(const std::string& description)
{
    _description = description;
}

void Metadata::addMaintainer(const Meta::Contact& maintainer)
{
    _maintainer.push_back(maintainer);
}

void Metadata::addLicense(const Meta::License& license)
{
    _license.push_back(license);
}

void Metadata::addUrl(const Meta::Url& url)
{
    _url.push_back(url);
}

void Metadata::addAuthor(const Meta::Contact& author)
{
    _author.push_back(author);
}

void Metadata::addDepend(const Meta::Dependency& dep)
{
    _depend.push_back(dep);
}

void Metadata::addConflict(const Meta::Dependency& dep)
{
    _conflict.push_back(dep);
}

void Metadata::addReplace(const Meta::Dependency& dep)
{
    _replace.push_back(dep);
}

void Metadata::addTag(const std::string& tag)
{
    _tag.push_back(tag);
}

void Metadata::setIcon(const boost::filesystem::path& path)
{
    _icon = path;
}

void Metadata::setClassname(const std::string& name)
{
    _classname = name;
}

void Metadata::addFile(const boost::filesystem::path& path)
{
    _file.push_back(path);
}

void Metadata::addContentItem(const std::string& tag, const Metadata& item)
{
    _content.insert(std::make_pair(tag, item));
}


DOMElement* appendSimpleXMLNode(DOMElement *baseNode, const std::string& nodeName, const std::string& nodeContents)
{
    // For convenience (and brevity of final output) don't create nodes that don't have contents
    if (nodeContents.empty())
        return nullptr;

    auto doc = baseNode->getOwnerDocument();
    DOMElement* namedElement = doc->createElement(XUTF8Str(nodeName.c_str()).unicodeForm());
    baseNode->appendChild(namedElement);
    DOMText* namedNode = doc->createTextNode(XUTF8Str(nodeContents.c_str()).unicodeForm());
    namedElement->appendChild(namedNode);
    return namedElement;
}

void addAttribute(DOMElement* node, const std::string& key, const std::string& value)
{
    if (value.empty())
        return;

    node->setAttribute(XUTF8Str(key.c_str()).unicodeForm(), XUTF8Str(value.c_str()).unicodeForm());
}

void addDependencyNode(DOMElement* root, const std::string& name, const Meta::Dependency& depend)
{
    auto element = appendSimpleXMLNode(root, name, depend.package);
    if (element) {
        addAttribute(element, "version_lt", depend.version_lt);
        addAttribute(element, "version_lte", depend.version_lte);
        addAttribute(element, "version_eq", depend.version_eq);
        addAttribute(element, "version_gte", depend.version_gte);
        addAttribute(element, "version_gt", depend.version_gt);
        addAttribute(element, "condition", depend.condition);
    }
}

void Metadata::write(const boost::filesystem::path& file) const
{
    DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(XUTF8Str("Core LS").unicodeForm());

    DOMDocument* doc = impl->createDocument(0, XUTF8Str("package").unicodeForm(), 0);
    DOMElement* root = doc->getDocumentElement();
    root->setAttribute(XUTF8Str("format").unicodeForm(), XUTF8Str("1").unicodeForm());

    appendToElement(root);

    DOMLSSerializer* theSerializer = ((DOMImplementationLS*)impl)->createLSSerializer();
    DOMConfiguration* config = theSerializer->getDomConfig();
    if (config->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
        config->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);

    // set feature if the serializer supports the feature/mode
    if (config->canSetParameter(XMLUni::fgDOMWRTSplitCdataSections, true))
        config->setParameter(XMLUni::fgDOMWRTSplitCdataSections, true);

    if (config->canSetParameter(XMLUni::fgDOMWRTDiscardDefaultContent, true))
        config->setParameter(XMLUni::fgDOMWRTDiscardDefaultContent, true);

    try {
        XMLFormatTarget* myFormTarget = new LocalFileFormatTarget(file.string().c_str());
        DOMLSOutput* theOutput = ((DOMImplementationLS*)impl)->createLSOutput();

        theOutput->setByteStream(myFormTarget);
        theSerializer->write(doc, theOutput);

        theOutput->release();
        theSerializer->release();
        delete myFormTarget;
    }
    catch(const XMLException& toCatch) {
        char* message = XMLString::transcode(toCatch.getMessage());
        THROWM(Base::RuntimeError, message); // This is leaky
        XMLString::release(&message);
    }
    catch (const DOMException& toCatch) {
        char* message = XMLString::transcode(toCatch.msg);
        THROWM(Base::RuntimeError, message); // This is leaky
        XMLString::release(&message);
    }

    doc->release();
}

void Metadata::appendToElement(DOMElement * root) const
{
    auto nameElement_unused = appendSimpleXMLNode(root, "name", _name);
    auto descriptionElement_unused = appendSimpleXMLNode(root, "description", _description);
    auto versionElement_unused = appendSimpleXMLNode(root, "version", _version);

    for (const auto& maintainer : _maintainer) {
        auto element = appendSimpleXMLNode(root, "maintainer", maintainer.name);
        if (element)
            addAttribute(element, "email", maintainer.email);
    }

    for (const auto& license : _license) {
        auto element = appendSimpleXMLNode(root, "license", license.name);
        if (element)
            addAttribute(element, "file", license.file.string());
    }

    for (const auto& url : _url) {
        auto element = appendSimpleXMLNode(root, "url", url.location);
        if (element) {
            std::string typeAsString("website");
            switch (url.type) {
                case Meta::UrlType::website:       typeAsString = "website";       break;
                case Meta::UrlType::repository:    typeAsString = "repository";    break;
                case Meta::UrlType::bugtracker:    typeAsString = "bugtracker";    break;
                case Meta::UrlType::readme:        typeAsString = "readme";        break;
                case Meta::UrlType::documentation: typeAsString = "documentation"; break;
            }
            addAttribute(element, "type", typeAsString);
        }
    }

    for (const auto& author : _author) {
        auto element = appendSimpleXMLNode(root, "author", author.name);
        if (element)
            addAttribute(element, "email", author.email);
    }

    for (const auto& depend : _depend) 
        addDependencyNode(root, "depend", depend);

    for (const auto& conflict : _conflict) 
        addDependencyNode(root, "conflict", conflict);

    for (const auto& replace : _replace) 
        addDependencyNode(root, "replace", replace);

    for (const auto &tag : _tag) 
        auto element_unused = appendSimpleXMLNode(root, "tag", tag);

    appendSimpleXMLNode(root, "icon", _icon.string());

    appendSimpleXMLNode(root, "classname", _classname);

    for (const auto& file : _file)
        auto element_unused = appendSimpleXMLNode(root, "file", file.string());

    for (const auto& md : _genericMetadata) {
        auto element = appendSimpleXMLNode(root, md.first, md.second.contents);
        for (const auto& attr : md.second.attributes)
            addAttribute(element, attr.first, attr.second);
    }

    if (!_content.empty()) {
        auto doc = root->getOwnerDocument();
        DOMElement* contentRootElement = doc->createElement(XUTF8Str("content").unicodeForm());
        root->appendChild(contentRootElement);
        for (const auto& content : _content) {
            DOMElement* contentElement = doc->createElement(XUTF8Str(content.first.c_str()).unicodeForm());
            contentRootElement->appendChild(contentElement);
            content.second.appendToElement(contentElement);
        }
    }
}


void Metadata::parseVersion1(const DOMNode *startNode)
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
            parseContentNodeVersion1(element); // Recursive call
        else if (child->getChildNodes()->getLength() == 0)
            _genericMetadata.insert(std::make_pair(tagString, Meta::GenericMetadata(element)));
        // else we don't recognize this tag, just ignore it, but leave it in the DOM tree
    }
}

void Metadata::parseContentNodeVersion1(const DOMElement* contentNode)
{
    auto children = contentNode->getChildNodes();
    for (int i = 0; i < children->getLength(); ++i) {
        auto child = dynamic_cast<const DOMElement*>(children->item(i));
        if (child) {
            auto tag = StrXUTF8(child->getTagName()).str;
            _content.insert(std::make_pair(tag, Metadata(child, 1)));
        }
    }
}

Meta::Contact::Contact(const std::string& name, const std::string& email) :
    name(name),
    email(email)
{
    // This has to be provided manually since we have another constructor
}

Meta::Contact::Contact(const XERCES_CPP_NAMESPACE::DOMElement* e)
{
    auto emailAttribute = e->getAttribute(XUTF8Str("email").unicodeForm());
    name = StrXUTF8(e->getTextContent()).str;
    email = StrXUTF8(emailAttribute).str;
}

Meta::License::License(const std::string& name, boost::filesystem::path file) :
    name(name),
    file(file)
{
    // This has to be provided manually since we have another constructor
}

Meta::License::License(const XERCES_CPP_NAMESPACE::DOMElement* e)
{
    auto fileAttribute = e->getAttribute(XUTF8Str("file").unicodeForm());
    if (XMLString::stringLen(fileAttribute) > 0) {
        file = fs::path(StrXUTF8(fileAttribute).str);
    }
    name = StrXUTF8(e->getTextContent()).str;
}

Meta::Url::Url(const std::string& location, UrlType type) :
    location(location),
    type(type)
{
    // This has to be provided manually since we have another constructor
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
