#!/bin/bash
cd include
doxygen ../docs/Doxyfile
cd ../docs
rm -r sphinx
sphinx-build -b html . sphinx
cd ..
