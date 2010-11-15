import sys
import IMP
import IMP.test
import IMP.domino2
import IMP.core
import IMP.atom
import IMP.container
import IMP.restrainer
import time
from IMP.algebra import *

class DOMINOTests(IMP.test.TestCase):
    def _create_rb(self, name, m, np=1):
        ps=[]
        for i in range(0, np):
            p= IMP.Particle(m)
            p.set_name(name+"particle"+str(i))
            d=IMP.core.XYZR.setup_particle(p)
            d.set_radius(1)
            d.set_coordinates(get_random_vector_in(get_unit_sphere_3d()))
            ps.append(p)
        rbp= IMP.Particle(m)
        rbp.set_name(name+"rb")
        rbd=IMP.core.RigidBody.setup_particle(rbp, ps)
        return rbd
    def test_global_min2(self):
        """Testing finding minima with rigid bodies"""
        m= IMP.Model()
        rbs= [self._create_rb("1",m), self._create_rb("2", m)]
        trs= [ReferenceFrame3D(Transformation3D(get_identity_rotation_3d(),
                                                Vector3D(0,0,0))),
              ReferenceFrame3D(Transformation3D(get_identity_rotation_3d(),
                                                Vector3D(2,0,0))),
              ReferenceFrame3D(Transformation3D(get_identity_rotation_3d(),
                                                Vector3D(4,0,0)))]
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
        #rg.show_dotty()
        print "ig"
        ig= IMP.domino2.get_interaction_graph(m.get_root_restraint_set(), pst)
        #ig.show_dotty()
        print "dg"
        dg= IMP.get_dependency_graph([m.get_root_restraint_set()])
        #dg.show_dotty()
        print "jt"
        jt= IMP.domino2.get_junction_tree(ig)
        #jt.show_dotty()
        s= IMP.domino2.DominoSampler(m, pst)
        s.set_log_level(IMP.VERBOSE)
        cg= s.get_sample()
        self.assertEqual( cg.get_number_of_configurations(), 4)
    def test_global_min1(self):
        """Testing splitting restraints with rigid bodies"""
        m= IMP.Model()
        rbs= [self._create_rb("1",m), self._create_rb("2", m),
              self._create_rb("3",m)]
        trs= [ReferenceFrame3D(Transformation3D(get_identity_rotation_3d(),
                                                Vector3D(0,0,0))),
              ReferenceFrame3D(Transformation3D(get_identity_rotation_3d(),
                                                Vector3D(2,0,0))),
              ReferenceFrame3D(Transformation3D(get_identity_rotation_3d(),
                                                Vector3D(4,0,0)))]
        ps= IMP.core.HarmonicSphereDistancePairScore(0,1)
        members=[x.get_members()[0] for x in rbs]
        pl=IMP.container.ListPairContainer([(members[0], members[1]), (members[1], members[2])])
        r= IMP.container.PairsRestraint(ps, pl)
        r.set_name("restraint")
        m.add_restraint(r)
        m.set_maximum_score(r, .5)
        pst= IMP.domino2.ParticleStatesTable()
        pstate= IMP.domino2.RigidBodyStates(trs)
        pst.set_particle_states(rbs[0], pstate)
        pst.set_particle_states(rbs[1], pstate)
        pst.set_particle_states(rbs[2], pstate)
        oc= IMP.domino2.OptimizeContainers(m.get_root_restraint_set(), pst)
        occ= IMP.domino2.OptimizeRestraints(m.get_root_restraint_set(), pst)
        allr= IMP.get_restraints(m.get_root_restraint_set())
        for r in allr:
            print r.get_name()
        self.assertEqual(len(allr), 2)
    def test_global_min3(self):
        """Testing splitting restraints with rigid bodies and interestings scores"""
        m= IMP.Model()
        rbs= [self._create_rb("1",m, 4), self._create_rb("2", m, 4),
              self._create_rb("3",m, 4)]
        trs= [ReferenceFrame3D(Transformation3D(get_identity_rotation_3d(),
                                                Vector3D(0,0,0))),
              ReferenceFrame3D(Transformation3D(get_identity_rotation_3d(),
                                                Vector3D(2,0,0))),
              ReferenceFrame3D(Transformation3D(get_identity_rotation_3d(),
                                                Vector3D(4,0,0)))]
        lps=IMP.core.SphereDistancePairScore(IMP.core.HarmonicUpperBound(0,1))
        ps= IMP.core.KClosePairsPairScore(lps, IMP.core.RigidMembersRefiner())
        pl=IMP.container.ListPairContainer([(rbs[0], rbs[1]), (rbs[1], rbs[2])])
        r= IMP.container.PairsRestraint(ps, pl)
        r.set_name("restraint")
        m.add_restraint(r)
        m.set_maximum_score(r, .5)
        dg= IMP.get_dependency_graph([m.get_root_restraint_set()])
        dg.show_dotty()
        pst= IMP.domino2.ParticleStatesTable()
        pstate= IMP.domino2.RigidBodyStates(trs)
        pst.set_particle_states(rbs[0], pstate)
        pst.set_particle_states(rbs[1], pstate)
        pst.set_particle_states(rbs[2], pstate)
        oc= IMP.domino2.OptimizeContainers(m.get_root_restraint_set(), pst)
        occ= IMP.domino2.OptimizeRestraints(m.get_root_restraint_set(), pst)
        allr= IMP.get_restraints(m.get_root_restraint_set())
        for r in allr:
            print r.get_name()
        self.assertEqual(len(allr), 2)


if __name__ == '__main__':
    IMP.test.main()
