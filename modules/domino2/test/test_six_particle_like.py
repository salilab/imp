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
        """Test filtering across sets"""
        def setup_restraints(m, ps):
            pairs=[[0,1],[0,2],[1,2],[2,3],[3,4],[4,5],[3,5]]
            sf = IMP.core.Harmonic(1.0, 1)
            for pair in pairs:
                r=IMP.core.DistanceRestraint(sf, ps[pair[0]], ps[pair[1]])
                m.add_restraint(r)

        IMP.set_log_level(IMP.TERSE)
        m=IMP.Model()
        m.set_log_level(IMP.SILENT)
        print "create sampler"
        s=IMP.domino2.DominoSampler(m)
        print "setting up particles"
        ps=IMP.Particles()
        for i in range(0,6):
            p=IMP.Particle(m)
            IMP.core.XYZ.setup_particle(p,IMP.algebra.Vector3D(0.,0.,0.))
            ps.append(p)

            print "setting up a discrete set of states"
        vs=[]
        for i in range(0,4):
            vs.append(IMP.algebra.Vector3D(i,0,0))
            vs.append(IMP.algebra.Vector3D(i,1,0))

        print "create states"
        states= IMP.domino2.XYZStates(vs)
        for p in ps:
            print p.get_name()
            s.set_particle_states(p, states)

        print "setting up restraints"
        setup_restraints(m, ps)

        s.set_maximum_score(.2)
        s.set_log_level(IMP.VERBOSE)

        print "sampling"
        s.set_log_level(IMP.TERSE)
        cs=s.get_sample()

        print "Found ", cs.get_number_of_configurations(), "solutions"
        sols=[]
        for i in range(cs.get_number_of_configurations()):
            cs.load_configuration(i)
            sol=[]
            for p in ps:
                sol.append(IMP.core.XYZ(p).get_coordinate(0))
                sol.append(IMP.core.XYZ(p).get_coordinate(1))
                sol.append(IMP.core.XYZ(p).get_coordinate(2))
            self.assert_(sol not in sols)
            sols.append(sol)
            for p0 in ps:
                for p1 in ps:
                    if p0==p1:
                        continue
                    uneq=False
                    for i in range(0,3):
                        if IMP.core.XYZ(p0).get_coordinate(i) != IMP.core.XYZ(p1).get_coordinate(i):
                            uneq=True
                    if not uneq:
                        print i
                        print p0.get_name(), IMP.core.XYZ(p0)
                        print p1.get_name(), IMP.core.XYZ(p1)
                    self.assert_(uneq)
if __name__ == '__main__':
    unittest.main()
