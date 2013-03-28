%pythoncode %{

import sys
import os

class CommandDispatcher(object):
    """Allow applications to easily implement commmands.
       Typically, an IMP application will use an instance of this class
       to provide a consistent interface to multiple distinct commands
       from a single binary, rather than providing a potentially large
       number of binaries.

       Each command is implemented with a Python module of the same name
       that can be imported from the module (for example, if module_name
       is 'IMP.foo', the 'bar' command is provided by the IMP.foo.bar Python
       module, which would usually be found as modules/foo/pyext/src/bar.py).
       Each such module should have a docstring (__doc__) and a main() method
       that takes no arguments (the module should also call its main() method
       if it is run directly, i.e. with something like
       'if __name__=="__main__": main()'). The encompassing module
       ('IMP.foo' in the example) should define _all_commands as a Python
       list of all valid commands.

       @param short_help A few words that describe the application.
       @param long_help Longer text, used in the 'help' command.
       @param module_name Name of the module (e.g. 'IMP.foo') that implements
                          the commands.
    """

    def __init__(self, short_help, long_help, module_name):
        self.short_help = short_help
        self.long_help = long_help
        self.module_name = module_name
        self._all_commands = self.import_module()._all_commands
        self._progname = os.path.basename(sys.argv[0])

    def main(self):
        if len(sys.argv) <= 1:
            print self.short_help + " Use '%s help' for help." % self._progname
        else:
            command = sys.argv[1]
            if command in ('help', '--help', '-h'):
                if len(sys.argv) == 3:
                    self.show_command_help(sys.argv[2])
                else:
                    self.show_help()
            elif command == '--version':
                self.show_version()
            elif command in self._all_commands:
                self.do_command(command)
            else:
                self.unknown_command(command)

    def import_module(self, mod=None):
        modname = self.module_name
        if mod is not None:
            modname += "." + mod
        return __import__(modname, {}, {}, [''])

    def unknown_command(self, command):
        print "Unknown command: '%s'" % command
        print "Use '%s help' for help." % self._progname
        sys.exit(1)

    def _get_version(self):
        return self.import_module().get_module_version()

    def show_version(self):
        print self._progname + ' ' + self._get_version()

    def show_help(self):
        ver = self._get_version()
        print "%s, version %s." % (self._progname, ver)
        print self.long_help + """

This program is part of IMP, the Integrative Modeling Platform,
which is Copyright 2007-2013 IMP Inventors.
For additional information about IMP, see http://salilab.org/imp/

Usage: %s <command> [options] [args]

Commands:""" % self._progname
        commands = self._all_commands[:] + ['help']
        commands.sort()
        cmdlen = max([len(c) for c in commands])
        for c in commands:
            if c == 'help':
                doc = 'Get help on using %s.' % self._progname
            else:
                doc = self.import_module(c).__doc__ or "<no help>"
            c += ' ' * (cmdlen - len(c))
            print '    ' + c + '  ' + doc
        print """
Use "%s help <command>" for detailed help on any command
 or "%s --version" to see the version number.""" % (self._progname,
                                                    self._progname)

    def do_command(self, command):
        mod = self.import_module(command)
        sys.argv[0] = self._progname + ' ' + command
        del sys.argv[1]
        mod.main()

    def show_command_help(self, command):
        if command == 'help':
            self.show_help()
        elif command in self._all_commands or command == 'help':
            mod = self.import_module(command)
            sys.argv = [self._progname + ' ' + command, '--help']
            mod.main()
        else:
            self.unknown_command(command)

%}
