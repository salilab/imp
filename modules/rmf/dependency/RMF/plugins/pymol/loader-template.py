import sys
import os.path

sys.path.append(os.path.join( % CMAKE_BINARY_DIR%, "lib"))

execfile(os.path.join( % PROJECT_SOURCE_DIR%, "plugins", "pymol", "plugin.py"))
