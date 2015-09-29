from __future__ import print_function, division
import IMP
import IMP.test
import IMP.core
import IMP.container
import IMP.algebra

class Tests(IMP.test.TestCase):
    def test_xyz_particles_identity_transf(self):
        m=IMP.Model()
        ps_reference=[]
        for i in range(10):
            p=IMP.Particle(m)
            d=IMP.core.XYZ.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(
                                                                            (0,0,0),10)))
            ps_reference.append(p)

        ps_copy=[]
        sm = IMP.core.TransformationSymmetry(IMP.algebra.get_identity_transformation_3d())
        lc = IMP.container.ListSingletonContainer(m)

        for i in range(10):
            p=IMP.Particle(m)
            d=IMP.core.XYZ.setup_particle(p)
            ps_copy.append(p)
            IMP.core.Reference.setup_particle(p, ps_reference[i])
            lc.add(p.get_index())


        c = IMP.container.SingletonsConstraint(sm, None, lc)
        m.add_score_state(c)


        m.update()

        for i in range(10):
            self.assertEqual(tuple(IMP.core.XYZ(ps_reference[i]).get_coordinates()),
                                    tuple(IMP.core.XYZ(ps_copy[i]).get_coordinates()))

    def test_xyz_particles_random_transf(self):
        m=IMP.Model()
        ps_reference=[]
        for i in range(10):
            p=IMP.Particle(m)
            d=IMP.core.XYZ.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(
                                                                            (0,0,0),10)))
            ps_reference.append(p)

        ps_copy=[]
        random_transformation=IMP.algebra.get_random_local_transformation(
                    IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D((0,0,0),10)))
        sm = IMP.core.TransformationSymmetry(random_transformation)
        lc = IMP.container.ListSingletonContainer(m)

        for i in range(10):
            p=IMP.Particle(m)
            d=IMP.core.XYZ.setup_particle(p)
            ps_copy.append(p)
            IMP.core.Reference.setup_particle(p, ps_reference[i])
            lc.add(p.get_index())


        c = IMP.container.SingletonsConstraint(sm, None, lc)
        m.add_score_state(c)


        m.update()

        for i in range(10):
            IMP.core.transform(IMP.core.XYZ(ps_reference[i]), random_transformation)
            self.assertEqual(tuple(IMP.core.XYZ(ps_reference[i]).get_coordinates()),
                                    tuple(IMP.core.XYZ(ps_copy[i]).get_coordinates()))


    def test_rigid_body_identity_transf(self):
        m=IMP.Model()



        ps_reference=[]
        for i in range(10):
            p=IMP.Particle(m)
            d=IMP.core.XYZ.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(
                                                                            (0,0,0),10)))
            ps_reference.append(p)

        p=IMP.Particle(m)
        rb_reference=IMP.core.RigidBody.setup_particle(p,ps_reference)

        p=IMP.Particle(m)
        sm = IMP.core.TransformationSymmetry(IMP.algebra.get_identity_transformation_3d())
        lc = IMP.container.ListSingletonContainer(m)

        ps_copy=[]
        for i in range(10):
            p=IMP.Particle(m)
            d=IMP.core.XYZ.setup_particle(p)
            ps_copy.append(p)
            IMP.core.Reference.setup_particle(p, ps_reference[i])
            lc.add(p.get_index())


        c = IMP.container.SingletonsConstraint(sm, None, lc)
        m.add_score_state(c)


        m.update()

        for i in range(10):
            self.assertEqual(tuple(IMP.core.XYZ(ps_reference[i]).get_coordinates()),
                                    tuple(IMP.core.XYZ(ps_copy[i]).get_coordinates()))

        for k in range(30):
            random_transformation=IMP.algebra.get_random_local_transformation(
                    IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D((0,0,0),10)))
            IMP.core.transform(rb_reference,random_transformation)
            m.update()
            for i in range(10):
                self.assertEqual(tuple(IMP.core.XYZ(ps_reference[i]).get_coordinates()),
                                        tuple(IMP.core.XYZ(ps_copy[i]).get_coordinates()))

    def test_rigid_body_random_transf(self):
        m=IMP.Model()



        ps_reference=[]
        for i in range(10):
            p=IMP.Particle(m)
            d=IMP.core.XYZ.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(
                                                        IMP.algebra.Sphere3D((0,0,0),10)))
            ps_reference.append(p)

        p=IMP.Particle(m)
        rb_reference=IMP.core.RigidBody.setup_particle(p,ps_reference)

        p=IMP.Particle(m)
        random_symmetry_transformation=IMP.algebra.get_random_local_transformation(
                    IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D((0,0,0),10)))
        sm = IMP.core.TransformationSymmetry(random_symmetry_transformation)
        lc = IMP.container.ListSingletonContainer(m)

        ps_copy=[]
        for i in range(10):
            p=IMP.Particle(m)
            d=IMP.core.XYZ.setup_particle(p)
            ps_copy.append(p)
            IMP.core.Reference.setup_particle(p, ps_reference[i])
            lc.add(p.get_index())


        c = IMP.container.SingletonsConstraint(sm, None, lc)
        m.add_score_state(c)


        m.update()

        IMP.core.transform(rb_reference,random_symmetry_transformation)
        for i in range(10):
            for k in range(3):
                self.assertAlmostEqual(
                                tuple(IMP.core.XYZ(ps_reference[i]).get_coordinates())[k],
                                tuple(IMP.core.XYZ(ps_copy[i]).get_coordinates())[k])

        for k in range(30):
            random_rb_transformation=IMP.algebra.get_random_local_transformation(
                    IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D((0,0,0),10)))
            IMP.core.transform(rb_reference,random_rb_transformation)
            m.update()
            IMP.core.transform(rb_reference,random_symmetry_transformation)
            for i in range(10):
                for k in range(3):
                    self.assertAlmostEqual(tuple(
                                    IMP.core.XYZ(ps_reference[i]).get_coordinates())[k],
                                    tuple(IMP.core.XYZ(ps_copy[i]).get_coordinates())[k])




if __name__ == '__main__':
    IMP.test.main()
