"""Simple configure checks for endianness"""

import scons_tools.module
from SCons.Script import Glob, Dir, File, Builder, Action, Exit
import os
import sys
import re

def _check(context):
    if context.env['endian'] == 'auto':
        context.Message("Checking endianess... ")
        text = """
#include <stdio.h>
int main(int argc, char ** argv) {
  union {
    char array[4];
    int integer;
  } TestUnion;
  TestUnion.array[0] = 'a';
  TestUnion.array[1] = 'b';
  TestUnion.array[2] = 'c';
  TestUnion.array[3] = 'd';
  if (TestUnion.integer == 0x64636261) {
    printf("little");
  } else if (TestUnion.integer == 0x61626364) {
    printf("big");
  } else {
    printf("unknown");
  }
  return 0;
}
"""
        ret = context.TryRun(text, ".cpp")
        if ret[0] == 0:
            context.env.Exit("Could not run endian check program")
        # Workaround for dumb systems (e.g. wine) which insert stuff into stdout:
        result = ret[1].split()[-1]
        # Make sure we got a sensible result:
        if result == 'little' or result == 'big' or result == 'unknown':
            context.Result(result)
            return result
        else:
            context.env.Exit("Got nonsensical endian: %s" % result)
    else:
        context.Message("Getting endianess... ")
        context.Result(context.env['endian'])
        return context.env['endian']


def configure_check(env):
    custom_tests = {'CheckEndian':_check}
    conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
    env['IMP_ENDIAN']=conf.CheckEndian()
    env.Append(IMP_CONFIGURATION=["endian='"+env['IMP_ENDIAN']+"'"])
    #else:
    #    env['IMP_ENDIAN']="not"
    conf.Finish()
