#! /usr/bin/env python
import os
import os.path
os.system("git submodule update --init --recursive --rebase")
os.system("git submodule update --recursive --rebase")
os.system(os.path.join("tools", "dev_tools", "git", "setup_git.py"))
