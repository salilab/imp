import IMP
import IMP.test

class RestraintSetTests(IMP.test.TestCase):
    """Test RestraintSets"""

    def test_weights(self):
        """Test that restraints decompose ok"""
        m= IMP.Model()
        r= IMP._ConstRestraint(1)
        m.add_restraint(r)
        rd= r.create_decomposition()
        self.assertEqual(r.evaluate(False), rd.evaluate(False))
        ra= IMP.get_restraints([r])
        rda= IMP.get_restraints([rd])
        self.assertEqual(sum(m.evaluate(ra, False), 0.0), sum(m.evaluate(rda, False), 0.0))
if __name__ == '__main__':
    IMP.test.main()
