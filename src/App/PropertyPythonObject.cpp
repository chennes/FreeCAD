// SPDX-License-Identifier: LGPL-2.1-or-later

/***************************************************************************
 *   Copyright (c) 2009 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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



#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <Base/Base64.h>
#include <Base/Console.h>
#include <Base/Interpreter.h>
#include <Base/Reader.h>
#include <Base/Writer.h>

#include "PropertyPythonObject.h"
#include "DocumentObject.h"


using namespace App;

namespace {

/**
 * @brief Check whether a path starts with a given directory prefix.
 *
 * @param[in] filePath The file path to check.
 * @param[in] directory The directory prefix to match against.
 * @return @c true if @p filePath starts with @p directory.
 */
bool isUnderDirectory(std::string filePath, std::string directory)
{
    std::ranges::replace(filePath, '\\', '/');
    std::ranges::replace(directory, '\\', '/');
    // Collapse repeated slashes (e.g. home path "build/debug//" + "Mod")
    auto collapseSlashes = [](std::string& s) {
        auto out = s.begin();
        for (auto it = s.begin(); it != s.end(); ++it) {
            if (*it == '/' && out != s.begin() && *(out - 1) == '/') {
                continue;
            }
            *out++ = *it;
        }
        s.erase(out, s.end());
    };
    collapseSlashes(filePath);
    collapseSlashes(directory);
    if (!directory.empty() && directory.back() != '/') {
        directory += '/';
    }
    return filePath.starts_with(directory);
}

/**
 * @brief Collect the directories that proxy modules may be imported from during document restore.
 *
 * These are FreeCAD's module and macro directories, plus every directory of the @c freecad
 * namespace package, which is reserved for addons wherever they are installed.
 *
 * @return The allowed directories.
 */
std::vector<std::string> allowedModuleDirectories()
{
    Py::Module freecad(PyImport_ImportModule("FreeCAD"), true);
    if (!freecad.hasAttr("__ModDirs__") || !freecad.hasAttr("__MacroDirs__")) {
        throw Py::RuntimeError(
            "FreeCAD.__ModDirs__ or FreeCAD.__MacroDirs__ not set -- FreeCADInit.py has not run yet"
        );
    }

    std::vector<std::string> directories;
    auto appendAll = [&directories](const Py::Sequence& paths) {
        for (const auto& path : paths) {
            directories.push_back(Py::String(path.ptr()).as_std_string());
        }
    };
    appendAll(freecad.getAttr("__ModDirs__"));
    appendAll(freecad.getAttr("__MacroDirs__"));

    Py::Dict sysModules(PyImport_GetModuleDict());
    if (sysModules.hasKey("freecad")) {
        Py::Object addonNamespace = sysModules.getItem("freecad");
        if (addonNamespace.hasAttr("__path__")) {
            appendAll(addonNamespace.getAttr("__path__"));
        }
    }
    return directories;
}

/**
 * @brief Check whether importing the module described by a spec only runs trusted code.
 *
 * A spec with a file origin must lie under an allowed directory. A spec without a loader is a
 * namespace package, whose import runs no code. Any other spec is a synthetic module from a
 * migration finder such as @c FemMigrateApp, and is trusted if the finder's own module is.
 *
 * @param[in] spec The result of @c importlib.util.find_spec().
 * @param[in] allowedDirectories The directories trusted modules may come from.
 * @return @c true if the module may be imported.
 */
bool isSpecAllowed(const Py::Object& spec, const std::vector<std::string>& allowedDirectories)
{
    auto isUnderAllowedDirectory = [&allowedDirectories](const std::string& path) {
        return std::ranges::any_of(allowedDirectories, [&path](const std::string& directory) {
            return isUnderDirectory(path, directory);
        });
    };

    Py::Object origin = spec.getAttr("origin");
    if (origin.isString()) {
        return isUnderAllowedDirectory(Py::String(origin).as_std_string());
    }

    Py::Object loader = spec.getAttr("loader");
    if (loader.isNone()) {
        return !spec.getAttr("submodule_search_locations").isNone();
    }

    Py::Object loaderType = loader.type();
    if (!loaderType.hasAttr("__module__")) {
        return false;
    }
    std::string loaderModuleName = Py::String(loaderType.getAttr("__module__")).as_std_string();
    Py::Dict sysModules(PyImport_GetModuleDict());
    if (!sysModules.hasKey(loaderModuleName)) {
        return false;
    }
    Py::Object loaderModule = sysModules.getItem(loaderModuleName);
    if (!loaderModule.hasAttr("__file__")) {
        return false;
    }
    Py::Object loaderFile = loaderModule.getAttr("__file__");
    return loaderFile.isString() && isUnderAllowedDirectory(Py::String(loaderFile).as_std_string());
}

/**
 * @brief Check every level of a dotted module name without running untrusted code.
 *
 * @c find_spec() imports the parent of a dotted name, so each level is vetted before the next
 * one is looked up. Levels already in @c sys.modules are skipped because importing them runs no
 * code, but their children are still checked.
 *
 * @param[in] moduleName The fully qualified Python module name to check.
 * @param[in] findSpec The @c importlib.util.find_spec() callable.
 * @param[in] allowedDirectories The directories trusted modules may come from.
 * @return @c true if importing @p moduleName only runs trusted code.
 */
bool isEveryLevelAllowed(
    const std::string& moduleName,
    const Py::Callable& findSpec,
    const std::vector<std::string>& allowedDirectories
)
{
    Py::Dict sysModules(PyImport_GetModuleDict());
    for (std::string::size_type dot = moduleName.find('.');; dot = moduleName.find('.', dot + 1)) {
        std::string prefix = moduleName.substr(0, dot);
        if (!sysModules.hasKey(prefix)) {
            Py::Object spec;
            try {
                spec = findSpec.apply(Py::TupleN(Py::String(prefix)));
            }
            catch (Py::Exception&) {
                PyErr_Clear();
                return false;
            }
            if (spec.isNone() || !isSpecAllowed(spec, allowedDirectories)) {
                return false;
            }
        }
        if (dot == std::string::npos) {
            return true;
        }
    }
}

/**
 * @brief Find the name under which a proxy module may be imported during document restore.
 *
 * A module already in @c sys.modules is permitted because importing it again runs no code. Any
 * other module must resolve, at every level of its dotted name, to code under a FreeCAD module,
 * macro, or addon directory. This prevents a crafted FCStd from importing arbitrary modules
 * (whose <tt>__init__.py</tt> could run malicious code on import) while still allowing
 * legitimate lazy-loaded FreeCAD workbench modules to restore.
 *
 * @param[in] moduleName The fully qualified Python module name stored in the document.
 * @return The name to import, or @c std::nullopt if the module is not permitted.
 */
std::optional<std::string> allowedModuleName(const std::string& moduleName)
{
    Py::Dict sysModules(PyImport_GetModuleDict());
    if (sysModules.hasKey(moduleName)) {
        return moduleName;
    }

    PyObject* importlibUtil = PyImport_ImportModule("importlib.util");
    if (!importlibUtil) {
        PyErr_Clear();
        return std::nullopt;
    }
    Py::Module importlib(importlibUtil, true);
    Py::Callable findSpec(importlib.getAttr("find_spec"));
    const std::vector<std::string> allowedDirectories = allowedModuleDirectories();

    if (isEveryLevelAllowed(moduleName, findSpec, allowedDirectories)) {
        return moduleName;
    }

    // FreeCAD adds each workbench directory to sys.path individually (e.g. .../Mod/Assembly/),
    // so a module stored as "Assembly.JointObject" may only be importable as "JointObject".
    std::string::size_type dot = moduleName.find('.');
    if (dot == std::string::npos) {
        return std::nullopt;
    }
    std::string shortName = moduleName.substr(dot + 1);
    if (sysModules.hasKey(shortName) || isEveryLevelAllowed(shortName, findSpec, allowedDirectories)) {
        return shortName;
    }
    return std::nullopt;
}

}  // anonymous namespace


TYPESYSTEM_SOURCE(App::PropertyPythonObject, App::Property)

PropertyPythonObject::PropertyPythonObject() = default;

PropertyPythonObject::~PropertyPythonObject()
{
    // this is needed because the release of the pickled object may need the
    // GIL. Thus, we grab the GIL and replace the pickled with an empty object
    Base::PyGILStateLocker lock;
    try {
        this->object = Py::Object();
    } catch (Py::TypeError &) {
        Base::Console().warning("Py::TypeError Exception caught while destroying PropertyPythonObject\n");
    }
}

void PropertyPythonObject::setValue(const Py::Object& py)
{
    Base::PyGILStateLocker lock;
    aboutToSetValue();
    this->object = py;
    hasSetValue();
}

Py::Object PropertyPythonObject::getValue() const
{
    return object;
}

PyObject* PropertyPythonObject::getPyObject()
{
    return Py::new_reference_to(this->object);
}

void PropertyPythonObject::setPyObject(PyObject* obj)
{
    Base::PyGILStateLocker lock;
    aboutToSetValue();
    this->object = obj;
    hasSetValue();
}

std::string PropertyPythonObject::toString() const
{
    std::string repr;
    Base::PyGILStateLocker lock;
    try {
        Py::Module pickle(PyImport_ImportModule("json"), true);
        if (pickle.isNull()) {
            throw Py::Exception();
        }
        Py::Callable method(pickle.getAttr(std::string("dumps")));
        Py::Object dump;
        if (this->object.hasAttr("dumps")) {
            Py::Tuple args;
            Py::Callable state(this->object.getAttr("dumps"));
            dump = state.apply(args);
        }
        // support add-ons that use the old method names
        else if (this->object.hasAttr("__getstate__")
#if PY_VERSION_HEX >= 0x030b0000
                 && this->object.getAttr("__getstate__").hasAttr("__func__")
#endif
        ) {
            Py::Tuple args;
            Py::Callable state(this->object.getAttr("__getstate__"));
            dump = state.apply(args);
        }
        else if (this->object.hasAttr("__dict__")) {
            dump = this->object.getAttr("__dict__");
        }
        else {
            dump = this->object;
        }

        Py::Tuple args(1);
        args.setItem(0, dump);
        Py::Object res = method.apply(args);
        Py::String str(res);
        repr = str.as_std_string("ascii");
    }
    catch (Py::Exception&) {
        Py::String typestr(this->object.type().str());
        Base::Console().error("PropertyPythonObject::toString(): failed for {}\n",
                              typestr.as_string());
        Base::PyException e;  // extract the Python error text
        e.reportException();
    }

    return repr;
}

void PropertyPythonObject::fromString(const std::string& repr)
{
    Base::PyGILStateLocker lock;
    try {
        if (repr.empty()) {
            return;
        }
        Py::Module pickle(PyImport_ImportModule("json"), true);
        if (pickle.isNull()) {
            throw Py::Exception();
        }
        Py::Callable method(pickle.getAttr(std::string("loads")));
        Py::Tuple args(1);
        args.setItem(0, Py::String(repr));
        Py::Object res = method.apply(args);

        if (this->object.hasAttr("loads")) {
            Py::Tuple args(1);
            args.setItem(0, res);
            Py::Callable state(this->object.getAttr("loads"));
            state.apply(args);
        }
        // support add-ons that use the old method names
        else if (this->object.hasAttr("__setstate__")
#if PY_VERSION_HEX >= 0x030b0000
                 && this->object.getAttr("__setstate__").hasAttr("__func__")
#endif
        ) {
            Py::Tuple args(1);
            args.setItem(0, res);
            Py::Callable state(this->object.getAttr("__setstate__"));
            state.apply(args);
        }
        else if (this->object.hasAttr("__dict__")) {
            if (!res.isNone()) {
                this->object.setAttr("__dict__", res);
            }
        }
        else {
            this->object = res;
        }
    }
    catch (Py::Exception&) {
        Base::PyException e;  // extract the Python error text
        e.reportException();
    }
}


std::string PropertyPythonObject::encodeValue(const std::string& str) const
{
    std::string tmp;
    for (char it : str) {
        if (it == '<') {
            tmp += "&lt;";
        }
        else if (it == '"') {
            tmp += "&quot;";
        }
        else if (it == '&') {
            tmp += "&amp;";
        }
        else if (it == '>') {
            tmp += "&gt";
        }
        else if (it == '\n') {
            tmp += "\\n";
        }
        else {
            tmp += it;
        }
    }

    return tmp;
}

std::string PropertyPythonObject::decodeValue(const std::string& str) const
{
    std::string tmp;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        if (*it == '\\') {
            ++it;
            if (it != str.end() && *it == 'n') {
                tmp += '\n';
            }
        }
        else {
            tmp += *it;
        }
    }

    return tmp;
}

void PropertyPythonObject::saveObject(Base::Writer& writer) const
{
    Base::PyGILStateLocker lock;
    try {
        PropertyContainer* parent = this->getContainer();
        if (parent->isDerivedFrom(Base::Type::fromName("App::DocumentObject"))) {
            if (this->object.hasAttr("__object__")) {
                writer.Stream() << " object=\"yes\"";
            }
        }
        if (parent->isDerivedFrom(Base::Type::fromName("Gui::ViewProvider"))) {
            if (this->object.hasAttr("__vobject__")) {
                writer.Stream() << " vobject=\"yes\"";
            }
        }
    }
    catch (Py::Exception& e) {
        e.clear();
    }
}

void PropertyPythonObject::restoreObject(Base::XMLReader& reader)
{
    Base::PyGILStateLocker lock;
    try {
        PropertyContainer* parent = this->getContainer();
        if (reader.hasAttribute("object")) {
            if (strcmp(reader.getAttribute<const char*>("object"), "yes") == 0) {
                Py::Object obj = Py::asObject(parent->getPyObject());
                this->object.setAttr("__object__", obj);
            }
        }
        if (reader.hasAttribute("vobject")) {
            if (strcmp(reader.getAttribute<const char*>("vobject"), "yes") == 0) {
                Py::Object obj = Py::asObject(parent->getPyObject());
                this->object.setAttr("__vobject__", obj);
            }
        }
    }
    catch (Py::Exception& e) {
        e.clear();
    }
    catch (const Base::Exception& e) {
        Base::Console().error("{}\n", e.what());
    }
    catch (...) {
        Base::Console().error("Critical error in PropertyPythonObject::restoreObject\n");
    }
}

void PropertyPythonObject::Save(Base::Writer& writer) const
{
    std::string repr = this->toString();
    repr = Base::base64_encode((const unsigned char*)repr.c_str(), repr.size());
    std::string val = /*encodeValue*/ (repr);
    writer.Stream() << writer.ind() << "<Python value=\"" << val << R"(" encoded="yes")";

    Base::PyGILStateLocker lock;
    try {
        if (this->object.hasAttr("__module__") && this->object.hasAttr("__class__")) {
            Py::String mod(this->object.getAttr("__module__"));
            Py::Object cls(this->object.getAttr("__class__"));
            if (cls.hasAttr("__name__")) {
                Py::String name(cls.getAttr("__name__"));
                writer.Stream() << " module=\"" << (std::string)mod << "\""
                                << " class=\"" << (std::string)name << "\"";
            }
        }
        else {
            writer.Stream() << " json=\"yes\"";
        }
    }
    catch (Py::Exception&) {
        Base::PyException e;  // extract the Python error text
        e.reportException();
    }

    saveObject(writer);
    writer.Stream() << "/>" << std::endl;
}

void PropertyPythonObject::Restore(Base::XMLReader& reader)
{
    reader.readElement("Python");
    if (reader.hasAttribute("file")) {
        std::string file(reader.getAttribute<const char*>("file"));
        reader.addFile(file.c_str(), this);
    }
    else {
        bool load_json = false;
        bool load_failed = false;
        std::string buffer = reader.getAttribute<const char*>("value");
        if (reader.hasAttribute("encoded") && strcmp(reader.getAttribute<const char*>("encoded"), "yes") == 0) {
            buffer = Base::base64_decode(buffer);
        }
        else {
            buffer = decodeValue(buffer);
        }

        Base::PyGILStateLocker lock;
        try {
            if (reader.hasAttribute("module") && reader.hasAttribute("class")) {
                std::string moduleName = reader.getAttribute<const char*>("module");
                std::optional<std::string> importName = allowedModuleName(moduleName);
                if (!importName) {
                    Base::Console().warning(
                        "PropertyPythonObject::Restore: blocked import of module '{}' during"
                        " document restore. Only modules from FreeCAD or installed addons"
                        " are permitted.\n",
                        moduleName);
                    throw Py::ImportError("module not permitted: " + moduleName);
                }
                Py::Module mod(PyImport_ImportModule(importName->c_str()), true);
                if (mod.isNull()) {
                    throw Py::Exception();
                }
                std::string className = reader.getAttribute<const char*>("class");
                PyObject* cls = mod.getAttr(className).ptr();
                if (!cls) {
                    std::stringstream s;
                    s << "Module " << moduleName << " has no class " << className;
                    throw Py::AttributeError(s.str());
                }
                if (PyType_Check(cls)) {
                    this->object = PyType_GenericAlloc((PyTypeObject*)cls, 0);
                }
                else {
                    throw Py::TypeError("neither class nor type object");
                }
                load_json = true;
            }
            else if (reader.hasAttribute("json")) {
                load_json = true;
            }
        }
        catch (Py::Exception&) {
            Base::PyException e;  // extract the Python error text
            e.reportException();
            this->object = Py::None();
            load_failed = true;
        }

        aboutToSetValue();
        if (load_json) {
            this->fromString(buffer);
        }
        else if (!load_failed) {
            Base::Console().warning(
                "PropertyPythonObject::Restore: unsupported serialisation: {}\n",
                buffer);
        }
        restoreObject(reader);
        hasSetValue();
    }
}

void PropertyPythonObject::SaveDocFile(Base::Writer& writer) const
{
    std::string buffer = this->toString();
    for (char it : buffer) {
        writer.Stream().put(it);
    }
}

void PropertyPythonObject::RestoreDocFile(Base::Reader& reader)
{
    aboutToSetValue();
    std::string buffer;
    char ch {};
    while (reader.get(ch)) {
        buffer.push_back(ch);
    }
    this->fromString(buffer);
    hasSetValue();
}

unsigned int PropertyPythonObject::getMemSize() const
{
    return sizeof(Py::Object);
}

Property* PropertyPythonObject::Copy() const
{
    PropertyPythonObject* p = new PropertyPythonObject();
    Base::PyGILStateLocker lock;
    p->object = this->object;
    return p;
}

void PropertyPythonObject::Paste(const Property& from)
{
    if (from.is<PropertyPythonObject>()) {
        Base::PyGILStateLocker lock;
        aboutToSetValue();
        this->object = static_cast<const PropertyPythonObject&>(from).object;
        hasSetValue();
    }
}
