#ifndef PYTHON_COMPAT_H
#define PYTHON_COMPAT_H

#include <Python.h>

#if PY_MAJOR_VERSION >= 3
#define PyInt_Check PyLong_Check
#define PyInt_AsLong PyLong_AsLong
#define PyInt_AS_LONG PyLong_AS_LONG
#define PyInt_FromLong PyLong_FromLong
#define PyNumber_Int PyNumber_Long
#define PyString_FromString PyUnicode_FromString
#define PyObject_Unicode PyObject_Str

static inline char *qtiPyString_AsString(PyObject *obj)
{
	if (PyUnicode_Check(obj))
		return const_cast<char *>(PyUnicode_AsUTF8(obj));
	if (PyBytes_Check(obj))
		return PyBytes_AsString(obj);
	return NULL;
}

#define PyString_AsString qtiPyString_AsString
#define PyString_AS_STRING qtiPyString_AsString
#define QTI_PyEval_EvalCode(code, globals, locals) PyEval_EvalCode((PyObject *)(code), globals, locals)
#else
#define QTI_PyEval_EvalCode(code, globals, locals) PyEval_EvalCode((PyCodeObject *)(code), globals, locals)
#endif

#endif
