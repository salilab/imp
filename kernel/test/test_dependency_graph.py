import unittest
import IMP
import IMP.test
import StringIO


class ModelTests(IMP.test.TestCase):
    def test_state_show(self):
        """Test dependency graph"""
        m = IMP.Model()
        dg= m.get_dependency_graph()
        print dir(dg)
        dg.show()

if __name__ == '__main__':
    unittest.main()
