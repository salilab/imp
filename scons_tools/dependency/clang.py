from SCons.Script import Exit
import platform
def get_is_clang(env):
    if env.get('CXX', None):
        return env['CXX'].find('clang') != -1
    else:
        return False
