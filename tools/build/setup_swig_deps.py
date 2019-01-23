#!/usr/bin/env python

""" Compute the dependencies of a swig file by running swig.
"""

import sys
from optparse import OptionParser
import os.path
import tools
import tools.thread_pool


parser = OptionParser()
parser.add_option("--build_dir", help="IMP build directory", default=None)
parser.add_option("--include", help="Extra header include path", default=None)
parser.add_option("-s", "--swig",
                  dest="swig", default="swig", help="The name of the swig command.")
parser.add_option("-b", "--build_system",
                  dest="build_system", help="The build system being used.")


def _fix(name, bs):
    if os.path.isabs(name):
        return name
    elif bs == "scons":
        return "#/build/" + name
    else:
        return os.path.join(os.getcwd(), "%s") % name


def get_dep_merged(finder, modules, name, extra_data_path):
    ret = []
    alldeps = finder.get_all_dependencies(modules)
    for d in alldeps:
        info = tools.get_dependency_info(d, extra_data_path)
        # cmake lists are semicolon-separated
        lst = tools.split(info[name], ';')
        ret.extend(lst)
    ret = sorted(set(ret))
    return ret


def setup_one(finder, module, build_system, swig, extra_data_path, include):
    includepath = get_dep_merged(finder, [module], "includepath",
                                 extra_data_path)
    swigpath = get_dep_merged(finder, [module], "swigpath", extra_data_path)

    depf = open("src/%s_swig.deps.in" % module.name, "w")
    baseincludes = []
    if include:
        baseincludes.append(include)
    baseincludes.extend(('include', 'swig'))
    cmd = [swig, "-MM"] + ["-I" + x for x in baseincludes] + ["-ignoremissing"]\
        + ["-I" + x for x in swigpath] + ["-I" + x for x in includepath]\
        + ["swig/IMP_%s.i" % module.name]

    lines = tools.run_subprocess(cmd).split("\n")
    names = []
    for x in lines:
        if x.endswith("\\"):
            x = x[:-1]
        x = x.strip()
        if not x.endswith(".h") and not x.endswith(".i") and not x.endswith(".i-in"):
            continue
        names.append(x)

    final_names = [_fix(x, build_system) for x in names]
    final_list = "\n".join(final_names)
    tools.rewrite("src/%s_swig.deps" % module.name, final_list)


def main():
    (options, args) = parser.parse_args()
    mf = tools.ModulesFinder(configured_dir="build_info",
                             external_dir=options.build_dir)
    pool = tools.thread_pool.ThreadPool()
    for m in [x for x in mf.values()
              if not isinstance(x, tools.ExternalModule) and x.ok]:
        pool.add_task(setup_one, mf, m, options.build_system, options.swig,
                      options.build_dir, options.include)
    err = pool.wait_completion()
    if err:
        sys.stderr.write(err + '\n')
        return 1
    return 0


if __name__ == '__main__':
    main()
