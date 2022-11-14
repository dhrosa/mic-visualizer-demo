import os

os.environ["CC"] = "clang"
os.environ["CXX"] = "clang++"

import cppimport

lut_cpp = cppimport.imp_from_filepath("lut_cpp.cpp")
Table = lut_cpp.Table
