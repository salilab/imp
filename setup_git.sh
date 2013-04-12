#!/bin/sh

if test \! -e .git; then
echo "Script must be run from git root"
exit
fi

cd tools/git/config
for i in */*
do
rm -f ../../../.git/$i
ln -s ../../tools/git/config/$i ../../../.git/$i
done
cd ../../..


if grep flow .git/config > /dev/null; then
    echo "Git flow assumed to be set up"
elif command -v git-flow > /dev/null; then
    echo "




" | git flow init
else
echo "No git flow found. If you are a developer, you should install it and rerun this script."
fi

echo "Setting the default push to nothing, so you must specify what to push each time"
git config push.default nothing
echo "Setting up nice colors"
git config color.branch auto
git config color.diff auto
git config color.status auto
git config color.branch.current yellow reverse
git config color.branch.local yellow
git config color.branch.remote green
git config color.diff.meta "yellow bold"
git config color.diff.frag "magenta bold"
git config color.diff.old red
git config color.diff.new cyan
git config color.status.added yellow
git config color.status.changed green
git config color.status.untracked cyan
echo "Telling git to clean up whitespace"
git config core.whitespace "fix,-indent-with-non-tab,trailing-space,cr-at-eol"
