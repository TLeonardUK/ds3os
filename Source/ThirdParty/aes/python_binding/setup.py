#!/usr/bin/env python

import platform
from distutils.core import setup, Extension

source_files = ['aesmodule.c', '../aeskey.c', '../aes_modes.c', '../aestab.c', '../aescrypt.c']

cflags = []
if platform.system() == 'Linux':
    cflags.append('-Wno-sequence-point')

if platform.machine() == 'x86_64':
    source_files.append('../aes_ni.c')
    cflags.append('-D__PROFILE_AES__')

setup(name='aes',
      version='1.0',
      ext_modules=[Extension('aes', source_files, include_dirs=['..'], extra_compile_args=cflags)],
     )

