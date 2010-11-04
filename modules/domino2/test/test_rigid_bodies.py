import sys
import IMP
import IMP.test
import IMP.domino2
import IMP.core
import IMP.atom
import IMP.restrainer
import time
from IMP.algebra import *

class DOMINOTests(IMP.test.TestCase):
    def _create_rb(self, name, m):
        p= IMP.Particle(m)
        p.set_name(name+"particle")
        d=IMP.core.XYZR.setup_particle(p)
        d.set_radius(1)
        d.set_coordinates(Vector3D(0,0,0))
        rbp= IMP.Particle(m)
        rbp.set_name(name+"rb")
        rbd=IMP.core.RigidBody.setup_particle(rbp, [p])
        return rbd
    def test_global_min1(self):
        """Testing finding minima with rigid bodies"""
        m= IMP.Model()
        rbs= [self._create_rb("1",m), self._create_rb("2", m)]
        trs= [Transformation3D(get_identity_rotation_3d(), Vector3D(0,0,0)),
              Transformation3D(get_identity_rotation_3d(), Vector3D(2,0,0)),
              Transformation3D(get_identity_rotation_3d(), Vector3D(4,0,0))]
        ps= IMP.core.HarmonicSphereDistancePairScore(0,1)
        r= IMP.core.PairRestraint(ps, (rbs[0].get_members()[0], rbs[1].get_members()[0]))
        r.set_name("restraint")
        m.add_restraint(r)
        m.set_maximum_score(r, .5)
        pst= IMP.domino2.ParticleStatesTable()
        pstate= IMP.domino2.RigidBodyStates(trs)
        pst.set_particle_states(rbs[0], pstate)
        pst.set_particle_states(rbs[1], pstate)
        rg= IMP.domino2.get_restraint_graph(m.get_root_restraint_set(), pst)
        rg.show_dotty()
        print "ig"
        ig= IMP.domino2.get_interaction_graph(m.get_root_restraint_set(), pst)
        ig.show_dotty()
        print "dg"
        dg= IMP.get_dependency_graph([m.get_root_restraint_set()])
        dg.show_dotty()
        print "jt"
        jt= IMP.domino2.get_junction_tree(ig)
        jt.show_dotty()
        s= IMP.domino2.DominoSampler(m, pst)
        s.set_log_level(IMP.VERBOSE)
        cg= s.get_sample()
        print cg.get_number_of_configurations()

if __name__ == '__main__':
    IMP.test.main()
