"""Pseudo-builders that use symlinks rather than copying files, on platforms
   where symlinks are available."""

import os

def _linkFunc(dest, source, env):
    """Link a file from source to dest"""
    if os.path.isdir(source):
        raise SCons.Errors.UserError("Source must be a file, not a directory")
    else:
        if os.path.lexists(dest):
            os.unlink(dest)
        os.symlink(os.path.abspath(source), dest)
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
