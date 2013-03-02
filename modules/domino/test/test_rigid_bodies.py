import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.container
from IMP.algebra import *

class Tests(IMP.test.TestCase):
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
        r.set_maximum_score(.5)
        pst= IMP.domino.ParticleStatesTable()
        pstate= IMP.domino.RigidBodyStates(trs)
        pst.set_particle_states(rbs[0], pstate)
        pst.set_particle_states(rbs[1], pstate)
        rg= IMP.domino.get_restraint_graph(m.get_root_restraint_set(), pst)
        #rg.show_dotty()
        print "ig"
        ig= IMP.domino.get_interaction_graph([m.get_root_restraint_set()], pst)
        #IMP.show_graphviz(ig)
        print "dg"
        IMP.base.set_log_level(IMP.base.VERBOSE)
        dg= IMP.get_dependency_graph(m)
        #IMP.show_graphviz(dg)
        print "jt"
        jt= IMP.domino.get_junction_tree(ig)
        #jt.show_dotty()
        s= IMP.domino.DominoSampler(m, pst)
        s.set_log_level(IMP.base.VERBOSE)
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
        r.set_maximum_score(.5)
        pst= IMP.domino.ParticleStatesTable()
        pstate= IMP.domino.RigidBodyStates(trs)
        pst.set_particle_states(rbs[0], pstate)
        pst.set_particle_states(rbs[1], pstate)
        pst.set_particle_states(rbs[2], pstate)
        rc = IMP.domino.RestraintCache(pst);
        rc.add_restraints([m])
        allr= rc.get_restraints()
        for r in allr:
            print r.get_name()
        self.assertEqual(len(allr), 3)
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
        r.set_maximum_score(.5)
        dg= IMP.get_dependency_graph(m)
        #dg.show_dotty()
        pst= IMP.domino.ParticleStatesTable()
        pstate= IMP.domino.RigidBodyStates(trs)
        pst.set_particle_states(rbs[0], pstate)
        pst.set_particle_states(rbs[1], pstate)
        pst.set_particle_states(rbs[2], pstate)
        #occ= IMP.domino.OptimizeRestraints(m.get_root_restraint_set(), pst)
        rc= IMP.domino.RestraintCache(pst)
        rc.add_restraints([m])
        allr=rc.get_restraints()
        for r in allr:
            print r.get_name()
        self.assertEqual(len(allr), 3)
    def _create_rigid_body(self, m):
        p= IMP.Particle(m, "rb")
        rb= IMP.core.RigidBody.setup_particle(p, IMP.algebra.ReferenceFrame3D())
        pm= IMP.Particle(m, "m")
        IMP.core.XYZ.setup_particle(pm)
        rb.add_member(pm)
        return p
    def test_interaction_graph(self):
        """Testing the interaction graph with rigid bodies"""
        m= IMP.Model()
        rb0 = self._create_rigid_body(m)
        rb1 = self._create_rigid_body(m)
        rb2 = self._create_rigid_body(m)
        r= IMP.core.PairRestraint(IMP.core.HarmonicDistancePairScore(0,1),
                                  (rb0, rb1))
        m.add_restraint(r)
        dg= IMP.get_dependency_graph(m)
        #IMP.base.show_graphviz(dg)
        ig= IMP.domino.get_interaction_graph(m, [rb0, rb1, rb2])
        #IMP.base.show_graphviz(ig)
        for v in ig.get_vertices():
            if ig.get_vertex_name(v) == rb0 or ig.get_vertex_name(v)==rb1:
                self.assertEqual(len(ig.get_out_neighbors(v)),
                                 1)
                self.assertEqual(len(ig.get_in_neighbors(v)),
                                 1)
            else:
                self.assertEqual(len(ig.get_out_neighbors(v)),
                                0)
                self.assertEqual(len(ig.get_in_neighbors(v)),
                                0)
if __name__ == '__main__':
    IMP.test.main()
