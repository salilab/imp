#!/usr/bin/env python

import sys
import os.path
import subprocess
import glob
from optparse import OptionParser
import shutil

dev_tools_path = os.path.join("tools", "dev_tools")


opt = OptionParser()
opt.add_option("-g", "--global",
               action="store_true", dest="glob", default=False,
               help="Set global git settings instead of repo settings [default]")

(options, args) = opt.parse_args()

if options.glob:
    git_config = "git config --global --replace-all"
    config_contents = ""
else:
    git_config = "git config --replace-all"

    if not os.path.exists(".git"):
        sys.stderr.write("Script must be run from a git root directory\n")
        exit(1)

    if not os.path.exists(os.path.join("tools", "dev_tools")):
        sys.stderr.write("Script expects to find tools/dev_tools\n")
        exit(1)

    cmd = subprocess.Popen(["git", "rev-parse", "--git-dir"],
                           stdout=subprocess.PIPE, universal_newlines=True)
    git_dir = cmd.stdout.read().split('\n')[0]
    hdir = os.path.join(git_dir, "hooks")
    for f in glob.glob(os.path.join(dev_tools_path, "git", "hooks", "*")):
        out = os.path.join(hdir, os.path.split(f)[1])
        if os.path.exists(out):
            os.unlink(out)
        shutil.copy2(f, out)
    shutil.copy2(os.path.join(dev_tools_path, "check_standards.py"), hdir)
    out_tools = os.path.join(hdir, "python_tools")
    if os.path.exists(out_tools):
        shutil.rmtree(out_tools)
    shutil.copytree(
        os.path.join(
            dev_tools_path,
            "python_tools"),
        out_tools)
    config_contents = open(os.path.join(git_dir, "config"), "r").read()

    # make sure version is updated
    os.system(os.path.join(".", git_dir, "hooks", "post-commit"))

# hard to check for
os.system(git_config + " push.default nothing")
os.system(git_config + " log.decorate full")

if config_contents.find("color \"branch\"") == -1:
    print("Updating git colors")
    os.system(git_config + " color.ui true")
    os.system(git_config + " color.branch true")
    os.system(git_config + " color.diff true")
    os.system(git_config + " color.status true")
    os.system(git_config + " color.branch.current yellow reverse")
    os.system(git_config + " color.branch.local yellow")
    os.system(git_config + " color.branch.remote green")
    os.system(git_config + " color.diff.meta \"yellow bold\"")
    os.system(git_config + " color.diff.frag \"magenta bold\"")
    os.system(git_config + " color.diff.old red")
    os.system(git_config + " color.diff.new cyan")
    os.system(git_config + " color.status.added yellow")
    os.system(git_config + " color.status.changed green")
    os.system(git_config + " color.status.untracked cyan")
if config_contents.find("whitespace = fix,-indent-with-non-tab,trailing-space,cr-at-eol") == -1:
    print("Telling git to clean up whitespace")
    os.system(
        git_config +
        " core.whitespace \"fix,-indent-with-non-tab,trailing-space,cr-at-eol\"")

if config_contents.find("autosetuprebase = always") == -1:
    print("Telling git to rebase by default on pull")
    os.system(git_config + " branch.autosetuprebase always")
# hard to check for
os.system(git_config + " branch.develop.rebase true")
os.system(git_config + " branch.master.rebase true")
