from __future__ import print_function
import IMP.test
import sys

def parse_args(parser, args, num_positional=None):
    """Run parser.parse_args() on args; return its result plus any unhandled
       args (which would normally be handled by IMP's internal boost-based
       parser)"""
    class MockSetup(object):
        def __call__(self, argv, desc, posdesc, num_pos):
            self.argv = argv
            return argv[1:]
    old_argv = sys.argv
    old_setup = IMP.setup_from_argv
    try:
        m = MockSetup()
        sys.argv = args
        IMP.setup_from_argv = m
        opts, args = parser.parse_args(num_positional)
        return opts, args, m.argv
    finally:
        IMP.setup_from_argv = old_setup
        sys.argv = old_argv


class Tests(IMP.test.TestCase):

    def test_optparse_variable_positional(self):
        """Test OptionParser with variable number of positional args"""
        with IMP.allow_deprecated():
            p = IMP.OptionParser()
        p.add_option("-p", nargs=1, dest="point")
        opts, args, rest = parse_args(p, ['prog', "-p", "bar", "baz"])
        self.assertEqual(opts.point, 'bar')
        self.assertEqual(args, ['baz'])
        self.assertEqual(rest, ['prog', 'baz', 'dummy'])

    def test_optparse_fixed_positional(self):
        """Test OptionParser with fixed number of positional args"""
        with IMP.allow_deprecated():
            p = IMP.OptionParser()
        p.add_option("-p", nargs=1, dest="point")
        opts, args, rest = parse_args(p, ['prog', "-p", "bar", "baz"], 1)
        self.assertEqual(opts.point, 'bar')
        self.assertEqual(args, ['baz'])
        self.assertEqual(rest, ['prog', 'baz'])

    def test_optparse_usage(self):
        """Test OptionParser with provided usage message"""
        with IMP.allow_deprecated():
            p = IMP.OptionParser(usage='foo')
        p.add_option("-p", nargs=1, dest="point")
        opts, args, rest = parse_args(p, ['prog', "-p", "bar", "baz"])
        self.assertEqual(opts.point, 'bar')
        self.assertEqual(args, ['baz'])
        self.assertEqual(rest, ['prog', 'baz', 'dummy'])

    def test_optparse_dashdash(self):
        """Test OptionParser with special -- argument"""
        with IMP.allow_deprecated():
            p = IMP.OptionParser(usage='foo')
        p.add_option("-p", nargs=1, dest="point")
        opts, args, rest = parse_args(p, ['prog', "--", "-p", "bar", "baz"])
        self.assertEqual(opts.point, None)
        self.assertEqual(args, ['--', '-p', 'bar', 'baz'])
        self.assertEqual(rest, ['prog', '--', '-p', 'bar', 'baz', 'dummy'])

    def test_optparse_longopt(self):
        """Test OptionParser with long options"""
        with IMP.allow_deprecated():
            p = IMP.OptionParser(usage='foo')
        p.add_option("--point", nargs=1, dest="point")
        opts, args, rest = parse_args(p, ['prog', "--point", "bar", "baz"])
        self.assertEqual(opts.point, 'bar')
        self.assertEqual(args, ['baz'])
        self.assertEqual(rest, ['prog', 'baz', 'dummy'])


if __name__ == '__main__':
    IMP.test.main()
