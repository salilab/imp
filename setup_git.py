#! /usr/bin/env python
import sys
import os.path
import subprocess
import glob

module = False

if len(sys.argv) > 1:
    if sys.argv[1] == "--module":
        module=True
    else:
        print >> sys.stderr, "usage:", sys.argv[0], "[--module]"
        exit(1)

if not os.path.exists(".git"):
    print >> sys.stderr, "Script must be run from a git root directory"
    exit(1)

imp_root = os.path.split(sys.argv[0])[0]

sys.path.append(os.path.join(imp_root, "tools"))
import build.tools

print "imp root is", imp_root

config = os.path.join(imp_root, "tools", "git")
if module:
    build.tools.link_dir(os.path.join(config, "module_config", "hooks"), os.path.join(".git", "hooks"))
else:
    build.tools.link_dir(os.path.join(config, "config", "hooks"), os.path.join(".git", "hooks"))

config_contents = open(os.path.join(".git", "config"), "r").read()
if config_contents.find("gitflow") != -1:
    print "Git flow already set up"
else:
    os.system("git checkout master")
    os.system("git checkout develop")
    cmd = subprocess.Popen(["git", "flow", "init"], stdin=subprocess.PIPE,
                           stdout = subprocess.PIPE,
                           stderr = subprocess.PIPE)
    cmd.stdin.write("\n\n\n\n\n\n\n")
    err = cmd.stderr.read()
    if len(err) > 0:
        print >> sys.stderr, "No git flow found. If you are a developer, you should install it and rerun this script."
        print >> sys.stderr, err

print "Setting the default push to nothing, so you must specify what to push each time"
os.system("git config push.default nothing")
print "Setting up nice colors"
os.system("git config color.branch auto")
os.system("git config color.diff auto")
os.system("git config color.status auto")
os.system("git config color.branch.current yellow reverse")
os.system("git config color.branch.local yellow")
os.system("git config color.branch.remote green")
os.system("git config color.diff.meta \"yellow bold\"")
os.system("git config color.diff.frag \"magenta bold\"")
os.system("git config color.diff.old red")
os.system("git config color.diff.new cyan")
os.system("git config color.status.added yellow")
os.system("git config color.status.changed green")
os.system("git config color.status.untracked cyan")
print "Telling git to clean up whitespace"
os.system("git config core.whitespace \"fix,-indent-with-non-tab,trailing-space,cr-at-eol\"")

if not module:
    # anyone who is confused by branches should be on master
    os.system("git checkout master")
