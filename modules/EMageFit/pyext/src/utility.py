import os
import IMP.base as base


def vararg_callback(option, opt_str, value, parser):
    """
        Snippet from Python website to process multiple values for
        an option with OptionParser
    """

    assert value is None
    value = []

    def floatable(str):
        try:
            float(str)
            return True
        except ValueError:
            return False

    for arg in parser.rargs:
        # stop on --foo like options
        if arg[:2] == "--" and len(arg) > 2:
            break
        # stop on -a, but not on -3 or -3.0
        if arg[:1] == "-" and len(arg) > 1 and not floatable(arg):
            break
        value.append(arg)

    del parser.rargs[:len(value)]
    setattr(parser.values, option.dest, value)

    return value


def get_experiment_params(fn_params):
    """
        Imports the configuration file
        @param fn_params configuration file
        @return Experiment Class with all the infomation from the config file
    """

    name, ext = os.path.splitext(fn_params)
    import imp
    foo = imp.load_source(name, fn_params)
    exp = foo.Experiment()
    # convert to absolute paths
    exp.fn_pdbs = [base.get_relative_path(fn_params, fn) for fn in exp.fn_pdbs]
    if hasattr(exp, "sampling_positions"):
        exp.sampling_positions.read = base.get_relative_path(
            fn_params, exp.sampling_positions.read)
    if hasattr(exp, "benchmark"):
        if hasattr(exp.benchmark, "fn_pdb_native"):
            exp.benchmark.fn_pdb_native = base.get_relative_path(
                fn_params, exp.benchmark.fn_pdb_native)
        if hasattr(exp.benchmark, "fn_pdbs_native"):
            fns = []
            for fn in exp.benchmark.fn_pdbs_native:
                fns.append(base.get_relative_path(fn_params, fn))
            exp.benchmark.fn_pdbs_native = fns

    if hasattr(exp, "dock_transforms"):
        for i in range(len(exp.dock_transforms)):
            exp.dock_transforms[i][2] = base.get_relative_path(
                fn_params, exp.dock_transforms[i][2])
    if hasattr(exp, "em2d_restraints"):
        for i in range(len(exp.em2d_restraints)):
            exp.em2d_restraints[i][1] = base.get_relative_path(
                fn_params, exp.em2d_restraints[i][1])
    return exp
