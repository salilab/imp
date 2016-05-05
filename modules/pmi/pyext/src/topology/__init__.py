"""@namespace IMP.pmi.topology
Set of python classes to create a multi-state, multi-resolution IMP hierarchy.
* Start by creating a System with `mdl = IMP.Model(); s = IMP.pmi.topology.System(mdl)`. The System will store all the states.
* Then call System.create_state(). You can easily create a multistate system by calling this function multiples times.
* For each State, call State.create_molecule() to add a Molecule (a uniquely named polymer). This function returns the Molecule object which can be passed to various PMI functions.
* Some useful functions to help you set up your Molecules:
 * Access the sequence residues with slicing (Molecule[a:b]) or functions like Molecule.get_atomic_residues() and Molecule.get_non_atomic_residues(). These functions all return python sets for easy set arithmetic using & (and), | (or), - (difference)
 * Molecule.add_structure() to add structural information from a PDB file.
 * Molecule.add_representation() to create a representation unit - here you can choose bead resolutions as well as alternate representations like densities or ideal helices.
 * Molecule.create_clone() lets you set up a molecule with identical representations, just a different chain ID. Use Molecule.create_copy() if you want a molecule with the same sequence but that allows custom representations.
* Once data has been added and representations chosen, call System.build() to create a canonical IMP hierarchy.
* Following hierarchy construction, setup rigid bodies, flexible beads, etc in IMP::pmi::dof.
* Check your representation with a nice printout: IMP::atom::show_with_representation()
See a [comprehensive example](https://integrativemodeling.org/nightly/doc/ref/pmi_2multiscale_8py-example.html) for using these classes.

Alternatively one can construct the entire topology and degrees of freedom via formatted text file with TopologyReader and IMP::pmi::macros::BuildSystem(). This is used in the [PMI tutorial](@ref rnapolii_stalk).
Note that this only allows a limited set of the full options available to PMI users (rigid bodies only, fixed resolutions).
"""

from __future__ import print_function
import IMP
import IMP.atom
import IMP.algebra
import IMP.pmi
import IMP.pmi.tools
import csv
import os
from collections import defaultdict
from . import system_tools
from bisect import bisect_left
from math import pi,cos,sin
from operator import itemgetter

def _build_ideal_helix(mdl, residues, coord_finder):
    """Creates an ideal helix from the specified residue range
    Residues MUST be contiguous.
    This function actually adds them to the TempResidue hierarchy
    """
    created_hiers = []

    # this function creates a CAlpha helix structure (which can be used for coarsening)
    for n, tempres in enumerate(residues):
        if tempres.get_has_structure():
            raise Exception("You tried to build ideal_helix for a residue "
                            "that already has structure:",tempres)
        if n>0 and (not tempres.get_index()==prev_idx+1):
            raise Exception("Passed non-contiguous segment to build_ideal_helix for",tempres.get_molecule())

        # New residue particle will replace the TempResidue's existing (empty) hierarchy
        rp = IMP.Particle(mdl)
        rp.set_name("Residue_%i" % tempres.get_index())

        # Copy the original residue type and index
        this_res = IMP.atom.Residue.setup_particle(rp,tempres.get_hierarchy())

        # Create the CAlpha
        ap = IMP.Particle(mdl)
        d = IMP.core.XYZR.setup_particle(ap)
        x = 2.3 * cos(n * 2 * pi / 3.6)
        y = 2.3 * sin(n * 2 * pi / 3.6)
        z = 6.2 / 3.6 / 2 * n * 2 * pi / 3.6
        d.set_coordinates(IMP.algebra.Vector3D(x, y, z))
        d.set_radius(1.0)
        a = IMP.atom.Atom.setup_particle(ap, IMP.atom.AT_CA)  # Decorating as Atom also adds Mass
        this_res.add_child(a)

        # Add this structure to the TempResidue
        tempres.set_structure(this_res)
        created_hiers.append(this_res)
        prev_idx = tempres.get_index()
    coord_finder.add_residues(created_hiers) #the coord finder is for placing beads (later)

class _SystemBase(object):
    """The base class for System, State and Molecule
    classes. It contains shared functions in common to these classes
    """

    def __init__(self,mdl=None):
        if mdl is None:
            self.mdl=IMP.Model()
        else:
            self.mdl=mdl

    def _create_hierarchy(self):
        """create a new hierarchy"""
        tmp_part=IMP.Particle(self.mdl)
        return IMP.atom.Hierarchy.setup_particle(tmp_part)

    def _create_child(self,parent_hierarchy):
        """create a new hierarchy, set it as child of the input
        one, and return it"""
        child_hierarchy=self._create_hierarchy()
        parent_hierarchy.add_child(child_hierarchy)
        return child_hierarchy

    def build(self):
        """Build the coordinates of the system.
        Loop through stored(?) hierarchies and set up coordinates!"""
        pass

#------------------------

class System(_SystemBase):
    """This class initializes the root node of the global IMP.atom.Hierarchy."""
    def __init__(self,mdl=None,name="System"):
        _SystemBase.__init__(self,mdl)
        self._number_of_states = 0
        self.states = []
        self.built=False

        # the root hierarchy node
        self.hier=self._create_hierarchy()
        self.hier.set_name(name)

    def get_states(self):
        return self.states

    def create_state(self):
        """returns a new IMP.pmi.representation_new.State(), increment the state index"""
        self._number_of_states+=1
        state = State(self,self._number_of_states-1)
        self.states.append(state)
        return state

    def __repr__(self):
        return self.hier.get_name()

    def get_number_of_states(self):
        """returns the total number of states generated"""
        return self._number_of_states

    def get_hierarchy(self):
        return self.hier

    def build(self,**kwargs):
        """call build on all states"""
        if not self.built:
            for state in self.states:
                state.build(**kwargs)
            self.built=True
        return self.hier

#------------------------

class State(_SystemBase):
    """Stores a list of Molecules all with the same State index.
    Also stores number of copies of each Molecule for easy Selection.
    """
    def __init__(self,system,state_index):
        """Define a new state
        @param system        the PMI System
        @param state_index   the index of the new state
        \note It's expected that you will not use this constructor directly,
        but rather create it with pmi::System::create_molecule()
        """
        self.mdl = system.get_hierarchy().get_model()
        self.system = system
        self.hier = self._create_child(system.get_hierarchy())
        self.hier.set_name("State_"+str(state_index))
        self.molecules = defaultdict(list) # key is molecule name. value are the molecule copies!
        IMP.atom.State.setup_particle(self.hier,state_index)
        self.built = False

    def __repr__(self):
        return self.system.__repr__()+'.'+self.hier.get_name()

    def get_molecules(self):
        """Return a dictionary where key is molecule name and value
        are the list of all copies of that molecule in setup order"""
        return self.molecules

    def get_molecule(self,name,copy_num=0):
        """Access a molecule by name and copy number
        @param name The molecule name used during setup
        @param copy_num The copy number based on input order.
        Default: 0. Set to 'all' to get all copies
        """
        if name not in self.molecules:
            raise Exception("get_molecule() could not find molname",name)
        if copy_num=='all':
            return self.molecules[name]
        else:
            if copy_num>len(self.molecules[name])-1:
                raise Exception("get_molecule() copy number is too high:",copy_num)
            return self.molecules[name][copy_num]

    def create_molecule(self,name,sequence='',chain_id=''):
        """Create a new Molecule within this State
        @param name                the name of the molecule (string) it must not
                                   be already used
        @param sequence            sequence (string)
        @param chain_id            Chain id to assign to this molecule
        """
        # check whether the molecule name is already assigned
        if name in self.molecules:
            raise Exception('Cannot use a molecule name already used')

        mol = Molecule(self,name,sequence,chain_id,copy_num=0)
        self.molecules[name].append(mol)
        return mol

    def get_hierarchy(self):
        return self.hier

    def get_number_of_copies(self,molname):
        return len(self.molecules[molname])

    def _register_copy(self,molecule):
        molname = molecule.get_hierarchy().get_name()
        if molname not in self.molecules:
            raise Exception("Trying to add a copy when the original doesn't exist!")
        self.molecules[molname].append(molecule)

    def build(self,**kwargs):
        """call build on all molecules (automatically makes clones)"""
        if not self.built:
            for molname in self.molecules:
                for mol in reversed(self.molecules[molname]):
                    mol.build(**kwargs)
            self.built=True
        return self.hier

#------------------------

class Molecule(_SystemBase):
    """Stores a named protein chain.
    This class is constructed from within the State class.
    It wraps an IMP.atom.Molecule and IMP.atom.Copy
    Structure is read using this class
    Resolutions and copies can be registered, but are only created when build() is called
    """

    def __init__(self,state,name,sequence,chain_id,copy_num,mol_to_clone=None):
        """The user should not call this directly; instead call State::create_molecule()
        @param state           The parent PMI State
        @param name            The name of the molecule (string)
        @param sequence        Sequence (string)
        @param chain_id        The chain of this molecule
        @param copy_num        Store the copy number
        @param mol_to_clone    The original molecule (for cloning ONLY)
        \note It's expected that you will not use this constructor directly,
        but rather create a Molecule with pmi::State::create_molecule()
        """
        # internal data storage
        self.mdl = state.get_hierarchy().get_model()
        self.state = state
        self.sequence = sequence
        self.built = False
        self.mol_to_clone = mol_to_clone
        self.representations = []  # list of stuff to build
        self._represented = IMP.pmi.tools.OrderedSet()   # residues with representation
        self.coord_finder = _FindCloseStructure() # helps you place beads by storing structure
        self._ideal_helices = [] # list of OrderedSets of tempresidues set to ideal helix

        # create root node and set it as child to passed parent hierarchy
        self.hier = self._create_child(self.state.get_hierarchy())
        self.hier.set_name(name)
        IMP.atom.Copy.setup_particle(self.hier,copy_num)
        IMP.atom.Chain.setup_particle(self.hier,chain_id)
        # create TempResidues from the sequence (if passed)
        self.residues=[]
        for ns,s in enumerate(sequence):
            r = TempResidue(self,s,ns+1,ns)
            self.residues.append(r)

    def __repr__(self):
        return self.state.__repr__()+'.'+self.get_name()+'.'+ \
            str(IMP.atom.Copy(self.hier).get_copy_index())

    def __getitem__(self,val):
        if isinstance(val,int):
            return self.residues[val]
        elif isinstance(val,str):
            return self.residues[int(val)-1]
        elif isinstance(val,slice):
            return IMP.pmi.tools.OrderedSet(self.residues[val])
        else:
            print("ERROR: range ends must be int or str. Stride must be int.")

    def get_hierarchy(self):
        """Return the IMP Hierarchy corresponding to this Molecule"""
        return self.hier

    def get_name(self):
        """Return this Molecule name"""
        return self.hier.get_name()

    def get_state(self):
        """Return the State containing this Molecule"""
        return self.state

    def get_ideal_helices(self):
        """Returns list of OrderedSets with requested ideal helices"""
        return self._ideal_helices

    def residue_range(self,a,b,stride=1):
        """get residue range from a to b, inclusive.
        Use integers to get 0-indexing, or strings to get PDB-indexing"""
        if isinstance(a,int) and isinstance(b,int) and isinstance(stride,int):
            return IMP.pmi.tools.OrderedSet(self.residues[a:b+1:stride])
        elif isinstance(a,str) and isinstance(b,str) and isinstance(stride,int):
            return IMP.pmi.tools.OrderedSet(self.residues[int(a)-1:int(b):stride])
        else:
            print("ERROR: range ends must be int or str. Stride must be int.")

    def get_residues(self):
        """ Return all modeled TempResidues as a set"""
        all_res = IMP.pmi.tools.OrderedSet(self.residues)
        return all_res

    def get_represented(self):
        """ Return set of TempResidues that have representation"""
        return self._represented

    def get_atomic_residues(self):
        """ Return a set of TempResidues that have associated structure coordinates """
        atomic_res = IMP.pmi.tools.OrderedSet()
        for res in self.residues:
            if res.get_has_structure():
                atomic_res.add(res)
        return atomic_res

    def get_non_atomic_residues(self):
        """ Return a set of TempResidues that don't have associated structure coordinates """
        non_atomic_res = IMP.pmi.tools.OrderedSet()
        for res in self.residues:
            if not res.get_has_structure():
                non_atomic_res.add(res)
        return non_atomic_res

    def create_copy(self,chain_id):
        """Create a new Molecule with the same name and sequence but a higher copy number.
        Returns the Molecule. No structure or representation will be copied!
        @param chain_id  Chain ID of the new molecule
        """
        mol = Molecule(self.state,self.get_name(),self.sequence,chain_id,
                       copy_num=self.state.get_number_of_copies(self.get_name()))
        self.state._register_copy(mol)
        return mol

    def create_clone(self,chain_id):
        """Create a Molecule clone (automatically builds same structure and representation)
        @param chain_id If you want to set the chain ID of the copy to something
        \note You cannot add structure or representations to a clone!
        """
        mol = Molecule(self.state,self.get_name(),self.sequence,chain_id,
                       copy_num=self.state.get_number_of_copies(self.get_name()),
                       mol_to_clone=self)
        self.state._register_copy(mol)
        return mol

    def add_structure(self,pdb_fn,chain_id,res_range=[],
                      offset=0,model_num=None,ca_only=False,
                      soft_check=False):
        """Read a structure and store the coordinates.
        Returns the atomic residues (as a set)
        @param pdb_fn    The file to read
        @param chain_id  Chain ID to read
        @param res_range Add only a specific set of residues from the PDB file.
        @param offset    Apply an offset to the residue indexes of the PDB file.
                         This number is added to the PDB sequence.
        @param model_num Read multi-model PDB and return that model
        @param ca_only   Only read the CA positions from the PDB file
        @param soft_check If True, it only warns if there are sequence mismatches between the pdb and
               the Molecules sequence. Actually replaces the fasta values.
                          If False (Default), it raises and exit when there are sequence mismatches.
        \note If you are adding structure without a FASTA file, set soft_check to True
        """
        if self.mol_to_clone is not None:
            raise Exception('You cannot call add_structure() for a clone')

        self.pdb_fn = pdb_fn

        # get IMP.atom.Residues from the pdb file
        rhs = system_tools.get_structure(self.mdl,pdb_fn,chain_id,res_range,offset,ca_only=ca_only)
        self.coord_finder.add_residues(rhs)

        if len(self.residues)==0:
            print("WARNING: Extracting sequence from structure. Potentially dangerous.")

        # load those into TempResidue object
        atomic_res = IMP.pmi.tools.OrderedSet() # collect integer indexes of atomic residues to return
        for nrh,rh in enumerate(rhs):
            pdb_idx = rh.get_index()
            raw_idx = pdb_idx - 1

            # add ALA to fill in gaps
            while len(self.residues)<pdb_idx:
                r = TempResidue(self,'A',len(self.residues)+1,len(self.residues))
                self.residues.append(r)

            internal_res = self.residues[raw_idx]
            internal_res.set_structure(rh,soft_check)
            atomic_res.add(internal_res)
        return atomic_res

    def add_representation(self,
                           residues=None,
                           resolutions=[],
                           bead_extra_breaks=[],
                           bead_ca_centers=True,
                           bead_default_coord=[0,0,0],
                           density_residues_per_component=None,
                           density_prefix=None,
                           density_force_compute=False,
                           density_voxel_size=1.0,
                           setup_particles_as_densities=False,
                           ideal_helix=False,
                           color=None):
        """Set the representation for some residues. Some options (beads, ideal helix)
        operate along the backbone. Others (density options) are volumetric.
        Some of these you can combine e.g., beads+densities or helix+densities
        See @ref pmi_resolution
        @param residues Set of PMI TempResidues for adding the representation.
               Can use Molecule slicing to get these, e.g. mol[a:b]+mol[c:d]
               If None, will select all residues for this Molecule.
        @param resolutions Resolutions for beads representations.
               If structured, will average along backbone, breaking at sequence breaks.
               If unstructured, will just create beads.
               Pass an integer or list of integers
        @param bead_extra_breaks Additional breakpoints for splitting beads.
               The value can be the 0-ordered position, after which it'll insert the break.
               Alternatively pass PDB-style (1-ordered) indices as a string.
               I.e., bead_extra_breaks=[5,25] is the same as ['6','26']
        @param bead_ca_centers Set to True if you want the resolution=1 beads to be at CA centers
               (otherwise will average atoms to get center). Defaults to True.
        @param bead_default_coord Advanced feature. Normally beads are placed at the nearest structure.
               If no structure provided (like an all bead molecule), the beads go here.
        @param density_residues_per_component Create density (Gaussian Mixture Model)
               for these residues. Must also supply density_prefix
        @param density_prefix Prefix (assuming '.txt') to read components from or write to.
               If exists, will read unless you set density_force_compute=True.
               Will also write map (prefix+'.mrc').
               Must also supply density_residues_per_component.
        @param density_force_compute Set true to force overwrite density file.
        @param density_voxel_size Advanced feature. Set larger if densities taking too long to rasterize.
               Set to 0 if you don't want to create the MRC file
        @param setup_particles_as_densities Set to True if you want each particle to be its own density.
               Useful for all-atom models or flexible beads.
               Mutually exclusive with density_ options
        @param ideal_helix Create idealized helix structures for these residues at resolution 1.
               Any other resolutions passed will be coarsened from there.
               Resolution 0 will not work, you may have to use MODELLER to do that (for now).
        @param color the color applied to the hierarchies generated.
               Format options: tuple (r,g,b) with values 0 to 1
               or float (from 0 to 1, a map from Blue to Green to Red)
               or IMP.display.Color object

        \note You cannot call add_representation multiple times for the same residues.
        """

        # can't customize clones
        if self.mol_to_clone is not None:
            raise Exception('You cannot call add_representation() for a clone.'
                            'Maybe use a copy instead')

        # format input
        if residues is None:
            res = IMP.pmi.tools.OrderedSet(self.residues)
        elif residues==self:
            res = IMP.pmi.tools.OrderedSet(self.residues)
        elif type(residues) is IMP.pmi.topology.TempResidue:
            res = IMP.pmi.tools.OrderedSet([residues])
        elif hasattr(residues,'__iter__'):
            if len(residues)==0:
                raise Exception('You passed an empty set to add_representation')
            if type(residues) is IMP.pmi.tools.OrderedSet and type(next(iter(residues))) is TempResidue:
                res = residues
            elif type(residues) is set and type(next(iter(residues))) is TempResidue:
                res = IMP.pmi.tools.OrderedSet(residues)
            elif type(residues) is list and type(residues[0]) is TempResidue:
                res = IMP.pmi.tools.OrderedSet(residues)
            else:
                raise Exception("You passed an iteratible of something other than TempResidue",res)
        else:
            raise Exception("add_representation: you must pass a set of residues or nothing(=all residues)")

        # check that each residue has not been represented yet
        ov = res & self._represented
        if ov:
            raise Exception('You have already added representation for '+self.get_hierarchy().get_name()+': '+ov.__repr__())
        self._represented|=res

        # check you aren't creating multiple resolutions without structure
        if not hasattr(resolutions,'__iter__'):
            if type(resolutions) is int:
                resolutions = [resolutions]
            else:
                raise Exception("you tried to pass resolutions that are not int or list-of-int")
        if len(resolutions)>1 and not ideal_helix:
            for r in res:
                if not r.get_has_structure():
                    raise Exception('You are creating multiple resolutions for '
                                    'unstructured regions. This will have unexpected results.')

        # check density info is consistent
        if density_residues_per_component or density_prefix:
            if not density_residues_per_component and density_prefix:
                raise Exception('If requesting density, must provide '
                                'density_residues_per_component AND density_prefix')
        if density_residues_per_component and setup_particles_as_densities:
            raise Exception('Cannot create both volumetric density '
                            '(density_residues_per_component) AND '
                            'individual densities (setup_particles_as_densities) '
                            'in the same representation')
        if len(resolutions)>1 and setup_particles_as_densities:
            raise Exception('You have multiple bead resolutions but are attempting to '
                            'set them all up as individual Densities. '
                            'This could have unexpected results.')

        # check helix not accompanied by other resolutions (densities OK though!)
        if ideal_helix:
            if 0 in resolutions:
                raise Exception("For ideal helices, cannot build resolution 0: "
                                "you have to do that in MODELLER")
            if 1 not in resolutions:
                resolutions = [1] + list(resolutions)
            self._ideal_helices.append(res)

        # check residues are all part of this molecule:
        for r in res:
            if r.get_molecule()!=self:
                raise Exception('You are adding residues from a different molecule to',self.__repr__())

        # unify formatting for extra breaks
        breaks = []
        for b in bead_extra_breaks:
            if type(b)==str:
                breaks.append(int(b)-1)
            else:
                breaks.append(b)
        # store the representation group
        self.representations.append(_Representation(res,
                                                    resolutions,
                                                    breaks,
                                                    bead_ca_centers,
                                                    bead_default_coord,
                                                    density_residues_per_component,
                                                    density_prefix,
                                                    density_force_compute,
                                                    density_voxel_size,
                                                    setup_particles_as_densities,
                                                    ideal_helix,
                                                    color))

    def build(self):
        """Create all parts of the IMP hierarchy
        including Atoms, Residues, and Fragments/Representations and, finally, Copies
        Will only build requested representations.
        /note Any residues assigned a resolution must have an IMP.atom.Residue hierarchy
              containing at least a CAlpha. For missing residues, these can be constructed
              from the PDB file
        """
        if not self.built:
            # if requested, clone structure and representations BEFORE building original
            if self.mol_to_clone is not None:
                for nr,r in enumerate(self.mol_to_clone.residues):
                    if r.get_has_structure():
                        clone = IMP.atom.create_clone(r.get_hierarchy())
                        self.residues[nr].set_structure(
                            IMP.atom.Residue(clone),soft_check=True)
                for old_rep in self.mol_to_clone.representations:
                    new_res = IMP.pmi.tools.OrderedSet()
                    for r in old_rep.residues:
                        new_res.add(self.residues[r.get_internal_index()])
                        self._represented.add(self.residues[r.get_internal_index()])
                    new_rep = _Representation(new_res,
                                              old_rep.bead_resolutions,
                                              old_rep.bead_extra_breaks,
                                              old_rep.bead_ca_centers,
                                              old_rep.bead_default_coord,
                                              old_rep.density_residues_per_component,
                                              old_rep.density_prefix,
                                              False,
                                              old_rep.density_voxel_size,
                                              old_rep.setup_particles_as_densities,
                                              old_rep.ideal_helix,
                                              old_rep.color)
                    self.representations.append(new_rep)
                self.coord_finder = self.mol_to_clone.coord_finder

            # give a warning for all residues that don't have representation
            no_rep = [r for r in self.residues if r not in self._represented]
            if len(no_rep)>0:
                print('WARNING: Residues without representation in molecule',
                      self.get_name(),':',system_tools.resnums2str(no_rep))

            # first build any ideal helices (fills in structure for the TempResidues)
            for rep in self.representations:
                if rep.ideal_helix:
                    _build_ideal_helix(self.mdl,rep.residues,self.coord_finder)

            # build all the representations
            for rep in self.representations:
                system_tools.build_representation(self.hier,rep,self.coord_finder)
            self.built = True

            for res in self.residues:
                idx = res.get_index()

                # first off, store the highest resolution available in residue.hier
                new_ps = IMP.atom.Selection(
                    self.hier,
                    residue_index=res.get_index(),
                    resolution=1).get_selected_particles()
                if len(new_ps)>0:
                    new_p = new_ps[0]
                    if IMP.atom.Atom.get_is_setup(new_p):
                        # if only found atomic, store the residue
                        new_hier = IMP.atom.get_residue(IMP.atom.Atom(new_p))
                    else:
                        # otherwise just store what you found
                        new_hier = IMP.atom.Hierarchy(new_p)
                    res.hier = new_hier
                else:
                    res.hier = None
            self._represented = IMP.pmi.tools.OrderedSet([a for a in self._represented])
        print('done building',self.get_hierarchy())
        return self.hier

    def get_particles_at_all_resolutions(self,residue_indexes=None):
        """Helpful utility for getting particles at all resolutions from this molecule.
        Can optionally pass a set of residue indexes"""
        if not self.built:
            raise Exception("Cannot get all resolutions until you build the Molecule")
        if residue_indexes is None:
            residue_indexes = [r.get_index() for r in self.get_residues()]
        ps = IMP.pmi.tools.select_at_all_resolutions(self.get_hierarchy(),
                                                      residue_indexes=residue_indexes)
        return ps

#------------------------

class _Representation(object):
    """Private class just to store a representation request"""
    def __init__(self,
                 residues,
                 bead_resolutions,
                 bead_extra_breaks,
                 bead_ca_centers,
                 bead_default_coord,
                 density_residues_per_component,
                 density_prefix,
                 density_force_compute,
                 density_voxel_size,
                 setup_particles_as_densities,
                 ideal_helix,
                 color):
        self.residues = residues
        self.bead_resolutions = bead_resolutions
        self.bead_extra_breaks = bead_extra_breaks
        self.bead_ca_centers = bead_ca_centers
        self.bead_default_coord = bead_default_coord
        self.density_residues_per_component = density_residues_per_component
        self.density_prefix = density_prefix
        self.density_force_compute = density_force_compute
        self.density_voxel_size = density_voxel_size
        self.setup_particles_as_densities = setup_particles_as_densities
        self.ideal_helix = ideal_helix
        self.color = color

class _FindCloseStructure(object):
    """Utility to get the nearest observed coordinate"""
    def __init__(self):
        self.coords=[]
    def add_residues(self,residues):
        for r in residues:
            idx = IMP.atom.Residue(r).get_index()
            ca = IMP.atom.Selection(r,atom_type=IMP.atom.AtomType("CA")).get_selected_particles()[0]
            xyz = IMP.core.XYZ(ca).get_coordinates()
            self.coords.append([idx,xyz])
        self.coords.sort(key=itemgetter(0))
    def find_nearest_coord(self,query):
        if self.coords==[]:
            return None
        keys = [r[0] for r in self.coords]
        pos = bisect_left(keys,query)
        if pos == 0:
            ret = self.coords[0]
        elif pos == len(self.coords):
            ret = self.coords[-1]
        else:
            before = self.coords[pos - 1]
            after = self.coords[pos]
            if after[0] - query < query - before[0]:
                ret = after
            else:
                ret = before
        return ret[1]

class Sequences(object):
    """A dictionary-like wrapper for reading and storing sequence data"""
    def __init__(self,fasta_fn,name_map=None):
        """read a fasta file and extract all the requested sequences
        @param fasta_fn sequence file
        @param name_map dictionary mapping the fasta name to final stored name
        """
        self.sequences = IMP.pmi.tools.OrderedDict()
        self.read_sequences(fasta_fn,name_map)
    def __len__(self):
        return len(self.sequences)
    def __contains__(self,x):
        return x in self.sequences
    def __getitem__(self,key):
        if type(key) is int:
            try:
                allseqs = list(self.sequences.keys())
                return self.sequences[allseqs[key]]
            except:
                raise Exception("You tried to access sequence num",key,"but there's only",len(self.sequences.keys()))
        else:
            return self.sequences[key]
    def __iter__(self):
        return self.sequences.__iter__()
    def __repr__(self):
        ret=''
        for s in self.sequences:
            ret += '%s\t%s\n'%(s,self.sequences[s])
        return ret
    def read_sequences(self,fasta_fn,name_map=None):
        code = None
        seq = None
        with open(fasta_fn,'r') as fh:
            for (num, line) in enumerate(fh):
                if line.startswith('>'):
                    if seq is not None:
                        self.sequences[code] = seq.strip('*')
                    code = line.rstrip()[1:]
                    if name_map is not None:
                        try:
                            code = name_map[code]
                        except:
                            pass
                    seq = ''
                else:
                    line = line.rstrip()
                    if line: # Skip blank lines
                        if seq is None:
                            raise Exception( \
    "Found FASTA sequence before first header at line %d: %s" % (num + 1, line))
                        seq += line
        if seq is not None:
            self.sequences[code] = seq.strip('*')

#------------------------


class TempResidue(object):
    """Temporarily stores residue information, even without structure available."""
    # Consider implementing __hash__ so you can select.
    def __init__(self,molecule,code,index,internal_index):
        """setup a TempResidue
        @param molecule PMI Molecule to which this residue belongs
        @param code     one-letter residue type code
        @param index    PDB index
        @param internal_index The number in the sequence
        """
        #these attributes should be immutable
        self.molecule = molecule
        self.rtype = IMP.pmi.tools.get_residue_type_from_one_letter_code(code)
        self.pdb_index = index
        self.internal_index = internal_index
        self.copy_index = IMP.atom.Copy(self.molecule.hier).get_copy_index()
        self.state_index = IMP.atom.State(self.molecule.state.hier).get_state_index()
        #these are expected to change
        self._structured = False
        self.hier = IMP.atom.Residue.setup_particle(IMP.Particle(molecule.mdl),
                                                    self.rtype,
                                                    index)
    def __str__(self):
        return str(self.state_index)+"_"+self.molecule.get_name()+"_"+str(self.copy_index)+"_"+self.get_code()+str(self.get_index())
    def __repr__(self):
        return self.__str__()
    def __key(self):
        #this returns the immutable attributes only
        return (self.state_index, self.molecule, self.copy_index, self.rtype, self.pdb_index, self.internal_index)
    def __eq__(self,other):
        return type(other)==type(self) and self.__key() == other.__key()
    def __hash__(self):
        return hash(self.__key())
    def get_index(self):
        return self.pdb_index
    def get_internal_index(self):
        return self.internal_index
    def get_code(self):
        return IMP.atom.get_one_letter_code(self.get_residue_type())
    def get_residue_type(self):
        return self.rtype
    def get_hierarchy(self):
        return self.hier
    def get_molecule(self):
        return self.molecule
    def get_has_structure(self):
        return self._structured
    def set_structure(self,res,soft_check=False):
        if res.get_residue_type()!=self.get_residue_type():
            if soft_check:
                print('WARNING: Replacing sequence residue',self.get_index(),self.hier.get_residue_type(),
                      'with PDB type',res.get_residue_type())
                self.hier.set_residue_type((res.get_residue_type()))
                self.rtype = res.get_residue_type()
            else:
                raise Exception('ERROR: PDB residue index',self.get_index(),'is',
                                IMP.atom.get_one_letter_code(res.get_residue_type()),
                                'and sequence residue is',self.get_code())

        for a in res.get_children():
            self.hier.add_child(a)
            atype = IMP.atom.Atom(a).get_atom_type()
            a.get_particle().set_name('Atom %s of residue %i'%(atype.__str__().strip('"'),
                                                               self.hier.get_index()))
        self._structured = True

class TopologyReader(object):
    """Automatically setup Sytem and Degrees of Freedom with a formatted text file.
    The topology file should be in a simple pipe-delimited format:
    @code{.txt}
|component_name|domain_name|fasta_fn|fasta_id|pdb_fn|chain|residue_range|pdb_offset|bead_size|em_residues_per_gaussian|rigid_body|super_rigid_body|chain_of_super_rigid_bodies|
|Rpb1 |Rpb1_1|1WCM.fasta|1WCM:A|1WCM.pdb|A|1,1140   |0|10|0|1|1,3|1|
|Rpb1 |Rpb1_2|1WCM.fasta|1WCM:A|1WCM.pdb|A|1141,1274|0|10|0|2|1,3|1|
|Rpb1 |Rpb1_3|1WCM.fasta|1WCM:A|1WCM.pdb|A|1275,-1  |0|10|0|3|1,3|1|
|Rpb2 |Rpb2  |1WCM.fasta|1WCM:B|1WCM.pdb|B|all      |0|10|0|4|2,3|2|
    @endcode

    All filenames are relative to the paths specified in the constructor.
    These are the fields you can enter:
    - `component_name`: Name of the component (chain). Serves as the parent
      hierarchy for this structure.
    - `domain_name`: Allows subdivision of chains into individual domains.
       A model consists of a number of individual units, referred to as
       domains. Each domain can be an individual chain, or a subset of a
       chain, and these domains are used to set rigid body movers. A chain
       may be separated into multiple domains if the user wishes different
       sections to move independently, and/or analyze the portions separately.
    - `fasta_fn`: Name of FASTA file containing this component.
    - `fasta_id`: String found in FASTA sequence header line.
    - `pdb_fn`: Name of PDB file with coordinates (if available).
       If left empty, will set up as BEADS (you can also specify "BEADS")
       Can also write "IDEAL_HELIX".
    - `chain`: Chain ID of this domain in the PDB file.
    - `residue_range`: Comma delimited pair defining range.
       Can leave empty or use 'all' for entire sequence from PDB file.
    - `pdb_offset`: Offset to sync PDB residue numbering with FASTA numbering.
    - `bead_size`: The size (in residues) of beads used to model areas not
      covered by PDB coordinates. These will be automatically built.
    - `em_residues`: The number of Gaussians used to model the electron
      density of this domain. Set to zero if no EM fitting will be done.
      The GMM files will be written to <gmm_dir>/<component_name>_<em_res>.txt (and .mrc)
    - `rigid_body`: Number corresponding to the rigid body containing this object.
    The number itself is used for grouping things.
    - `super_rigid_body`: Like a rigid_body, except things are only occasionally rigid
    - `chain_of_super_rigid_bodies` For a polymer, create SRBs from groups.

    The file is read in and each part of the topology is stored as a
    ComponentTopology object for input into IMP::pmi::macros::BuildModel.
    """
    def __init__(self,
                 topology_file,
                 resolutions=[1,10],
                 pdb_dir='./',
                 fasta_dir='./',
                 gmm_dir='./'):
        """Constructor.
        @param topology_file Pipe-delimited file specifying the topology
        @param resolutions What resolutions to build for ALL structured components
        @param pdb_dir Relative path to the pdb directory
        @param fasta_dir Relative path to the fasta directory
        @param gmm_dir Relative path to the GMM directory
        """
        self.topology_file = topology_file
        self.component_list = []
        self.unique_molecules = {}
        self.resolutions = resolutions
        self.pdb_dir = pdb_dir
        self.fasta_dir = fasta_dir
        self.gmm_dir = gmm_dir
        self.component_list = self.import_topology_file(topology_file)

    def write_topology_file(self,outfile):
        with open(outfile, "w") as f:
            f.write("|component_name|domain_name|fasta_fn|fasta_id|pdb_fn|chain|residue_range|pdb_offset|bead_size|em_residues_per_gaussian|rigid_body|super_rigid_body|chain_of_super_rigid_bodies|\n")
            for c in self.component_list:
                output = c.get_str()+'\n'
                f.write(output)
        return outfile

    def get_component_topologies(self, topology_list = "all"):
        """ Return list of ComponentTopologies for selected components
        @param topology_list List of indices to return"""
        if topology_list == "all":
            topologies = self.component_list
        else:
            topologies=[]
            for i in topology_list:
                topologies.append(self.component_list[i])
        return topologies

    def get_unique_molecules(self):
        return self.unique_molecules

    def import_topology_file(self, topology_file, append=False):
        """Read system components from topology file. append=False will erase
        current topology and overwrite with new
        """
        is_topology = False
        is_defaults = False
        linenum = 1
        if append==False:
            self.component_list=[]
        with open(topology_file) as infile:
            for line in infile:
                if line.lstrip()=="" or line[0]=="#":
                    continue
                elif line.split('|')[1] in ("topology_dictionary","component_name"):
                    is_topology=True
                    is_defaults=False
                elif line.split('|')[1]=="directories":
                    is_defaults=True
                    print("WARNING: You no longer need to set directories in the topology file. "
                          "Please do so through the TopologyReader constructor. "
                          "Note that new-style paths are relative to the current working directory, "
                          "not the topology file")
                elif is_topology:
                    # create a component_topology from this line
                    new_component = self._create_component_topology(line, linenum)
                    self.component_list.append(new_component)
                elif is_defaults:
                    # THIS WILL GO AWAY, switch to files relative to the modeling file!
                    fields = line.split('|')
                    setattr(self,fields[1],IMP.get_relative_path(self.topology_file,fields[2]))
                else:
                    raise Exception("FOUND A WEIRD LINE")
                linenum += 1
        return self.component_list

    def _create_component_topology(self, component_line, linenum, color="0.1"):
        """Parse a line of topology values and matches them to their key.
        Checks each value for correct syntax
        Returns a list of ComponentTopology objects
        fields:
        """

        c = ComponentTopology()
        values = [s.strip() for s in component_line.split('|')]
        errors = []

        ### Required fields
        c.name          = values[1]
        c.domain_name   = values[2]
        c._orig_fasta_file = values[3] # in case you need to write the file!
        c.fasta_file    = os.path.join(self.fasta_dir,values[3])
        c.fasta_id      = values[4]
        c._orig_pdb_input = values[5]
        pdb_input       = values[5]
        if pdb_input=="None" or pdb_input=="":
            c.pdb_file      = "BEADS"
        elif pdb_input=="IDEAL_HELIX":
            c.pdb_file = "IDEAL_HELIX"
        elif pdb_input=="BEADS":
            c.pdb_file = "BEADS"
        else:
            c.pdb_file = os.path.join(self.pdb_dir,pdb_input)

        # PDB chain must be one or two characters
        t_chain = values[6]
        if len(t_chain)==1 or len(t_chain)==2:
            c.chain = t_chain
        else:
            errors.append("PDB Chain identifier must be one or two characters.")
            errors.append("For component %s line %d is not correct |%s| was given." % (c.name,linenum,t_chain))

        # Multiple domains must all use the same fasta file!
        if c.name not in self.unique_molecules:
            self.unique_molecules[c.name] = [c]
        else:
            if (c.fasta_file!=self.unique_molecules[c.name][0].fasta_file or \
                c.fasta_id!=self.unique_molecules[c.name][0].fasta_id or \
                c.chain!=self.unique_molecules[c.name][0].chain):
                errors.append("All domains with the same component name must have the same sequence. %s doesn't match %s"%(c.domain_name,c.name))
            self.unique_molecules[c.name].append(c)

        ### Optional fields
        # Residue Range
        f = values[7]
        if f.strip()=='all' or str(f)=="":
            c.residue_range = None
        elif len(f.split(','))==2 and self._is_int(f.split(',')[0]) and self._is_int(f.split(',')[1]):
            # Make sure that is residue range is given, there are only two values and they are integers
            c.residue_range = (int(f.split(',')[0]), int(f.split(',')[1]))
        else:
            errors.append("Residue Range format for component %s line %d is not correct" % (c.name, linenum))
            errors.append("Correct syntax is two comma separated integers:  |start_res, end_res|. |%s| was given." % f)
            errors.append("To select all residues, indicate |\"all\"|")

        # PDB Offset
        f = values[8]
        if self._is_int(f):
            c.pdb_offset=int(f)
        elif len(f)==0:
            c.pdb_offset = 0
        else:
            errors.append("PDB Offset format for component %s line %d is not correct" % (c.name, linenum))
            errors.append("The value must be a single integer. |%s| was given." % f)

        # Bead Size
        f = values[9]
        if self._is_int(f):
            c.bead_size=int(f)
        elif len(f)==0:
            c.bead_size = 0
        else:
            errors.append("Bead Size format for component %s line %d is not correct" % (c.name, linenum))
            errors.append("The value must be a single integer. |%s| was given." % f)

        # EM Residues Per Gaussian
        f = values[10]
        if self._is_int(f):
            if int(f) > 0:
                c.density_prefix = os.path.join(self.gmm_dir,c.domain_name)
                c.gmm_file = c.density_prefix+'.txt'
                c.mrc_file = c.density_prefix+'.gmm'

                c.em_residues_per_gaussian=int(f)
            else:
                c.em_residues_per_gaussian = 0
        elif len(f)==0:
            c.em_residues_per_gaussian = 0
        else:
            errors.append("em_residues_per_gaussian format for component "
                          "%s line %d is not correct" % (c.name, linenum))
            errors.append("The value must be a single integer. |%s| was given." % f)

        # DOF fields are for new-style topology files
        if len(values)>12:
            # rigid bodies
            f = values[11]
            if len(f)>0:
                if not self._is_int(f):
                    errors.append("rigid bodies format for component "
                                  "%s line %d is not correct" % (c.name, linenum))
                    errors.append("Each RB must be a single integer. |%s| was given." % f)
                c.rigid_bodies = f

            # super rigid bodies
            f = values[12]
            if len(f)>0:
                f = f.split(',')
                for i in f:
                    if not self._is_int(i):
                        errors.append("super rigid bodies format for component "
                                      "%s line %d is not correct" % (c.name, linenum))
                        errors.append("Each SRB must be a single integer. |%s| was given." % f)
                c.super_rigid_bodies = f

            # chain of super rigid bodies
            f = values[13]
            if len(f)>0:
                if not self._is_int(f):
                    errors.append("em_residues_per_gaussian format for component "
                                  "%s line %d is not correct" % (c.name, linenum))
                    errors.append("Each CSRB must be a single integer. |%s| was given." % f)
                c.chain_of_super_rigid_bodies = f

        # done
        if errors:
            raise ValueError("Fix Topology File syntax errors and rerun: " \
                             + "\n".join(errors))
        else:
            return c


    def set_gmm_dir(self,gmm_dir):
        """Change the GMM dir"""
        self.gmm_dir = gmm_dir
        for c in self.component_list:
            c.gmm_file = os.path.join(self.gmm_dir,c.domain_name+".txt")
            c.mrc_file = os.path.join(self.gmm_dir,c.domain_name+".mrc")
            print('new gmm',c.gmm_file)

    def set_pdb_dir(self,pdb_dir):
        """Change the PDB dir"""
        self.pdb_dir = pdb_dir
        for c in self.component_list:
            if not c._orig_pdb_input in ("","None","IDEAL_HELIX","BEADS"):
                c.pdb_file = os.path.join(self.pdb_dir,c._orig_pdb_input)

    def set_fasta_dir(self,fasta_dir):
        """Change the FASTA dir"""
        self.fasta_dir = fasta_dir
        for c in self.component_list:
            c.fasta_file = os.path.join(self.fasta_dir,c._orig_fasta_file)

    def set_dir(self, default_dir, new_dir):
        """DEPRECATED: This old function sets things relative to topology file"""
        print("WARNING: set_dir() is deprecated, use set_gmm_dir, set_pdb_dir, or set_fasta_dir. "
              "Paths in the TopologyReader constructor or in those functions are relative "
              "to the current working directory, not the topology file.")
        if default_dir=="gmm_dir":
            self.set_gmm_dir(IMP.get_relative_path(self.topology_file,new_dir))
        elif default_dir=="pdb_dir":
            self.set_pdb_dir(IMP.get_relative_path(self.topology_file,new_dir))
        elif default_dir=="fasta_dir":
            self.set_fasta_dir(nIMP.get_relative_path(self.topology_file,new_dir))
        else:
            raise Exception(default_dir, "is not a correct directory key")

    def _is_int(self, s):
       # is this string an integer?
        try:
            float(s)
            return float(s).is_integer()
        except ValueError:
            return False

    def get_rigid_bodies(self):
        """Return list of lists of rigid bodies (as domain name)"""
        rbl = defaultdict(list)
        for c in self.component_list:
            for rbnum in c.rigid_bodies:
                rbl[rbnum].append(c.domain_name)
        return rbl.values()

    def get_super_rigid_bodies(self):
        """Return list of lists of super rigid bodies (as domain name)"""
        rbl = defaultdict(list)
        for c in self.component_list:
            for rbnum in c.super_rigid_bodies:
                rbl[rbnum].append(c.domain_name)
        return rbl.values()

    def get_chains_of_super_rigid_bodies(self):
        """Return list of lists of chains of super rigid bodies (as domain name)"""
        rbl = defaultdict(list)
        for c in self.component_list:
            for rbnum in c.chain_of_super_rigid_bodies:
                rbl[rbnum].append(c.domain_name)
        return rbl.values()

class ComponentTopology(object):
    """Stores the components required to build a standard IMP hierarchy
    using IMP.pmi.BuildModel()
    """
    def __init__(self):
        self.name = None
        self.num_clones = None
        self.domain_name = None
        self.fasta_file = None
        self._orig_fasta_file = None
        self.fasta_id = None
        self.pdb_file = None
        self._orig_pdb_input = None
        self.chain = None
        self.residue_range = None
        self.pdb_offset = 0
        self.bead_size = 10
        self.em_residues_per_gaussian = 0
        self.gmm_file = ''
        self.mrc_file = ''
        self.density_prefix = ''
        self.color = 0.1
        self.rigid_bodies = []
        self.super_rigid_bodies = []
        self.chain_of_super_rigid_bodies = []
    def _l2s(self,l):
        l = str(l).strip('[').strip(']')
        return l
    def __repr__(self):
        return self.get_str()
    def get_str(self):
        res_range = self.residue_range
        if self.residue_range is None:
            res_range = []
        return '|'+'|'.join([self.name,self.domain_name,self._orig_fasta_file,self.fasta_id,
                         self._orig_pdb_input,self.chain,self._l2s(list(res_range)),str(self.pdb_offset),
                         str(self.bead_size),str(self.em_residues_per_gaussian),self._l2s(self.rigid_bodies),
                         self._l2s(self.super_rigid_bodies),self._l2s(self.chain_of_super_rigid_bodies)])+'|'
