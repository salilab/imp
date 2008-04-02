import re, math, unittest
import random
import IMP


class TestCase(unittest.TestCase):
    """Super class for IMP test cases"""

    def assertInTolerance(self, num1, num2, tolerance, msg=None):
        """Assert that the difference between num1 and num2 is less than
           tolerance"""
        diff = abs(num1 - num2)
        if msg is None:
            msg = "%f != %f within %g" % (num1, num2, tolerance)
        self.assert_(diff < tolerance, msg)

    def create_point_particle(self, model, x, y, z):
        """Make a particle with optimizable x, y and z attributes, and
           add it to the model."""
        p = IMP.Particle()
        model.add_particle(p)
        p.add_attribute(IMP.FloatKey("x"), x, True)
        p.add_attribute(IMP.FloatKey("y"), y, True)
        p.add_attribute(IMP.FloatKey("z"), z, True)
        return p

    def randomize_particles(self, particles, deviation):
        """Randomize the xyz coordinates of a list of particles"""
        for p in particles:
            d= IMP.XYZDecorator.cast(p)
            d.set_x(random.uniform(-deviation, deviation))
            d.set_y(random.uniform(-deviation, deviation))
            d.set_z(random.uniform(-deviation, deviation))

    def particle_distance(self, particles, idx0, idx1):
        """Return distance between two given particles"""
        dx = particles[idx0].get_value(IMP.FloatKey("x")) -\
             particles[idx1].get_value(IMP.FloatKey("x"))
        dy = particles[idx0].get_value(IMP.FloatKey("y")) -\
             particles[idx1].get_value(IMP.FloatKey("y"))
        dz = particles[idx0].get_value(IMP.FloatKey("z")) -\
             particles[idx1].get_value(IMP.FloatKey("z"))
        return math.sqrt(dx*dx + dy*dy + dz*dz)
