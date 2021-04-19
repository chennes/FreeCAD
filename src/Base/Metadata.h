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

#ifndef BASE_METADATAREADER_H 
#define BASE_METADATAREADER_H 

#include "FCConfig.h"

#include <boost/filesystem.hpp>

#include <string>
#include <vector>
#include <map>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>


namespace Base {

    namespace Meta {

        /**
         * \struct Contact
         * \brief A person or company representing a point of contact for the package (either author or maintainer).
         */
        struct Contact {
            explicit Contact(const XERCES_CPP_NAMESPACE::DOMElement* e);
            std::string name; //< Contact name - required
            std::string email; //< Contact email - may be optional
        };

        /**
         * \struct License
         * \brief A license that covers some or all of this package.
         * 
         * Many licenses also require the inclusion of the complete license text, specified in this struct
         * using the "file" member.
         */
        struct License {
            explicit License(const XERCES_CPP_NAMESPACE::DOMElement* e);
            std::string name; //< Short name of license, e.g. "LGPL2", "MIT", "Mozilla Public License", etc.
            boost::filesystem::path file; //< Optional path to the license file, relative to the XML file's location
        };

        enum class UrlType {
            website,
            repository,
            bugtracker,
            readme,
            documentation
        };

        /**
         * \struct Url
         * \brief A URL, including type information (e.g. website, repository, or bugtracker, in package.xml v3)
         */
        struct Url {
            explicit Url(const XERCES_CPP_NAMESPACE::DOMElement* e);
            std::string location; //< The actual URL, including protocol
            UrlType type; //< What kind of URL this is
        };

        /**
         * \struct Dependency
         * \brief Another package that this package depends on, conflicts with, or replaces
         */
        struct Dependency {
            explicit Dependency(const XERCES_CPP_NAMESPACE::DOMElement* e);
            std::string package; //< Required: must exactly match the contents of the "name" element in the referenced package's package.xml file.
            std::string version_lt; //< Optional: The dependency to the package is restricted to versions less than the stated version number.
            std::string version_lte; //< Optional: The dependency to the package is restricted to versions less or equal than the stated version number.
            std::string version_eq; //< Optional: The dependency to the package is restricted to a version equal than the stated version number.
            std::string version_gte; //< Optional: The dependency to the package is restricted to versions greater or equal than the stated version number.
            std::string version_gt; //< Optional: The dependency to the package is restricted to versions greater than the stated version number.
            std::string condition; //< Optional: Conditional expression as documented in REP149.

            /**
             * Check a version string against this dependency: if the string meets the dependency, 
             * true is returned. If not, false is returned. In general this operates on standard-format
             * version triplets, and ignores any information that does not meet that format. The exceptions
             * are if the dependency specifies "version_eq" then an exact string match is performed, and if
             * "condition" is specified, then that code is executed with this string provided as the variable
             * "$VERSION".
             */
            bool matchesDependency(const std::string &version) const;
        };

        /**
         * \struct GenericMetadata
         * A structure to hold unrecognized single-level metadata.
         * 
         * Most unrecognized metadata is simple: when parsing the XML, if the parser finds a tag it
         * does not recognize, and that tag has no children, it is parsed into this data structure
         * for convenient access by client code.
         */
        struct GenericMetadata {
            explicit GenericMetadata(const XERCES_CPP_NAMESPACE::DOMElement* e);
            std::string contents; //< The contents of the tag
            std::map<std::string,std::string> attributes; //< The XML attributes of the tag
        };

    }

    /**
     * \class Metadata 
     * \brief Reads data from a metadata file.
     * 
     * The metadata format is based on https://ros.org/reps/rep-0149.html, modified for FreeCAD
     * use. Full format documentation is available at the FreeCAD Wiki.
     * 
     * The following metadata is required, and guaranteed to exist upon class creation:
     * <name> (As displayed by package managers)
     * <version> (Symantic triplet format)
     * <description> (Text-only, should be only a few sentences)
     * <maintainer> (multiple, but at least one: "email" attribute is required)
     * <license> (multiple, but at least one: "file" attribute may contain path to full license)
     * <url> (multiple: "type" attribute may be website (default), bugtracker, repository, readme, documentation. One repository is required.)
     * 
     * Optional package-level metadata:
     * <author> (multiple: "email" attribute is optional)
     * <depend> (multiple: attributes described in Depend struct declaration)
     * <conflict> (multiple: see depend)
     * <replace> (multiple: see depend)
     * 
     * In addition to the package-level metadata, each package consists of an arbitrary number
     * of content items: at this time "macro", "workbench", and "theme" items, though nothing
     * in this class restricts input to those tags and nothing need be changed to support others.
     * Each content item itself can contain all of the metadata listed above. In addition, for
     * convenience the following additional tags are supported directly:
     * 
     * <file> (Any number of files associated with this content item may be listed -- their meaning depends on the type.)
     * <classname> (Typically only used for workbenches, the main Python class for the workbench)
     * 
     * Any unrecognized metadata can be accessed by accessing the DOM tree directly using the 
     * provided member function, or (in the case of simple single-level metadata) by using
     * operator[].
     */
    class BaseExport Metadata {
    public:

        /**
         * Read the data from a file on disk
         * 
         * This constructor takes a path to an XML file and loads the XML from that file as
         * metadata.
         */
        explicit Metadata(const boost::filesystem::path& metadataFile);

        /**
         * Construct a Metadata object from a DOM node.
         * 
         * This node may have any tag name: it is only accessed via its children, which are
         * expected to follow the standard Metadata format for the contents of the <package> element.
         */
        Metadata(const XERCES_CPP_NAMESPACE::DOMNode* domNode, int format);
        
        ~Metadata();


        //////////////////////////////////////////////////////////////
        // Recognized Metadata
        //////////////////////////////////////////////////////////////

        std::string name() const; //< A short name for this package, often used as a menu entry.
        std::string version() const; //< Version string in symantic triplet format, e.g. "1.2.3".
        std::string description() const; //< Text-only description of the package. No markup.
        std::vector<Meta::Contact> maintainer() const; //< Must be at least one, and must specify an email address.
        std::vector<Meta::License> license() const; //< Must be at least one, and most licenses require including a license file.
        std::vector<Meta::Url> url() const; //< Any number of URLs may be specified, but at least one repository URL must be included at the package level.
        std::vector<Meta::Contact> author() const; //< Any number of authors may be specified, and email addresses are optional.
        std::vector<Meta::Dependency> depend() const; //< Zero or more packages this package requires prior to use.
        std::vector<Meta::Dependency> conflict() const; //< Zero of more packages this package conflicts with.
        std::vector<Meta::Dependency> replace() const; //< Zero or more packages this package is intended to replace.
        std::vector<std::string> tag() const; //< Zero or more text tags related to this package
        boost::filesystem::path icon() const; //< Path to an icon file
        std::string classname() const; //< Recognized for convenience -- generally only used by Workbenches
        std::vector<boost::filesystem::path> file() const; //< Arbitrary files associated with this package or content item

        /**
         * Access the metadata for the content elements of this package
         * 
         * In addition to the overall package metadata, this class reads in metadata contained in a
         * <content> element. Each entry in the content element is an element representing some
         * type of package content (e.g. add-on, macro, theme, etc.). This class places no restriction
         * on the types, it is up to client code to place requirements on the metadata included
         * here.
         * 
         * For example, themes might be specified:
         * <content>
         *   <theme>
         *     <name>High Contrast</name>
         *   </theme>
         * </content>
         */
        std::multimap<std::string, Metadata> content() const;

        /**
         * Convenience accessor for unrecognized simple metadata.
         * 
         * If the XML parser encounters tags that it does not recognize, and those tags have
         * no children, a GenericMetadata object is created. Those objects can be accessed using
         * operator[], which returns a (potentially empty) vector containing all instances of the
         * given tag.
         */
        std::vector<Meta::GenericMetadata> operator[] (const std::string &tag) const;
        
        /**
         * Directly access the DOM tree to support unrecognized multi-level metadata
         */
        XERCES_CPP_NAMESPACE::DOMElement* dom() const;

    private:

        std::string _name;
        std::string _version;
        std::string _description;
        std::vector<Meta::Contact> _maintainer;
        std::vector<Meta::License> _license;
        std::vector<Meta::Url> _url;
        std::vector<Meta::Contact> _author;
        std::vector<Meta::Dependency> _depend;
        std::vector<Meta::Dependency> _conflict;
        std::vector<Meta::Dependency> _replace;
        std::vector<std::string> _tag;
        boost::filesystem::path _icon;
        std::string _classname;
        std::vector<boost::filesystem::path> _file;

        std::multimap<std::string, Metadata> _content;

        std::multimap<std::string, Meta::GenericMetadata> _genericMetadata;

        XERCES_CPP_NAMESPACE::DOMElement* _dom;
        std::shared_ptr<XERCES_CPP_NAMESPACE::XercesDOMParser> _parser;

        void parseVersion3(const XERCES_CPP_NAMESPACE::DOMNode* startNode);
        void parseContentNodeVersion3(const XERCES_CPP_NAMESPACE::DOMElement* contentNode);

    };

}

#endif