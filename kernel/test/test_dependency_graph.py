import unittest
import IMP
import IMP.test
import StringIO


class ModelTests(IMP.test.TestCase):
    def test_state_show(self):
        """Test dependency graph"""
        m = IMP.Model()
        dg= IMP.get_dependency_graph(m.get_score_states(),
                                     IMP.get_restraints(m.get_restraints())[0])
        print dir(dg)
        dg.show()

if __name__ == '__main__':
    unittest.main()
