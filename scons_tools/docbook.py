# Simple DocBook tool and builder. Uses xmlto to build XHTML docs from
# DocBook sources.

import os.path
import re
from SCons.Script import File

def action_docbook(target, source, env):
    outdir = os.path.dirname(target[0].path)
    if env['DOCBOOK_FRAGMENT']:
        frag = '-m ' + env['DOCBOOK_FRAGMENT'].path
    else:
        frag = ''
    app = "xmlto -o %s %s xhtml %s" % (outdir, frag, source[0].path)
    return env.Execute(app)

def emit_docbook(target, source, env):
    # Pull out all files included by the first source (should really parse XML
    # instead of this regex)
    ent_re = re.compile('<!ENTITY\s+\S+\s+SYSTEM\s+\"(\S+)\">')
    sourcedir = os.path.dirname(source[0].abspath)
    for line in file(source[0].abspath, 'r'):
        m = ent_re.search(line)
        if m:
            # Make sure included files are relative to the path of first source
            source.append(File(os.path.join(sourcedir, m.group(1))))
    if env['DOCBOOK_FRAGMENT']:
        source.append(env['DOCBOOK_FRAGMENT'])
    return target, source

def generate(env):
    """Add builders and construction variables for the docbook tool."""
    from SCons.Builder import Builder
    builder = Builder(action=action_docbook, emitter=emit_docbook)
    # Use Unix 'install' rather than env.InstallAs(), due to scons bug #1751
    install = Builder(action="install -d ${TARGET.dir} && " + \
                             "install ${SOURCE.dir}/*.html ${TARGET.dir}")
    env.Append(BUILDERS = {'DocBook': builder, 'DocBookInstall':install})
    env.AppendUnique(DOCBOOK_FRAGMENT=None)

def exists(env):
    """Make sure docbook tools exist."""
    return env.Detect("xmlto")
