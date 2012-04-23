import os


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


#
def get_experiment_params(fn_params):
    """
        Imports the configuration file
        @param fn_params configuration file
        @return Experiment Class with all the infomation from the config file
    """

    base, ext = os.path.splitext(fn_params)
    import imp
    foo = imp.load_source(base,fn_params)
    return foo.Experiment()
