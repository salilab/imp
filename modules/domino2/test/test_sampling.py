import sys
import IMP
import IMP.test
import IMP.domino2
import IMP.core
import IMP.atom
import IMP.helper
import time


class DOMINOTests(IMP.test.TestCase):

    def _test_global_min2(self):
        """Test sampling"""
        m= IMP.Model()
        m.set_log_level(IMP.SILENT)
        ps=[]
        print 1
        for i in range(0,3):
            p= IMP.Particle(m)
            d= IMP.core.XYZ.setup_particle(p)
            ps.append(p)
            print 2
        pts=[IMP.algebra.Vector3D(0,0,0),
             IMP.algebra.Vector3D(1,0,0),
             IMP.algebra.Vector3D(2,0,0),
             IMP.algebra.Vector3D(3,0,0)]
        particle_state= IMP.domino2.XYZStates(pts)
        pst= IMP.domino2.ParticleStatesTable()
        print 3
        for p in ps:
            pst.set_particle_states(p, particle_state)
        print 4
        sst= IMP.domino2.DefaultSubsetStatesTable(pst)
        m.add_restraint(IMP.core.DistanceRestraint(IMP.core.Harmonic(1,1), ps[0], ps[1]))
        m.add_restraint(IMP.core.DistanceRestraint(IMP.core.Harmonic(1,1), ps[1], ps[2]))
        print 5
        sampler= IMP.domino2.DominoSampler(m, pst)
        sampler.set_log_level(IMP.VERBOSE)
        sampler.set_subset_states_table(sst)
        sampler.set_maximum_score(.1)
        cs= sampler.get_sample()
        print 6
        self.assertGreater(cs.get_number_of_configurations(), 0)
        for i in range(0, cs.get_number_of_configurations()):
            print 7,i
            cs.load_configuration(i)
            print 7,i,0
            d0=IMP.core.XYZ(ps[0])
            d1=IMP.core.XYZ(ps[1])
            d2=IMP.core.XYZ(ps[2])
            print 7,1,1.5
            self.assertLess((IMP.core.get_distance(d0,d1)-1)**2, .1)
            print 7,i,1
            self.assertLess((IMP.core.get_distance(d1,d2)-1)**2, .1)

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
        particle_state= IMP.domino2.XYZStates(pts)
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
        self.assertGreater(cs.get_number_of_configurations(), 0)
        for i in range(0, cs.get_number_of_configurations()):
            cs.load_configuration(i)
            self.assertLess((IMP.core.get_distance(IMP.core.XYZ(ps[0]),IMP.core.XYZ(ps[1]))-1)**2, .1)
            self.assertLess((IMP.core.get_distance(IMP.core.XYZ(ps[1]),IMP.core.XYZ(ps[2]))-1)**2, .1)

        # test that they are the right distance
if __name__ == '__main__':
    IMP.test.main()
