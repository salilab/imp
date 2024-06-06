#!/usr/bin/env python

""" Compute the dependencies of a swig file by running swig.
"""

import sys
from argparse import ArgumentParser
import os.path
import tools
import tools.thread_pool


parser = ArgumentParser()
parser.add_argument("--build_dir", help="IMP build directory", default=None)
parser.add_argument("--module_name", help="Module name", default=None)
parser.add_argument("--include", help="Extra header include path",
                    default=None)
parser.add_argument("-s", "--swig", dest="swig", default="swig",
                    help="The name of the swig command.")


def _fix(name):
    if os.path.isabs(name):
        return name
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


def setup_one(finder, module, swig, extra_data_path, include):
    includepath = get_dep_merged(finder, [module], "includepath",
                                 extra_data_path)
    swigpath = get_dep_merged(finder, [module], "swigpath", extra_data_path)

    with open("src/%s_swig.deps.in" % module.name, "w") as _:
        pass
    baseincludes = []
    if include:
        baseincludes.append(include)
    baseincludes.extend(('include', 'swig'))
    cmd = [swig, "-python", "-MM"] + ["-I" + x for x in baseincludes] \
        + ["-ignoremissing"] \
        + ["-I" + x for x in swigpath] + ["-I" + x for x in includepath] \
        + ["swig/IMP_%s.i" % module.name]

    lines = tools.run_subprocess(cmd).split("\n")
    names = []
    for x in lines:
        if x.endswith("\\"):
            x = x[:-1]
        x = x.strip()
        if not x.endswith(".h") and not x.endswith(".i") \
                and not x.endswith(".i-in"):
            continue
        names.append(x)

    final_names = [_fix(x) for x in names]
    final_list = "\n".join(final_names)
    tools.rewrite("src/%s_swig.deps" % module.name, final_list)


def main():
    args = parser.parse_args()
    mf = tools.ModulesFinder(configured_dir="build_info",
                             external_dir=args.build_dir,
                             module_name=args.module_name)
    pool = tools.thread_pool.ThreadPool()
    for m in [x for x in mf.values()
              if not isinstance(x, tools.ExternalModule) and x.ok
              and not x.python_only]:
        pool.add_task(setup_one, mf, m, args.swig,
                      args.build_dir, args.include)
    err = pool.wait_completion()
    if err:
        sys.stderr.write(err + '\n')
        return 1
    return 0


if __name__ == '__main__':
    main()
