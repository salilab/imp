%pythoncode %{

import math
import imp
import os
import sys
import tempfile
import shutil
import IMP.atom

def _import_modeller_scripts_optimizers():
    """Do the equivalent of "import modeller.scripts, modeller.optimizers".
       (We can't do the regular import because Python tries a relative import
       first, and that would load the modeller module in IMP.) This is an
       absolute import. Once we can require that everybody uses Python 2.6,
       this should no longer be required."""
    modeller = _import_module("modeller", "modeller", None)
    scripts = _import_module("scripts", "modeller.scripts", modeller)
    optimizers = _import_module("optimizers", "modeller.optimizers", modeller)
    modeller.scripts = scripts
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

modeller = _import_modeller_scripts_optimizers()


class _TempDir(object):
    """Make a temporary directory that is deleted when the object is."""

    def __init__(self):
        self.tmpdir = tempfile.mkdtemp()

    def __del__(self):
        shutil.rmtree(self.tmpdir, ignore_errors=True)


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
        self._imp_model = imp_model
        self._particles = particles

    def eval(self, mdl, deriv, indats):
        atoms = self.indices_to_atoms(mdl, indats)
        copy_modeller_coords_to_imp(atoms, self._particles)
        dvx = [0.] * len(indats)
        dvy = [0.] * len(indats)
        dvz = [0.] * len(indats)

        score = self._imp_model.evaluate(deriv)
        if deriv:
            get_imp_derivs(self._particles, dvx, dvy, dvz)
            return (score, dvx, dvy, dvz)
        else:
            return score


class ModellerRestraints(IMP.Restraint):
    """An IMP restraint using all defined Modeller restraints. Useful if
       you want to use Modeller restraints with an IMP optimizer."""

    def __init__(self, imp_model, modeller_model, particles):
        IMP.Restraint.__init__(self)
        self._modeller_model = modeller_model
        self._imp_model = imp_model
        self._particles = particles

    def evaluate(self, calc_deriv):
        atoms = self._modeller_model.atoms
        sel = modeller.selection(self._modeller_model)
        copy_imp_coords_to_modeller(self._particles, atoms)
        energies = sel.energy()
        if calc_deriv:
            add_modeller_derivs_to_imp(atoms, self._particles)

        return energies[0]


# ============== Creating particles ==============

def create_particles(num_particles, env, model, particles):
    """@deprecated Create Modeller atoms by building a PDB file, and loading
       them from the file into Modeller. Additionally, copy them to the
       IMP model.
       @return the Modeller model.
    """
    fp = open ("./temp_particles.pdb", "w")
    for i in range(0, num_particles):
        fp.write("ATOM  %5d  N   ALA     0       0.000   0.000   0.000  1.00  0.00           C  \n" % (i))
    fp.close()
    mdl = modeller.model(env, file='./temp_particles.pdb')
    os.unlink('temp_particles.pdb')
    init_imp_from_modeller(model, particles, mdl.atoms)
    return mdl


def create_particles_from_imp(env, model):
    """@deprecated Create Modeller atoms from the current IMP particles.
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
    """@deprecated Add rigid body to Modeller model based on given int value"""
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
    """@deprecated Add rigid body to Modeller model based on given expression
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
    """@deprecated Move all particles in the rigid body by the given amount"""
    atom_idx = 0
    for p in model.particles:
        if p.get_int(name) == value:
            p.set_x(p.x() + dx)
            p.set_y(p.y() + dy)
            p.set_z(p.z() + dz)

        atom_idx = atom_idx + 1

    copy_imp_coords_to_modeller(model.particles, mdl.atoms)



def move_rigid_body_expr(model, expr, dx, dy, dz, mdl):
    """@deprecated Move all particles in the rigid body by the given amount"""
    atom_idx = 0
    for p in model.particles:
        if eval(expr):
            p.set_x(p.x() + dx)
            p.set_y(p.y() + dy)
            p.set_z(p.z() + dz)

        atom_idx = atom_idx + 1

    copy_imp_coords_to_modeller(model.particles, mdl.atoms)


def init_imp_from_modeller(model, particles, atoms):
    """@deprecated Init IMP particles from Modeller atoms"""
    xk= IMP.FloatKey("x")
    yk= IMP.FloatKey("y")
    zk= IMP.FloatKey("z")
    for (num, at) in enumerate(atoms):
        p = IMP.Particle(model)
        p.add_attribute(xk, at.x, True)
        p.add_attribute(yk, at.y, True)
        p.add_attribute(zk, at.z, True)
        particles.append(p)


def copy_imp_coords_to_modeller(particles, atoms):
    """@deprecated Copy atom coordinates from IMP to Modeller"""
    xkey = IMP.FloatKey("x")
    ykey = IMP.FloatKey("y")
    zkey = IMP.FloatKey("z")
    for (num, at) in enumerate(atoms):
        at.x = particles[num].get_value(xkey)
        at.y = particles[num].get_value(ykey)
        at.z = particles[num].get_value(zkey)


def copy_modeller_coords_to_imp(atoms, particles):
    """@deprecated Copy atom coordinates from Modeller to IMP"""
    xkey = IMP.FloatKey("x")
    ykey = IMP.FloatKey("y")
    zkey = IMP.FloatKey("z")
    for (num, at) in enumerate(atoms):
        particles[num].set_value(xkey, at.x)
        particles[num].set_value(ykey, at.y)
        particles[num].set_value(zkey, at.z)


def add_modeller_derivs_to_imp(atoms, particles):
    """@deprecated Add atom derivatives from Modeller to IMP"""
    for (num, at) in enumerate(atoms):
        particles[num].add_to_dx(at.dvx)
        particles[num].add_to_dy(at.dvy)
        particles[num].add_to_dz(at.dvz)


def get_imp_derivs(particles, dvx, dvy, dvz):
    """@deprecated Move atom derivatives from IMP to Modeller"""
    xkey = IMP.FloatKey("x")
    ykey = IMP.FloatKey("y")
    zkey = IMP.FloatKey("z")
    for idx in range(0, len(dvx)):
        dvx[idx] = particles[idx].get_derivative(xkey)
        dvy[idx] = particles[idx].get_derivative(ykey)
        dvz[idx] = particles[idx].get_derivative(zkey)


def show_modeller_and_imp(atoms, particles):
    """@deprecated Show Modeller and IMP atoms and their partial derivatives"""
    print "Modeller:"
    for (num, at) in enumerate(atoms):
        print "(", at.x, ", ", at.y, ", ", at.z, ") (", at.dvx, ", ", at.dvy, ", ", at.dvz, ")"
    print "IMP:"
    for (num, at) in enumerate(atoms):
        print "(", particles[num].x(), ", ", particles[num].y(), ", ", particles[num].z(), ") (", particles[num].dx(), ", ", particles[num].dy(), ", ", particles[num].dz(), ")"

# Generators to create IMP UnaryFunction objects from Modeller parameters:
def _HarmonicLowerBoundGenerator(parameters, modalities):
    (mean, stdev) = parameters
    k = IMP.core.Harmonic.k_from_standard_deviation(stdev)
    return IMP.core.HarmonicLowerBound(mean, k)

def _HarmonicUpperBoundGenerator(parameters, modalities):
    (mean, stdev) = parameters
    k = IMP.core.Harmonic.k_from_standard_deviation(stdev)
    return IMP.core.HarmonicUpperBound(mean, k)

def _HarmonicGenerator(parameters, modalities):
    (mean, stdev) = parameters
    k = IMP.core.Harmonic.k_from_standard_deviation(stdev)
    return IMP.core.Harmonic(mean, k)

def _CosineGenerator(parameters, modalities):
    (phase, force_constant) = parameters
    (periodicity,) = modalities
    return IMP.core.Cosine(force_constant, periodicity, phase)

def _LinearGenerator(parameters, modalities):
    (scale,) = parameters
    return IMP.core.Linear(0, scale)

def _SplineGenerator(parameters, modalities):
    (open, low, high, delta, lowderiv, highderiv) = parameters[:6]
    values = IMP.Floats()
    for v in parameters[6:]:
        values.append(v)
    if open < 0.0:
        return IMP.core.ClosedCubicSpline(values, low, delta)
    else:
        return IMP.core.OpenCubicSpline(values, low, delta)

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
    return IMP.core.DistanceRestraint(unary_func_gen(parameters, modalities),
                                      atoms[0], atoms[1])

def _AngleRestraintGenerator(form, modalities, atoms, parameters):
    unary_func_gen = _unary_func_generators[form]
    return IMP.core.AngleRestraint(unary_func_gen(parameters, modalities),
                                   atoms[0], atoms[1], atoms[2])

def _MultiBinormalGenerator(form, modalities, atoms, parameters):
    nterms = modalities[0]
    if len(parameters) != nterms * 6:
        raise ValueError("Incorrect number of parameters (%d) for multiple "
                         "binormal restraint - expecting %d (%d terms * 6)" \
                         % (len(parameters), nterms * 6, nterms))
    r = IMP.modeller.MultipleBinormalRestraint(IMP.ParticleQuad(*atoms[:4]),
                                               IMP.ParticleQuad(*atoms[4:8]))
    for i in range(nterms):
        t = IMP.modeller.BinormalTerm()
        t.set_weight(parameters[i])
        t.set_means((parameters[nterms + i * 2],
                     parameters[nterms + i * 2 + 1]))
        t.set_standard_deviations((parameters[nterms * 3 + i * 2],
                                   parameters[nterms * 3 + i * 2 + 1]))
        t.set_correlation(parameters[nterms * 5 + i])
        r.add_term(t)
    return r

def _DihedralRestraintGenerator(form, modalities, atoms, parameters):
    if form == 9:
        return _MultiBinormalGenerator(form, modalities, atoms, parameters)
    unary_func_gen = _unary_func_generators[form]
    return IMP.core.DihedralRestraint(unary_func_gen(parameters, modalities),
                                      atoms[0], atoms[1], atoms[2], atoms[3])

def _get_protein_atom_particles(protein):
    """Given a protein particle, get the flattened list of all child atoms"""
    atom_particles = []
    #protein = IMP.core.Hierarchy.decorate_particle(protein)
    for ichain in range(protein.get_number_of_children()):
        chain = protein.get_child(ichain)
        for ires in range(chain.get_number_of_children()):
            residue = chain.get_child(ires)
            for iatom in range(residue.get_number_of_children()):
                atom = residue.get_child(iatom)
                atom_particles.append(atom.get_particle())
    return atom_particles

def _load_restraints_line(line, atom_particles):
    """Parse a single Modeller restraints file line and return the
       corresponding IMP restraint."""
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
    return restraint_gen(form, modalities, atoms, parameters)


def _load_entire_restraints_file(filename, protein):
    """Yield a set of IMP restraints from a Modeller restraints file."""
    atoms = _get_protein_atom_particles(protein)
    fh = open(filename, 'r')
    for line in fh:
        try:
            rsr = _load_restraints_line(line, atoms)
            if rsr is not None:
                yield rsr
        except Exception, err:
            print "Cannot read restraints file line:\n" + line
            raise


def load_restraints_file(filename, protein):
    """@deprecated Use ModelLoader instead.

       Convert a Modeller restraints file into equivalent IMP::Restraints.
       @param filename Name of the Modeller restraints file.
       @param protein An IMP::atom::Hierarchy containing the protein atoms
                      (e.g. as returned by read_pdb). The Modeller restraints
                      file is assumed to act on the same PDB described by
                      protein.
       @return A Python list of the newly-created IMP::Restraint objects.
    """
    return list(_load_entire_restraints_file(filename, protein))


def _copy_residue(r, model):
    """Copy residue information from modeller to imp"""
    #print "residue "+str(r)
    p=IMP.Particle(model)
    rp= IMP.atom.Residue.setup_particle(p, IMP.atom.ResidueType(r.pdb_name),
                                         r.index)
    p.set_name(str("residue "+r.num));
    return p


def _copy_atom(a, model):
    """Copy atom information from modeller"""
    #print "atom "+str(a)
    p=IMP.Particle(model)
    ap= IMP.atom.Atom.setup_particle(p,IMP.atom.AtomType(a.name))
    xyzd= IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(a.x, a.y, a.z))
    #ap.set_atom_type(IMP.atom.AtomType(a.name))
    #IMP.core.Name.setup_particle(p).set_name(str("atom "+a._atom__get_num()));
    if (a.charge != 0):
        ap.set_charge(a.charge)
    ap.set_input_index(a.index)
    return p

def _copy_chain(c, model):
    """Copy chain information from modeller"""
    #print "atom "+str(a)
    p=IMP.Particle(model)
    #set the chain name
    cp = IMP.atom.Chain.setup_particle(p,c.name)
    return p


def _copy_bonds(pdb, atoms, model):
    for b in pdb.bonds:
        maa= b[0]
        mab= b[1]
        pa=atoms[maa.index]
        pb=atoms[mab.index]
        if IMP.atom.Bonded.particle_is_instance(pa):
            ba= IMP.atom.Bonded.decorate_particle(pa)
        else:
            ba= IMP.atom.Bonded.setup_particle(pa)
        if IMP.atom.Bonded.particle_is_instance(pb):
            bb= IMP.atom.Bonded.decorate_particle(pb)
        else:
            bb= IMP.atom.Bonded.setup_particle(pb)
        bp= IMP.atom.bond(ba, bb, IMP.atom.Bond.COVALENT)


class ModelLoader(object):
    """Read a Modeller model into IMP. After creating this object, the atoms
       in the Modeller model can be loaded into IMP using the load_atoms()
       method, then optionally any Modeller static restraints can be read in
       with load_static_restraints() or load_static_restraints_file().
       @param modeller_model The Modeller model object to read.
    """

    def __init__(self, modeller_model):
        self._modeller_model = modeller_model

    def load_atoms(self, model):
        """Construct an IMP::atom::Hierarchy that contains the same atoms as
           the Modeller model.
           @param model The IMP::Model object in which the hierarchy will be
                        created. The highest level hierarchy node is a PROTEIN.
           @return the newly-created root IMP::atom::Hierarchy.
        """
        pp = IMP.Particle(model)
        hpp = IMP.atom.Hierarchy.setup_particle(pp)
        atoms = {}
        for chain in self._modeller_model.chains:
            cp = IMP.Particle(model)
            hcp = IMP.atom.Chain.setup_particle(cp, chain.name)
            # We don't really know the type yet
            hpp.add_child(hcp)
            for residue in chain.residues:
                rp = _copy_residue(residue, model)
                hrp = IMP.atom.Hierarchy.decorate_particle(rp)
                hcp.add_child(hrp)
                for atom in residue.atoms:
                    ap = _copy_atom(atom, model)
                    hap = IMP.atom.Hierarchy.decorate_particle(ap)
                    hrp.add_child(hap)
                    atoms[atom.index] = ap
                lastres = hrp
        _copy_bonds(self._modeller_model, atoms, model)
        self._modeller_hierarchy = hpp
        return hpp

    def load_static_restraints_file(self, filename):
        """Convert a Modeller static restraints file into equivalent
           IMP::Restraints. load_atoms() must have been called first to read
           in the atoms that the restraints will act upon.
           @param filename Name of the Modeller restraints file. The restraints
                  in this file are assumed to act upon the model read in by
                  load_atoms(); no checking is done to enforce this.
           @return A Python generator of the newly-created IMP::Restraint
                   objects.
        """
        if not hasattr(self, '_modeller_hierarchy'):
            raise ValueError("Call load_atoms() first.")
        return _load_entire_restraints_file(filename, self._modeller_hierarchy)


    def load_static_restraints(self):
        """Convert the current set of Modeller static restraints into equivalent
           IMP::Restraints. load_atoms() must have been called first to read
           in the atoms that the restraints will act upon.
           @return A Python generator of the newly-created IMP::Restraint
                   objects.
        """
        class _RestraintGenerator(object):
            """Simple generator wrapper"""
            def __init__(self, gen):
                self._gen = gen
            def __iter__(self, *args, **keys):
                return self
            def close(self, *args, **keys):
                return self._gen.close(*args, **keys)
            def next(self, *args, **keys):
                return self._gen.next(*args, **keys)
            def send(self, *args, **keys):
                return self._gen.send(*args, **keys)
            def throw(self, *args, **keys):
                return self._gen.throw(*args, **keys)
        # Write current restraints into a temporary file
        t = _TempDir()
        rsrfile = os.path.join(t.tmpdir, 'restraints.rsr')
        self._modeller_model.restraints.write(file=rsrfile)
        # Read the file back in
        gen = self.load_static_restraints_file(rsrfile)
        wrap = _RestraintGenerator(gen)
        # Ensure that tmpdir remains until we're done with the generator
        wrap._tempdir = t
        return wrap


def read_pdb(name, model, special_patches=None):
    """@deprecated Use IMP::atom::read_pdb() instead to read a PDB file,
                   or ModelLoader to read a Modeller model.

       Construct an IMP::atom::Hierarchy from a PDB file.
       @param name The name of the PDB file to read.
       @param model The IMP::Model object in which the hierarchy will be
                    created. The highest level hierarchy node is a PROTEIN.
       @param special_patches If given, a function that applies patches
                              (e.g. nucleic acid termini) to the Modeller model.
       @return the newly-created root IMP::atom::Hierarchy.
    """
    e = modeller.environ()
    e.libs.topology.read('${LIB}/top_heav.lib')
    e.libs.parameters.read('${LIB}/par.lib')
    e.io.hetatm=True
    modeller_model = modeller.scripts.complete_pdb(e, name,
                                           special_patches=special_patches)
    loader = ModelLoader(modeller_model)
    hpp = loader.load_atoms(model)
    hpp.get_particle().set_name(name)
    return hpp

%}
