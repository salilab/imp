#!/bin/sh

if test -e .git; then
git checkout develop
git pull
cd tools/git/config
for i in */*
do
rm -f ../../../.git/$i
ln -s ../../tools/git/config/$i ../../../.git/$i
done
cd ../../..
if test `git branch| grep master`; then
echo "Git flow assumed to be set up"
else
git checkout -b master origin/master
git checkout develop
echo "




" | git flow init
fi
else

echo "Script must be run from git root"
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
