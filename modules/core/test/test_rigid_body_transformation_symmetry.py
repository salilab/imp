from __future__ import print_function, division
import IMP
import IMP.test
import IMP.core
import IMP.container
import IMP.algebra

class Tests(IMP.test.TestCase):
    def test_xyz_particles(self):
        m=IMP.Model()
        ps_reference=[]
        for i in range(10):
            p=IMP.Particle(m)
            d=IMP.core.XYZ.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(
                                    IMP.algebra.Sphere3D((0,0,0),10)))
            ps_reference.append(p)

        ps_copy=[]
        t=IMP.algebra.get_identity_transformation_3d()

        p=IMP.Particle(m)
        rb=IMP.core.RigidBody.setup_particle(p,IMP.algebra.ReferenceFrame3D(t))

        sm = IMP.core.TransformationSymmetry(p.get_index())
        lc = IMP.container.ListSingletonContainer(m)

        for i in range(10):
            p=IMP.Particle(m)
            d=IMP.core.XYZ.setup_particle(p)
            ps_copy.append(p)
            IMP.core.Reference.setup_particle(p, ps_reference[i])
            lc.add(p)


        c = IMP.container.SingletonsConstraint(sm, None, lc)
        m.add_score_state(c)
        m.update()

        for i in range(10):
            self.assertEqual(tuple(IMP.core.XYZ(ps_reference[i]).get_coordinates()),
                                    tuple(IMP.core.XYZ(ps_copy[i]).get_coordinates()))

        for k in range(30):
            random_rb_transformation=IMP.algebra.get_random_local_transformation(
                    IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D((0,0,0),10)))
            rb.set_reference_frame(IMP.algebra.ReferenceFrame3D(random_rb_transformation))
            m.update()
            for i in range(10):
                d=IMP.core.XYZ(ps_reference[i])
                d.set_coordinates(random_rb_transformation.get_transformed(
                                                                    d.get_coordinates()))
                self.assertEqual(tuple(IMP.core.XYZ(ps_reference[i]).get_coordinates()),
                                    tuple(IMP.core.XYZ(ps_copy[i]).get_coordinates()))

    def test_rigid_body(self):
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
        t=IMP.algebra.get_identity_transformation_3d()
        rb=IMP.core.RigidBody.setup_particle(p,IMP.algebra.ReferenceFrame3D(t))

        sm = IMP.core.TransformationSymmetry(p.get_index())
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

        # test identity
        for i in range(10):
            for k in range(3):
                self.assertAlmostEqual(tuple(
                                    IMP.core.XYZ(ps_reference[i]).get_coordinates())[k],
                                    tuple(IMP.core.XYZ(ps_copy[i]).get_coordinates())[k])


        # move the reference rb
        for k in range(30):
            random_rb_transformation=IMP.algebra.get_random_local_transformation(
                    IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D((0,0,0),10)))
            IMP.core.transform(rb_reference,random_rb_transformation)
            m.update()
            for i in range(10):
                for k in range(3):
                    self.assertAlmostEqual(
                                tuple(IMP.core.XYZ(ps_reference[i]).get_coordinates())[k],
                                tuple(IMP.core.XYZ(ps_copy[i]).get_coordinates())[k])

        # move the reference rb as well as the symmetry rb
        for k in range(30):
            random_reference_rb_transformation= \
                IMP.algebra.get_random_local_transformation(
                IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D((0,0,0),10)))
            IMP.core.transform(rb_reference,random_reference_rb_transformation)

            random_symmetry_rb_transformation=IMP.algebra.get_random_local_transformation(
                IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D((0,0,0),10)))
            rb.set_reference_frame(IMP.algebra.ReferenceFrame3D(
                                                    random_symmetry_rb_transformation))
            m.update()
            for i in range(10):
                d=IMP.core.XYZ(ps_reference[i])
                d.set_coordinates(random_symmetry_rb_transformation.get_transformed(
                                                                    d.get_coordinates()))
                self.assertEqual(tuple(IMP.core.XYZ(ps_reference[i]).get_coordinates()),
                                    tuple(IMP.core.XYZ(ps_copy[i]).get_coordinates()))

if __name__ == '__main__':
    IMP.test.main()
