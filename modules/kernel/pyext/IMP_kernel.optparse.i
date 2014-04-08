%pythoncode %{

import optparse
import sys

class OptionParser(optparse.OptionParser):
    """IMP-specific subclass of optparse.OptionParser.
       This adds options common to all IMP applications
       (see IMP::base::setup_from_argv()).
    """

    _use_boost_parser = True

    def __init__(self, *args, **kwargs):
        # Don't add --help or --version options (since the Boost
        # option parser handles those)
        kwargs['add_help_option'] = False
        kwargs['version'] = None
        # Handle old users of IMP.OptionParser that set imp_module
        if 'imp_module' in kwargs:
            del kwargs['imp_module']
        try:
            optparse.OptionParser.__init__(self, *args, **kwargs)
        except TypeError:
            if 'epilog' in kwargs:
                # Older optparse doesn't support the epilog keyword
                del kwargs['epilog']
                optparse.OptionParser.__init__(self, *args, **kwargs)
            else:
                raise

    # Don't complain if invalid options are encountered; pass them through
    # unmodified
    def _process_long_opt(self, rargs, values):
        if self._use_boost_parser:
            try:
                optparse.OptionParser._process_long_opt(self, rargs, values)
            except optparse.BadOptionError, err:
                if not hasattr(err, 'opt_str') \
                   and err.msg.startswith('no such option:'):
                    self.largs.append(err.msg[16:])
                else:
                    self.largs.append(err.opt_str)
        else:
            optparse.OptionParser._process_long_opt(self, rargs, values)
    def _process_short_opts(self, rargs, values):
        if self._use_boost_parser:
            try:
                optparse.OptionParser._process_short_opts(self, rargs, values)
            except optparse.BadOptionError, err:
                self.largs.append(err.opt_str)
        else:
            optparse.OptionParser._process_short_opts(self, rargs, values)

    def _process_args(self, largs, rargs, values):
        if self._use_boost_parser:
            # If the special argument -- is present, preserve it (by default
            # optparse will remove it). Otherwise, the Boost option parser will
            # not treat positional arguments after -- correctly.
            try:
                dashdash = rargs.index('--')
            except ValueError:
                dashdash = len(rargs)
            saved_args = rargs[dashdash:]
            del rargs[dashdash:]
            optparse.OptionParser._process_args(self, largs, rargs, values)
            rargs.extend(saved_args)
        else:
            optparse.OptionParser._process_args(self, largs, rargs, values)

    def parse_args(self, num_positional=None):
        """Parse the command line and return options and positional arguments.
           This functions in the same way as the method in the base class
           optparse.OptionParser, except that it also processes options common
           to all IMP applications (these are not returned in opts, but can be
           obtained in the usual way, e.g. by calling
           IMP::base::get_string_flag()).
           @param[in] num_positional If a positive integer, exactly that
                      many positional arguments must be specified on the
                      command line; if a negative integer, that many or
                      more must be given; if None, any number of positional
                      arguments can be given.
           @returns   opts, args
        """
        import IMP.base
        # First, parse the command line with optparse
        opts, args = optparse.OptionParser.parse_args(self)
        if not self._use_boost_parser:
            return opts, args
        orig_desc = self.description
        orig_usage = self.usage
        if self.usage:
            usage = self.usage.replace('%prog ', '')
        else:
            usage = ''
        if num_positional is None:
            # If num_positional was not specified, prevent the Boost parser
            # from complaining about the number of positional arguments
            numpos = -1
            args.append('dummy')
        else:
            numpos = num_positional
        try:
            self.usage = self.description = None
            # Pass the rest of the command line to the Boost parser; add the
            # optparse-generated help to that from Boost
            args = IMP.base.setup_from_argv([sys.argv[0]] + args,
                           self.format_help() \
                           + "\nOptions common to all IMP applications:",
                           usage, numpos)
        finally:
            self.description = orig_desc
            self.usage = orig_usage
        if num_positional is None:
            args.pop()
        return opts, args
%}
