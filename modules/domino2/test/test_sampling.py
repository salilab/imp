import sys
import unittest
import IMP
import IMP.test
import IMP.domino2
import IMP.core
import IMP.atom
import IMP.helper
import time


class DOMINOTests(IMP.test.TestCase):

    def test_global_min2(self):
        """Test sampling"""
        m= IMP.Model()
        m.set_log_level(IMP.SILENT)
        ps=[]
        for i in range(0,3):
            p= IMP.Particle(m)
            d= IMP.core.XYZ.setup_particle(p)
            ps.append(p)
        pts=[IMP.algebra.Vector3D(0,0,0),
             IMP.algebra.Vector3D(1,0,0),
             IMP.algebra.Vector3D(2,0,0),
             IMP.algebra.Vector3D(3,0,0)]
        particle_state= IMP.domino2.XYZsStates(pts)
        pst= IMP.domino2.ParticleStatesTable()
        for p in ps:
            pst.set_particle_states(p, particle_state)
        sst= IMP.domino2.DefaultSubsetStatesTable(pst)
        m.add_restraint(IMP.core.DistanceRestraint(IMP.core.Harmonic(1,1), ps[0], ps[1]))
        m.add_restraint(IMP.core.DistanceRestraint(IMP.core.Harmonic(1,1), ps[1], ps[2]))

        sampler= IMP.domino2.DominoSampler(m, pst)
        sampler.set_log_level(IMP.VERBOSE)
        sampler.set_subset_states_table(sst)
        sampler.set_maximum_score(.1)
        cs= sampler.get_sample()
        self.assert_((IMP.core.get_distance(IMP.core.XYZ(ps[0]),IMP.core.XYZ(ps[1]))-1)**2 < .1)
        self.assert_((IMP.core.get_distance(IMP.core.XYZ(ps[1]),IMP.core.XYZ(ps[2]))-1)**2 < .1)

    def test_global_min3(self):
        """Test sampling with edge scores"""
        m= IMP.Model()
        m.set_log_level(IMP.SILENT)
        ps=[]
        for i in range(0,3):
            p= IMP.Particle(m)
            d= IMP.core.XYZ.setup_particle(p)
            ps.append(p)
        pts=[IMP.algebra.Vector3D(0,0,0),
             IMP.algebra.Vector3D(1,0,0),
             IMP.algebra.Vector3D(2,0,0),
             IMP.algebra.Vector3D(3,0,0)]
        particle_state= IMP.domino2.XYZsStates(pts)
        pst= IMP.domino2.ParticleStatesTable()
        for p in ps:
            pst.set_particle_states(p, particle_state)
            m.add_restraint(IMP._ConstRestraint(1, [p]))
        sst= IMP.domino2.DefaultSubsetStatesTable(pst)
        m.add_restraint(IMP.core.DistanceRestraint(IMP.core.Harmonic(1,1), ps[0], ps[1]))
        m.add_restraint(IMP.core.DistanceRestraint(IMP.core.Harmonic(1,1), ps[1], ps[2]))

        sampler= IMP.domino2.DominoSampler(m, pst)
        sampler.set_log_level(IMP.VERBOSE)
        sampler.set_subset_states_table(sst)
        sampler.set_maximum_score(3.1)
        cs= sampler.get_sample()
        self.assert_((IMP.core.get_distance(IMP.core.XYZ(ps[0]),IMP.core.XYZ(ps[1]))-1)**2 < .1)
        self.assert_((IMP.core.get_distance(IMP.core.XYZ(ps[1]),IMP.core.XYZ(ps[2]))-1)**2 < .1)

        # test that they are the right distance
if __name__ == '__main__':
    unittest.main()
