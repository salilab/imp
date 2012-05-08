import IMP.test
import sys

class ExampleApplicationTest(IMP.test.ApplicationTestCase):
    def test_simple(self):
        """Simple test of example application"""
        p = self.run_application('example', [])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

if __name__ == '__main__':
    IMP.test.main()
