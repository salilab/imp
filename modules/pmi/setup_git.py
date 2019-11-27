#! /usr/bin/env python
"""Call the main setup_git.py. This should be
copied to the main directory of your project and named setup_git.py."""

import subprocess
import os.path
import sys


subprocess.check_call(
    [sys.executable, os.path.join("tools", "dev_tools", "git", "setup_git.py")])
