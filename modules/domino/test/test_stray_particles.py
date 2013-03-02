import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.algebra
import random



class Tests(IMP.test.TestCase):
    def test_global_min1(self):
        """Test handling of unattached particles"""
        m= IMP.Model()
        p= IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p)
        pst= IMP.domino.ParticleStatesTable()
        s= IMP.domino.XYZStates([IMP.algebra.Vector3D(0,0,0)])
        pst.set_particle_states(p, s)
        #rft= IMP.domino.RestraintScoreSubsetFilterTable(m, pst)
        sampler= IMP.domino.DominoSampler(m, pst)
        sample=sampler.get_sample()
        self.assertEqual(sample.get_number_of_configurations(), 1)
if __name__ == '__main__':
    IMP.test.main()
