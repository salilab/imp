%{
#include <IMP/internal/base_static.h>
namespace IMP {
namespace internal {
extern IMPKERNELEXPORT std::string exe_usage;
extern IMPKERNELEXPORT std::string exe_description;
extern IMPKERNELEXPORT Flag<bool> help_advanced;
}
}

void get_flag_subset(const boost::program_options::options_description &f,
                     unsigned ntokens, Strings &s) {
  for (std::vector<boost::shared_ptr<
       boost::program_options::option_description> >::const_iterator
       it = f.options().begin(); it != f.options().end(); ++it) {
    if (it->get()->semantic()->min_tokens() == ntokens) {
      s.push_back(it->get()->long_name());
    }
  }
}
%}

%inline %{

// Get names of all IMP flags taking the given number of arguments (0 or 1)
IMP::Strings _get_all_flags(unsigned ntokens) {
  Strings s;
  get_flag_subset(internal::flags, ntokens, s);
  get_flag_subset(internal::advanced_flags, ntokens, s);
  return s;
}

// Print help message of internal Boost-based command line parser
void _print_internal_help(std::ostream &out, std::string description) {
  // Python already handles the usage message
  internal::exe_usage = "==SUPPRESS==";
  internal::exe_description = description;
  internal::help_advanced = false;
  write_help(out);
}

%}

%pythoncode %{

try:
    import argparse
except ImportError:
    from . import _compat_argparse as argparse
import sys

class _PassThroughAction(argparse.Action):
    """Pass an argument through to the IMP Boost parser"""
    def __call__(self, parser, namespace, values, option_string=None):
        parser._boost_command_line.append(option_string)
        if values != []:
            parser._boost_command_line.append(values)
        # Terminate processing immediately if necessary (otherwise argparse
        # may fail if we're missing positional arguments)
        if option_string in ('-h', '--help', '--help_advanced', '--version'):
            parser._handle_boost()


class ArgumentParser(argparse.ArgumentParser):
    """IMP-specific subclass of argparse.ArgumentParser.
       This adds options common to all IMP applications
       (see IMP::setup_from_argv()).
    """

    def __init__(self, *args, **kwargs):
        # Don't add --help option (since the Boost option parser handles it)
        kwargs['add_help'] = False
        super(ArgumentParser, self).__init__(*args, **kwargs)
        for ntoken in (0, 1):
            flags = _get_all_flags(ntoken)
            for f in flags:
                arg = ['-h', '--help'] if f == 'help' else ['--' + f]
                self.add_argument(*arg, help=argparse.SUPPRESS,
                                  nargs=None if ntoken else 0,
                                  action=_PassThroughAction,
                                  default=argparse.SUPPRESS)

    def parse_args(self, args=None, namespace=None):
        """Parse the command line and return optional and positional arguments.
           This functions in the same way as the method in the base class
           argparse.ArgumentParser, except that it also processes optional
           arguments common to all IMP applications (these are not returned
           in `args`, but can be obtained in the usual way, e.g. by calling
           IMP::get_string_flag()).
           @returns   args
        """
        self._boost_command_line = [sys.argv[0]]
        ret = super(ArgumentParser, self).parse_args(args, namespace)
        if len(self._boost_command_line) > 1:
            self._handle_boost()
        return ret

    def _get_description(self):
        return self.format_help() + "\nOptions common to all IMP applications:"

    def print_help(self, file=None):
        _print_internal_help(file if file else sys.stdout,
                             self._get_description())

    def _handle_boost(self):
        setup_from_argv(self._boost_command_line,
                        self._get_description(),
                        '==SUPPRESS==', 0)
%}
