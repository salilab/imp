#! /usr/bin/env python
import os
import os.path
os.system("git submodule update --init --recursive")
os.system("git submodule update --recursive")
os.system(os.path.join("tools", "dev_tools", "git", "setup_git.py"))
