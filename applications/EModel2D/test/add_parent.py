import os
import sys
import inspect


def add_parent_to_python_path():
    """
        add the parent directory to the path
    """
    cfr = inspect.currentframe()
    cf = inspect.getfile(cfr)
    print os.path.split(cf)[0]
    cmd_dir = os.path.abspath(os.path.split(cf)[0])
    parent_dir = os.path.split(cmd_dir)[0]
    if parent_dir not in sys.path:
        sys.path.insert(0, parent_dir)
