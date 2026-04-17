

// generated out of PythonNavigationStyle.pyi
#include "Navigation/PythonNavigationStylePy.h"
#include "Navigation/PythonNavigationStylePy.cpp"


using namespace Gui;

PyObject* PythonNavigationStylePy::PyMake(PyTypeObject*, PyObject*, PyObject*)
{
    return new PythonNavigationStylePy(new PythonNavigationStyle);
}

int PythonNavigationStylePy::PyInit(PyObject* /*args*/, PyObject* /*kwd*/)
{
    return 0;
}

std::string PythonNavigationStylePy::representation() const
{
    return "<PythonNavigationStyle>";
}

PyObject* PythonNavigationStylePy::getCustomAttributes(const char* attr) const
{
    return nullptr;
}

int PythonNavigationStylePy::setCustomAttributes(const char* attr, PyObject* obj)
{
    return -1;
}
