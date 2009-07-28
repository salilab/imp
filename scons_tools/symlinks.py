"""Pseudo-builders that use symlinks rather than copying files, on platforms
   where symlinks are available."""

import os

def _linkFunc(dest, source, env):
    """Link a file from source to dest"""
    if os.path.isdir(source):
        raise SCons.Errors.UserError("Source must be a file, not a directory")
    else:
        # Cannot use os.path.exists here (does not work for broken symlinks);
        # cannot use os.path.lexists either (not present in older Pythons)
        try:
            os.unlink(dest)
        except OSError:
            pass
        if os.path.isabs(source) or os.path.isabs(dest):
            os.symlink(os.path.abspath(source), dest)
        else:
            # If both paths are relative to top-level directory, figure out how
            # to get from dest to the top level, then make a relative symlink:
            updirs = len(os.path.normpath(dest).split(os.path.sep)) - 1
            uppath = os.path.sep.join([os.path.pardir] * updirs)
            os.symlink(os.path.join(uppath, source), dest)
    return 0

def LinkInstall(env, target, source, **keys):
    """Like the standard Install builder, but using symlinks if available."""
    if hasattr(os, 'symlink'):
        return env.Install(target, source, INSTALL=_linkFunc, **keys)
    else:
        return env.Install(target, source, **keys)

def LinkInstallAs(env, target, source, **keys):
    """Like the standard InstallAs builder, but using symlinks if available."""
    if hasattr(os, 'symlink'):
        return env.InstallAs(target, source, INSTALL=_linkFunc, **keys)
    else:
        return env.InstallAs(target, source, **keys)
