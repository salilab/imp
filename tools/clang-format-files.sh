#!/usr/bin/env sh

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
