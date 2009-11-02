import unittest
from IMP import *
from IMP.algebra import *
from IMP.core import *
import IMP.test

import math

class Volume(IMP.test.TestCase):
    """Tests for angle restraints"""
    def test_volume(self):
        """Testing that volume restraint can separate balls"""
        m= Model()
        ps= IMP.Particles()
        IMP.set_log_level(IMP.VERBOSE)
        for i in range(0,3):
            p= Particle(m)
            v= random_vector_in_box(Vector3D(0,0,0),
                                    Vector3D(5,5,5))
            d=XYZR.setup_particle(p, Sphere3D(v, 4))
            ps.append(p)
            p.set_is_optimized(FloatKey("x"), True)
            p.set_is_optimized(FloatKey("y"), True)
            p.set_is_optimized(FloatKey("z"), True)
        sc= ListSingletonContainer(ps)
        vr= VolumeRestraint(Harmonic(0,1), sc, 4**3*3.1415*4.0/3.0*len(ps))
        m.add_restraint(vr)
        c= ConjugateGradients(m)
        c.optimize(1000)
    def test_volume(self):
        """Testing that volume restraint can change radius"""
        m= Model()
        IMP.set_log_level(IMP.VERBOSE)
        ps= IMP.Particles()
        p= Particle(m)
        inits=Sphere3D(random_vector_in_box(BoundingBox3D(Vector3D(0,0,0),
                                                          Vector3D(5,5,5))),
                       4)
        print inits
        d=XYZR.setup_particle(p, inits)
        print d
        ps.append(p)
        d.set_coordinates_are_optimized(True)
        d.get_particle().set_is_optimized(XYZR.get_default_radius_key(), True)
        sc= ListSingletonContainer(ps)
        vr= VolumeRestraint(Harmonic(0,.001), sc, 5**3*3.1415*4.0/3.0*len(ps))
        m.add_restraint(vr)
        #c= SteepestDescent()
        #c.set_step_size(.1)
        #c.set_threshold(1)
        c= ConjugateGradients(m)
        c.optimize(1000)
        print d
        self.assertInTolerance(d.get_radius(), 5, .1)

if __name__ == '__main__':
    unittest.main()
