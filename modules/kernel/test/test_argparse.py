from __future__ import print_function
import IMP.test
import sys

def parse_args(parser, args):
    """Run parser.parse_args() on args; return its result plus any unhandled
       args (which would normally be handled by IMP's internal boost-based
       parser)"""
    class MockSetup(object):
        def __call__(self, argv, desc, posdesc, num_pos):
            self.argv = argv
            return argv[1:]
    old_setup = IMP.setup_from_argv
    try:
        m = MockSetup()
        IMP.setup_from_argv = m
        args = parser.parse_args(args)
        return args, m.argv
    finally:
        IMP.setup_from_argv = old_setup


class Tests(IMP.test.TestCase):

    def test_argparse_variable_positional(self):
        """Test ArgumentParser with variable number of positional args"""
        p = IMP.ArgumentParser()
        p.add_argument("-p", dest="point")
        p.add_argument("foo")
        args, rest = parse_args(p, ["-p", "bar", "baz", "xyz"])
        self.assertEqual(args.point, 'bar')
        self.assertEqual(args.foo, 'baz')
        self.assertEqual(rest, [sys.argv[0], 'xyz'])


if __name__ == '__main__':
    IMP.test.main()
