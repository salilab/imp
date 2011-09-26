import IMP
import IMP.test
import StringIO


class ModelTests(IMP.test.TestCase):
    def test_state_show(self):
        """Test dependency graph"""
        m = IMP.Model()
        dg= IMP.get_dependency_graph([m.get_root_restraint_set()])
        print dir(dg)
        dg.show()

if __name__ == '__main__':
    IMP.test.main()
