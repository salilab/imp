"""Simple configure checks for graphviz"""
import checks
import os
import sys

def _check_dot(context):
    has_dot = context.env['dot']
    context.Message('Checking for graphviz ...')
    if has_dot is False:
        context.Result("disabled")
        context.env['dot']=False
        return False
    os.environ['PATH']=context.env['ENV']['PATH']
    try:
        os.system("dot -V >&/dev/null")
    except:
        context.Result("not found")
        context.env['dot']=False
        return False
    context.Result("found")
    context.env['dot']=True
    return True

def configure_check_dot(env):
    custom_tests = {'CheckDot':_check_dot}
    conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
    conf.CheckDot()
    #else:
    #    env['dot']=False
    conf.Finish()


def _check_doxygen(context):
    os.environ['PATH']=context.env['ENV']['PATH']
    context.Message('Checking for doxygen ...')
    try:
        os.system("doxygen --version>/dev/null")
        # unfortunately this outputs a newline to stderr
        # I don't know how to avoid that
        ret= os.popen("doxygen --version").read()
    except:
        context.Result("not found")
        context.env['doxygen']=False
        return False
    if ret.find("1.6.1")!= -1:
        context.Result("disabled. Doxygen 1.6.1 does not work with IMP. Sorry.")
        context.env['doxygen']=False
        return False
    context.Result(ret)
    context.env['doxygen']=True
    return True

def configure_check_doxygen(env):
    custom_tests = {'CheckDoxygen':_check_doxygen}
    conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
    conf.CheckDoxygen()
    #else:
    #    env['doxygen']=False
    conf.Finish()
