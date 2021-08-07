/*
---------------------------------------------------------------------------
Copyright (c) 2014, Michael Mohr, San Jose, CA, USA. All rights reserved.
Copyright (c) 2014, Brian Gladman, Worcester, UK. All rights reserved.

The redistribution and use of this software (with or without changes)
is allowed without the payment of fees or royalties provided that:

  source code distributions include the above copyright notice, this
  list of conditions and the following disclaimer;

  binary distributions include the above copyright notice, this list
  of conditions and the following disclaimer in their documentation.

This software is provided 'as is' with no explicit or implied warranties
in respect of its operation, including, but not limited to, correctness
and fitness for purpose.
---------------------------------------------------------------------------
Issue Date: 30/08/2014
*/

#define HAVE_ROUND 1
#define PY_SSIZE_T_CLEAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Python.h>
#include <structmember.h>

#ifndef _MSC_VER
#  include <sys/mman.h>
#else
#  include <Windows.h>
#  include <malloc.h>
#  define strncasecmp _strnicmp
#endif

#include "aes.h"
#if defined( __PROFILE_AES__ )
#include "rdtsc.h"
#endif

typedef enum
{
    AES_MODE_ECB,
    AES_MODE_CBC,
    AES_MODE_CFB,
    AES_MODE_OFB,
    AES_MODE_CTR
} aes_mode;

#ifdef _MSC_VER

typedef struct
{
    PyObject_HEAD
    aes_mode mode;
    __declspec(align(16)) aes_encrypt_ctx ectx[1];
    __declspec(align(16)) aes_decrypt_ctx dctx[1];
    __declspec(align(16)) unsigned char iv[AES_BLOCK_SIZE];
    __declspec(align(16)) unsigned char iv_o[AES_BLOCK_SIZE];
} aes_AESObject;

#else

typedef struct
{
    PyObject_HEAD
    aes_mode mode;
    aes_encrypt_ctx ectx[1] __attribute__ ((aligned(16)));
    aes_decrypt_ctx dctx[1] __attribute__ ((aligned(16)));
    unsigned char iv[AES_BLOCK_SIZE] __attribute__ ((aligned(16)));
    unsigned char iv_o[AES_BLOCK_SIZE] __attribute__ ((aligned(16)));
} aes_AESObject;

#endif

/*
This subroutine implements the CTR mode standard incrementing function.
See NIST Special Publication 800-38A, Appendix B for details:
http://csrc.nist.gov/publications/nistpubs/800-38a/sp800-38a.pdf
*/
#define CTR_POS 8

void ctr_inc(unsigned char *cbuf)
{
    unsigned char *p = cbuf + AES_BLOCK_SIZE, *e = cbuf + CTR_POS;
    while(p-- > e && !++(*p))
        ;
}

/*
A discussion of buffers in Python can be found here:
https://mail.python.org/pipermail/python-dev/2000-October/009974.html
Suggested data type for {en|de}cryption: Python array class
*/

static PyObject *py_aes_encrypt(aes_AESObject *self, PyObject *args, PyObject *kwds)
{
    aes_mode mode;
    PyObject *data;
    Py_buffer dbuf;
    AES_RETURN ret = EXIT_FAILURE;
#if defined( __PROFILE_AES__ )
    unsigned long long enter = 0, exit = 0;
#endif

    char *kwlist[] = {"data", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &data))
    {
        PyErr_SetString(PyExc_ValueError, "Failed to parse arguments");
        return NULL;
    }

    if(!PyObject_CheckBuffer(data))
    {
        PyErr_SetString(PyExc_ValueError, "Check failed for data buffer");
        return NULL;
    }

    if(PyObject_GetBuffer(data, &dbuf, PyBUF_WRITABLE | PyBUF_C_CONTIGUOUS) < 0)
    {
        PyErr_SetString(PyExc_ValueError, "Failed to get data buffer");
        return NULL;
    }

    /* Verify constraints based on mode */
    mode = self->mode;
    if(((mode == AES_MODE_ECB) || (mode == AES_MODE_CBC)) && ((dbuf.len & 15) != 0))
    {
        PyErr_SetString(PyExc_ValueError, "Data size must be a multiple of 16 bytes");
        return NULL;
    }

    /* Perform the real encryption operation */
#if defined( __PROFILE_AES__ )
    enter = read_tsc();
#endif
    switch(mode)
    {
    case AES_MODE_ECB:
        ret = aes_ecb_encrypt(dbuf.buf, dbuf.buf, (int)dbuf.len, self->ectx);
        break;
    case AES_MODE_CBC:
        ret = aes_cbc_encrypt(dbuf.buf, dbuf.buf, (int)dbuf.len, self->iv, self->ectx);
        break;
    case AES_MODE_CFB:
        ret = aes_cfb_encrypt(dbuf.buf, dbuf.buf, (int)dbuf.len, self->iv, self->ectx);
        break;
    case AES_MODE_OFB:
        ret = aes_ofb_encrypt(dbuf.buf, dbuf.buf, (int)dbuf.len, self->iv, self->ectx);
        break;
    case AES_MODE_CTR:
        /* cbuf data is passed as iv */
        ret = aes_ctr_encrypt(dbuf.buf, dbuf.buf, (int)dbuf.len, self->iv, ctr_inc, self->ectx);
        break;
    }
#if defined( __PROFILE_AES__ )
    exit = read_tsc();
#endif

    PyBuffer_Release(&dbuf);
    /* Verify result and return */
    if(ret != EXIT_SUCCESS)
    {
        PyErr_SetString(PyExc_ValueError, "Failed to encrypt data");
        return NULL;
    }

#if defined( __PROFILE_AES__ )
    return PyLong_FromUnsignedLongLong(exit - enter);
#else
    Py_INCREF(Py_None);
    return Py_None;
#endif
}

static PyObject *py_aes_decrypt(aes_AESObject *self, PyObject *args, PyObject *kwds)
{
    aes_mode mode;
    PyObject *data;
    Py_buffer dbuf;
    AES_RETURN ret = EXIT_FAILURE;
#if defined( __PROFILE_AES__ )
    unsigned long long enter = 0, exit = 0;
#endif

    char *kwlist[] = {"data", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &data))
    {
        PyErr_SetString(PyExc_ValueError, "Failed to parse arguments");
        return NULL;
    }

    if(!PyObject_CheckBuffer(data))
    {
        PyErr_SetString(PyExc_ValueError, "Check failed for data buffer");
        return NULL;
    }

    if(PyObject_GetBuffer(data, &dbuf, PyBUF_WRITABLE | PyBUF_C_CONTIGUOUS) < 0)
    {
        PyErr_SetString(PyExc_ValueError, "Failed to get data buffer");
        return NULL;
    }

    /* Verify constraints based on mode */
    mode = self->mode;
    if(((mode == AES_MODE_ECB) || (mode == AES_MODE_CBC)) && ((dbuf.len & 15) != 0)) {
        PyErr_SetString(PyExc_ValueError, "Data size must be a multiple of 16 bytes");
        return NULL;
    }

    /* Perform the real encryption operation */
#if defined( __PROFILE_AES__ )
    enter = read_tsc();
#endif
    switch(mode)
    {
    case AES_MODE_ECB:
        ret = aes_ecb_decrypt(dbuf.buf, dbuf.buf, (int)dbuf.len, self->dctx);
        break;
    case AES_MODE_CBC:
        ret = aes_cbc_decrypt(dbuf.buf, dbuf.buf, (int)dbuf.len, self->iv, self->dctx);
        break;
    case AES_MODE_CFB:
        ret = aes_cfb_decrypt(dbuf.buf, dbuf.buf, (int)dbuf.len, self->iv, self->ectx);
        break;
    case AES_MODE_OFB:
        ret = aes_ofb_decrypt(dbuf.buf, dbuf.buf, (int)dbuf.len, self->iv, self->ectx);
        break;
    case AES_MODE_CTR:
        /* cbuf data is passed as iv */
        ret = aes_ctr_decrypt(dbuf.buf, dbuf.buf, (int)dbuf.len, self->iv, ctr_inc, self->ectx);
        break;
    }
#if defined( __PROFILE_AES__ )
    exit = read_tsc();
#endif

    PyBuffer_Release(&dbuf);
    /* Verify result and return */
    if(ret != EXIT_SUCCESS)
    {
        PyErr_SetString(PyExc_ValueError, "Failed to decrypt data");
        return NULL;
    }

#if defined( __PROFILE_AES__ )
    return PyLong_FromUnsignedLongLong(exit - enter);
#else
    Py_INCREF(Py_None);
    return Py_None;
#endif
}

static PyObject *py_aes_reset(aes_AESObject *self)
{
    switch(self->mode) {
    case AES_MODE_ECB:
        break;
    case AES_MODE_CBC:
        memcpy(self->iv, self->iv_o, AES_BLOCK_SIZE);
        break;
    case AES_MODE_CFB:
    case AES_MODE_OFB:
    case AES_MODE_CTR:
        memcpy(self->iv, self->iv_o, AES_BLOCK_SIZE);
        aes_mode_reset(self->ectx);
        break;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef aes_AESmethods[] =
{
    { "encrypt", (PyCFunction)py_aes_encrypt, METH_VARARGS | METH_KEYWORDS, "encrypts a series of blocks" },
    { "decrypt", (PyCFunction)py_aes_decrypt, METH_VARARGS | METH_KEYWORDS, "decrypts a series of blocks" },
    { "reset",   (PyCFunction)py_aes_reset,   METH_NOARGS,                  "resets the object state"     },
    {NULL}  /* Sentinel */
};

static PyMemberDef aes_members[] =
{
    {NULL}  /* Sentinel */
};

static int py_aes_init(aes_AESObject *self, PyObject *args, PyObject *kwds)
{
    size_t mode_len = 0;
    const char *mode = NULL;
    PyObject *key = NULL;
    Py_buffer key_buf;
    PyObject *iv = NULL;
    Py_buffer iv_buf;

    char *kwlist[] = {"mode", "key", "iv", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, kwds, "sO|O", kwlist, &mode, &key, &iv))
    {
        PyErr_SetString(PyExc_ValueError, "Failed to parse arguments");
        return -1;
    }

    /* determine the operation mode */
    mode_len = strlen(mode);
    if(strncasecmp(mode, "ecb", mode_len) == 0)
    {
        self->mode = AES_MODE_ECB;
    }
    else if(strncasecmp(mode, "cbc", mode_len) == 0)
    {
        self->mode = AES_MODE_CBC;
    }
    else if(strncasecmp(mode, "cfb", mode_len) == 0)
    {
        self->mode = AES_MODE_CFB;
    }
    else if(strncasecmp(mode, "ofb", mode_len) == 0)
    {
        self->mode = AES_MODE_OFB;
    }
    else if(strncasecmp(mode, "ctr", mode_len) == 0)
    {
        self->mode = AES_MODE_CTR;
    }
    else
    {
        PyErr_SetString(PyExc_ValueError, "Unsupported AES mode");
        return -1;
    }

    if(self->mode != AES_MODE_ECB)
    {
        /* iv parameter required */
        if(!PyObject_CheckBuffer(iv))
        {
            PyErr_SetString(PyExc_ValueError, "Check failed for IV buffer");
            return -1;
        }
        if(PyObject_GetBuffer(iv, &iv_buf, PyBUF_SIMPLE) < 0)
        {
            PyErr_SetString(PyExc_ValueError, "Failed to get IV buffer");
            return -1;
        }
        if(iv_buf.len != AES_BLOCK_SIZE) {
            PyBuffer_Release(&iv_buf);
            PyErr_SetString(PyExc_ValueError, "IV/CTR buffer must be 16 bytes long");
            return -1;
        }
        memcpy(self->iv, iv_buf.buf, AES_BLOCK_SIZE);
        /* Save a copy of the original IV, for possible reset later */
        memcpy(self->iv_o, iv_buf.buf, AES_BLOCK_SIZE);
        PyBuffer_Release(&iv_buf);
    }

    if(!PyObject_CheckBuffer(key))
    {
        PyErr_SetString(PyExc_ValueError, "Check failed for key buffer");
        return -1;
    }

    if(PyObject_GetBuffer(key, &key_buf, PyBUF_SIMPLE) < 0)
    {
        PyErr_SetString(PyExc_ValueError, "Failed to get key buffer");
        return -1;
    }

    /* validate key length and initialize encryption / decryption states */
    switch(key_buf.len)
    {
    case 16:
        aes_encrypt_key128(key_buf.buf, self->ectx);
        aes_decrypt_key128(key_buf.buf, self->dctx);
        break;
    case 24:
        aes_encrypt_key192(key_buf.buf, self->ectx);
        aes_decrypt_key192(key_buf.buf, self->dctx);
        break;
    case 32:
        aes_encrypt_key256(key_buf.buf, self->ectx);
        aes_decrypt_key256(key_buf.buf, self->dctx);
        break;
    default:
        PyBuffer_Release(&key_buf);
        PyErr_SetString(PyExc_ValueError, "Invalid AES key length");
        return -1;
    }

    PyBuffer_Release(&key_buf);
    return 0;
}

/* https://docs.python.org/2/c-api/typeobj.html#PyTypeObject.tp_alloc */
static PyObject *secure_alloc(PyTypeObject *type, Py_ssize_t nitems)
{
    aes_AESObject *self;
    size_t required_mem, extra, tmp;

    required_mem = (size_t)type->tp_basicsize;
    if(type->tp_itemsize != 0)
    {
        extra = Py_SIZE(type) * type->tp_itemsize;
        /* round up to a multiple of sizeof(void *) */
        tmp = extra % sizeof(void *);
        if(tmp > 0)
            extra += (sizeof(void *) - tmp);
        required_mem += extra;
    }
#ifdef _MSC_VER
    if((self = _aligned_malloc(required_mem, 16)) == NULL)
        return (PyObject *)PyErr_NoMemory();
    if(VirtualLock(self, required_mem) == 0)
    {
        _aligned_free(self);
        return (PyObject *)PyErr_NoMemory();
    }
#else
    if(posix_memalign((void **)&self, 16, required_mem) != 0)
        return (PyObject *)PyErr_NoMemory();
    if(mlock(self, required_mem) != 0)
    {
        free(self);
        return (PyObject *)PyErr_NoMemory();
    }
#endif

    memset(self, 0, required_mem);
    PyObject_INIT(self, type);
    return (PyObject *)self;
}

void secure_free(void *self)
{
    memset(self, 0, sizeof(aes_AESObject));
#ifdef _MSC_VER
    VirtualUnlock(self, sizeof(aes_AESObject));
    _aligned_free(self);
#else
    munlock(self, sizeof(aes_AESObject));
    free(self);
#endif
    self = NULL;
    return;
}

#if PY_MAJOR_VERSION >= 3
static PyTypeObject aes_AESType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
#else
static PyTypeObject aes_AESType =
{
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
#endif
    "aes.AES",                 /*tp_name */
    sizeof(aes_AESObject),     /*tp_basicsize */
    0,                         /*tp_itemsize */
    0,                         /*tp_dealloc */
    0,                         /*tp_print */
    0,                         /*tp_getattr */
    0,                         /*tp_setattr */
    0,                         /*tp_compare */
    0,                         /*tp_repr */
    0,                         /*tp_as_number */
    0,                         /*tp_as_sequence */
    0,                         /*tp_as_mapping */
    0,                         /*tp_hash */
    0,                         /*tp_call */
    0,                         /*tp_str */
    0,                         /*tp_getattro */
    0,                         /*tp_setattro */
    0,                         /*tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /*tp_flags */
    "AES objects",             /*tp_doc */
    0,		               /*tp_traverse */
    0,		               /*tp_clear */
    0,		               /*tp_richcompare */
    0,		               /*tp_weaklistoffset */
    0,		               /*tp_iter */
    0,		               /*tp_iternext */
    aes_AESmethods,            /*tp_methods */
    aes_members,               /*tp_members */
    0,                         /*tp_getset */
    0,                         /*tp_base */
    0,                         /*tp_dict */
    0,                         /*tp_descr_get */
    0,                         /*tp_descr_set */
    0,                         /*tp_dictoffset */
    (initproc)py_aes_init,     /*tp_init */
    (allocfunc)secure_alloc,   /*tp_alloc */
    (newfunc)PyType_GenericNew,/*tp_new */
    (freefunc)secure_free,     /*tp_free */
};

/* module methods (none for now) */
static PyMethodDef aes_methods[] =
{
    {NULL}  /* Sentinel */
};

/* declarations for DLL import/export (if used in an embedded */ 
/* Python interpreter it is necessary to suppress the export  */
/* of the module initialisation function)                     */
#if defined( SUPPRESS_INITFUNC_EXPORT )
#  undef PyMODINIT_FUNC
#  if PY_MAJOR_VERSION >= 3
#    define PyMODINIT_FUNC PyObject *
#  else
#    define PyMODINIT_FUNC void
#  endif
#elif !defined( PyMODINIT_FUNC )
#define PyMODINIT_FUNC void
#endif

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef moduledef =
{
    PyModuleDef_HEAD_INIT,
    "AES",              /* m_name     */
    "Python Bindings",  /* m_doc      */
    -1,                 /* m_size     */
    aes_methods,        /* m_methods  */
    NULL,               /* m_reload   */
    NULL,               /* m_traverse */
    NULL,               /* m_clear    */
    NULL,               /* m_free     */
};

PyMODINIT_FUNC PyInit_aes(void)
{
    PyObject *m;

    /* aes_AESType.tp_new = PyType_GenericNew; */
    if (PyType_Ready(&aes_AESType) < 0)
        return NULL;

    m = PyModule_Create(&moduledef);
    Py_INCREF(&aes_AESType);
    PyModule_AddObject(m, "AES", (PyObject *)&aes_AESType);
    return m;
}

#else

PyMODINIT_FUNC initaes(void) {
    PyObject *m;

    /*aes_AESType.tp_new = PyType_GenericNew;*/
    if (PyType_Ready(&aes_AESType) < 0)
        return;

    m = Py_InitModule3("aes", aes_methods,
                       "Python bindings for Brian Gladman's AES code");

    Py_INCREF(&aes_AESType);
    PyModule_AddObject(m, "AES", (PyObject *)&aes_AESType);
}

#endif
