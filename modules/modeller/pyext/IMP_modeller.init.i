%pythoncode %{

import math
import imp
import os
import sys
import tempfile
import shutil
import IMP.atom
import IMP.container
import modeller.scripts
import modeller.optimizers

class _TempDir(object):
    """Make a temporary directory that is deleted when the object is."""

    def __init__(self):
        self.tmpdir = tempfile.mkdtemp()

    def __del__(self):
        shutil.rmtree(self.tmpdir, ignore_errors=True)


class IMPRestraints(modeller.terms.energy_term):
    """A Modeller restraint which evaluates an IMP scoring function.
       This can be used to incorporate IMP Restraints into an existing
       comparative modeling pipeline, or to use Modeller optimizers or
       protocols.
    """

    _physical_type = modeller.physical.absposition

    def __init__(self, particles, scoring_function=None):
        """Constructor.
           @param particles A list of the IMP atoms (as Particle objects),
                            same order as the Modeller atoms.
           @param scoring_function An IMP::ScoringFunction object that will
                            be incorporated into the Modeller score (molpdf).
           @note since Modeller, unlike IMP, is sensitive to the ordering
                 of atoms, it usually makes sense to create the model in
                 Modeller and then use ModelLoader to load it into IMP,
                 since that will preserve the Modeller atom ordering in IMP.
        """
        modeller.terms.energy_term.__init__(self)
        self._particles = particles
        if scoring_function:
            self._sf = scoring_function
        else:
            self._sf = particles[0].get_model()

    def eval(self, mdl, deriv, indats):
        atoms = self.indices_to_atoms(mdl, indats)
        _copy_modeller_coords_to_imp(atoms, self._particles)
        if len(self._particles) == 0:
            score = 0.
        else:
            score = self._sf.evaluate(deriv)
        if deriv:
            dvx = [0.] * len(indats)
            dvy = [0.] * len(indats)
            dvz = [0.] * len(indats)
            _get_imp_derivs(self._particles, dvx, dvy, dvz)
            return (score, dvx, dvy, dvz)
        else:
            return score


class ModellerRestraints(IMP.Restraint):
    """An IMP restraint using all defined Modeller restraints.
       This is useful if you want to use Modeller restraints with an IMP
       optimizer, or in combination with IMP restraints.

       @note Currently only the coordinates of the atoms are translated
             between Modeller and IMP; thus, a Modeller restraint which
             uses any other attribute (e.g. charge) will not react if
             this attribute is changed by IMP.
    """

    def __init__(self, model, modeller_model, particles):
        """Constructor.
           @param model The IMP Model object.
           @param modeller_model The Modeller model object.
           @param particles A list of the IMP atoms (as Particle objects),
                            in the same order as the Modeller atoms.
           @note since Modeller, unlike IMP, is sensitive to the ordering
                 of atoms, it usually makes sense to create the model in
                 Modeller and then use ModelLoader to load it into IMP,
                 since that will preserve the Modeller atom ordering in IMP.
        """
        def get_particle(x):
            if hasattr(x, 'get_particle'):
                return x.get_particle()
            else:
                return x
        IMP.Restraint.__init__(self, model, "ModellerRestraints %1%")
        self._modeller_model = modeller_model
        self._particles = [get_particle(x) for x in particles]

    def unprotected_evaluate(self, accum):
        atoms = self._modeller_model.atoms
        sel = modeller.selection(self._modeller_model)
        _copy_imp_coords_to_modeller(self._particles, atoms)
        energies = sel.energy()
        if accum:
            _add_modeller_derivs_to_imp(atoms, self._particles, accum)

        return energies[0]

    def get_version_info(self):
        return IMP.VersionInfo("IMP developers", "0.1")
    def do_show(self, fh):
        fh.write("ModellerRestraints")
    def do_get_inputs(self):
        return self._particles


def _copy_imp_coords_to_modeller(particles, atoms):
    """Copy atom coordinates from IMP to Modeller"""
    xkey = IMP.FloatKey("x")
    ykey = IMP.FloatKey("y")
    zkey = IMP.FloatKey("z")
    for (num, at) in enumerate(atoms):
        at.x = particles[num].get_value(xkey)
        at.y = particles[num].get_value(ykey)
        at.z = particles[num].get_value(zkey)


def _copy_modeller_coords_to_imp(atoms, particles):
    """Copy atom coordinates from Modeller to IMP"""
    xkey = IMP.FloatKey("x")
    ykey = IMP.FloatKey("y")
    zkey = IMP.FloatKey("z")
    for (num, at) in enumerate(atoms):
        particles[num].set_value(xkey, at.x)
        particles[num].set_value(ykey, at.y)
        particles[num].set_value(zkey, at.z)


def _add_modeller_derivs_to_imp(atoms, particles, accum):
    """Add atom derivatives from Modeller to IMP"""
    for (num, at) in enumerate(atoms):
        xyz = IMP.core.XYZ(particles[num])
        xyz.add_to_derivative(0, at.dvx, accum)
        xyz.add_to_derivative(1, at.dvy, accum)
        xyz.add_to_derivative(2, at.dvz, accum)


def _get_imp_derivs(particles, dvx, dvy, dvz):
    """Move atom derivatives from IMP to Modeller"""
    xkey = IMP.FloatKey("x")
    ykey = IMP.FloatKey("y")
    zkey = IMP.FloatKey("z")
    for idx in range(0, len(dvx)):
        dvx[idx] = particles[idx].get_derivative(xkey)
        dvy[idx] = particles[idx].get_derivative(ykey)
        dvz[idx] = particles[idx].get_derivative(zkey)


# Generators to create IMP UnaryFunction objects from Modeller parameters:
def _HarmonicLowerBoundGenerator(parameters, modalities):
    (mean, stdev) = parameters
    k = IMP.core.Harmonic.get_k_from_standard_deviation(stdev)
    return IMP.core.HarmonicLowerBound(mean, k)

def _HarmonicUpperBoundGenerator(parameters, modalities):
    (mean, stdev) = parameters
    k = IMP.core.Harmonic.get_k_from_standard_deviation(stdev)
    return IMP.core.HarmonicUpperBound(mean, k)

def _HarmonicGenerator(parameters, modalities):
    (mean, stdev) = parameters
    k = IMP.core.Harmonic.get_k_from_standard_deviation(stdev)
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
    values = []
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
    return IMP.core.DistanceRestraint(atoms[0].get_model(),
                                      unary_func_gen(parameters, modalities),
                                      atoms[0], atoms[1])

def _AngleRestraintGenerator(form, modalities, atoms, parameters):
    unary_func_gen = _unary_func_generators[form]
    return IMP.core.AngleRestraint(atoms[0].get_model(),
                                   unary_func_gen(parameters, modalities),
                                   atoms[0], atoms[1], atoms[2])

def _MultiBinormalGenerator(form, modalities, atoms, parameters):
    nterms = modalities[0]
    if len(parameters) != nterms * 6:
        raise ValueError("Incorrect number of parameters (%d) for multiple "
                         "binormal restraint - expecting %d (%d terms * 6)" \
                         % (len(parameters), nterms * 6, nterms))
    r = IMP.core.MultipleBinormalRestraint(atoms[0].get_model(),
                                           atoms[:4], atoms[4:8])
    for i in range(nterms):
        t = IMP.core.BinormalTerm()
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
    return IMP.core.DihedralRestraint(atoms[0].get_model(),
                                      unary_func_gen(parameters, modalities),
                                      atoms[0], atoms[1], atoms[2], atoms[3])

def _get_protein_atom_particles(protein):
    """Given a protein particle, get the flattened list of all child atoms"""
    atom_particles = []
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
        except Exception as err:
            print("Cannot read restraints file line:\n" + line)
            raise


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
    # Alignment structures don't have charges or atom types; models do
    if hasattr(a, 'charge'):
        IMP.atom.Charged.setup_particle(p, a.charge)
    if hasattr(a, 'type'):
        IMP.atom.CHARMMAtom.setup_particle(p, a.type.name)
    ap.set_input_index(a.index)
    return p

def _copy_chain(c, model):
    """Copy chain information from modeller"""
    #print "atom "+str(a)
    p=IMP.Particle(model)
    #set the chain name
    cp = IMP.atom.Chain.setup_particle(p,c.name)
    return p

def _get_forcefield(submodel):
    if submodel == 3:
        ff = IMP.atom.CHARMMParameters(
                 IMP.atom.get_data_path('top_heav.lib'),
                 IMP.atom.get_data_path('par.lib'))
    else:
        ff = IMP.atom.CHARMMParameters(
                 IMP.atom.get_data_path('top.lib'),
                 IMP.atom.get_data_path('par.lib'))
    return ff

def add_soft_sphere_radii(hierarchy, submodel, scale=1.0, filename=None):
    """Add radii to the hierarchy using the Modeller radius library, radii.lib.
       Each radius is scaled by the given scale (Modeller usually scales radii
       by a factor of 0.82). submodel specifies the topology submodel, which is
       the column in radii.lib to use."""
    if filename is None:
        filename = IMP.atom.get_data_path('radii.lib')
    radii = {}
    for line in open(filename):
        if line.startswith('#'): continue
        spl = line.split()
        if len(spl) > 11:
            radii[spl[0]] = float(spl[submodel])
    atoms = IMP.atom.get_by_type(hierarchy, IMP.atom.ATOM_TYPE)
    for a in atoms:
        p = a.get_particle()
        ct = IMP.atom.CHARMMAtom(p).get_charmm_type()
        if ct in radii:
            IMP.core.XYZR.setup_particle(p, radii[ct] * scale)


class ModelLoader(object):
    """Read a Modeller model into IMP. After creating this object, the atoms
       in the Modeller model can be loaded into IMP using the load_atoms()
       method, then optionally any Modeller static restraints can be read in
       with load_static_restraints() or load_static_restraints_file().

       This class can also be used to read Modeller alignment structures;
       however, only load_atoms() will be useful in such a case (since
       alignment structures don't have restraints or other information).

    """

    def __init__(self, modeller_model):
        """Constructor.
           @param modeller_model The Modeller model or alignment structure
                                 object to read.
        """
        self._modeller_model = modeller_model

    def load_atoms(self, model):
        """Construct an IMP::atom::Hierarchy that contains the same atoms as
           the Modeller model or alignment structure.

           IMP atoms created from a Modeller model will be given charges and
           CHARMM types, extracted from the model. Alignment structures don't
           contain this information, so the IMP atoms won't either.

           @param model The IMP::Model object in which the hierarchy will be
                        created. The highest level hierarchy node is a PROTEIN.
           @return the newly-created root IMP::atom::Hierarchy.
        """
        pp = IMP.Particle(model)
        hpp = IMP.atom.Hierarchy.setup_particle(pp)
        self._atoms = {}
        for chain in self._modeller_model.chains:
            cp = IMP.Particle(model)
            hcp = IMP.atom.Chain.setup_particle(cp, chain.name)
            # We don't really know the type yet
            hpp.add_child(hcp)
            for residue in chain.residues:
                rp = _copy_residue(residue, model)
                hrp = IMP.atom.Hierarchy(rp)
                hcp.add_child(hrp)
                for atom in residue.atoms:
                    ap = _copy_atom(atom, model)
                    hap = IMP.atom.Hierarchy(ap)
                    hrp.add_child(hap)
                    self._atoms[atom.index] = ap
                lastres = hrp
        self._modeller_hierarchy = hpp
        return hpp

    def _get_nonbonded_list(self, atoms, pair_filter, edat, distance):
        nbl = IMP.container.ClosePairContainer(atoms, distance,
                                               edat.update_dynamic)
    
        # Exclude the same sets of atoms as Modeller
        if pair_filter is None:
            pair_filter = IMP.atom.StereochemistryPairFilter()
            if edat.excl_local[0]:
                pair_filter.set_bonds(list(self.load_bonds()))
            if edat.excl_local[1]:
                pair_filter.set_angles(list(self.load_angles()))
            if edat.excl_local[2]:
                pair_filter.set_dihedrals(list(self.load_dihedrals()))
        nbl.add_pair_filter(pair_filter)
        return nbl

    def load_bonds(self):
        """Load the Modeller bond topology into the IMP model. Each bond is
           represented in IMP as an IMP::atom::Bond, with no defined length
           or stiffness. These bonds are primarily useful as input to
           IMP::atom::StereochemistryPairFilter, to exclude bond interactions
           from the nonbonded list. Typically the contribution to the scoring
           function from the bonds is included in the Modeller static restraints
           (use load_static_restraints() or load_static_restraints_file() to
           load these). If you want to regenerate the stereochemistry in IMP,
           do not use these functions (as then stereochemistry scoring terms
           and exclusions would be double-counted) and instead use the
           IMP::atom::CHARMMTopology class.

           You must call load_atoms() prior to using this function.
           @see load_angles(), load_dihedrals(), load_impropers()
           @return A generator listing all of the bonds.
        """
        if not hasattr(self, '_modeller_hierarchy'):
            raise ValueError("Call load_atoms() first.")
        for (maa, mab) in self._modeller_model.bonds:
            pa = self._atoms[maa.index]
            pb = self._atoms[mab.index]
            if IMP.atom.Bonded.get_is_setup(pa):
                ba= IMP.atom.Bonded(pa)
            else:
                ba= IMP.atom.Bonded.setup_particle(pa)
            if IMP.atom.Bonded.get_is_setup(pb):
                bb= IMP.atom.Bonded(pb)
            else:
                bb= IMP.atom.Bonded.setup_particle(pb)
            yield IMP.atom.create_bond(ba, bb,
                                       IMP.atom.Bond.SINGLE).get_particle()

    def load_angles(self):
        """Load the Modeller angle topology into the IMP model.
           See load_bonds() for more details."""
        return self._internal_load_angles(self._modeller_model.angles,
                                          IMP.atom.Angle)

    def load_dihedrals(self):
        """Load the Modeller dihedral topology into the IMP model.
           See load_bonds() for more details."""
        return self._internal_load_angles(self._modeller_model.dihedrals,
                                          IMP.atom.Dihedral)

    def load_impropers(self):
        """Load the Modeller improper topology into the IMP model.
           See load_bonds() for more details."""
        return self._internal_load_angles(self._modeller_model.impropers,
                                          IMP.atom.Dihedral)

    def _internal_load_angles(self, angles, angle_class):
        if not hasattr(self, '_modeller_hierarchy'):
            raise ValueError("Call load_atoms() first.")
        for modeller_atoms in angles:
            imp_particles = [self._atoms[x.index] for x in modeller_atoms]
            p = IMP.Particle(imp_particles[0].get_model())
            a = angle_class.setup_particle(p,
                                 *[IMP.core.XYZ(x) for x in imp_particles])
            yield a.get_particle()

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
            def next(self):
                return next(self._gen)
            __next__ = next
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

    def load_dynamic_restraints(self, pair_filter=None):
        """Convert Modeller dynamic restraints into IMP::Restraint objects.

           For each currently active Modeller dynamic restraint
           (e.g. soft-sphere, electrostatics) an equivalent IMP::Restraint
           is created.
           load_atoms() must have been called first to read
           in the atoms that the restraints will act upon.

           If pair_filter is given, it is an IMP::PairFilter object to exclude
           pairs from the nonbonded lists used by the dynamic restraints.
           Otherwise, an IMP::atom::StereochemistryPairFilter object is created
           to exclude Modeller bonds, angles and dihedrals, as specified by
           edat.excl_local. (Note that this calls load_bonds(), load_angles()
           and load_dihedrals(), so will create duplicate lists of bonds if
           those methods are called manually as well.)

           @note Currently only soft-sphere, electrostatic and Lennard-Jones
                 restraints are loaded.
           @return A Python generator of the newly-created IMP::Restraint
                   objects.
        """
        if not hasattr(self, '_modeller_hierarchy'):
            raise ValueError("Call load_atoms() first.")
        edat = self._modeller_model.env.edat
        libs = self._modeller_model.env.libs
        atoms = IMP.atom.get_leaves(self._modeller_hierarchy)
        m = atoms[0].get_model()
        atoms = IMP.container.ListSingletonContainer(m, IMP.get_indexes(atoms))
        
        if edat.dynamic_sphere:
            # Note: cannot use Modeller's cutoff distance, as that is
            # center-to-center; IMP's is sphere surface-surface
            nbl = self._get_nonbonded_list(atoms, pair_filter, edat, 0.)
            # No way to get Modeller radii, so we have to reassign them
            add_soft_sphere_radii(self._modeller_hierarchy,
                                  libs.topology.submodel, edat.radii_factor)
            k = \
              IMP.core.Harmonic.get_k_from_standard_deviation(edat.sphere_stdv)
            ps = IMP.core.SphereDistancePairScore(
                              IMP.core.HarmonicLowerBound(0, k))
            yield IMP.container.PairsRestraint(ps, nbl)

        if edat.dynamic_lennard or edat.dynamic_coulomb:
            # 3.0 is roughly the max. atom diameter
            d = max(edat.contact_shell - 3.0, 0.0)
            nbl = self._get_nonbonded_list(atoms, pair_filter, edat, d)
            ff = _get_forcefield(libs.topology.submodel)
            ff.add_radii(self._modeller_hierarchy)

            if edat.dynamic_lennard:
                ff.add_well_depths(self._modeller_hierarchy)
                sf = IMP.atom.ForceSwitch(edat.lennard_jones_switch[0],
                                          edat.lennard_jones_switch[1])
                ps = IMP.atom.LennardJonesPairScore(sf)
                yield IMP.container.PairsRestraint(ps, nbl)

            if edat.dynamic_coulomb:
                sf = IMP.atom.ForceSwitch(edat.coulomb_switch[0],
                                          edat.coulomb_switch[1])
                ps = IMP.atom.CoulombPairScore(sf)
                ps.set_relative_dielectric(edat.relative_dielectric)
                yield IMP.container.PairsRestraint(ps, nbl)

%}
