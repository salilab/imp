import modeller
import modeller.optimizers
import math
import sys
sys.path.append("../")
import imp2

# ============== Using a Modeller Optimizer ==============

class IMP_Restraints(modeller.terms.energy_term):
    """Calculate score and derivs based on all IMP-defined restraints"""

    _physical_type = modeller.physical.absposition

    # There is a sight assymetry (with respect to the Modeller_Restraints
    # ... class here because the Modeller model is not available
    # ... when this init is called, but it is passed through
    # ... the eval interface anyway.
    def __init__(self, imp_model, particles):
        modeller.terms.energy_term.__init__(self)
        self.imp_model = imp_model
        self.particles = particles

    def eval(self, mdl, deriv, indats):
        """Calculate score of current state of model with respect to the IMP restraints. If calc_deriv, also calculate the partial derivatives associated with these restraints."""
        atoms = self.indices_to_atoms(mdl, indats)
        Copy_Modeller_Coords_To_IMP(atoms, self.particles)
        dvx = [0.] * len(indats)
        dvy = [0.] * len(indats)
        dvz = [0.] * len(indats)

        score = self.imp_model.evaluate(deriv)
        if deriv:
            Get_IMP_Derivs(self.particles, dvx, dvy, dvz)
            return (score, dvx, dvy, dvz)
        else:
            return score


# ============== Using an IMP Optimizer ==============

class Modeller_Restraints(imp2.Restraint):
    """Calculate score and derivs based on all Modeller-defined restraints"""

    def __init__(self, imp_model, modeller_model, particles):
        imp2.Restraint.__init__(self)
        self.modeller_model = modeller_model
        self.imp_model = imp_model
        self.particles = particles

    def evaluate(self, calc_deriv):
        """Calculate score of current state of model with respect to the Modeller restraints. If calc_deriv, also calculate the partial derivatives associated with these restraints."""
        atoms = self.modeller_model.atoms
        sel = modeller.selection(self.modeller_model)
        Copy_IMP_Coords_To_Modeller(self.particles, atoms)
        energies = sel.energy()
        if calc_deriv:
            Add_Modeller_Derivs_To_IMP(atoms, self.particles)

        return energies[0]


# ============== Utilities ==============

class Particle:
    """Wrapper for IMP particles that focuses on x,y,z coordinates"""

    def __init__(self, model, x, y, z):
        """Initialize particle with IMP model it belongs to and its xyz coordinates"""
        self.imp_particle = imp2.Particle()
        model.add_particle(self.imp_particle);
        self.imp_particle.add_float("X", x, True)
        self.imp_particle.add_float("Y", y, True)
        self.imp_particle.add_float("Z", z, True)
        self.model_data = self.imp_particle.model_data()

    def imp(self):
        """Return IMP particle pointer"""
        return self.imp_particle

    def get_float(self, name):
        """Get float attribute of particle with given name"""
        return self.model_data.get_float(self.imp_particle.float_index(name))

    def get_int(self, name):
        """Get int attribute of particle with given name"""
        return self.model_data.get_int(self.imp_particle.int_index(name))

    def get_string(self, name):
        """Get string attribute of particle with given name"""
        return self.model_data.get_string(self.imp_particle.string_index(name))

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
        self.model_data.set_float(self.imp_particle.float_index("X"), value)

    def set_y(self, value):
        """Set y position of particle"""
        self.model_data.set_float(self.imp_particle.float_index("Y"), value)

    def set_z(self, value):
        """Set z position of particle"""
        self.model_data.set_float(self.imp_particle.float_index("Z"), value)

    def dx(self):
        """Get partial derivative of score with respect to particle's x position"""
        return self.model_data.get_deriv(self.imp_particle.float_index("X"))

    def dy(self):
        """Get partial derivative of score with respect to particle's y position"""
        return self.model_data.get_deriv(self.imp_particle.float_index("Y"))

    def dz(self):
        """Get partial derivative of score with respect to particle's z position"""
        return self.model_data.get_deriv(self.imp_particle.float_index("Z"))

    def add_to_dx(self, value):
        """Add to partial derivative of score with respect to particle's x position"""
        self.model_data.add_to_deriv(self.imp_particle.float_index("X"), value)

    def add_to_dy(self, value):
        """Add to partial derivative of score with respect to particle's y position"""
        self.model_data.add_to_deriv(self.imp_particle.float_index("Y"), value)

    def add_to_dz(self, value):
        """Add to partial derivative of score with respect to particle's z position"""
        self.model_data.add_to_deriv(self.imp_particle.float_index("Z"), value)


def Init_IMP_From_Modeller(model, particles, atoms):
    """ Init IMP particles from Modeller atoms """
    for (num, at) in enumerate(atoms):
        particles.append(Particle(model, at.x, at.y, at.z))


def Create_Particles(num_particles, env, model, particles):
    """ Create Modeller atoms by building a PDB file. Load them from the file into Modeller. Then copy them to the IMP model. """
    fp = open ("./temp_particles.pdb", "w")
    for i in range(0, num_particles):
        fp.write("ATOM  %5d  N   ALA     0       0.000   0.000   0.000  1.00  0.00           C  \n" % (i))
    fp.close()
    mdl = modeller.model(env, file='./temp_particles.pdb')
    Init_IMP_From_Modeller(model, particles, mdl.atoms)
    return mdl


def Create_Particles_From_IMP(env, model):
    """ Create Modeller atoms by building a PDB file. Load them from the file into Modeller. Return the Modeller model."""

    num_particles = len(model.particles)
    fp = open ("./temp_particles.pdb", "w")
    for i in range(0, num_particles):
        fp.write("ATOM  %5d  N   ALA     0       0.000   0.000   0.000  1.00  0.00           C  \n" % (i))
    fp.close()
    mdl = modeller.model(env, file='./temp_particles.pdb')
    Copy_IMP_Coords_To_Modeller(model.particles, mdl.atoms)
    return mdl


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
            rsrs.append(imp2.RSR_Distance(model, particles[i].imp(), particles[j].imp(), mean, sd, score_func))


def Add_Rigid_Body(model, name, value, mdl):
    """ Add rigid body to Modeller model based on given int value """
    atoms = mdl.atoms
    atom_idx = 0
    rb_sel = modeller.selection()
    for p in model.particles:
        if p.get_int(name) == value:
            rb_sel.add(atoms[atom_idx])

        atom_idx = atom_idx + 1

    rb = modeller.rigid_body(rb_sel)
    mdl.restraints.rigid_bodies.append(rb)


def Add_Rigid_Body_Expr(model, expr, mdl):
    """ Add rigid body to Modeller model based on given expression (e.g. p.get_float('radius') < 2.0 )"""
    atoms = mdl.atoms
    atom_idx = 0
    rb_sel = modeller.selection()
    for p in model.particles:
        if eval(expr):
            rb_sel.add(atoms[atom_idx])

        atom_idx = atom_idx + 1

    rb = modeller.rigid_body(rb_sel)
    mdl.restraints.rigid_bodies.append(rb)


def Move_Rigid_Body(model, name, value, dx, dy, dz, mdl):
    """ Move all particles in the rigid body by the given amount """
    atom_idx = 0
    for p in model.particles:
        if p.get_int(name) == value:
            p.set_x(p.x() + dx)
            p.set_y(p.y() + dy)
            p.set_z(p.z() + dz)

        atom_idx = atom_idx + 1

    Copy_IMP_Coords_To_Modeller(model.particles, mdl.atoms)



def Move_Rigid_Body_Expr(model, expr, dx, dy, dz, mdl):
    """ Move all particles in the rigid body by the given amount """
    atom_idx = 0
    for p in model.particles:
        if eval(expr):
            p.set_x(p.x() + dx)
            p.set_y(p.y() + dy)
            p.set_z(p.z() + dz)

        atom_idx = atom_idx + 1

    Copy_IMP_Coords_To_Modeller(model.particles, mdl.atoms)


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
