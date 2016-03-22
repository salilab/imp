from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container

class TestEM2D(IMP.test.TestCase):
    def test_something(self):
        try:
            import IMP.em2d
        except:
            self.skipTest("No EM2D module")
        import IMP.pmi.restraints.em2d

if __name__ == '__main__':
    IMP.test.main()
