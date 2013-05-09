#!/usr/bin/env sh

modfiles=`git status --porcelain | grep "^ M "`
if test -n "$modfiles"; then
  echo "This script can only be run on a clean repository."
  exit 1
fi

sed -i.old "s#END_NAMESPACE#END_NAMESPACE;#g" $*
sed -i.old "s#END_INTERNAL_NAMESPACE#END_INTERNAL_NAMESPACE;#g" $*
sed -i.old "s#BEGIN_NAMESPACE#BEGIN_NAMESPACE;#g" $*
sed -i.old "s#BEGIN_INTERNAL_NAMESPACE#BEGIN_INTERNAL_NAMESPACE;#g" $*

clang-format -i  -style=Google $*

sed -i.old "s#END_NAMESPACE;#END_NAMESPACE#g" $*
sed -i.old "s#END_INTERNAL_NAMESPACE;#END_INTERNAL_NAMESPACE#g" $*
sed -i.old "s#BEGIN_NAMESPACE;#BEGIN_NAMESPACE#g" $*
sed -i.old "s#BEGIN_INTERNAL_NAMESPACE;#BEGIN_INTERNAL_NAMESPACE#g" $*
sed -i.old "s# *\$##g" $*

for i in $*
do
match=`echo $i | grep _macros.h`
if test -n "$match"; then
git checkout $i
fi
done