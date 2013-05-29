#!/usr/bin/env sh
sed -i.old "s#IMP_COMPILER_ENABLE_WARNINGS#IMP_COMPILER_ENABLE_WARNINGS;#g" $*
sed -i.old "s#IMP_COMPILER_DISABLE_WARNINGS#IMP_COMPILER_DISABLE_WARNINGS;#g" $*
sed -i.old "s#END_NAMESPACE#END_NAMESPACE;#g" $*
sed -i.old "s#END_INTERNAL_NAMESPACE#END_INTERNAL_NAMESPACE;#g" $*
sed -i.old "s#BEGIN_NAMESPACE#BEGIN_NAMESPACE;#g" $*
sed -i.old "s#BEGIN_INTERNAL_NAMESPACE#BEGIN_INTERNAL_NAMESPACE;#g" $*

for i in $*; do
echo $i
clang-format -i -style=Google $i >& /dev/null
done

sed -i.old "s#END_NAMESPACE;#END_NAMESPACE#g" $*
sed -i.old "s#END_INTERNAL_NAMESPACE;#END_INTERNAL_NAMESPACE#g" $*
sed -i.old "s#BEGIN_NAMESPACE;#BEGIN_NAMESPACE#g" $*
sed -i.old "s#BEGIN_INTERNAL_NAMESPACE;#BEGIN_INTERNAL_NAMESPACE#g" $*
# no spaces at end of lines
sed -i.old "s# *\$##g" $*
sed -i.old "s#IMP_COMPILER_ENABLE_WARNINGS;#IMP_COMPILER_ENABLE_WARNINGS#g" $*
sed -i.old "s#IMP_COMPILER_DISABLE_WARNINGS;#IMP_COMPILER_DISABLE_WARNINGS#g" $*
sed -i.old "s#IMP_COMPILER_DISABLE_WARNINGS;#IMP_COMPILER_DISABLE_WARNINGS#g" $*
# fix swig directives in sources
sed -i.old "s#^% template#%template#g" $*
