import IMP
import IMP.test

class RestraintSetTests(IMP.test.TestCase):
    """Test RestraintSets"""

    def test_weights(self):
        """Test that restraints decompose ok"""
        m= IMP.Model()
        p= IMP.Particle(m)
        r= IMP._ConstRestraint(1, [p])
        m.add_restraint(r)
        rd= r.create_decomposition()
        self.assertEqual(r.evaluate(False), rd.evaluate(False))
        ra= IMP.get_restraints([r])
        rda= IMP.get_restraints([rd])
        sf= IMP.RestraintsScoringFunction(ra)
        sfda= IMP.RestraintsScoringFunction(rda)
        self.assertEqual(sf.evaluate( False), sfda.evaluate(False))
if __name__ == '__main__':
    IMP.test.main()
