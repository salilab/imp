import unittest
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom



class RBDTests(IMP.test.TestCase):
    """Tests for RigidBodyDecorator function"""
    def _add_rb_restraints(self, rbd):
        #intentionally kept trivial to ensure convergence
        r= IMP.algebra.identity_rotation()
        t= IMP.algebra.random_vector_in_unit_sphere()
        tr= IMP.algebra.Transformation3D(r,t)
        mbs= rbd.get_member_particles()
        m= rbd.get_particle().get_model()
        for b in mbs:
            mb= IMP.core.RigidMemberDecorator(b)
            lc= mb.get_internal_coordinates()
            lct= tr.transform(lc)
            dt= IMP.core.DistanceToSingletonScore(IMP.core.Harmonic(0,1), lct)
            r= IMP.core.SingletonRestraint(dt, mb.get_particle())
            m.add_restraint(r)
    def _create_hierarchy(self, m, htr, n=10):
        rd= IMP.core.XYZDecorator.create(IMP.Particle(m),
                                         IMP.algebra.random_vector_in_unit_box())
        hd= IMP.core.HierarchyDecorator.create(rd.get_particle(), htr)
        for i in range(0,n):
            crd= IMP.core.XYZDecorator.create(IMP.Particle(m),
                                              IMP.algebra.random_vector_in_unit_box())
            chd= IMP.core.HierarchyDecorator.create(crd.get_particle(), htr)
            hd.add_child(chd)
        return rd.get_particle()

    def _test_create_one(self, htr, snap):
        count=1
        success=0
        for i in range(0, count):
            m= IMP.Model()
            IMP.set_log_level(IMP.SILENT)
            p= self._create_hierarchy(m,  htr)
            ss=IMP.core.create_rigid_body(p, IMP.core.HierarchyDecorator(p, htr).get_child_particles(), snap)
            rbd= IMP.core.RigidBodyDecorator(p)
            p.show()
            rbd.set_coordinates_are_optimized(True, snap)
            self. _add_rb_restraints(rbd)
            cg= IMP.core.ConjugateGradients()
            cg.set_model(m)
            print "Initial score is " + str(m.evaluate(False))
            cg.optimize(1000)
            if m.evaluate(False) < .1:
                success=success+1
        self.assert_(success > count/2)

    def _test_create_many(self, htr, snap):
        count=10
        success=0
        for i in range(0, count):
            m= IMP.Model()
            IMP.set_log_level(IMP.SILENT)
            l= IMP.core.ListSingletonContainer()
            for i in range(0,2):
                p= self._create_hierarchy(m, htr)
                l.add_particle(p)
            IMP.core.create_rigid_bodies(l, IMP.core.ChildrenRefiner(htr), snap)
            for p in l.get_particles():
                rbd= IMP.core.RigidBodyDecorator(p)
                self._add_rb_restraints(rbd)
            cg= IMP.core.ConjugateGradients()
            cg.set_model(m)
            print "Initial score is " + str(m.evaluate(False))
            cg.optimize(1000)
            if m.evaluate(False) < .1:
                success=success+1
        self.assert_(success > count/2)


    def test_create_one(self):
        """Testing create_rigid_body"""
        htr= IMP.core.HierarchyDecorator.get_default_traits()
        self._test_create_one(htr, False)
        htr= IMP.atom.MolecularHierarchyDecorator.get_traits()
        self._test_create_one(htr, False)
        htr= IMP.atom.MolecularHierarchyDecorator.get_traits()
        self._test_create_one(htr, True)
    def test_create_many(self):
        """Testing create_rigid_bodies"""
        htr= IMP.core.HierarchyDecorator.get_default_traits()
        self._test_create_many(htr, False)
        htr= IMP.core.HierarchyDecorator.get_default_traits()
        self._test_create_many(htr, True)


    # test one with snap and non-snap
    # test setting things to be optimized or not

if __name__ == '__main__':
    unittest.main()
