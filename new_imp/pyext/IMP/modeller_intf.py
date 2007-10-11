import modeller
import modeller.optimizers
import math
import IMP
from IMP.utils import *

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

class Modeller_Restraints(IMP.Restraint):
    """Calculate score and derivs based on all Modeller-defined restraints"""

    def __init__(self, imp_model, modeller_model, particles):
        IMP.Restraint.__init__(self)
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


# ============== Creating particles ==============

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


# ============== Rigid bodies ==============

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
