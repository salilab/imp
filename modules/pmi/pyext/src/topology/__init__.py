"""@namespace IMP.pmi.topology
   Set up of system representation from topology files.

   * Class for storing topology elements of PMI components
   * Functions for reading these elementsfrom a formatted PMI topology file
   * Functions for converting an existing IMP hierarchy into PMI topology
   * TopologyWriter for writing PMI topology files
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


def get_particles_within_zone(hier,
                              target_ps,
                              sel_zone,
                              entire_residues,
                              exclude_backbone):
    """Utility to retrieve particles from a hierarchy within a
    zone around a set of ps.
    @param hier The hierarchy in which to look for neighbors
    @param target_ps The particles for zoning
    @param sel_zone The maximum distance
    @param entire_residues If True, will grab entire residues
    @param exclude_backbone If True, will only return sidechain particles
    """

    test_sel = IMP.atom.Selection(hier)
    backbone_types=['C','N','CB','O']
    if exclude_backbone:
        test_sel -= IMP.atom.Selection(hier,atom_types=[IMP.atom.AtomType(n)
                                                        for n in backbone_types])
    test_ps = test_sel.get_selected_particles()
    nn = IMP.algebra.NearestNeighbor3D([IMP.core.XYZ(p).get_coordinates()
                                         for p in test_ps])
    zone = set()
    for target in target_ps:
        zone|=set(nn.get_in_ball(IMP.core.XYZ(target).get_coordinates(),sel_zone))
    zone_ps = [test_ps[z] for z in zone]
    if entire_residues:
        final_ps = set()
        for z in zone_ps:
            final_ps|=set(IMP.atom.Hierarchy(z).get_parent().get_children())
        zone_ps = [h.get_particle() for h in final_ps]
    return zone_ps

class StructureError(Exception):
    pass

#------------------------

class SystemBase(object):
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

class System(SystemBase):
    """This class initializes the root node of the global IMP.atom.Hierarchy."""
    def __init__(self,mdl=None,name="System"):
        SystemBase.__init__(self,mdl)
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

class State(SystemBase):
    """This private class is constructed from within the System class.
    It wraps an IMP.atom.State
    """
    def __init__(self,system,state_index):
        """Define a new state
        @param system        the PMI System
        @param state_index   the index of the new state
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

    def create_molecule(self,name,sequence=None,chain_id=''):
        """Create a new Molecule within this State
        @param name                the name of the molecule (string) it must not
                                   be already used
        @param sequence            sequence (string)
        @param chain_id            Chain id to assign to this molecule
        """
        # check whether the molecule name is already assigned
        if name in self.molecules:
            raise WrongMoleculeName('Cannot use a molecule name already used')

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
            raise StructureError("Trying to add a copy when the original doesn't exist!")
        self.molecules[molname].append(molecule)

    def build(self,**kwargs):
        """call build on all molecules (automatically makes clones)"""
        if not self.built:
            for molname in self.molecules:
                for mol in self.molecules[molname]:
                    mol.build(**kwargs)
            self.built=True
        return self.hier

#------------------------

class Molecule(SystemBase):
    """This class is constructed from within the State class.
    It wraps an IMP.atom.Molecule and IMP.atom.Copy
    Structure is read using this class
    Resolutions and copies can be registered, but are only created when build() is called
    """

    def __init__(self,state,name,sequence,chain_id,copy_num,mol_to_clone=None,transformation=None):
        """The user should not call this direclty, instead call State::create_molecule()
        @param state           The parent PMI State
        @param name            The name of the molecule (string)
        @param sequence        Sequence (string)
        @param mol_to_clone    The original molecule (for cloning ONLY)
        @param transformation  A transform to apply during building (primarily for cloning)
        """
        # internal data storage
        self.mdl = state.get_hierarchy().get_model()
        self.state = state
        self.sequence = sequence
        self.built = False
        self.mol_to_clone = mol_to_clone
        self.transformation = transformation

        # create root node and set it as child to passed parent hierarchy
        self.hier = self._create_child(self.state.get_hierarchy())
        self.hier.set_name(name)
        IMP.atom.Copy.setup_particle(self.hier,copy_num)
        IMP.atom.Chain.setup_particle(self.hier,chain_id)

        # create Residues from the sequence
        self.residues=[]
        for ns,s in enumerate(sequence):
            r=Residue(self,s,ns+1)
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

    def residue_range(self,a,b,stride=1):
        """get residue range. Use integers to get 0-indexing, or strings to get PDB-indexing"""
        if isinstance(a,int) and isinstance(b,int) and isinstance(stride,int):
            return set(self.residues[a:b:stride])
        elif isinstance(a,str) and isinstance(b,str) and isinstance(stride,int):
            return set(self.residues[int(a)-1:int(b)-1:stride])
        else:
            print("ERROR: range ends must be int or str. Stride must be int.")

    def get_residues(self):
        """ Return all Residues as a set"""
        all_res=set()
        for res in self.residues:
            all_res.add(res)
        return all_res

    def get_atomic_residues(self):
        """ Return a set of Residues that have associated structure coordinates """
        atomic_res=set()
        for res in self.residues:
            if res.get_has_structure_source():
                atomic_res.add(res)
        return atomic_res

    def get_non_atomic_residues(self):
        """ Return a set of Residues that don't have associated structure coordinates """
        non_atomic_res=set()
        for res in self.residues:
            if not res.get_has_structure_source():
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

    def create_clone(self,chain_id,transformation=None):
        """Create a Molecule clone (automatically builds same structure and representation)
        @param chain_id If you want to set the chain ID of the copy to something
        @param transformation Apply transformation after building (at the end)
        \note You cannot add structure or representations to a clone!
        """
        mol = Molecule(self.state,self.get_name(),self.sequence,chain_id,
                       copy_num=self.state.get_number_of_copies(self.get_name()),
                       mol_to_clone=self,transformation=transformation)
        self.state._register_copy(mol)
        return mol

    def add_structure(self,pdb_fn,chain_id,res_range=[],offset=0,model_num=None,ca_only=False,soft_check=False,pdb_code=None):
        """Read a structure and store the coordinates.
        Returns the atomic residues (as a set)
        @param pdb_fn    The file to read
        @param chain_id  Chain ID to read
        @param res_range Add only a specific set of residues
        @param offset    Apply an offset to the residue indexes of the PDB file
        @param model_num Read multi-model PDB and return that model
        @param soft_check If True, it only warns if there are sequence mismatches between the pdb and the Molecules sequence
                          If False (Default), it raises and exit when there are sequence mismatches.
        @param pdb_code  Use if you want to store the PDB code (otherwise will extract from filename)
        \note After offset, we expect the PDB residue numbering to match the FASTA file
        """

        if self.mol_to_clone is not None:
            raise Exception('You cannot call add_structure() for a clone')

        self.pdb_fn=pdb_fn

        # get IMP.atom.Residues from the pdb file
        rhs = system_tools.get_structure(self.mdl,pdb_fn,chain_id,res_range,offset,ca_only=ca_only)
        if len(rhs)>len(self.residues):
            print('ERROR: You are loading',len(rhs), \
                'pdb residues for a sequence of length',len(self.residues),'(too many)')

        # load those into the existing pmi Residue objects, and return contiguous regions
        atomic_res=set() # collect integer indexes of atomic residues!
        for nrh,rh in enumerate(rhs):
            idx = rh.get_index()
            internal_res = self.residues[idx-1]
            if pdb_code is None:
                pdb_code = os.path.splitext(os.path.basename(pdb_fn))[0]
            internal_res.set_structure(rh,soft_check)
            internal_res.set_structure_source(pdb_code,chain_id)
            atomic_res.add(internal_res)
        return atomic_res

    def add_representation(self,res_set=None,representation_type="balls",resolutions=[]):
        """handles the IMP.atom.Representation decorators, such as multi-scale,
        density, etc.
        @param res_set             set of PMI residues for adding the representation
        @param representation_type currently supports only balls
        @param resolutions         what resolutions to add to the
               residues (see @ref pmi_resolution)
        """

        if self.mol_to_clone is not None:
            raise Exception('You cannot call add_representation() for a clone')

        allowed_types=["balls"]
        if representation_type not in allowed_types:
            print("ERROR: Allowed representation types:",allowed_types)
            return
        if res_set is None:
            res_set=set(self.residues)
        for res in res_set:
            res.add_representation(representation_type,resolutions)

    def get_representations(self):
        '''Returns list of representation dictionaries for each residue
        '''
        reps=[]
        for res in self.residues:
            reps.append(res.representations)
        return reps

    def build(self,merge_type="backbone",ca_centers=True,fill_in_missing_residues=True):
        """Create all parts of the IMP hierarchy
        including Atoms, Residues, and Fragments/Representations and, finally, Copies
        /note Any residues assigned a resolution must have an IMP.atom.Residue hierarchy
              containing at least a CAlpha. For missing residues, these can be constructed
              from the PDB file
        @param merge_type Principle for grouping into fragments.
                          "backbone": linear sequences along backbone are grouped
                          into fragments if they have identical sets of representations.
                          "volume": at each resolution, groups are made based on
                          spatial distance (not currently implemented)
        @param ca_centers For single-bead-per-residue only. Set the center over the CA position.
        """
        allowed_types=("backbone")
        if merge_type not in allowed_types:
            print("ERROR: Allowed merge types:",allowed_types)
            return
        if not self.built:

            # if this is a clone, first copy all representations and structure
            if self.mol_to_clone is not None:
                for nr,r in enumerate(self.mol_to_clone.residues):
                    self.residues[nr].set_structure(IMP.atom.Residue(IMP.atom.create_clone(r.get_hierarchy())),soft_check=True)
                    if r.get_structure_source() is not None:
                        self.residues[nr].set_structure_source(*r.get_structure_source())
                if self.transformation is not None:
                    for r in self.residues:
                        IMP.atom.transform(r.hier,self.transformation)
                for orig,new in zip(self.mol_to_clone.residues,self.residues):
                    new.representations=orig.representations


            # group into Fragments along backbone
            if merge_type=="backbone":
                system_tools.build_along_backbone(self.mdl,self.hier,self.residues,
                                                     IMP.atom.BALLS,ca_centers)

            # group into Fragments by volume
            elif merge_type=="volume":
                pass

            self.built=True

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


class Residue(object):
    """Temporarily stores residue information, even without structure available."""
    # Consider implementing __hash__ so you can select.
    def __init__(self,molecule,code,index):
        """setup a Residue
        @param molecule PMI Molecule to which this residue belongs
        @param code     one-letter residue type code
        @param index    PDB index
        """
        self.molecule = molecule
        self.hier = IMP.atom.Residue.setup_particle(IMP.Particle(molecule.mdl),
                                IMP.pmi.tools.get_residue_type_from_one_letter_code(code),
                                index)
        self.representations = defaultdict(set)
        self.structure_source = None
    def __str__(self):
        return self.get_code()+str(self.get_index())
    def __repr__(self):
        return self.__str__()
    def __key(self):
        return (self.molecule,self.hier,
                frozenset((k,tuple(self.representations[k])) for k in self.representations))
    def __eq__(self,other):
        return type(other)==type(self) and self.__key() == other.__key()
    def __hash__(self):
        return hash(self.__key())
    def get_index(self):
        return self.hier.get_index()
    def get_code(self):
        return IMP.atom.get_one_letter_code(self.hier.get_residue_type())
    def get_residue_type(self):
        return self.hier.get_residue_type()
    def get_hierarchy(self):
        return self.hier
    def get_molecule(self):
        return self.molecule

    def get_has_structure_source(self):
        return (self.structure_source is not None)
    def set_structure(self,res,soft_check=False):
        if res.get_residue_type()!=self.hier.get_residue_type():
            if soft_check:
                self.hier.set_residue_type((res.get_residue_type()))
                print('WARNING: PDB residue index',self.get_index(),'is',
                      IMP.atom.get_one_letter_code(res.get_residue_type()),
                      'and sequence residue is',self.get_code())
            else:
                raise StructureError('ERROR: PDB residue index',self.get_index(),'is',
                                     IMP.atom.get_one_letter_code(res.get_residue_type()),
                                     'and sequence residue is',self.get_code())

        for a in res.get_children():
            self.hier.add_child(a)
            atype=IMP.atom.Atom(a).get_atom_type()
            a.get_particle().set_name('Atom %s of residue %i'%(atype.__str__().strip('"'),
                                                               self.hier.get_index()))
    def set_structure_source(self,pdb_code,chain_id):
        self.structure_source = (pdb_code,chain_id)

    def get_structure_source(self):
        return self.structure_source

    def add_representation(self,rep_type,resolutions):
        self.representations[rep_type] |= set(resolutions)


class TopologyReader(object):
    '''
    Read a pipe-delimited PMI topology file.

    The topology file should be in a simple pipe-delimited format, such as
    @code{.txt}
|directories|
|pdb_dir|./|
|fasta_dir|./|
|gmm_dir|./|

|topology_dictionary|
|component_name|domain_name|fasta_fn|fasta_id|pdb_fn|chain|residue_range|pdb_offset|bead_size|em_residues_per_gaussian|rmf_file|rmf_frame_number|
|Rpb1 |Rpb1_1|1WCM.fasta|1WCM:A|1WCM.pdb|A|1,1140   |0|10|0 |None   | None|
|Rpb1 |Rpb1_2|1WCM.fasta|1WCM:A|1WCM.pdb|A|1141,1274|0|10|0 |0.rmf3 | 0   |
|Rpb1 |Rpb1_3|1WCM.fasta|1WCM:A|1WCM.pdb|A|1275,-1  |0|10|0 |None   | None|
|Rpb2 |Rpb2  |1WCM.fasta|1WCM:B|1WCM.pdb|B|all      |0|10|0 |None   | None|
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
      covered by PDB coordinates.
    - `em_residues`: The number of Gaussians used to model the electron
      density of this domain. Set to zero if no EM fitting will be done.
    - `rmf_file`: File path of rmf file with coordinates (if available).
    - `rmf_frame_number`: File path of rmf file.

    The file is read in and each part of the topology is stored as a
    ComponentTopology object for input into IMP::pmi::macros::BuildModel.
    '''
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
        #print self.defaults
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

    def recompute_default_dirs(self, topology):
        pdb_filename=self.pdb_file.split("/")[-1]
        self.pdb_filename=IMP.get_relative_path(topology.topology_file, topology.defaults)
