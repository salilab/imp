%pythoncode %{

try:
    import argparse
except ImportError:
    import _compat_argparse as argparse
import sys

class _PassThroughAction(argparse.Action):
    """Pass through a non-arg option to the IMP Boost parser"""
    def __init__(self, option_strings, dest=argparse.SUPPRESS,
                 default=argparse.SUPPRESS, help=argparse.SUPPRESS):
        super(_PassThroughAction, self).__init__(
            option_strings=option_strings, dest=dest, default=default,
            nargs=0, help=help)

    def __call__(self, parser, namespace, values, option_string=None):
        parser._handle_boost([option_string])


class ArgumentParser(argparse.ArgumentParser):
    """IMP-specific subclass of argparse.ArgumentParser.
       This adds options common to all IMP applications
       (see IMP::setup_from_argv()).
    """

    def __init__(self, *args, **kwargs):
        # Don't add --help option (since the Boost option parser handles it)
        kwargs['add_help'] = False
        super(ArgumentParser, self).__init__(*args, **kwargs)
        for arg in (['-h', '--help'],
                    ['--help_advanced'],
                    ['--version']):
            self.add_argument(*arg, help=argparse.SUPPRESS,
                              action=_PassThroughAction)

    def parse_args(self, args=None, namespace=None):
        """Parse the command line and return optional and positional arguments.
           This functions in the same way as the method in the base class
           argparse.ArgumentParser, except that it also processes optional
           arguments common to all IMP applications (these are not returned
           in `args`, but can be obtained in the usual way, e.g. by calling
           IMP::get_string_flag()).
           @returns   args
        """
        n, remain = super(ArgumentParser, self).parse_known_args(
                                                        args, namespace)
        # Pass the rest of the command line to the Boost parser; add the
        # argparse-generated help to that from Boost
        self._handle_boost(remain)
        return n

    def _handle_boost(self, args):
        setup_from_argv([sys.argv[0]] + args,
                        self.format_help() \
                         + "\nOptions common to all IMP applications:",
                         '==SUPPRESS==', 0)
%}
