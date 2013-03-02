import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.algebra
import random

class LogRestraint(IMP.Restraint):
    def __init__(self, m, ps):
        IMP.Restraint.__init__(self, m)
        self.count=0
        self.ps=ps
    def unprotected_evaluate(self, da):
        self.count+=1
        return 1
    def get_input_particles(self):
        return self.ps
    def reset(self):
        self.count=0
    def get_input_containers(self):
        return []
    def show(self, out):
        return


class Tests(IMP.test.TestCase):
    def test_global_min1(self):
        """Test caching of restraint scores"""
        m= IMP.Model()
        p= IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p)
        lr= LogRestraint(m, [p])
        lr.set_maximum_score(0)
        m.add_restraint(lr)
        pst= IMP.domino.ParticleStatesTable()
        s= IMP.domino.XYZStates([IMP.algebra.Vector3D(0,0,0)])
        pst.set_particle_states(p, s)
        rft= IMP.domino.RestraintScoreSubsetFilterTable(m, pst)
        f= rft.get_subset_filter(IMP.domino.Subset([p]),[])
        IMP.base.set_check_level(IMP.base.NONE)
        lr.reset()
        f.get_is_ok(IMP.domino.Assignment([0]))
        self.assertEqual(lr.count, 1)
        f.get_is_ok(IMP.domino.Assignment([0]))
        # they can get re-evaluated for checking purposes
        self.assertEqual(lr.count, 1)
    def test_global_min2(self):
        """Test non-caching of restraint scores"""
        m= IMP.Model()
        p= IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p)
        lr= LogRestraint(m, [p])
        lr.set_maximum_score(0)
        m.add_restraint(lr)
        pst= IMP.domino.ParticleStatesTable()
        s= IMP.domino.XYZStates([IMP.algebra.Vector3D(0,0,0)])
        pst.set_particle_states(p, s)
        rc= IMP.domino.RestraintCache(pst, 0)
        rc.add_restraints([m])
        rft= IMP.domino.RestraintScoreSubsetFilterTable(rc)
        f= rft.get_subset_filter(IMP.domino.Subset([p]),[])
        lr.reset()
        f.get_is_ok(IMP.domino.Assignment([0]))
        self.assertEqual(lr.count, 1)
        f.get_is_ok(IMP.domino.Assignment([0]))
        self.assertEqual(lr.count, 2)
    def test_global_min3(self):
        """Test capped caching of restraint scores"""
        m= IMP.Model()
        p= IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p)
        lr= LogRestraint(m, [p])
        lr.set_maximum_score(0)
        m.add_restraint(lr)
        pst= IMP.domino.ParticleStatesTable()
        s= IMP.domino.XYZStates([IMP.algebra.Vector3D(0,0,0),
                                 IMP.algebra.Vector3D(0,0,1)])
        pst.set_particle_states(p, s)
        rc= IMP.domino.RestraintCache(pst, 1)
        rc.add_restraints([m])
        rft= IMP.domino.RestraintScoreSubsetFilterTable(rc)
        f= rft.get_subset_filter(IMP.domino.Subset([p]),[])
        f.set_log_level(IMP.base.VERBOSE)
        IMP.base.set_log_level(IMP.base.VERBOSE)
        # turn off checks to avoid restraint re-evals
        IMP.base.set_check_level(IMP.base.NONE)
        lr.reset()
        f.get_is_ok(IMP.domino.Assignment([0]))
        f.get_is_ok(IMP.domino.Assignment([0]))
        f.get_is_ok(IMP.domino.Assignment([1]))
        f.get_is_ok(IMP.domino.Assignment([1]))
        f.get_is_ok(IMP.domino.Assignment([0]))
        self.assertEqual(lr.count, 3)

if __name__ == '__main__':
    IMP.test.main()
