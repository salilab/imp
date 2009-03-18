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
        mbs= rbd.get_members()
        m= rbd.get_particle().get_model()
        for mb in mbs:
            lc= mb.get_internal_coordinates()
            lct= tr.transform(lc)
            dt= IMP.core.DistanceToSingletonScore(IMP.core.Harmonic(0,1), lct)
            r= IMP.core.SingletonRestraint(dt, mb.get_particle())
            m.add_restraint(r)
    def _create_hierarchy(self, m, tr, htr, n=10):
        rd= IMP.core.XYZDecorator.create(IMP.Particle(m),
                                         IMP.algebra.random_vector_in_unit_box())
        hd= IMP.core.HierarchyDecorator.create(rd.get_particle(), htr)
        for i in range(0,n):
            crd= IMP.core.XYZDecorator.create(IMP.Particle(m),
                                              IMP.algebra.random_vector_in_unit_box())
            chd= IMP.core.HierarchyDecorator.create(crd.get_particle(), htr)
            hd.add_child(chd)
        return rd.get_particle()

    def _test_create_one(self, tr, htr):
        m= IMP.Model()
        IMP.set_log_level(IMP.VERBOSE)
        p= self._create_hierarchy(m, tr, htr)
        ss=IMP.core.create_rigid_body(p, tr)
        rbd= IMP.core.RigidBodyDecorator(p, tr)
        rbd.set_coordinates_are_optimized(True)
        self. _add_rb_restraints(rbd)
        cg= IMP.core.ConjugateGradients()
        cg.set_model(m)
        print "Initial score is " + str(m.evaluate(False))
        cg.optimize(1000)
        self.assert_(m.evaluate(False) < .1)

    def _test_create_many(self, tr, htr):
        m= IMP.Model()
        IMP.set_log_level(IMP.VERBOSE)
        l= IMP.core.ListSingletonContainer()
        for i in range(0,2):
            p= self._create_hierarchy(m, tr, htr)
            l.add_particle(p)
        IMP.core.create_rigid_bodies(l, tr)
        for p in l.get_particles():
            rbd= IMP.core.RigidBodyDecorator(p, tr)
            self. _add_rb_restraints(rbd)
        cg= IMP.core.ConjugateGradients()
        cg.set_model(m)
        print "Initial score is " + str(m.evaluate(False))
        cg.optimize(1000)
        self.assert_(m.evaluate(False) < .1)


    def test_create_one(self):
        """Testing create_rigid_body"""
        tr= IMP.core.RigidBodyDecorator.get_default_traits()
        htr= IMP.core.HierarchyDecorator.get_default_traits()
        #self._test_create_one(tr, htr)
        htr= IMP.atom.MolecularHierarchyDecorator.get_traits()
        tr= IMP.core.RigidBodyTraits(IMP.core.ChildrenParticleRefiner(htr),
                                        "new_string")
        #self._test_create_one(tr, htr)
        htr= IMP.atom.MolecularHierarchyDecorator.get_traits()
        tr= IMP.core.RigidBodyTraits(IMP.core.ChildrenParticleRefiner(htr),
                                        "new_string",
                                     IMP.FloatKey(),
                                     IMP.FloatKey(),
                                     True)
        self._test_create_one(tr, htr)
        htr= IMP.atom.MolecularHierarchyDecorator.get_traits()
        tr= IMP.atom.get_molecular_rigid_body_traits()
        self._test_create_one(tr, htr)
    def test_create_many(self):
        """Testing create_rigid_bodies"""
        tr= IMP.core.RigidBodyDecorator.get_default_traits()
        htr= IMP.core.HierarchyDecorator.get_default_traits()
        self._test_create_many(tr, htr)


    # test one with snap and non-snap
    # test setting things to be optimized or not

if __name__ == '__main__':
    unittest.main()
