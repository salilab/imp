"""Interface between IMP and MODELLER."""

import math
import imp
import IMP
import IMP.utils

def _import_modeller_optimizers():
    """Do the equivalent of "import modeller.optimizers".
       (We can't do the regular import because Python tries a relative import
       first, and that would load ourselves.) This is an absolute import. Once
       we can require that everybody uses Python 2.6, this should no longer
       be required."""
    modeller = _import_module("modeller", "modeller", None)
    optimizers = _import_module("optimizers", "modeller.optimizers", modeller)
    modeller.optimizers = optimizers
    return modeller

def _import_module(partname, fqname, parent):
    """Import a single Python module, possibly from a parent."""
    fp, pathname, description = imp.find_module(partname,
                                                parent and parent.__path__)
    try:
        m = imp.load_module(fqname, fp, pathname, description)
    finally:
        # imp module requires that we explicitly close fp, even on exception
        if fp:
            fp.close()
    return m

modeller = _import_modeller_optimizers()


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
    xk= IMP.FloatKey("x")
    yk= IMP.FloatKey("y")
    zk= IMP.FloatKey("z")
    for (num, at) in enumerate(atoms):
        p = IMP.Particle()
        model.add_particle(p)
        p.add_attribute(xk, at.x, True)
        p.add_attribute(yk, at.y, True)
        p.add_attribute(zk, at.z, True)
        particles.append(p)


def copy_imp_coords_to_modeller(particles, atoms):
    """Copy atom coordinates from IMP to Modeller"""
    xkey = IMP.FloatKey("x")
    ykey = IMP.FloatKey("y")
    zkey = IMP.FloatKey("z")
    for (num, at) in enumerate(atoms):
        at.x = particles[num].get_value(xkey)
        at.y = particles[num].get_value(ykey)
        at.z = particles[num].get_value(zkey)


def copy_modeller_coords_to_imp(atoms, particles):
    """Copy atom coordinates from Modeller to IMP"""
    xkey = IMP.FloatKey("x")
    ykey = IMP.FloatKey("y")
    zkey = IMP.FloatKey("z")
    for (num, at) in enumerate(atoms):
        particles[num].set_value(xkey, at.x)
        particles[num].set_value(ykey, at.y)
        particles[num].set_value(zkey, at.z)


def add_modeller_derivs_to_imp(atoms, particles):
    """Add atom derivatives from Modeller to IMP"""
    for (num, at) in enumerate(atoms):
        particles[num].add_to_dx(at.dvx)
        particles[num].add_to_dy(at.dvy)
        particles[num].add_to_dz(at.dvz)


def get_imp_derivs(particles, dvx, dvy, dvz):
    """Move atom derivatives from IMP to Modeller"""
    xkey = IMP.FloatKey("x")
    ykey = IMP.FloatKey("y")
    zkey = IMP.FloatKey("z")
    for idx in range(0, len(dvx)):
        dvx[idx] = particles[idx].get_derivative(xkey)
        dvy[idx] = particles[idx].get_derivative(ykey)
        dvz[idx] = particles[idx].get_derivative(zkey)


def show_modeller_and_imp(atoms, particles):
    """Show Modeller and IMP atoms and their partial derivatives"""
    print "Modeller:"
    for (num, at) in enumerate(atoms):
        print "(", at.x, ", ", at.y, ", ", at.z, ") (", at.dvx, ", ", at.dvy, ", ", at.dvz, ")"
    print "IMP:"
    for (num, at) in enumerate(atoms):
        print "(", particles[num].x(), ", ", particles[num].y(), ", ", particles[num].z(), ") (", particles[num].dx(), ", ", particles[num].dy(), ", ", particles[num].dz(), ")"

# Generators to create IMP UnaryFunction objects from Modeller parameters:
def _HarmonicLowerBoundGenerator(parameters, modalities):
    (mean, stdev) = parameters
    k = IMP.Harmonic.k_from_standard_deviation(stdev)
    return IMP.HarmonicLowerBound(mean, k)

def _HarmonicUpperBoundGenerator(parameters, modalities):
    (mean, stdev) = parameters
    k = IMP.Harmonic.k_from_standard_deviation(stdev)
    return IMP.HarmonicUpperBound(mean, k)

def _HarmonicGenerator(parameters, modalities):
    (mean, stdev) = parameters
    k = IMP.Harmonic.k_from_standard_deviation(stdev)
    return IMP.Harmonic(mean, k)

def _CosineGenerator(parameters, modalities):
    (phase, force_constant) = parameters
    (periodicity,) = modalities
    return IMP.Cosine(force_constant, periodicity, phase)

def _LinearGenerator(parameters, modalities):
    (scale,) = parameters
    return IMP.Linear(0, scale)

def _SplineGenerator(parameters, modalities):
    (open, low, high, delta, lowderiv, highderiv) = parameters[:6]
    values = IMP.Floats()
    for v in parameters[6:]:
        values.append(v)
    if open < 0.0:
        return IMP.ClosedCubicSpline(values, low, delta)
    else:
        return IMP.OpenCubicSpline(values, low, delta)

#: Mapping from Modeller math form number to a unary function generator
_unary_func_generators = {
    1: _HarmonicLowerBoundGenerator,
    2: _HarmonicUpperBoundGenerator,
    3: _HarmonicGenerator,
    7: _CosineGenerator,
    8: _LinearGenerator,
    10: _SplineGenerator,
}

# Generators to make IMP Restraint objects from Modeller features
def _DistanceRestraintGenerator(form, modalities, atoms, parameters):
    unary_func_gen = _unary_func_generators[form]
    return IMP.DistanceRestraint(unary_func_gen(parameters, modalities),
                                 atoms[0], atoms[1])

def _AngleRestraintGenerator(form, modalities, atoms, parameters):
    unary_func_gen = _unary_func_generators[form]
    return IMP.AngleRestraint(unary_func_gen(parameters, modalities),
                              atoms[0], atoms[1], atoms[2])

def _DihedralRestraintGenerator(form, modalities, atoms, parameters):
    unary_func_gen = _unary_func_generators[form]
    return IMP.DihedralRestraint(unary_func_gen(parameters, modalities),
                                 atoms[0], atoms[1], atoms[2], atoms[3])

def _get_protein_atom_particles(protein):
    """Given a protein particle, get the flattened list of all child atoms"""
    atom_particles = []
    #protein = IMP.MolecularHierarchyDecorator.cast(protein)
    for ichain in range(protein.get_number_of_children()):
        chain = protein.get_child(ichain)
        for ires in range(chain.get_number_of_children()):
            residue = chain.get_child(ires)
            for iatom in range(residue.get_number_of_children()):
                atom = residue.get_child(iatom)
                atom_particles.append(atom.get_particle())
    return atom_particles

def _load_restraints_line(line, atom_particles, rsr):
    """Parse a single Modeller restraints file line"""
    spl = line.split()
    typ = spl.pop(0)
    if typ == 'MODELLER5':
        return
    elif typ != 'R':
        raise NotImplementedError("Only 'R' lines currently read from " + \
                                  "Modeller restraints files")
    form = int(spl.pop(0))
    modalities = [int(spl.pop(0))]
    features = [int(spl.pop(0))]
    # Discard group
    spl.pop(0)
    natoms = [int(spl.pop(0))]
    nparam = int(spl.pop(0))
    nfeat = int(spl.pop(0))
    for i in range(nfeat - 1):
        modalities.append(int(spl.pop(0)))
        features.append(int(spl.pop(0)))
        natoms.append(int(spl.pop(0)))
    atoms = [int(spl.pop(0)) for x in range(natoms[0])]
    for i in range(len(atoms)):
        atoms[i] = atom_particles[atoms[i] - 1]
    parameters = [float(spl.pop(0)) for x in range(nparam)]
    restraint_generators = {
        1 : _DistanceRestraintGenerator,
        2 : _AngleRestraintGenerator,
        3 : _DihedralRestraintGenerator,
    }
    restraint_gen = restraint_generators[features[0]]
    rsr.append(restraint_gen(form, modalities, atoms, parameters))


def load_restraints_file(filename, protein):
    """Load the specified Modeller restraints file and generate equivalent
       IMP restraints on `protein` (a protein particle, as returned from
       `pdb.read_pdb`). The Modeller restraints file is assumed to act on
       the same PDB described by `protein`. The list of all restraints
       is returned."""
    atoms = _get_protein_atom_particles(protein)
    fh = file(filename, 'r')
    rsr = []
    for line in fh:
        try:
            _load_restraints_line(line, atoms, rsr)
        except Exception, err:
            print "Cannot read restraints file line:\n" + line
            raise
    return rsr
