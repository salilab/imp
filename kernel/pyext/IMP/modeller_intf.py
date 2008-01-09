"""Interface between IMP and MODELLER."""

import modeller
import modeller.optimizers
import math
import IMP
import IMP.utils

class IMPRestraints(modeller.terms.energy_term):
    """A Modeller restraint using all defined IMP restraints. Useful if you want
       to use IMP restraints with a Modeller optimizer."""

    _physical_type = modeller.physical.absposition

    # There is a sight asymmetry (with respect to the ModellerRestraints
    # class) here because the Modeller model is not available
    # when this init is called, but it is passed through
    # the eval interface anyway.
    def __init__(self, imp_model, particles):
        modeller.terms.energy_term.__init__(self)
        self.imp_model = imp_model
        self.particles = particles

    def eval(self, mdl, deriv, indats):
        """Calculate score of current state of model with respect to the IMP
           restraints. If calc_deriv, also calculate the partial derivatives
           associated with these restraints."""
        atoms = self.indices_to_atoms(mdl, indats)
        copy_modeller_coords_to_imp(atoms, self.particles)
        dvx = [0.] * len(indats)
        dvy = [0.] * len(indats)
        dvz = [0.] * len(indats)

        score = self.imp_model.evaluate(deriv)
        if deriv:
            get_imp_derivs(self.particles, dvx, dvy, dvz)
            return (score, dvx, dvy, dvz)
        else:
            return score


class ModellerRestraints(IMP.Restraint):
    """An IMP restraint using all defined Modeller restraints. Useful if
       you want to use Modeller restraints with an IMP optimizer."""

    def __init__(self, imp_model, modeller_model, particles):
        IMP.Restraint.__init__(self)
        self.modeller_model = modeller_model
        self.imp_model = imp_model
        self.particles = particles

    def evaluate(self, calc_deriv):
        """Calculate score of current state of model with respect to the
           Modeller restraints. If calc_deriv, also calculate the partial
           derivatives associated with these restraints."""
        atoms = self.modeller_model.atoms
        sel = modeller.selection(self.modeller_model)
        copy_imp_coords_to_modeller(self.particles, atoms)
        energies = sel.energy()
        if calc_deriv:
            add_modeller_derivs_to_imp(atoms, self.particles)

        return energies[0]


# ============== Creating particles ==============

def create_particles(num_particles, env, model, particles):
    """Create Modeller atoms by building a PDB file, and loading them from the
       file into Modeller. Additionally, copy them to the IMP model.
       Returns the Modeller model."""
    fp = open ("./temp_particles.pdb", "w")
    for i in range(0, num_particles):
        fp.write("ATOM  %5d  N   ALA     0       0.000   0.000   0.000  1.00  0.00           C  \n" % (i))
    fp.close()
    mdl = modeller.model(env, file='./temp_particles.pdb')
    init_imp_from_modeller(model, particles, mdl.atoms)
    return mdl


def create_particles_from_imp(env, model):
    """Create Modeller atoms from the current IMP particles.
       (This builds a PDB file, loads the atoms from the file into Modeller,
       and returns the Modeller model."""
    num_particles = len(model.particles)
    fp = open ("./temp_particles.pdb", "w")
    for i in range(0, num_particles):
        fp.write("ATOM  %5d  N   ALA     0       0.000   0.000   0.000  1.00  0.00           C  \n" % (i))
    fp.close()
    mdl = modeller.model(env, file='./temp_particles.pdb')
    copy_imp_coords_to_modeller(model.particles, mdl.atoms)
    return mdl


# ============== Rigid bodies ==============

def add_rigid_body(model, name, value, mdl):
    """Add rigid body to Modeller model based on given int value"""
    atoms = mdl.atoms
    atom_idx = 0
    rb_sel = modeller.selection()
    for p in model.particles:
        if p.get_int(name) == value:
            rb_sel.add(atoms[atom_idx])

        atom_idx = atom_idx + 1

    rb = modeller.rigid_body(rb_sel)
    mdl.restraints.rigid_bodies.append(rb)


def add_rigid_body_expr(model, expr, mdl):
    """Add rigid body to Modeller model based on given expression
       (e.g. p.get_float('radius') < 2.0)"""
    atoms = mdl.atoms
    atom_idx = 0
    rb_sel = modeller.selection()
    for p in model.particles:
        if eval(expr):
            rb_sel.add(atoms[atom_idx])

        atom_idx = atom_idx + 1

    rb = modeller.rigid_body(rb_sel)
    mdl.restraints.rigid_bodies.append(rb)


def move_rigid_body(model, name, value, dx, dy, dz, mdl):
    """Move all particles in the rigid body by the given amount"""
    atom_idx = 0
    for p in model.particles:
        if p.get_int(name) == value:
            p.set_x(p.x() + dx)
            p.set_y(p.y() + dy)
            p.set_z(p.z() + dz)

        atom_idx = atom_idx + 1

    copy_imp_coords_to_modeller(model.particles, mdl.atoms)



def move_rigid_body_expr(model, expr, dx, dy, dz, mdl):
    """Move all particles in the rigid body by the given amount"""
    atom_idx = 0
    for p in model.particles:
        if eval(expr):
            p.set_x(p.x() + dx)
            p.set_y(p.y() + dy)
            p.set_z(p.z() + dz)

        atom_idx = atom_idx + 1

    copy_imp_coords_to_modeller(model.particles, mdl.atoms)


def init_imp_from_modeller(model, particles, atoms):
    """Init IMP particles from Modeller atoms"""
    for (num, at) in enumerate(atoms):
        particles.append(IMP.utils.XYZParticle(model, at.x, at.y, at.z))


def copy_imp_coords_to_modeller(particles, atoms):
    """Copy atom coordinates from IMP to Modeller"""
    for (num, at) in enumerate(atoms):
        at.x = particles[num].x()
        at.y = particles[num].y()
        at.z = particles[num].z()


def copy_modeller_coords_to_imp(atoms, particles):
    """Copy atom coordinates from Modeller to IMP"""
    for (num, at) in enumerate(atoms):
        particles[num].set_x(at.x)
        particles[num].set_y(at.y)
        particles[num].set_z(at.z)


def add_modeller_derivs_to_imp(atoms, particles):
    """Add atom derivatives from Modeller to IMP"""
    for (num, at) in enumerate(atoms):
        particles[num].add_to_dx(at.dvx)
        particles[num].add_to_dy(at.dvy)
        particles[num].add_to_dz(at.dvz)


def get_imp_derivs(particles, dvx, dvy, dvz):
    """Move atom derivatives from IMP to Modeller"""
    for idx in range(0, len(dvx)):
        dvx[idx] = particles[idx].dx()
        dvy[idx] = particles[idx].dy()
        dvz[idx] = particles[idx].dz()


def show_modeller_and_imp(atoms, particles):
    """Show Modeller and IMP atoms and their partial derivatives"""
    print "Modeller:"
    for (num, at) in enumerate(atoms):
        print "(", at.x, ", ", at.y, ", ", at.z, ") (", at.dvx, ", ", at.dvy, ", ", at.dvz, ")"
    print "IMP:"
    for (num, at) in enumerate(atoms):
        print "(", particles[num].x(), ", ", particles[num].y(), ", ", particles[num].z(), ") (", particles[num].dx(), ", ", particles[num].dy(), ", ", particles[num].dz(), ")"
