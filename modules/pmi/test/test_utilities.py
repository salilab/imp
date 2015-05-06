from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.pmi
import random


class TestUtilities(IMP.test.TestCase):

    def python_version_min_distance(self,xyzrs1,xyzrs2):
        mindist=-1.0
        for xyzr1 in xyzrs1:
            for xyzr2 in xyzrs2:
                d=IMP.core.get_distance(xyzr1,xyzr2)
                if mindist < 0 : mindist = d
                if mindist > d : mindist = d
        if mindist < 0 : mindist = 0
        return mindist

    def python_version_array_of_distances(self,particlestemps):
        array_of_dists=[]
        for n,ps1 in enumerate(particlestemps):
            for ps2 in particlestemps[n+1:]:
                array_of_dists.append(self.python_version_min_distance(ps1,ps2))
        return array_of_dists

    def test_simple(self):
        """ simple test """
        m=IMP.Model()
        p1=IMP.Particle(m)
        d1=IMP.core.XYZR.setup_particle(p1)
        d1.set_coordinates((0,0,0))
        d1.set_radius(1.0)

        p2=IMP.Particle(m)
        d2=IMP.core.XYZR.setup_particle(p2)
        d2.set_coordinates((10,0,0))
        d2.set_radius(2.0)

        dist=IMP.pmi.get_bipartite_minimum_sphere_distance([d1],[d2])
        self.assertEqual(dist,7.0)

    def test_two_composites(self):
        import random
        m=IMP.Model()
        xyzrs1=[]
        xyzrs2=[]
        for i in range(10):
            p=IMP.Particle(m)
            d=IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D((0,0,0),100.0)))
            d.set_radius(10.0*random.random())
            xyzrs1.append(d)

        for i in range(15):
            p=IMP.Particle(m)
            d=IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D((0,0,0),100.0)))
            d.set_radius(10.0*random.random())
            xyzrs2.append(d)

        dist=IMP.pmi.get_bipartite_minimum_sphere_distance(xyzrs1,xyzrs2)
        self.assertEqual(dist,self.python_version_min_distance(xyzrs1,xyzrs2))

    def test_multiple_composite(self):
        import random
        m=IMP.Model()
        ps1=[]
        ps2=[]
        ps3=[]
        ps4=[]

        for i in range(10):
            p=IMP.Particle(m)
            d=IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D((0,0,0),100.0)))
            d.set_radius(10.0*random.random())
            ps1.append(d)

        for i in range(15):
            p=IMP.Particle(m)
            d=IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D((0,0,0),100.0)))
            d.set_radius(10.0*random.random())
            ps2.append(d)

        for i in range(5):
            p=IMP.Particle(m)
            d=IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D((0,0,0),100.0)))
            d.set_radius(10.0*random.random())
            ps3.append(d)

        for i in range(11):
            p=IMP.Particle(m)
            d=IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D((0,0,0),100.0)))
            d.set_radius(10.0*random.random())
            ps4.append(d)

        particlestemps=[ps1,ps2,ps3,ps4]
        dist_array=IMP.pmi.get_list_of_bipartite_minimum_sphere_distance(particlestemps)
        python_dist_array=self.python_version_array_of_distances(particlestemps)
        for n,dist in enumerate(dist_array):
            self.assertAlmostEqual(dist,python_dist_array[n],delta=0.00001)

if __name__ == '__main__':
    IMP.test.main()
