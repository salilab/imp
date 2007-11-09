"""Utilities"""

import IMP
import math

class XYZParticle(IMP.Particle):
    """Wrapper for IMP particles that focuses on x,y,z coordinates"""

    def __init__(self, model, x=None, y=None, z=None):
        """Initialize particle with IMP model it belongs to and its xyz
           coordinates"""
        IMP.Particle.__init__(self)
        model.add_particle(self);
        self.model_data = self.get_model().get_model_data()
        if x is not None:
            self.add_attribute(IMP.FloatKey("x"), x, True)
        if y is not None:
            self.add_attribute(IMP.FloatKey("y"), y, True)
        if z is not None:
            self.add_attribute(IMP.FloatKey("z"), z, True)

    def x(self):
        """Get x position of particle"""
        return self.get_value(IMP.FloatKey("x"))

    def y(self):
        """Get y position of particle"""
        return self.get_value(IMP.FloatKey("y"))

    def z(self):
        """Get z position of particle"""
        return self.get_value(IMP.FloatKey("z"))

    def set_x(self, value):
        """Set x position of particle"""
        self.set_value(IMP.FloatKey("x"), value)

    def set_y(self, value):
        """Set y position of particle"""
        self.set_value(IMP.FloatKey("y"), value)

    def set_z(self, value):
        """Set z position of particle"""
        self.set_value(IMP.FloatKey("z"), value)

    def dx(self):
        """Get partial derivative of score with respect to particle's x position"""
        return self.get_derivative(IMP.FloatKey("x"))

    def dy(self):
        """Get partial derivative of score with respect to particle's y position"""
        return self.get_derivative(IMP.FloatKey("y"))

    def dz(self):
        """Get partial derivative of score with respect to particle's z position"""
        return self.get_derivative(IMP.FloatKey("z"))

    def add_to_dx(self, value, da):
        """Add to partial derivative of score with respect to particle's x position"""
        self.add_to_derivative(IMP.FloatKey("x"), value, da)

    def add_to_dy(self, value, da):
        """Add to partial derivative of score with respect to particle's y position"""
        self.add_to_derivative(IMP.FloatKey("y"), value, da)

    def add_to_dz(self, value, da):
        """Add to partial derivative of score with respect to particle's z position"""
        self.add_to_derivative(IMP.FloatKey("z"), value, da)


def set_restraint_set_is_active(model, restraint_set_name, is_active):
    """Set whether the given restraint set is active (True) or inactive
       (False)"""
    for i in range(len(model.restraint_sets)):
        if restraint_set_name == model.restraint_sets[i].name():
            model.restraint_sets[i].set_is_active(is_active)


def set_up_exclusion_volumes(model, particles, radius_name, rsrs, sd = 0.1):
    """Add all needed exclusion volumes to the restraint list"""
    for i in range(len(particles)-1):
        for j in range(i+1, len(particles)):
            mean = particles[i].get_value(radius_name) \
                   + particles[j].get_value(radius_name)
            score_func_params = IMP.BasicScoreFuncParams("harmonic_lower_bound",
                                                         mean, sd)
            rsrs.append(IMP.DistanceRestraint(model, particles[i], particles[j],
                                              score_func_params))


def write_pdb(model, fname):
    """Write PDB based on particle attributes."""
# based on:
# 0         1         2         3         4         5         6         7
# 01234567890123456789012345678901234567890123456789012345678901234567890123456789
# ATOM   1887  N   LEU A 411      39.891  85.957  24.294  1.00 46.58           N
    fp = open (fname, "w")
    i = 0
    for p in model.particles:
        fp.write("ATOM %6d%3s   %s %s%4d      %5.3f  %5.3f  %5.3f  1.00  0.00           %s\n" % (i, 'CA', 'ALA', chr(64 + p.get_int("chain")), p.get_int("aa_idx"), p.x(), p.y(), p.z(), 'C'))
        i = i + 1
    fp.close()


def show_particles_pos(particles):
    """Show IMP particles' positions"""
    for i in range(0, len(particles)):
        print 'particle', i,
        print 'X', particles[i].x(),
        print 'Y', particles[i].y(),
        print 'Z', particles[i].z()


def show_particles(particles):
    """ Show IMP particle' attributes"""
    for i in range(0, len(particles)):
        print 'particle', i
        print '  float attributes:'
        float_attr_iter = IMP.FloatAttributeIterator()
        float_attr_iter.reset(particles[i])
        while float_attr_iter.next():
            print '    ', float_attr_iter.get_key(), ": ", \
                  float_attr_iter.get_value()

        print '  int attributes:'
        int_attr_iter = IMP.IntAttributeIterator()
        int_attr_iter.reset(particles[i])
        while int_attr_iter.next():
            print '    ', int_attr_iter.get_key(), ": ", \
                  int_attr_iter.get_value()

        print '  string attributes:'
        string_attr_iter = IMP.StringAttributeIterator()
        string_attr_iter.reset(particles[i])
        while string_attr_iter.next():
            print '    ', string_attr_iter.get_key(), ": ", \
                  string_attr_iter.get_value()
        print


def show_distances(particles):
    """Show distances using IMP particles"""
    for i in range(0, len(particles)):
        for j in range(i + 1, len(particles)):
            dx = particles[i].x() - particles[j].x()
            dy = particles[i].y() - particles[j].y()
            dz = particles[i].z() - particles[j].z()

            print "(", i, ",", j, " : ", math.sqrt(dx*dx + dy*dy + dz*dz), \
                  ")  ",

        print ""
