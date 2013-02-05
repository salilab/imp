from SCons.Script import Glob, Dir, File, Builder, Action, Exit, Scanner, Move
import SCons
import os
import sys
import re
import scons_tools.environment

def _action(target, source, env):
    file="""
import IMP
import IMP.test
import %(module)s

spelling_exceptions=%(spelling_exceptions)s

class StandardsTest(IMP.test.TestCase):
    def test_value_objects(self):
        "Test that module classes are either values or objects"
        exceptions= %(value_object_exceptions)s
        return self.assertValueObjects(%(module)s,exceptions)
    def test_classes(self):
        "Test that module class names follow the standards"
        exceptions=%(value_object_exceptions)s
        return self.assertClassNames(%(module)s, exceptions,
                                     spelling_exceptions)
    def test_functions(self):
        "Test that module function names follow the standards"
        exceptions= %(function_name_exceptions)s
        return self.assertFunctionNames(%(module)s, exceptions,
                                        spelling_exceptions)
    def test_show(self):
        "Test all objects have show"
        exceptions=%(show_exceptions)s
        return self.assertShow(%(module)s, exceptions)

if __name__ == '__main__':
    IMP.test.main()
    """ %({'module':source[0].get_contents(),
    'plural_exceptions':source[1].get_contents(),
    'show_exceptions':source[2].get_contents(),
    'function_name_exceptions':source[3].get_contents(),
    'value_object_exceptions':source[4].get_contents(),
    'class_name_exceptions':source[5].get_contents(),
    'spelling_exceptions':source[6].get_contents()})
    open(target[0].abspath, 'w').write(file)


def _print(target, source, env):
    print "Creating standards test"

_Standards = Builder(action=Action(_action,
                                  _print))


def add(env, plural_exceptions=[], show_exceptions=[], function_name_exceptions=[],
        value_object_exceptions=[], class_name_exceptions=[],
        spelling_exceptions=[]):
    name= scons_tools.environment.get_current_name(env)
    module="IMP."+scons_tools.module._get_module_name(env)
    return _Standards(env, target=["#/build/test/test_%s_standards.py"%name],
                      source=[env.Value(module),
                              env.Value(plural_exceptions),
                              env.Value(show_exceptions),
                              env.Value(function_name_exceptions),
                              env.Value(value_object_exceptions),
                              env.Value(class_name_exceptions),
                              env.Value([x.lower() \
                                         for x in spelling_exceptions])])[0]
