#! /usr/bin/env python
"""Call the main setup_git.py. This should be
copied to the main directory of your project and named setup_git.py."""

import os
import os.path
os.system(os.path.join("tools", "dev_tools", "git", "setup_git.py"))
os.system("git config commit.template tools/git/commit_message.txt")
