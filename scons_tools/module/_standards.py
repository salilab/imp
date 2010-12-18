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


class StandardsTest(IMP.test.TestCase):
    def test_value_objects(self):
        "Test that module classes are either values or objects"
        return self.assertValueObjects(%(module)s, %(value_object_exceptions)s)
    def test_classes(self):
        "Test that module class names follow the standards"
        return self.assertClassNames(%(module)s, %(value_object_exceptions)s)
    def test_functions(self):
        "Test that module function names follow the standards"
        return self.assertFunctionNames(%(module)s, %(function_name_exceptions)s)
    def test_show(self):
        "Test all objects have show"
        return self.assertShow(%(module)s, %(show_exceptions)s)

if __name__ == '__main__':
    IMP.test.main()
    """ %({'module':source[0].get_contents(),
    'plural_exceptions':source[1].get_contents(),
    'show_exceptions':source[2].get_contents(),
    'function_name_exceptions':source[3].get_contents(),
    'value_object_exceptions':source[4].get_contents(),
    'class_name_exceptions':source[5].get_contents()})
    open(target[0].abspath, 'w').write(file)


def _print(target, source, env):
    print "Creating standards test"

_Standards = Builder(action=Action(_action,
                                  _print))


def add(env, plural_exceptions=[], show_exceptions=[], function_name_exceptions=[],
        value_object_exceptions=[], class_name_exceptions=[]):
    name= scons_tools.environment.get_current_name(env)
    if name=='kernel':
        module="IMP"
    else:
        module="IMP."+name
    return _Standards(env, target=["test_standards.py"],
                      source=[env.Value(module),
                              env.Value(plural_exceptions),
                              env.Value(show_exceptions),
                              env.Value(function_name_exceptions),
                              env.Value(value_object_exceptions),
                              env.Value(class_name_exceptions)])[0]
