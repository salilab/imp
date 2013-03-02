import IMP
import IMP.test
import StringIO


class Tests(IMP.test.TestCase):
    def test_state_show(self):
        """Test dependency graph"""
        m = IMP.Model("dependency graph")
        dg= IMP.get_dependency_graph(m)
        print dir(dg)
        dg.show()

if __name__ == '__main__':
    IMP.test.main()
