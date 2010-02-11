import os
import sys

def get_example_path(file):
    return os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), file))
