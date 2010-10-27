import sys
import IMP
import IMP.test
import IMP.domino2
import IMP.core
import IMP.atom
import IMP.restrainer
import time

class DOMINOTests(IMP.test.TestCase):

    def test_global_min2(self):
        """Test optimization of model"""
        m= IMP.Model()
        m.set_log_level(IMP.VERBOSE)
        ps=[]
        vs=[IMP.algebra.Vector3D(0,0,0),
            IMP.algebra.Vector3D(0,0,100),
            IMP.algebra.Vector3D(100,0,0)]
        pst = IMP.domino2.ParticleStatesTable()
        for i in range(10):
            p= IMP.Particle(m)
            IMP.core.XYZR.setup_particle(p,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(i,i,i), i))
            ps.append(p)
            pst.set_particle_states(p, IMP.domino2.XYZStates(vs))
        IMP.set_log_level(IMP.VERBOSE)
        cp= IMP.container.ClosePairContainer(IMP.container.ListSingletonContainer(ps), 1)
        r=IMP.container.PairsRestraint(IMP.core.DistancePairScore(IMP.core.HarmonicLowerBound(0,1)), cp);
        m.add_restraint(r)
        print "getting graph"
        dg= IMP.get_dependency_graph([m.get_root_restraint_set()])
        dg.show()
        print "optimizing"
        doc= IMP.domino2.OptimizeContainers(m.get_root_restraint_set(), pst)
        dg= IMP.get_dependency_graph([m.get_root_restraint_set()])
        #dg.show()
        print "before"
        IMP.show_restraint_hierarchy(m.get_root_restraint_set())
        dor= IMP.domino2.OptimizeRestraints(m.get_root_restraint_set(), pst)
        print "after"
        IMP.show_restraint_hierarchy(m.get_root_restraint_set())
        print "optimized restraints"
        dg= IMP.get_dependency_graph([m.get_root_restraint_set()])
        #dg.show()
        self.assertGreater(len(IMP.get_restraints(m.get_root_restraint_set())),
                           1)
        del dor
        print "after delete"
        IMP.show_restraint_hierarchy(m.get_root_restraint_set())
        self.assertEqual(len(IMP.get_restraints(m.get_root_restraint_set())),
                         1)
if __name__ == '__main__':
    IMP.test.main()
