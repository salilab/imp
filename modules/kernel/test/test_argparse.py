from __future__ import print_function
import IMP.test
import sys

# argparse processing terminates early for --help or --version
class HelpTermination(Exception): pass
class AdvancedHelpTermination(Exception): pass
class VersionTermination(Exception): pass

def get_test_argparse():
    a = IMP.ArgumentParser()
    a.add_argument('--opt', help='test optional argument')
    a.add_argument('pos', help='test positional argument')
    return a

def parse_args(parser, args):
    """Run parser.parse_args() on args; return its result plus the command line
       that was passed on to IMP's internal Boost-based parser"""
    class MockSetup(object):
        def __init__(self):
            self.argv = []
        def __call__(self, argv, desc, posdesc, num_pos):
            self.argv = argv[1:]
            if '-h' in argv or '--help' in argv:
                raise HelpTermination()
            elif '--help_advanced' in argv:
                raise AdvancedHelpTermination()
            elif '--version' in argv:
                raise VersionTermination()
    old_setup = IMP.setup_from_argv
    try:
        m = MockSetup()
        IMP.setup_from_argv = m
        args = parser.parse_args(args)
        return args, m.argv
    finally:
        IMP.setup_from_argv = old_setup


class Tests(IMP.test.TestCase):

    def test_argparse_no_boost(self):
        """Test ArgumentParser with no Boost options"""
        p = get_test_argparse()
        args, boost = parse_args(p, ["--opt", "bar", "baz"])
        self.assertTrue('opt' in args)
        self.assertTrue('pos' in args)
        # Boost options shouldn't show up in output namespace
        self.assertFalse('help_advanced' in args)
        self.assertEqual(args.opt, 'bar')
        self.assertEqual(args.pos, 'baz')
        self.assertEqual(boost, [])
        # Bad optional argument
        self.assertRaises(SystemExit, parse_args, p, ["--bar", "bar", "baz"])
        # Missing positional argument
        self.assertRaises(SystemExit, parse_args, p, [])
        # dashdash should terminate handling of optional arguments
        args, boost = parse_args(p, ["--", "--opt"])
        self.assertEqual(args.opt, None)
        self.assertEqual(args.pos, '--opt')
        self.assertEqual(boost, [])

    def test_argparse_boost_terminate(self):
        """Test ArgumentParser with terminating Boost options"""
        p = get_test_argparse()
        # help/version should be handled immediately, before argparse notices
        # we're missing positional arguments
        self.assertRaises(HelpTermination, parse_args, p, ["--help"])
        self.assertRaises(AdvancedHelpTermination,
                          parse_args, p, ["--help_advanced"])
        self.assertRaises(VersionTermination, parse_args, p, ["--version"])

    def test_argparse_boost_equal(self):
        """Test ArgumentParser with Boost option=value"""
        p = get_test_argparse()
        args, boost = parse_args(p, ["--opt", "bar", "--show_seed",
                                     "--random_seed=5", "baz"])
        self.assertEqual(args.opt, 'bar')
        self.assertEqual(args.pos, 'baz')
        self.assertEqual(boost, ['--show_seed', '--random_seed', '5'])

    def test_argparse_boost_spaced(self):
        """Test ArgumentParser with Boost option and value as separate words"""
        p = get_test_argparse()
        args, boost = parse_args(p, ["--opt", "bar", "--show_seed",
                                     "--random_seed", "5", "baz"])
        self.assertEqual(args.opt, 'bar')
        self.assertEqual(args.pos, 'baz')
        self.assertEqual(boost, ['--show_seed', '--random_seed', '5'])


if __name__ == '__main__':
    IMP.test.main()
