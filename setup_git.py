#! /usr/bin/env python
"""Update submodules and then call the main setup_git.py. This should be
copied to the main directory of your project and named setup_git.py."""

import os
import os.path
os.system("git submodule update --init --recursive")
os.system(os.path.join("tools", "dev_tools", "git", "setup_git.py"))
os.system("git config commit.template tools/git/commit_message.txt")
