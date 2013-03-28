import IMP
import sys
import IMP.test
from StringIO import StringIO

class TestModule:
    _all_commands = ['foo', 'bar']
    def get_module_version():
        return 'testver'
    get_module_version = staticmethod(get_module_version)
    class submodule:
        pass
    class foo:
        __doc__ = 'doc for foo'
    class bar:
        def main():
            TestModule.bar_main_called = True
        main = staticmethod(main)

class Tests(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.old_argv = sys.argv
        self.old_stdout = sys.stdout
        sys.stdout = StringIO()
        sys.modules['TestModule'] = TestModule
        sys.modules['TestModule.submodule'] = TestModule.submodule
        sys.modules['TestModule.foo'] = TestModule.foo
        sys.modules['TestModule.bar'] = TestModule.bar

    def tearDown(self):
        sys.argv = self.old_argv
        sys.stdout = self.old_stdout
        del sys.modules['TestModule']
        del sys.modules['TestModule.submodule']
        del sys.modules['TestModule.foo']
        del sys.modules['TestModule.bar']
        IMP.test.TestCase.tearDown(self)

    def test_init(self):
        """Test CommandDispatcher init"""
        sys.argv = ['/foo/bar/testprogname', 'arg1']
        c = IMP.CommandDispatcher("short", "long", "TestModule")
        self.assertEqual(c._progname, 'testprogname')

    def test_import_module(self):
        """Test CommandDispatcher.import_module()"""
        c = IMP.CommandDispatcher("short", "long", "TestModule")
        self.assertEqual(id(c.import_module()), id(TestModule))
        self.assertEqual(id(c.import_module('submodule')),
                         id(TestModule.submodule))

    def test_unknown_command(self):
        """Test CommandDispatcher unknown command"""
        sys.argv = ['testprog', 'garbage']
        c = IMP.CommandDispatcher("short", "long", "TestModule")
        self.assertRaises(SystemExit, c.main)
        out = sys.stdout.getvalue()
        self.assertIn("Unknown command: 'garbage'", out)

    def test_no_args(self):
        """Test CommandDispatcher run with no arguments"""
        sys.argv = ['testprog']
        c = IMP.CommandDispatcher("short", "long", "TestModule")
        c.main()
        out = sys.stdout.getvalue()
        self.assertIn("short Use 'testprog help' for help.", out)

    def test_version(self):
        """Test CommandDispatcher show version"""
        sys.argv = ['testprog', '--version']
        c = IMP.CommandDispatcher("short", "long", "TestModule")
        c.main()
        out = sys.stdout.getvalue()
        self.assertEquals("testprog testver\n", out)

    def test_show_help(self):
        """Test CommandDispatcher show help"""
        for arg in ('help', '--help', '-h'):
            sys.argv = ['testprog', arg]
            c = IMP.CommandDispatcher("short", "long", "TestModule")
            c.main()
            self.assert_help_out_ok()

    def assert_help_out_ok(self):
        out = sys.stdout.getvalue()
        self.assertIn("testprog, version testver.", out)
        self.assertIn("bar   <no help>", out)
        self.assertIn("foo   doc for foo", out)
        self.assertIn("help  Get help on using testprog.", out)

    def test_do_command(self):
        """Test CommandDispatcher do command"""
        sys.argv = ['testprog', 'bar', 'arg1', 'arg2']
        c = IMP.CommandDispatcher("short", "long", "TestModule")
        TestModule.bar_main_called = False
        c.main()
        self.assertTrue(TestModule.bar_main_called)
        self.assertEqual(sys.argv, ['testprog bar', 'arg1', 'arg2'])

    def test_show_command_help(self):
        """Test CommandDispatcher show command help ok"""
        sys.argv = ['testprog', 'help', 'bar']
        c = IMP.CommandDispatcher("short", "long", "TestModule")
        TestModule.bar_main_called = False
        c.main()
        self.assertTrue(TestModule.bar_main_called)
        self.assertEqual(sys.argv, ['testprog bar', '--help'])

    def test_show_command_help_help(self):
        """Test CommandDispatcher show help command help"""
        sys.argv = ['testprog', 'help', 'help']
        c = IMP.CommandDispatcher("short", "long", "TestModule")
        c.main()
        self.assert_help_out_ok()

    def test_show_command_help_unknown(self):
        """Test CommandDispatcher show help of unknown command"""
        sys.argv = ['testprog', 'help', 'garbage']
        c = IMP.CommandDispatcher("short", "long", "TestModule")
        self.assertRaises(SystemExit, c.main)
        out = sys.stdout.getvalue()
        self.assertIn("Unknown command: 'garbage'", out)

if __name__ == '__main__':
    IMP.test.main()
