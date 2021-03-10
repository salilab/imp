#!/usr/bin/env python

"""
Create the imppy.sh script using the passed parameters.
"""

import tools
from optparse import OptionParser
import os.path
import os
import platform
import stat


def uniq(ls):
    """Return the list with duplicate items removed"""
    seen = set()
    for obj in ls:
        if obj not in seen:
            seen.add(obj)
            yield obj


class FileGenerator(object):
    body = ["@LDPATH@", "", "@PYTHONPATH@", "",
            "# Where to find data for the various modules",
            "@IMP_DATA@", "",
            "# Extra places to look for imp modules",
            "@IMP_EXAMPLE_DATA@", "",
            "# Location of binaries (for wine builds, which don't get PATH)",
            "@IMP_BIN_DIR@", "",
            "@PATH@", "", "@PRECOMMAND@", "", "@TMPDIR@"]

    def __init__(self, options):
        self.options = options

    def native_paths(self, paths, also_with_suffix=False):
        """Convert cmake-provided paths into native paths"""
        ret = [tools.from_cmake_path(x) for x in paths]
        if self.options.suffix and also_with_suffix:
            ret += [os.path.join(tools.from_cmake_path(x),
                                 self.options.suffix) for x in paths]
        return ret

    def get_abs_binary_path(self, reldir):
        """Get an absolute path to a binary directory"""
        if self.options.suffix:
            reldir = os.path.join(reldir, self.options.suffix)
        return os.path.abspath(reldir)

    def get_path(self):
        modbin = [os.path.abspath(x) for x in tools.get_glob(["module_bin/*"])]
        if self.options.suffix:
            modbin += [os.path.join(x, self.options.suffix) for x in modbin]
        return (modbin + [self.get_abs_binary_path("bin")]
                + self.native_paths(self.options.path, True))

    def write_file(self):
        pypathsep = ";" if self.options.python_pathsep == 'w32' else os.pathsep
        outfile = self.options.output
        pythonpath = self.native_paths(self.options.python_path, True)
        ldpath = self.native_paths(self.options.ld_path)
        precommand = self.options.precommand
        path = self.get_path()
        externdata = self.native_paths(self.options.external_data)

        libdir = self.get_abs_binary_path("lib")
        bindir = self.get_abs_binary_path("bin")
        datadir = os.path.abspath("data")
        exampledir = os.path.abspath(os.path.join("doc", "examples"))
        tmpdir = os.path.abspath("tmp")

        if platform.system() == 'Linux':
            varname = "LD_LIBRARY_PATH"
        elif platform.system() == 'Darwin':
            varname = "DYLD_LIBRARY_PATH"
        else:
            varname = None

        lines = {"@LDPATH@": (varname, os.pathsep.join([libdir] + ldpath),
                              True, True),
                 "@PYTHONPATH@": ("PYTHONPATH",
                                  pypathsep.join(
                                      [libdir] + pythonpath), True, True),
                 "@IMP_BIN_DIR@": ("IMP_BIN_DIR", bindir, True, False),
                 "@PATH@":
                 ("PATH", os.pathsep.join(uniq([bindir] + path)), True, True),
                 "@PRECOMMAND@": ("precommand", precommand, False, False),
                 "@IMP_DATA@": ("IMP_DATA", ":".join([datadir] + externdata),
                                True, False),
                 "@IMP_EXAMPLE_DATA@": ("IMP_EXAMPLE_DATA",
                                        os.pathsep.join([exampledir]),
                                        True, False),
                 "@TMPDIR@": ("IMP_TMP_DIR", tmpdir, True, False)}

        contents = []

        for line in self.template:

            if line in lines:
                val = lines[line]
                if val[0] and len(val[1]) > 0:
                    # ick
                    if self.options.propagate == "no" or not val[3]:
                        contents.extend(self.set_variable(val[0], val[1],
                                                          val[2]))
                    else:
                        if 'PYTHONPATH' in val[0]:
                            sep = pypathsep
                        else:
                            sep = os.pathsep
                        contents.extend(self.set_variable_propagate(
                            val[0], val[1], val[2], sep))
            else:
                contents.append(line)
        tools.rewrite(outfile, "\n".join(contents), verbose=False)
        os.chmod(outfile, stat.S_IRWXU)


class ShellScriptFileGenerator(FileGenerator):
    template = ["#!/usr/bin/env sh", "", ""] + FileGenerator.body \
        + ["", "", "mkdir -p \"${IMP_TMP_DIR}\"", "",
           "exec ${precommand} \"$@\""]

    def _internal_set(self, setstr, varname, export):
        if export:
            return [setstr, "export " + varname]
        else:
            return [setstr]

    def set_variable(self, varname, value, export):
        return (
            self._internal_set(varname + '="' + value + '"', varname, export)
        )

    def set_variable_propagate(self, varname, value, export, sep):
        return self._internal_set(
            varname + '="' + value + '%s$%s"' % (sep, varname),
            varname, export)


class BatchFileGenerator(FileGenerator):
    template = [x for x in FileGenerator.body if not x.startswith('#')] \
        + ["", 'mkdir "%IMP_TMP_DIR%"']

    def set_variable(self, varname, value, export):
        return ['set %s=%s' % (varname, value)]

    def set_variable_propagate(self, varname, value, export, sep):
        return ['set %s=%s%s%%%s%%' % (varname, value, sep, varname)]

    def get_path(self):
        # Windows looks for libraries in PATH, not LD_LIBRARY_PATH
        return FileGenerator.get_path(self) \
            + self.native_paths(self.options.ld_path, True)


parser = OptionParser()
parser.add_option("-p", "--python_path", dest="python_path", default=[],
                  action="append", help="PYTHONPATH.")
parser.add_option("-l", "--ld_path", dest="ld_path", default=[],
                  action="append", help="LD_LIB_PATH.")
parser.add_option("-c", "--precommand", dest="precommand", default="",
                  help="Command to run before all executables.")
parser.add_option("-P", "--path", dest="path", default=[],
                  action="append", help="The PATH.")
parser.add_option("--python_pathsep", default="",
                  help="The Python path separator style "
                       "to use ('w32' or empty)")
parser.add_option("-d", "--external_data", dest="external_data", default=[],
                  action="append", help="External data.")
parser.add_option("-e", "--propagate", dest="propagate", default="no",
                  help="Whether to pass the relevant environment variables "
                       "through.")
parser.add_option("-o", "--output", dest="output", default="imppy.sh",
                  help="Name of the file to produce.")
parser.add_option("--suffix", default="",
                  help="Subdirectory to suffix to binary directories")


def main():
    (options, args) = parser.parse_args()
    if options.output.endswith('.bat'):
        gen = BatchFileGenerator(options)
    else:
        gen = ShellScriptFileGenerator(options)
    gen.write_file()


if __name__ == '__main__':
    main()
