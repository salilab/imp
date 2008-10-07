"""Simple configure checks for CGAL"""

import os.path
from SCons.Script import *

def _check(context):
    context.Message('Checking for CGAL ...')
    cgal = context.env['cgal']
    if cgal is False or cgal is 0:
        context.Result("disabled")
        return False

    ret = context.TryLink("""
    #include <CGAL/box_intersection_d.h>
    int main()
    {
        return 0;
    }
    """, '.cpp')
    if ret:
        context.env['CGAL_LIBS'] = ['CGAL']
    context.Result(ret)
    return ret

def configure_check(env):
    env['CGAL_LIBS'] = ''
    custom_tests = {'CheckCGAL':_check}
    conf = env.Configure(custom_tests=custom_tests)
    if not env.GetOption('clean') and not env.GetOption('help'):
        conf.CheckCGAL()
    conf.Finish()
