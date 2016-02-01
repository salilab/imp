"""@namespace IMP.pmi.topology
Set of python classes to create a multi-state, multi-resolution IMP hierarchy.
* Start by creating a System with `mdl = IMP.Model(); s = IMP.pmi.topology.System(model)`. The system will store all the states.
* Then call System.create_state(). You can easily create a multistate system by calling this function multiples times.
* For each State, call State.create_molecule() to add a Molecule (a uniquely named polymer). This function returns the Molecule object which can be passed to various PMI functions.
* Some useful functions to help you set up your Molecules:
 * Access the sequence residues with slicing (Molecule[a:b]) or functions like Molecule.get_atomic_residues() and Molecule.get_non_atomic_residues(). These functions all return python sets for easy set arithmetic using & (and), | (or), - (difference)
 * Molecule.add_structure() to add structural information from a PDB file.
 * Molecule.add_representation() to create a representation unit - here you can choose bead resolutions as well as alternate representations like densities or ideal helices.
 * Molecule.create_clone() lets you set up a molecule with identical representations, just a different chain ID. Use Molecule.create_copy() if you want a molecule with the same sequence but that allows custom representations.
* Once data has been added and representations chosen, call System.build() to create a canonical IMP hierarchy.
* Following hierarchy construction, setup rigid bodies, flexible beads, etc in IMP::pmi::dof.

See a [comprehensive example](https://integrativemodeling.org/nightly/doc/ref/pmi_2multiscale_8py-example.html) for using these classes.

Alternatively one can construct the entire topology and degrees of freedom via formatted text file with TopologyReader and IMP::pmi::macros::BuildModel(). This is used in the [PMI tutorial](@ref rnapolii_stalk).
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
from Bio import SeqIO
from bisect import bisect_left

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
        self.built=False

    def __repr__(self):
        return self.system.__repr__()+'.'+self.hier.get_name()

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
        """The user should not call this direclty, instead call State::create_molecule()
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
        self.represented = set()   # residues with representation
        self.coord_finder = _FindCloseStructure() # helps you place beads by storing structure

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
            return set(self.residues[val])
        else:
            print("ERROR: range ends must be int or str. Stride must be int.")

    def get_hierarchy(self):
        return self.hier

    def get_name(self):
        return self.hier.get_name()

    def get_state(self):
        return self.state

    def residue_range(self,a,b,stride=1):
        """get residue range. Use integers to get 0-indexing, or strings to get PDB-indexing"""
        if isinstance(a,int) and isinstance(b,int) and isinstance(stride,int):
            return set(self.residues[a:b:stride])
        elif isinstance(a,str) and isinstance(b,str) and isinstance(stride,int):
            return set(self.residues[int(a)-1:int(b)-1:stride])
        else:
            print("ERROR: range ends must be int or str. Stride must be int.")

    def get_residues(self):
        """ Return all modeled TempResidues as a set"""
        all_res=set()
        for res in self.residues:
            all_res.add(res)
        return all_res


    def get_atomic_residues(self):
        """ Return a set of TempResidues that have associated structure coordinates """
        atomic_res=set()
        for res in self.residues:
            if res.get_has_structure():
                atomic_res.add(res)
        return atomic_res


    def get_non_atomic_residues(self):
        """ Return a set of TempResidues that don't have associated structure coordinates """
        non_atomic_res=set()
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
        @param res_range Add only a specific set of residues
        @param offset    Apply an offset to the residue indexes of the PDB file
        @param model_num Read multi-model PDB and return that model
        @param ca_only   Only read the CA positions from the PDB file
        @param soft_check If True, it only warns if there are sequence mismatches between the pdb and the Molecules sequence. Actually replaces the fasta values.
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
        atomic_res = set() # collect integer indexes of atomic residues to return
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
                           ideal_helix=False):
        """Set the representation for some residues. Some options (beads, ideal helix)
        operate along the backbone. Others (density options) are volumetric.
        Some of these you can combine e.g., beads+densities or helix+densities
        See @ref pmi_resolution
        @param residues Set of PMI TempResidues for adding the representation.
               Can use Molecule slicing to get these, e.g. mol[a:b]+mol[c:d]
               If None, will select all residues for this Molecule.
        @param resolutions Resolutions for beads representations.
               If structured, will average along backbone, breaking at sequence breaks.
               If unstructured, will just create beads
        @param bead_extra_breaks Additional breakpoints for splitting beads.
               The number is the first PDB-style index that belongs in the second bead
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
        @param setup_particles_as_densities Set to True if you want each particle to be its own density.
               Useful for all-atom models or flexible beads.
               Mutually exclusive with density_ options
        @param ideal_helix Create idealized helix structures for these residues.
               NOT CURRENLTY IMPLEMENTED
        \note You cannot call add_representation multiple times for the same residues.
        """

        # can't customize clones
        if self.mol_to_clone is not None:
            raise Exception('You cannot call add_representation() for a clone.'
                            'Maybe use a copy instead')

        # format input
        if residues is None:
            res = set(self.residues)
        elif residues==self:
            res = set(self.residues)
        elif hasattr(residues,'__iter__'):
            if len(residues)==0:
                raise Exception('You passed an empty set to add_representation')
            if type(residues) is set and type(next(iter(residues))) is TempResidue:
                res = residues
            elif type(residues) is list and type(residues[0]) is TempResidue:
                res = set(residues)
        else:
            raise Exception("add_representation: you must pass a set of residues or nothing(=all residues)")

        # check that each residue has not been represented yet
        ov = res&self.represented
        if ov:
            raise Exception('You have already added representation for '+ov.__repr__())
        self.represented|=res

        # check you aren't creating multiple resolutions without structure
        if len(resolutions)>1:
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

        # store the representation group
        self.representations.append(_Representation(res,
                                                    resolutions,
                                                    bead_extra_breaks,
                                                    bead_ca_centers,
                                                    bead_default_coord,
                                                    density_residues_per_component,
                                                    density_prefix,
                                                    density_force_compute,
                                                    density_voxel_size,
                                                    setup_particles_as_densities))

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
                    new_res = set()
                    for r in old_rep.residues:
                        new_res.add(self.residues[r.get_internal_index()])
                        self.represented.add(self.residues[r.get_internal_index()])
                    new_rep = _Representation(new_res,
                                              old_rep.bead_resolutions,
                                              old_rep.bead_extra_breaks,
                                              old_rep.bead_ca_centers,
                                              old_rep.bead_default_coord,
                                              old_rep.density_residues_per_component,
                                              old_rep.density_prefix,
                                              old_rep.density_voxel_size,
                                              False,
                                              old_rep.setup_particles_as_densities)
                    self.representations.append(new_rep)
                self.coord_finder = self.mol_to_clone.coord_finder

            # give a warning for all residues that don't have representation
            no_rep = [r for r in self.residues if r not in self.represented]
            if len(no_rep)>0:
                print('WARNING: Residues without representation: ',system_tools.resnums2str(no_rep))

            # build all the representations
            # get the first available struture position
            # pass the nearest structure position as you go.

            for rep in self.representations:
                hiers = system_tools.build_representation(self.mdl,rep,self.coord_finder)
                for h in hiers:
                    self.hier.add_child(h)
            self.built=True

            # have to store SOMETHING in each residue slot.
            #  so just select highest resolution.
            #  alternative is store all resolutions?
            for res in self.residues:
                idx = res.get_index()
                new_ps = IMP.atom.Selection(
                    self.hier,
                    residue_index=res.get_index(),
                    resolution=1).get_selected_particles()
                if len(new_ps)>0:
                    new_p = new_ps[0]
                    if IMP.atom.Atom.get_is_setup(new_p):
                        new_hier = IMP.atom.get_residue(IMP.atom.Atom(new_p))
                    else:
                        new_hier = IMP.atom.Hierarchy(new_p)
                    res.hier = new_hier
                else:
                    res.hier = None
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
                 setup_particles_as_densities):
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
        self.coords.sort()
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
        @param name_map dictionary mapping the fasta name to the stored name
        """
        self.sequences={}
        self.read_sequences(fasta_fn,name_map)
    def __len__(self):
        return len(self.sequences)
    def __contains__(self,x):
        return x in self.sequences
    def __getitem__(self,key):
        return self.sequences[key]
    def __repr__(self):
        ret=''
        for s in self.sequences:
            ret+='%s\t%s\n'%(s,self.sequences[s])
        return ret
    def read_sequences(self,fasta_fn,name_map=None):
        # read all sequences
        with open(fasta_fn) as handle:
            record_dict = SeqIO.to_dict(SeqIO.parse(handle, "fasta"))
        if name_map is None:
            for pn in record_dict:
                self.sequences[pn]=str(record_dict[pn].seq).replace("*", "")
        else:
            for pn in name_map:
                try:
                    self.sequences[name_map[pn]]=str(record_dict[pn].seq).replace("*", "")
                except:
                    print("tried to add sequence but: id %s not found in fasta file" % pn)
                    exit()

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
        self.molecule = molecule
        self.rtype = IMP.pmi.tools.get_residue_type_from_one_letter_code(code)
        self.hier = IMP.atom.Residue.setup_particle(IMP.Particle(molecule.mdl),
                                                    self.rtype,
                                                    index)
        self.pdb_index = index
        self.internal_index = internal_index
        self._structured = False
    def __str__(self):
        return self.get_code()+str(self.get_index())
    def __repr__(self):
        return self.__str__()
    def __key(self):
        return (self.molecule,self.hier)
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
            atype=IMP.atom.Atom(a).get_atom_type()
            a.get_particle().set_name('Atom %s of residue %i'%(atype.__str__().strip('"'),
                                                               self.hier.get_index()))
        self._structured = True
class TopologyReader(object):
    """Automatically setup Sytem and Degrees of Freedom with a formatted text file.
    The topology file should be in a simple pipe-delimited format, such as
    @code{.txt}
|directories|
|pdb_dir|./|
|fasta_dir|./|
|gmm_dir|./|

|topology_dictionary|
|component_name|domain_name|fasta_fn|fasta_id|pdb_fn|chain|residue_range|pdb_offset|bead_size|em_residues_per_gaussian|rmf_file|rmf_frame_number|rigid_body|super_rigid_body|chain_of_super_rigid_bodies|
|Rpb1 |Rpb1_1|1WCM.fasta|1WCM:A|1WCM.pdb|A|1,1140   |0|10|0 |None   | None|1|1,3|1|
|Rpb1 |Rpb1_2|1WCM.fasta|1WCM:A|1WCM.pdb|A|1141,1274|0|10|0 |0.rmf3 | 0   |2|1,3|1|
|Rpb1 |Rpb1_3|1WCM.fasta|1WCM:A|1WCM.pdb|A|1275,-1  |0|10|0 |None   | None|3|1,3|1|
|Rpb2 |Rpb2  |1WCM.fasta|1WCM:B|1WCM.pdb|B|all      |0|10|0 |None   | None|4|2,3|2|
    @endcode

    The `|directories|` section lists paths (relative to the topology file)
    where various inputs can be found.

    The columns under `|topology_dictionary|`:
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
    - `chain`: Chain ID of this domain in the PDB file.
    - `residue_range`: Comma delimited pair defining range. -1 = last residue.
      all = [1,-1]
    - `pdb_offset`: Offset to sync PDB residue numbering with FASTA numbering.
    - `bead_size`: The size (in residues) of beads used to model areas not
      covered by PDB coordinates. These will be automatically built.
    - `em_residues`: The number of Gaussians used to model the electron
      density of this domain. Set to zero if no EM fitting will be done.
      The GMM files will be written to <gmm_dir>/<component_name>_<em_res>.txt (and .mrc)
    - `rmf_file`: File path of rmf file with coordinates (if available).
    - `rmf_frame_number`: Frame number to extract
    - `rigid_body`: Number corresponding to the rigid body containing this object.
    The number itself is used for grouping things.
    - `super_rigid_body`: Like a rigid_body, except things are only occasionally rigid
    - `chain_of_super_rigid_bodies` For a polymer, create SRBs from groups.

    The file is read in and each part of the topology is stored as a
    ComponentTopology object for input into IMP::pmi::macros::BuildModel.
    """
    def __init__(self, topology_file):
        self.topology_file=topology_file
        self.component_list=[]
        self.defaults={'bead_size'                : 10,
                       'residue_range'            : 'all',
                       'pdb_offset'               : 0,
                       'em_residues_per_gaussian' : 0,
                       'rmf_file'                 : None,
                       'rmf_frame_number'          : None};
        self.component_list=self.import_topology_file(topology_file)


    def write_topology_file(self,outfile):
        f=open(outfile, "w")
        f.write("|directories|\n")
        for key, value in self.defaults.items():
            output="|"+str(key)+"|"+str(value)+"|\n"
            f.write(output)
        f.write("\n\n")
        f.write("|topology_dictionary|\n")
        f.write("|component_name|domain_name|fasta_fn|fasta_id|pdb_fn|chain|residue_range|pdb_offset|bead_size|em_residues_per_gaussian|rmf_file|rmf_frame_number|\n")
        for c in self.component_list:
            output="|"+str(c.name)+"|"+str(c.domain_name)+"|"+str(c.fasta_file)+"|"+str(c.fasta_id)+"|"+str(c.pdb_file)+"|"+str(c.chain)+"|"+str(c.residue_range).strip("(").strip(")")+"|"+str(c.pdb_offset)+"|"+str(c.bead_size)+"|"+str(c.em_residues_per_gaussian)+"|"+str(c.rmf_file)+"|"+str(c.rmf_frame_number)+"|\n"
            f.write(output)
        return outfile

    def get_component_topologies(self, topology_list = "all"):
        """ Return list of ComponentTopologies for selected components given a list of indices"""
        if topology_list == "all":
            topologies = self.component_list
        else:
            topologies=[]
            for i in topology_list:
                topologies.append(self.component_list[i])
        return topologies

    def set_dir(self, default_dir, new_dir):
        """ Changes the default directories and renames the files for each ComponentTopology object """
        if default_dir in self.defaults.keys():
            self.defaults[default_dir]=new_dir
        else:
            print(default_dir, "is not a correct directory key")
            exit()
        for c in self.component_list:
            pdb_file=c.pdb_file.split("/")[-1]
            c.pdb_file=self._make_path(self.defaults['pdb_dir'],
                                        pdb_file)
            fasta_file=c.fasta_file.split("/")[-1]
            c.fasta_file=self._make_path(self.defaults['fasta_dir'],
                                        fasta_file)
            if c.gmm_file is not None:
                gmm_file=c.gmm_file.split("/")[-1]
                c.gmm_file=self._make_path(self.defaults['gmm_dir'],
                                        gmm_file)
                mrc_file=c.mrc_file.split("/")[-1]
                c.mrc_file=self._make_path(self.defaults['gmm_dir'],
                                        mrc_file)


    def import_topology_file(self, topology_file, append=False):
        """ Import system components from topology file. append=False will erase current topology and overwrite with new """
        is_defaults=False
        is_topology=False
        defaults_dict={}
        linenum=1

        if append==False:
            self.component_list=[]

        with open(topology_file) as infile:
            for line in infile:

                if line.lstrip()=="" or line[0]=="#":
                    continue

                elif line.split('|')[1]=="topology_dictionary":
                    is_topology=True

                elif is_topology==True and is_defaults==True:
                # Store the field names for this topology grid
                    topology_fields=line
                    is_defaults=False

                elif is_topology==True:
                # create a component_topology from this line
                    new_component=self.create_component_topology(line, topology_fields, self.defaults, linenum)
                    self.component_list.append(new_component)

                elif is_defaults==True:
                # grab value for default and put into class attribute
                    self.add_default_parameter(line, linenum)

                elif line.split('|')[1]=="directories":
                    is_defaults=True

                #print line, is_defaults, is_topology
                linenum=linenum+1
                #print self.defaults
        return self.component_list

    def _make_path(self, dirname, fname):
        "Get the full path to a file, possibly relative to the topology file"
        dirname = IMP.get_relative_path(self.topology_file, dirname)
        return os.path.join(dirname, fname)

    def create_component_topology(self, component_line, topology_fields, defaults, linenum, color="0.1"):

    #Reads a grid of topology values and matches them to their key.
    #Checks each value for correct syntax
    #Returns a list of ComponentTopology objects

        fields=topology_fields.split('|')
        values=component_line.split('|')
        c=ComponentTopology()
        errors=[]
    ##### Required fields
        c.name          = values[fields.index("component_name")].strip()
        c.domain_name   = values[fields.index("domain_name")].strip()
        c.fasta_file    = self._make_path(defaults['fasta_dir'],
                                          values[fields.index("fasta_fn")])
        c.fasta_id      = values[fields.index("fasta_id")].strip()

        pdb_input=values[fields.index("pdb_fn")]
        if pdb_input=="None" or pdb_input=="":
            c.pdb_file      = None
        elif pdb_input=="IDEAL_HELIX":
            c.pdb_file="IDEAL_HELIX"
        elif pdb_input=="BEADS":
            c.pdb_file="BEADS"
        else:
            c.pdb_file      = self._make_path(defaults['pdb_dir'],
                                          values[fields.index("pdb_fn")])
        # Need to find a way to define color
        c.color         = 0.1

        t_chain = values[fields.index("chain")].strip()
        # PDB Chain
        # Must be one or two characters
        if len(t_chain)==1 or len(t_chain)==2:
            c.chain = t_chain
        else:
            errors.append("PDB Chain identifier must be one or two characters.")
            errors.append("For component %s line %d is not correct |%s| was given." % (c.name,linenum,t_chain))

    ##### Optional fields
        # Residue Range
        if "residue_range" in fields:
            f=values[fields.index("residue_range")].strip()
            if f.strip()=='all' or str(f)=="":
                c.residue_range=(1,-1)
            # Make sure that is residue range is given, there are only two values and they are integers
            elif len(f.split(','))==2 and self.is_int(f.split(',')[0]) and self.is_int(f.split(',')[1]):
                c.residue_range=(int(f.split(',')[0]), int(f.split(',')[1]))
            else:
                errors.append("Residue Range format for component %s line %d is not correct" % (c.name, linenum))
                errors.append("Correct syntax is two comma separated integers:  |start_res, end_res|. |%s| was given." % f)
                errors.append("To select all residues, indicate |\"all\"|")
        else:
            c.residue_range=defaults["residue_range"]


        # PDB Offset
        if "pdb_offset" in fields:
            f=values[fields.index("pdb_offset")].strip()
            if self.is_int(f):
                c.pdb_offset=int(f)
            else:
                errors.append("PDB Offset format for component %s line %d is not correct" % (c.name, linenum))
                errors.append("The value must be a single integer. |%s| was given." % f)
        else:
            c.pdb_offset=defaults["pdb_offset"]

        # Bead Size
        if "bead_size" in fields:
            f=values[fields.index("bead_size")].strip()
            if self.is_int(f):
                c.bead_size=int(f)
            else:
                errors.append("Bead Size format for component %s line %d is not correct" % (c.name, linenum))
                errors.append("The value must be a single integer. |%s| was given." % f)
        else:
            c.bead_size=defaults["bead_size"]

        # EM Residues Per Gaussian
        if "em_residues_per_gaussian" in fields:
            f=values[fields.index("em_residues_per_gaussian")].strip()
            if self.is_int(f):
                if int(f) > 0:
                    c.gmm_file=self._make_path(defaults['gmm_dir'],
                                           c.domain_name.strip() + ".txt")
                    c.mrc_file=self._make_path(defaults['gmm_dir'],
                                           c.domain_name.strip() + ".mrc")
                c.em_residues_per_gaussian=int(f)
            else:
                errors.append("em_residues_per_gaussian format for component %s line %d is not correct" % (c.name, linenum))
                errors.append("The value must be a single integer. |%s| was given." % f)
        else:
            c.em_residues_per_gaussian=defaults["em_residues_per_gaussian"]

        if "rmf_file" in fields:
            f=values[fields.index("rmf_file")].strip()
            if f == "None":
                c.rmf_file=f
            else:
                if not os.path.isfile(f):
                    errors.append("rmf_file %s must be an existing file or None" % c.name)
                else:
                    c.rmf_file=f
        else:
            c.rmf_file=defaults["rmf_file"]

        if "rmf_frame_number" in fields:
            f=values[fields.index("rmf_frame_number")].strip()
            if f == "None":
                c.rmf_frame_number=f
            else:
                if not self.is_int(f):
                    errors.append("rmf_frame_number %s must be an integer or None" % c.name)
                else:
                    c.rmf_file=f
        else:
            c.rmf_frame_number=defaults["rmf_frame_number"]

        if errors:
            raise ValueError("Fix Topology File syntax errors and rerun: " \
                             + "\n".join(errors))
        else:
            return c

    def is_int(self, s):
       # is this string an integer?
        try:
            float(s)
            return float(s).is_integer()
        except ValueError:
            return False


    def add_default_parameter(self,line, linenum):
    #Separates a line into a key:value pair.

        f=line.split('|')
        if len(f) != 4:
            print("Default value syntax not correct for ", line)
            print("Line number", linenum," contains ", len(f)-2, " fields.")
            print("Please reformat to |KEY|VALUE|")
        self.defaults[f[1]]=f[2]

class ComponentTopology(object):
    '''
    Topology class stores the components required to build a standard IMP hierarchy
    using IMP.pmi.autobuild_model()
    '''
    def __init__(self):
        self.name=None
        self.domain_name=None
        self.fasta_file=None
        self.fasta_id=None
        self.pdb_file=None
        self.chain=None
        self.residue_range=None
        self.pdb_offset=None
        self.bead_size=None
        self.em_residues_per_gaussian=None
        self.gmm_file=None
        self.mrc_file=None
        self.color=None
        self.rmf_file_name=None
        self.rmf_frame_number=None
        self.rigid_bodies=None
        self.super_rigid_bodies=None
        self.chain_of_super_rigid_bodies=None
    def recompute_default_dirs(self, topology):
        pdb_filename=self.pdb_file.split("/")[-1]
        self.pdb_filename=IMP.get_relative_path(topology.topology_file, topology.defaults)
