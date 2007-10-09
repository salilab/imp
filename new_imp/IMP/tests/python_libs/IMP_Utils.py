import imp2
import math


# ============== Utilities ==============

class XYZParticle(imp2.Particle):
    """Wrapper for IMP particles that focuses on x,y,z coordinates"""

    def __init__(self, model, x, y, z):
        """Initialize particle with IMP model it belongs to and its xyz
           coordinates"""
        imp2.Particle.__init__(self)
        model.add_particle(self);
        self.add_float("X", x, True)
        self.add_float("Y", y, True)
        self.add_float("Z", z, True)
        self.model_data = self.model_data()

    def get_float(self, name):
        """Get float attribute of particle with given name"""
        return self.model_data.get_float(self.float_index(name))

    def get_int(self, name):
        """Get int attribute of particle with given name"""
        return self.model_data.get_int(self.int_index(name))

    def get_string(self, name):
        """Get string attribute of particle with given name"""
        return self.model_data.get_string(self.string_index(name))

    def x(self):
        """Get x position of particle"""
        return self.get_float("X")

    def y(self):
        """Get y position of particle"""
        return self.get_float("Y")

    def z(self):
        """Get z position of particle"""
        return self.get_float("Z")

    def set_x(self, value):
        """Set x position of particle"""
        self.model_data.set_float(self.float_index("X"), value)

    def set_y(self, value):
        """Set y position of particle"""
        self.model_data.set_float(self.float_index("Y"), value)

    def set_z(self, value):
        """Set z position of particle"""
        self.model_data.set_float(self.float_index("Z"), value)

    def dx(self):
        """Get partial derivative of score with respect to particle's x position"""
        return self.model_data.get_deriv(self.float_index("X"))

    def dy(self):
        """Get partial derivative of score with respect to particle's y position"""
        return self.model_data.get_deriv(self.float_index("Y"))

    def dz(self):
        """Get partial derivative of score with respect to particle's z position"""
        return self.model_data.get_deriv(self.float_index("Z"))

    def add_to_dx(self, value):
        """Add to partial derivative of score with respect to particle's x position"""
        self.model_data.add_to_deriv(self.float_index("X"), value)

    def add_to_dy(self, value):
        """Add to partial derivative of score with respect to particle's y position"""
        self.model_data.add_to_deriv(self.float_index("Y"), value)

    def add_to_dz(self, value):
        """Add to partial derivative of score with respect to particle's z position"""
        self.model_data.add_to_deriv(self.float_index("Z"), value)


def Init_IMP_From_Modeller(model, particles, atoms):
    """ Init IMP particles from Modeller atoms """
    for (num, at) in enumerate(atoms):
        particles.append(XYZParticle(model, at.x, at.y, at.z))


def Copy_IMP_Coords_To_Modeller(particles, atoms):
    """ Copy atom coordinates from IMP to Modeller """
    for (num, at) in enumerate(atoms):
        at.x = particles[num].x()
        at.y = particles[num].y()
        at.z = particles[num].z()


def Copy_Modeller_Coords_To_IMP(atoms, particles):
    """ Copy atom coordinates from Modeller to IMP """
    for (num, at) in enumerate(atoms):
        particles[num].set_x(at.x)
        particles[num].set_y(at.y)
        particles[num].set_z(at.z)


def Add_Modeller_Derivs_To_IMP(atoms, particles):
    """ Add atom derivatives from Modeller to IMP """
    for (num, at) in enumerate(atoms):
        particles[num].add_to_dx(at.dvx)
        particles[num].add_to_dy(at.dvy)
        particles[num].add_to_dz(at.dvz)


def Get_IMP_Derivs(particles, dvx, dvy, dvz):
    """ Move atom derivatives from IMP to Modeller"""
    for idx in range(0, len(dvx)):
        dvx[idx] = particles[idx].dx()
        dvy[idx] = particles[idx].dy()
        dvz[idx] = particles[idx].dz()


def Set_Restraint_Set_Is_Active(model, restraint_set_name, is_active):
    """ Set whether the given restraint set is active (True) or inactive (False) """
    for i in range(len(model.restraint_sets)):
        if restraint_set_name == model.restraint_sets[i].name():
            model.restraint_sets[i].set_is_active(is_active)


def Set_Up_Exclusion_Volumes(model, particles, radius_name, rsrs, score_func, sd = 0.1):
    """ Add all needed exclusion volumes to the restraint list """
    for i in range(len(particles)-1):
        for j in range(i+1, len(particles)):
            mean = particles[i].get_float(radius_name) + particles[j].get_float(radius_name)
            rsrs.append(imp2.RSR_Distance(model, particles[i], particles[j], mean, sd, score_func))


def Write_PDB(model, fname):
    """ Write PDB based on particle attributes. """
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


def Show_Modeller_and_IMP(atoms, particles):
    """ Show Modeller and IMP atoms and their partial derivatives"""
    print "Modeller:"
    for (num, at) in enumerate(atoms):
        print "(", at.x, ", ", at.y, ", ", at.z, ") (", at.dvx, ", ", at.dvy, ", ", at.dvz, ")"
    print "IMP:"
    for (num, at) in enumerate(atoms):
        print "(", particles[num].x(), ", ", particles[num].y(), ", ", particles[num].z(), ") (", particles[num].dx(), ", ", particles[num].dy(), ", ", particles[num].dz(), ")"


def Show_IMP_Particles_Pos(particles):
    """ Show IMP particles' positions"""
    for i in range(0, len(particles)):
        print 'particle', i,
        print 'X', particles[i].x(),
        print 'Y', particles[i].y(),
        print 'Z', particles[i].z()


def Show_IMP_Particles(particles, attrs):
    """ Show IMP particle' attributes e.g. attrs = (('float', 'X'), ('float', 'Y'), ('float', 'Z'), ('float', 'radius'), ('int', 'id'))"""
    for i in range(0, len(particles)):
        print 'particle', i,
        for j in range(0, len(attrs)):
            type = attrs[j][0]
            name = attrs[j][1]
            if type == 'float':
                print name, particles[i].get_float(name),
            elif type == 'int':
                print name, particles[i].get_int(name),
            elif type == 'string':
                print name, particles[i].get_string(name),
        print


def Show_Distances(particles):
    """ Show Distances using IMP particles """
    for i in range(0, len(particles)):
        for j in range(i + 1, len(particles)):
            dx = particles[i].x() - particles[j].x()
            dy = particles[i].y() - particles[j].y()
            dz = particles[i].z() - particles[j].z()

            print "(", i, ",", j, " : ", math.sqrt(dx*dx + dy*dy + dz*dz), ")  ",

        print ""
