## \example parallel/tasks.py
# This module contains the setup and task functions used by local_distance.py.
#

import IMP
import IMP.algebra
import IMP.core


def setup():
    """Create a Model containing two XYZ particles linked by a harmonic
       distance restraint, one fixed at the origin. Return the Model,
       the ScoringFunction, and the free XYZ particle."""
    m = IMP.Model()
    p1 = IMP.Particle(m)
    d1 = IMP.core.XYZ.setup_particle(p1)
    p2 = IMP.Particle(m)
    d2 = IMP.core.XYZ.setup_particle(p2)
    d1.set_coordinates(IMP.algebra.Vector3D(0, 0, 0))
    r = IMP.core.DistanceRestraint(m, IMP.core.Harmonic(0, 1), p1, p2)
    sf = IMP.core.RestraintsScoringFunction([r])
    return m, sf, d2

# Note that setup and tasks are Python callables, i.e. functions (like setup
# above) or classes that implement the __call__ method (like Task below).
# The latter allows for parameters (Python objects) to be passed from the
# manager to the workers.


class Task:

    def __init__(self, dist):
        self.dist = dist

    def __call__(self, m, sf, d):
        """Place the free XYZ particle at the specified distance from the
           origin. Return the distance and the model's score. Note that the
           input parameters to this method (m, sf, and d) are those returned by
           the setup function above."""
        d.set_coordinates(IMP.algebra.Vector3D(0, 0, self.dist))
        return (self.dist, sf.evaluate(False))
