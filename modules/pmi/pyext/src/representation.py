#!/usr/bin/env python

"""@namespace IMP.pmi.representation
   Representation of the system.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.display
import IMP.isd
import IMP.pmi
import IMP.pmi.tools
import IMP.pmi.output
import IMP.rmf
import IMP.pmi.topology
import RMF
from math import pi, sqrt
from operator import itemgetter
import os
import weakref

class _Repo(object):
    def __init__(self, doi, root):
        self.doi = doi
        self._root = root

    def get_fname(self, fname):
        """Return a path relative to the top of the repository"""
        return os.path.relpath(fname, self._root)

class _StateInfo(object):
    """Score state-specific information about this representation."""
    short_name = None
    long_name = None

class Representation(object):
    # Authors: Peter Cimermancic, Riccardo Pellarin, Charles Greenberg

    '''
    Set up the representation of all proteins and nucleic acid macromolecules.

    Create the molecular hierarchies, representation,
    sequence connectivity for the various involved proteins and
    nucleic acid macromolecules:

    Create a protein, DNA or RNA, represent it as a set of connected balls of appropriate
    radii and number of residues, PDB at given resolution(s), or ideal helices.

    How to use the SimplifiedModel class (typical use):

    see test/test_hierarchy_contruction.py

    examples:

    1) Create a chain of helices and flexible parts

    c_1_119   =self.add_component_necklace("prot1",1,119,20)
    c_120_131 =self.add_component_ideal_helix("prot1",resolutions=[1,10],resrange=(120,131))
    c_132_138 =self.add_component_beads("prot1",[(132,138)])
    c_139_156 =self.add_component_ideal_helix("prot1",resolutions=[1,10],resrange=(139,156))
    c_157_174 =self.add_component_beads("prot1",[(157,174)])
    c_175_182 =self.add_component_ideal_helix("prot1",resolutions=[1,10],resrange=(175,182))
    c_183_194 =self.add_component_beads("prot1",[(183,194)])
    c_195_216 =self.add_component_ideal_helix("prot1",resolutions=[1,10],resrange=(195,216))
    c_217_250 =self.add_component_beads("prot1",[(217,250)])


    self.set_rigid_body_from_hierarchies(c_120_131)
    self.set_rigid_body_from_hierarchies(c_139_156)
    self.set_rigid_body_from_hierarchies(c_175_182)
    self.set_rigid_body_from_hierarchies(c_195_216)

    clist=[c_1_119,c_120_131,c_132_138,c_139_156,c_157_174,c_175_182,c_183_194,c_195_216,
      c_217_250]

    self.set_chain_of_super_rigid_bodies(clist,2,3)

    self.set_super_rigid_bodies(["prot1"])

    '''

    def __init__(self, m, upperharmonic=True, disorderedlength=True):
        """Constructor.
           @param m                the model
           @param upperharmonic    This flag uses either harmonic (False)
                     or upperharmonic (True) in the intra-pair
                     connectivity restraint.
           @param disorderedlength This flag uses either disordered length
                     calculated for random coil peptides (True) or zero
                     surface-to-surface distance between beads (False)
                     as optimal distance for the sequence connectivity
                     restraint.
        """

        self.state = _StateInfo()
        self._metadata = []
        self._file_dataset = {}
        self._protocol_output = []
        self._non_modeled_components = {}

        # this flag uses either harmonic (False) or upperharmonic (True)
        # in the intra-pair connectivity restraint. Harmonic is used whe you want to
        # remove the intra-ev term from energy calculations, e.g.:
        # upperharmonic=False
        # ip=self.get_connected_intra_pairs()
        # ev.add_excluded_particle_pairs(ip)

        self.upperharmonic = upperharmonic
        self.disorderedlength = disorderedlength
        self.rigid_bodies = []
        self.fixed_rigid_bodies = []
        self.fixed_floppy_bodies = []
        self.floppy_bodies = []
        # self.super_rigid_bodies is a list of tuples.
        # each tuple, corresponding to a particular super rigid body
        # the tuple is (super_rigid_xyzs,super_rigid_rbs)
        # where super_rigid_xyzs are the flexible xyz particles
        # and super_rigid_rbs is the list of rigid bodies.
        self.super_rigid_bodies = []
        self.rigid_body_symmetries = []
        self.output_level = "low"
        self.label = "None"

        self.maxtrans_rb = 2.0
        self.maxrot_rb = 0.04
        self.maxtrans_srb = 2.0
        self.maxrot_srb = 0.2
        self.rigidbodiesarefixed = False
        self.floppybodiesarefixed = False
        self.maxtrans_fb = 3.0
        self.resolution = 10.0
        self.bblenght = 100.0
        self.kappa = 100.0
        self.m = m

        self.representation_is_modified = False
        self.unmodeledregions_cr_dict = {}
        self.sortedsegments_cr_dict = {}
        self.prot = IMP.atom.Hierarchy.setup_particle(IMP.Particle(self.m))
        self.connected_intra_pairs = []
        self.hier_dict = {}
        self.color_dict = {}
        self.sequence_dict = {}
        self.hier_geometry_pairs = {}
        self.hier_db = IMP.pmi.tools.HierarchyDatabase()
        # this dictionary stores the hierarchies by component name and representation type
        # self.hier_representation[name][representation_type]
        # where representation type is Res:X, Beads, Densities, Representation,
        # etc...
        self.hier_representation = {}
        self.hier_resolution = {}
        # reference structures is a dictionary that contains the coordinates of
        # structures that are used to calculate the rmsd
        self.reference_structures = {}
        self.elements = {}
        self.linker_restraints = IMP.RestraintSet(self.m, "linker_restraints")
        self.linker_restraints.set_was_used(True)
        self.linker_restraints_dict = {}
        self.threetoone = {'ALA': 'A', 'ARG': 'R', 'ASN': 'N', 'ASP': 'D',
                           'CYS': 'C', 'GLU': 'E', 'GLN': 'Q', 'GLY': 'G',
                           'HIS': 'H', 'ILE': 'I', 'LEU': 'L', 'LYS': 'K',
                           'MET': 'M', 'PHE': 'F', 'PRO': 'P', 'SER': 'S',
                           'THR': 'T', 'TRP': 'W', 'TYR': 'Y', 'VAL': 'V', 'UNK': 'X'}

        self.onetothree = dict((v, k) for k, v in self.threetoone.items())

        self.residuenamekey = IMP.StringKey("ResidueName")

    def add_metadata(self, m):
        """Associate some metadata with this modeling.
           @param m an instance of IMP.pmi.metadata.Metadata or a subclass.
        """
        self._metadata.append(m)

    def set_file_dataset(self, fname, dataset):
        """Associate a dataset with a filename.
           This can be used to identify how the file was produced (in many
           cases IMP can determine this automatically from a file header or
           other metadata, but not always). For example, a manually-produced
           PDB file (not from the PDB database or Modeller) can be
           identified this way.
           @param fname filename
           @dataset the IMP.pmi.metadata.Dataset object to associate.
        """
        self._file_dataset[os.path.abspath(fname)] = dataset

    def get_file_dataset(self, fname):
        """Get the dataset associated with a filename, or None.
           @param fname filename
           @return an IMP.pmi.metadata.Dataset, or None.
        """
        return self._file_dataset.get(os.path.abspath(fname), None)

    def add_protocol_output(self, p):
        """Capture details of the modeling protocol.
           @param p an instance of IMP.pmi.output.ProtocolOutput or a subclass.
        """
        state = p._add_state(self)
        self._protocol_output.append((p, state))
        p._each_metadata.append(self._metadata)
        p._file_datasets.append(self._file_dataset)
        state.m = self.m
        state.prot = self.prot
    protocol_output = property(lambda self:
                               [x[0] for x in self._protocol_output])

    def set_label(self, label):
        self.label = label

    def create_component(self, name, color=0.0):
        # Note that we set up the component as *both* a Chain and a Molecule.
        # This is because old PMI1 code expects the top-level particle to be
        # a Molecule, but we also need it to be a Chain to set the sequence.
        # This looks a little odd but is a valid IMP atom::Hierarchy.
        protein_h = IMP.atom.Chain.setup_particle(IMP.Particle(self.m), 'X')
        IMP.atom.Molecule.setup_particle(protein_h)
        protein_h.set_name(name)
        self.hier_dict[name] = protein_h
        self.hier_representation[name] = {}
        self.hier_db.add_name(name)
        self.prot.add_child(protein_h)
        self.color_dict[name] = color
        self.elements[name] = []
        for p, state in self._protocol_output:
            p.create_component(state, name, True)

    def create_non_modeled_component(self, name):
        """Create a component that isn't used in the modeling.
           No coordinates or other structural data for this component will
           be read or written, but a primary sequence can be assigned. This
           is useful if the input experimental data is of a system larger
           than that modeled. Any references to these non-modeled components
           can then be correctly resolved later."""
        self._non_modeled_components[name] = None
        self.elements[name] = []
        for p, state in self._protocol_output:
            p.create_component(state, name, False)

    # Deprecation warning

    @IMP.deprecated_method("2.5", "Use create_component() instead.")
    def add_component_name(self, *args, **kwargs):
        self.create_component(*args, **kwargs)

    def get_component_names(self):
        return list(self.hier_dict.keys())

    def add_component_sequence(self, name, filename, id=None, offs=None,
                               format="FASTA"):
        '''
        Add the primary sequence for a single component.

        @param name Human-readable name of the component
        @param filename Name of the FASTA file
        @param id Identifier of the sequence in the FASTA file header
                  (if not provided, use `name` instead)
        '''
        record_dict = IMP.pmi.topology.Sequences(filename)
        if id is None:
            id = name
        if id not in record_dict:
            raise KeyError("id %s not found in fasta file" % id)
        length = len(record_dict[id])
        self.sequence_dict[name] = str(record_dict[id])
        # No Hierarchy for this component if it is non-modeled
        if name not in self._non_modeled_components:
            protein_h = self.hier_dict[name]
            protein_h.set_sequence(self.sequence_dict[name])
        if offs is not None:
            offs_str="-"*offs
            self.sequence_dict[name]=offs_str+self.sequence_dict[name]

        self.elements[name].append((length, length, " ", "end"))
        for p, state in self._protocol_output:
            p.add_component_sequence(name, self.sequence_dict[name])

    def autobuild_model(self, name, pdbname, chain,
                        resolutions=None, resrange=None,
                        missingbeadsize=20,
                        color=None, pdbresrange=None, offset=0,
                        show=False, isnucleicacid=False,
                        attachbeads=False):

        self.representation_is_modified = True
        outhiers = []

        if color is None:
            color = self.color_dict[name]
        else:
            self.color_dict[name] = color

        if resolutions is None:
            resolutions = [1]
        print("autobuild_model: constructing %s from pdb %s and chain %s" % (name, pdbname, str(chain)))

        # get the initial and end residues of the pdb
        t = IMP.atom.read_pdb(pdbname, self.m,
                              IMP.atom.AndPDBSelector(IMP.atom.ChainPDBSelector(chain), IMP.atom.CAlphaPDBSelector()))

        # find start and end indexes

        start = IMP.atom.Residue(
            t.get_children()[0].get_children()[0]).get_index()
        end = IMP.atom.Residue(
            t.get_children()[0].get_children()[-1]).get_index()

        # check if resrange was defined, otherwise
        # use the sequence, or the pdb resrange

        if resrange is None:
            if name in self.sequence_dict:
                resrange = (1, len(self.sequence_dict[name]))
            else:
                resrange = (start + offset, end + offset)
        else:
            if resrange[1] in (-1, 'END'):
                resrange = (resrange[0],end)
            start = resrange[0] - offset
            end = resrange[1] - offset

        gaps = IMP.pmi.tools.get_residue_gaps_in_hierarchy(
            t,
            resrange[0],
            resrange[1])

        xyznter = IMP.pmi.tools.get_closest_residue_position(
            t,
            resrange[0],
            terminus="N")
        xyzcter = IMP.pmi.tools.get_closest_residue_position(
            t,
            resrange[1],
            terminus="C")
        # Done with the PDB
        IMP.atom.destroy(t)

        # construct pdb fragments and intervening beads
        for n, g in enumerate(gaps):
            first = g[0]
            last = g[1]
            if g[2] == "cont":
                print("autobuild_model: constructing fragment %s from pdb" % (str((first, last))))
                outhiers += self.add_component_pdb(name, pdbname,
                                                   chain, resolutions=resolutions,
                                                   color=color, cacenters=True,
                                                   resrange=(first, last),
                                                   offset=offset, isnucleicacid=isnucleicacid)
            elif g[2] == "gap" and n > 0:
                print("autobuild_model: constructing fragment %s as a bead" % (str((first, last))))
                parts = self.hier_db.get_particles_at_closest_resolution(name,
                                                                         first + offset - 1,
                                                                         1)
                xyz = IMP.core.XYZ(parts[0]).get_coordinates()
                outhiers += self.add_component_necklace(name,
                                                        first+offset, last+offset, missingbeadsize, incoord=xyz)

            elif g[2] == "gap" and n == 0:
                # add pre-beads
                print("autobuild_model: constructing fragment %s as a bead" % (str((first, last))))
                outhiers += self.add_component_necklace(name,
                                                        first+offset, last+offset, missingbeadsize, incoord=xyznter)

        return outhiers

    # Deprecation warning

    @IMP.deprecated_method("2.5", "Use autobuild_model() instead.")
    def autobuild_pdb_and_intervening_beads(self, *args, **kwargs):
        r = self.autobuild_model(*args, **kwargs)
        return r

    def add_component_pdb(self, name, pdbname, chain, resolutions, color=None,
                          resrange=None, offset=0, cacenters=True, show=False,
                          isnucleicacid=False, readnonwateratoms=False,
                          read_ca_cb_only=False):
        '''
        Add a component that has an associated 3D structure in a PDB file.

        Reads the PDB, and constructs the fragments corresponding to contiguous
        sequence stretches.

        @return a list of hierarchies.

        @param name (string) the name of the component
        @param pdbname (string) the name of the PDB file
        @param chain (string or integer) can be either a string (eg, "A")
                     or an integer (eg, 0 or 1) in case you want
                     to get the corresponding chain number in the PDB.
        @param resolutions (integers) a list of integers that corresponds
                     to the resolutions that have to be generated
        @param color (float from 0 to 1) the color applied to the
                     hierarchies generated
        @param resrange (tuple of integers): the residue range to extract
                     from the PDB. It is a tuple (beg,end). If not specified,
                     all residues belonging to the specified chain are read.
        @param offset (integer) specifies the residue index offset to be
                     applied when reading the PDB (the FASTA sequence is
                     assumed to start from residue 1, so use this parameter
                     if the PDB file does not start at residue 1)
        @param cacenters (boolean) if True generates resolution=1 beads
                     centered on C-alpha atoms.
        @param show (boolean) print out the molecular hierarchy at the end.
        @param isnucleicacid (boolean) use True if you're reading a PDB
                     with nucleic acids.
        @param readnonwateratoms (boolean) if True fixes some pathological PDB.
        @param read_ca_cb_only (boolean) if True, only reads CA/CB
        '''

        self.representation_is_modified = True
        if color is None:
            # if the color is not passed, then get the stored color
            color = self.color_dict[name]
        protein_h = self.hier_dict[name]
        outhiers = []

        # determine selector
        sel = IMP.atom.NonWaterNonHydrogenPDBSelector()
        if read_ca_cb_only:
            cacbsel = IMP.atom.OrPDBSelector(
                IMP.atom.CAlphaPDBSelector(),
                IMP.atom.CBetaPDBSelector())
            sel = IMP.atom.AndPDBSelector(cacbsel, sel)
        if type(chain) == str:
            sel = IMP.atom.AndPDBSelector(
                IMP.atom.ChainPDBSelector(chain),
                sel)
            t = IMP.atom.read_pdb(pdbname, self.m, sel)

            # get the first and last residue
            start = IMP.atom.Residue(
                t.get_children()[0].get_children()[0]).get_index()
            end = IMP.atom.Residue(
                t.get_children()[0].get_children()[-1]).get_index()
            c = IMP.atom.Chain(IMP.atom.get_by_type(t, IMP.atom.CHAIN_TYPE)[0])
        else:
            t = IMP.atom.read_pdb(pdbname, self.m, sel)
            c = IMP.atom.Chain(
                IMP.atom.get_by_type(t, IMP.atom.CHAIN_TYPE)[chain])

            # get the first and last residue
            start = IMP.atom.Residue(c.get_children()[0]).get_index()
            end = IMP.atom.Residue(c.get_children()[-1]).get_index()
            chain = c.get_id()

        if not resrange is None:
            if resrange[0] > start:
                start = resrange[0]
            if resrange[1] < end:
                end = resrange[1]

        if not isnucleicacid:
            # do what you have to do for proteins
            sel = IMP.atom.Selection(
                c,
                residue_indexes=list(range(
                    start,
                    end + 1)),
                atom_type=IMP.atom.AT_CA)

        else:
            # do what you have to do for nucleic-acids
            # to work, nucleic acids should not be indicated as HETATM in the pdb
            sel = IMP.atom.Selection(
                c,
                residue_indexes=list(range(
                    start,
                    end + 1)),
                atom_type=IMP.atom.AT_P)

        ps = sel.get_selected_particles()
        if len(ps) == 0:
            raise ValueError("%s no residue found in pdb %s chain %s that overlaps with the queried stretch %s-%s" \
                  % (name, pdbname, str(chain), str(resrange[0]), str(resrange[1])))
        c0 = IMP.atom.Chain.setup_particle(IMP.Particle(self.m), "X")

        for p in ps:
            par = IMP.atom.Atom(p).get_parent()
            ri = IMP.atom.Residue(par).get_index()
            # Move residue from original PDB hierarchy to new chain c0
            IMP.atom.Residue(par).set_index(ri + offset)
            if par.get_parent() != c0:
                par.get_parent().remove_child(par)
            c0.add_child(par)
        start = start + offset
        end = end + offset

        self.elements[name].append(
            (start, end, pdbname.split("/")[-1] + ":" + chain, "pdb"))

        hiers = self.coarse_hierarchy(name, start, end,
                                      resolutions, isnucleicacid, c0, protein_h, "pdb", color)
        self._copy_pdb_provenance(t, hiers[0], offset)
        outhiers += hiers
        for p, state in self._protocol_output:
            p.add_pdb_element(state, name, start, end, offset, pdbname, chain,
                              hiers[0])

        if show:
            IMP.atom.show_molecular_hierarchy(protein_h)

        # We cannot simply destroy(c0) since it might not be a well-behaved
        # hierarchy; in some cases it could contain a given residue more than
        # once (this is surely a bug but we need to keep this behavior for
        # backwards compatibility).
        residues = {}
        for p in ps:
            par = IMP.atom.Atom(p).get_parent()
            residues[par] = None
        for r in residues.keys():
            IMP.atom.destroy(r)
        self.m.remove_particle(c0)

        IMP.atom.destroy(t)

        return outhiers

    def _copy_pdb_provenance(self, pdb, h, offset):
        """Copy the provenance information from the PDB to our hierarchy"""
        c = IMP.atom.Chain(IMP.atom.get_by_type(pdb, IMP.atom.CHAIN_TYPE)[0])
        prov = IMP.core.Provenanced(c).get_provenance()
        newprov = IMP.core.create_clone(prov)
        IMP.core.Provenanced.setup_particle(h, newprov)
        assert(IMP.core.StructureProvenance.get_is_setup(newprov))
        IMP.core.StructureProvenance(newprov).set_residue_offset(offset)

    def add_component_ideal_helix(
        self,
        name,
        resolutions,
        resrange,
        color=None,
        show=False):

        self.representation_is_modified = True
        from math import pi, cos, sin

        protein_h = self.hier_dict[name]
        outhiers = []
        if color is None:
            color = self.color_dict[name]

        start = resrange[0]
        end = resrange[1]
        self.elements[name].append((start, end, " ", "helix"))
        c0 = IMP.atom.Chain.setup_particle(IMP.Particle(self.m), "X")
        for n, res in enumerate(range(start, end + 1)):
            if name in self.sequence_dict:
                try:
                    rtstr = self.onetothree[
                        self.sequence_dict[name][res-1]]
                except:
                    rtstr = "UNK"
                rt = IMP.atom.ResidueType(rtstr)
            else:
                rt = IMP.atom.ResidueType("ALA")

            # get the residue volume
            try:
                vol = IMP.atom.get_volume_from_residue_type(rt)
                # mass=IMP.atom.get_mass_from_residue_type(rt)
            except IMP.ValueException:
                vol = IMP.atom.get_volume_from_residue_type(
                IMP.atom.ResidueType("ALA"))
                # mass=IMP.atom.get_mass_from_residue_type(IMP.atom.ResidueType("ALA"))
            radius = IMP.algebra.get_ball_radius_from_volume_3d(vol)

            r = IMP.atom.Residue.setup_particle(IMP.Particle(self.m), rt, res)
            p = IMP.Particle(self.m)
            d = IMP.core.XYZR.setup_particle(p)
            x = 2.3 * cos(n * 2 * pi / 3.6)
            y = 2.3 * sin(n * 2 * pi / 3.6)
            z = 6.2 / 3.6 / 2 * n * 2 * pi / 3.6
            d.set_coordinates(IMP.algebra.Vector3D(x, y, z))
            d.set_radius(radius)
            # print d
            a = IMP.atom.Atom.setup_particle(p, IMP.atom.AT_CA)
            r.add_child(a)
            c0.add_child(r)

        outhiers += self.coarse_hierarchy(name, start, end,
                                          resolutions, False, c0, protein_h, "helix", color)

        if show:
            IMP.atom.show_molecular_hierarchy(protein_h)

        IMP.atom.destroy(c0)
        return outhiers

    def add_component_beads(self, name, ds, colors=None, incoord=None):
        """ add beads to the representation
        @param name the component name
        @param ds a list of tuples corresponding to the residue ranges
                  of the beads
        @param colors a list of colors associated to the beads
        @param incoord the coordinate tuple corresponding to the position
                  of the beads
        """

        from math import pi
        self.representation_is_modified = True

        protein_h = self.hier_dict[name]
        outhiers = []
        if colors is None:
            colors = [self.color_dict[name]]


        for n, dss in enumerate(ds):
            ds_frag = (dss[0], dss[1])
            self.elements[name].append((dss[0], dss[1], " ", "bead"))
            prt = IMP.Particle(self.m)
            if ds_frag[0] == ds_frag[1]:
                # if the bead represent a single residue
                if name in self.sequence_dict:
                    try:
                        rtstr = self.onetothree[
                            self.sequence_dict[name][ds_frag[0]-1]]
                    except:
                        rtstr = "UNK"
                    rt = IMP.atom.ResidueType(rtstr)
                else:
                    rt = IMP.atom.ResidueType("ALA")
                h = IMP.atom.Residue.setup_particle(prt, rt, ds_frag[0])
                h.set_name(name + '_%i_bead' % (ds_frag[0]))
                prt.set_name(name + '_%i_bead' % (ds_frag[0]))
                resolution = 1
            else:
                h = IMP.atom.Fragment.setup_particle(prt)
                h.set_name(name + '_%i-%i_bead' % (ds_frag[0], ds_frag[1]))
                prt.set_name(name + '_%i-%i_bead' % (ds_frag[0], ds_frag[1]))
                h.set_residue_indexes(list(range(ds_frag[0], ds_frag[1] + 1)))
                resolution = len(h.get_residue_indexes())
            if "Beads" not in self.hier_representation[name]:
                root = IMP.atom.Hierarchy.setup_particle(IMP.Particle(self.m))
                root.set_name("Beads")
                self.hier_representation[name]["Beads"] = root
                protein_h.add_child(root)
            self.hier_representation[name]["Beads"].add_child(h)

            for kk in range(ds_frag[0], ds_frag[1] + 1):
                self.hier_db.add_particles(name, kk, resolution, [h])

            try:
                clr = IMP.display.get_rgb_color(colors[n])
            except:
                clr = IMP.display.get_rgb_color(colors[0])

            IMP.display.Colored.setup_particle(prt, clr)

            # decorate particles according to their resolution
            IMP.pmi.Resolution.setup_particle(prt, resolution)

            IMP.core.XYZR.setup_particle(prt)
            ptem = IMP.core.XYZR(prt)
            mass = IMP.atom.get_mass_from_number_of_residues(resolution)
            if resolution == 1:
                try:
                    vol = IMP.atom.get_volume_from_residue_type(rt)
                except IMP.ValueException:
                    vol = IMP.atom.get_volume_from_residue_type(
                        IMP.atom.ResidueType("ALA"))
                radius = IMP.algebra.get_ball_radius_from_volume_3d(vol)
                IMP.atom.Mass.setup_particle(prt, mass)
                ptem.set_radius(radius)
            else:
                volume = IMP.atom.get_volume_from_mass(mass)
                radius = 0.8 * (3.0 / 4.0 / pi * volume) ** (1.0 / 3.0)
                IMP.atom.Mass.setup_particle(prt, mass)
                ptem.set_radius(radius)
            try:
                if not tuple(incoord) is None:
                    ptem.set_coordinates(incoord)
            except TypeError:
                pass
            IMP.pmi.Uncertainty.setup_particle(prt, radius)
            IMP.pmi.Symmetric.setup_particle(prt, 0)
            self.floppy_bodies.append(prt)
            IMP.core.XYZ(prt).set_coordinates_are_optimized(True)
            outhiers += [h]

        for p, state in self._protocol_output:
            p.add_bead_element(state, name, ds[0][0], ds[-1][1], len(ds),
                               outhiers[0])

        return outhiers

    def add_component_necklace(self, name, begin, end, length, color=None,
                               incoord=None):
        '''
        Generates a string of beads with given length.
        '''
        self.representation_is_modified = True
        outhiers = []
        if color is None:
            colors=None
        else:
            colors=[color]
        for chunk in IMP.pmi.tools.list_chunks_iterator(range(begin, end + 1), length):
            outhiers += self.add_component_beads(name,
                                                 [(chunk[0], chunk[-1])], colors=colors,incoord=incoord)
        return outhiers

    def add_component_density(
        self, name, hierarchies=None, selection_tuples=None,
        particles=None,
        resolution=0.0, num_components=10,
        inputfile=None, outputfile=None,
        outputmap=None,
        kernel_type=None,
        covariance_type='full', voxel_size=1.0,
        out_hier_name='',
        sampled_points=1000000, num_iter=100,
        simulation_res=1.0,
        multiply_by_total_mass=True,
        transform=None,
        intermediate_map_fn=None,
        density_ps_to_copy=None,
        use_precomputed_gaussians=False):
        '''
        Sets up a Gaussian Mixture Model for this component.
        Can specify input GMM file or it will be computed.
        @param name component name
        @param hierarchies set up GMM for some hierarchies
        @param selection_tuples (list of tuples) example (first_residue,last_residue,component_name)
        @param particles set up GMM for particles directly
        @param resolution usual PMI resolution for selecting particles from the hierarchies
        @param inputfile read the GMM from this file
        @param outputfile fit and write the GMM to this file (text)
        @param outputmap after fitting, create GMM density file (mrc)
        @param kernel_type for creating the intermediate density (points are sampled to make GMM). Options are IMP.em.GAUSSIAN, IMP.em.SPHERE, and IMP.em.BINARIZED_SPHERE
        @param covariance_type for fitting the GMM. options are 'full', 'diagonal' and 'spherical'
        @param voxel_size for creating the intermediate density map and output map.
                                  lower number increases accuracy but also rasterizing time grows
        @param out_hier_name name of the output density hierarchy
        @param sampled_points number of points to sample. more will increase accuracy and fitting time
        @param num_iter num GMM iterations. more will increase accuracy and fitting time
        @param multiply_by_total_mass multiply the weights of the GMM by this value (only works on creation!)
        @param transform for input file only, apply a transformation (eg for multiple copies same GMM)
        @param intermediate_map_fn for debugging, this will write the intermediate (simulated) map
        @param density_ps_to_copy in case you already created the appropriate GMM (eg, for beads)
        @param use_precomputed_gaussians Set this flag and pass fragments - will use roughly spherical Gaussian setup
        '''
        import numpy as np
        import sys
        import IMP.em
        import IMP.isd.gmm_tools

        # prepare output
        self.representation_is_modified = True
        out_hier = []
        protein_h = self.hier_dict[name]
        if "Densities" not in self.hier_representation[name]:
            root = IMP.atom.Hierarchy.setup_particle(IMP.Particle(self.m))
            root.set_name("Densities")
            self.hier_representation[name]["Densities"] = root
            protein_h.add_child(root)

        # gather passed particles
        fragment_particles = []
        if not particles is None:
            fragment_particles += particles
        if not hierarchies is None:
            fragment_particles += IMP.pmi.tools.select(
                self, resolution=resolution,
                hierarchies=hierarchies)
        if not selection_tuples is None:
            for st in selection_tuples:
                fragment_particles += IMP.pmi.tools.select_by_tuple(
                    self, tupleselection=st,
                    resolution=resolution,
                    name_is_ambiguous=False)

        # compute or read gaussians
        density_particles = []
        if inputfile:
            IMP.isd.gmm_tools.decorate_gmm_from_text(
                inputfile, density_particles,
                self.m, transform)
        elif density_ps_to_copy:
            for ip in density_ps_to_copy:
                p = IMP.Particle(self.m)
                shape = IMP.core.Gaussian(ip).get_gaussian()
                mass = IMP.atom.Mass(ip).get_mass()
                IMP.core.Gaussian.setup_particle(p, shape)
                IMP.atom.Mass.setup_particle(p, mass)
                density_particles.append(p)
        elif use_precomputed_gaussians:
            if len(fragment_particles) == 0:
                print("add_component_density: no particle was selected")
                return out_hier
            for p in fragment_particles:
                if not (IMP.atom.Fragment.get_is_setup(self.m,p.get_particle_index()) and
                        IMP.core.XYZ.get_is_setup(self.m,p.get_particle_index())):
                    raise Exception("The particles you selected must be Fragments and XYZs")
                nres=len(IMP.atom.Fragment(self.m,p.get_particle_index()).get_residue_indexes())
                pos=IMP.core.XYZ(self.m,p.get_particle_index()).get_coordinates()
                density_particles=[]
                try:
                    IMP.isd.get_data_path("beads/bead_%i.txt"%nres)
                except:
                    raise Exception("We haven't created a bead file for",nres,"residues")
                transform = IMP.algebra.Transformation3D(pos)
                IMP.isd.gmm_tools.decorate_gmm_from_text(
                    IMP.isd.get_data_path("beads/bead_%i.txt"%nres), density_particles,
                    self.m, transform)
        else:
            #compute the gaussians here
            if len(fragment_particles) == 0:
                print("add_component_density: no particle was selected")
                return out_hier

            density_particles = IMP.isd.gmm_tools.sample_and_fit_to_particles(
                self.m,
                fragment_particles,
                num_components,
                sampled_points,
                simulation_res,
                voxel_size,
                num_iter,
                covariance_type,
                multiply_by_total_mass,
                outputmap,
                outputfile)

        # prepare output hierarchy
        s0 = IMP.atom.Fragment.setup_particle(IMP.Particle(self.m))
        s0.set_name(out_hier_name)
        self.hier_representation[name]["Densities"].add_child(s0)
        out_hier.append(s0)
        for nps, p in enumerate(density_particles):
            s0.add_child(p)
            p.set_name(s0.get_name() + '_gaussian_%i' % nps)
        return out_hier

    def get_component_density(self, name):
        return self.hier_representation[name]["Densities"]

    def add_all_atom_densities(self, name, hierarchies=None,
                               selection_tuples=None,
                               particles=None,
                               resolution=0,
                               output_map=None,
                               voxel_size=1.0):
        '''Decorates all specified particles as Gaussians directly.
        @param name component name
        @param hierarchies set up GMM for some hierarchies
        @param selection_tuples (list of tuples) example (first_residue,last_residue,component_name)
        @param particles set up GMM for particles directly
        @param resolution usual PMI resolution for selecting particles from the hierarchies
        '''

        import IMP.em
        import numpy as np
        import sys
        from math import sqrt
        self.representation_is_modified = True

        if particles is None:
            fragment_particles = []
        else:
            fragment_particles = particles

        if not hierarchies is None:
            fragment_particles += IMP.pmi.tools.select(
                self, resolution=resolution,
                hierarchies=hierarchies)

        if not selection_tuples is None:
            for st in selection_tuples:
                fragment_particles += IMP.pmi.tools.select_by_tuple(
                    self, tupleselection=st,
                    resolution=resolution,
                    name_is_ambiguous=False)

        if len(fragment_particles) == 0:
            print("add all atom densities: no particle was selected")
            return

        # create a spherical gaussian for each particle based on atom type
        print('setting up all atom gaussians num_particles',len(fragment_particles))
        for n,p in enumerate(fragment_particles):
            if IMP.core.Gaussian.get_is_setup(p): continue
            center=IMP.core.XYZ(p).get_coordinates()
            rad=IMP.core.XYZR(p).get_radius()
            mass=IMP.atom.Mass(p).get_mass()
            trans=IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),center)
            shape=IMP.algebra.Gaussian3D(IMP.algebra.ReferenceFrame3D(trans),[rad]*3)
            IMP.core.Gaussian.setup_particle(p,shape)
            print('setting up particle',p.get_name(), " as individual gaussian particle")

        if not output_map is None:
            print('writing map to', output_map)
            IMP.isd.gmm_tools.write_gmm_to_map(
                fragment_particles,
                output_map,
                voxel_size)

    def add_component_hierarchy_clone(self, name, hierarchy):
        '''
        Make a copy of a hierarchy and append it to a component.
        '''
        outhier = []
        self.representation_is_modified = True
        protein_h = self.hier_dict[name]
        hierclone = IMP.atom.create_clone(hierarchy)
        hierclone.set_name(hierclone.get_name() + "_clone")
        protein_h.add_child(hierclone)
        outhier.append(hierclone)

        psmain = IMP.atom.get_leaves(hierarchy)
        psclone = IMP.atom.get_leaves(hierclone)

        # copying attributes
        for n, pmain in enumerate(psmain):
            pclone = psclone[n]
            if IMP.pmi.Resolution.get_is_setup(pmain):
                resolution = IMP.pmi.Resolution(pmain).get_resolution()
                IMP.pmi.Resolution.setup_particle(pclone, resolution)
                for kk in IMP.pmi.tools.get_residue_indexes(pclone):
                    self.hier_db.add_particles(
                        name,
                        kk,
                        IMP.pmi.Resolution(pclone).get_resolution(),
                        [pclone])

            if IMP.pmi.Uncertainty.get_is_setup(pmain):
                uncertainty = IMP.pmi.Uncertainty(pmain).get_uncertainty()
                IMP.pmi.Uncertainty.setup_particle(pclone, uncertainty)

            if IMP.pmi.Symmetric.get_is_setup(pmain):
                symmetric = IMP.pmi.Symmetric(pmain).get_symmetric()
                IMP.pmi.Symmetric.setup_particle(pclone, symmetric)

        return outhier

    def dump_particle_descriptors(self):
        import numpy
        import pickle
        import IMP.isd
        import IMP.isd.gmm_tools

        particles_attributes={}
        floppy_body_attributes={}
        gaussians=[]
        for h in IMP.atom.get_leaves(self.prot):
            leaf=h
            name=h.get_name()
            hroot=self.prot
            hparent=h.get_parent()
            while hparent != hroot:
                hparent=h.get_parent()
                name+="|"+hparent.get_name()
                h=hparent
            particles_attributes[name]={"COORDINATES":numpy.array(IMP.core.XYZR(leaf.get_particle()).get_coordinates()),
            "RADIUS":IMP.core.XYZR(leaf.get_particle()).get_radius(),
            "MASS":IMP.atom.Mass(leaf.get_particle()).get_mass()}
            if IMP.core.Gaussian.get_is_setup(leaf.get_particle()):
                gaussians.append(IMP.core.Gaussian(leaf.get_particle()))

        rigid_body_attributes={}
        for rb in self.rigid_bodies:
            name=rb.get_name()
            rf=rb.get_reference_frame()
            t=rf.get_transformation_to()
            trans=t.get_translation()
            rot=t.get_rotation()
            rigid_body_attributes[name]={"TRANSLATION":numpy.array(trans),
                                         "ROTATION":numpy.array(rot.get_quaternion()),
                                         "COORDINATES_NONRIGID_MEMBER":{},
                                         "COORDINATES_RIGID_MEMBER":{}}
            for mi in rb.get_member_indexes():
                rm=self.m.get_particle(mi)
                if IMP.core.NonRigidMember.get_is_setup(rm):
                    name_part=rm.get_name()
                    xyz=[self.m.get_attribute(fk, rm) for fk in [IMP.FloatKey(4), IMP.FloatKey(5), IMP.FloatKey(6)]]
                    rigid_body_attributes[name]["COORDINATES_NONRIGID_MEMBER"][name_part]=numpy.array(xyz)
                else:
                    name_part=rm.get_name()
                    xyz=IMP.core.XYZ(rm).get_coordinates()
                    rigid_body_attributes[name]["COORDINATES_RIGID_MEMBER"][name_part]=numpy.array(xyz)


        IMP.isd.gmm_tools.write_gmm_to_text(gaussians,"model_gmm.txt")
        pickle.dump(particles_attributes,
                    open("particles_attributes.pkl", "wb"))
        pickle.dump(rigid_body_attributes,
                    open("rigid_body_attributes.pkl", "wb"))



    def load_particle_descriptors(self):
        import numpy
        import pickle
        import IMP.isd
        import IMP.isd.gmm_tools

        particles_attributes = pickle.load(open("particles_attributes.pkl",
                                                "rb"))
        rigid_body_attributes = pickle.load(open("rigid_body_attributes.pkl",
                                                 "rb"))

        particles=[]
        hierarchies=[]
        gaussians=[]
        for h in IMP.atom.get_leaves(self.prot):
            leaf=h
            name=h.get_name()
            hroot=self.prot
            hparent=h.get_parent()
            while hparent != hroot:
                hparent=h.get_parent()
                name+="|"+hparent.get_name()
                h=hparent

            xyzr=IMP.core.XYZR(leaf.get_particle())
            xyzr.set_coordinates(particles_attributes[name]["COORDINATES"])
            #xyzr.set_radius(particles_attributes[name]["RADIUS"])
            #IMP.atom.Mass(leaf.get_particle()).set_mass(particles_attributes[name]["MASS"])
            if IMP.core.Gaussian.get_is_setup(leaf.get_particle()):
                gaussians.append(IMP.core.Gaussian(leaf.get_particle()))

        for rb in self.rigid_bodies:
            name=rb.get_name()
            trans=rigid_body_attributes[name]["TRANSLATION"]
            rot=rigid_body_attributes[name]["ROTATION"]
            t=IMP.algebra.Transformation3D(IMP.algebra.Rotation3D(rot),trans)
            rf=IMP.algebra.ReferenceFrame3D(t)
            rb.set_reference_frame(rf)
            coor_nrm_ref=rigid_body_attributes[name]["COORDINATES_NONRIGID_MEMBER"]
            coor_rm_ref_dict=rigid_body_attributes[name]["COORDINATES_RIGID_MEMBER"]
            coor_rm_model=[]
            coor_rm_ref=[]
            for mi in rb.get_member_indexes():
                rm=self.m.get_particle(mi)
                if IMP.core.NonRigidMember.get_is_setup(rm):
                    name_part=rm.get_name()
                    xyz=coor_nrm_ref[name_part]
                    for n,fk in enumerate([IMP.FloatKey(4), IMP.FloatKey(5), IMP.FloatKey(6)]):
                        self.m.set_attribute(fk, rm,xyz[n])
                else:
                    name_part=rm.get_name()
                    coor_rm_ref.append(IMP.algebra.Vector3D(coor_rm_ref_dict[name_part]))
                    coor_rm_model.append(IMP.core.XYZ(rm).get_coordinates())
            if len(coor_rm_model)==0: continue
            t=IMP.algebra.get_transformation_aligning_first_to_second(coor_rm_model,coor_rm_ref)
            IMP.core.transform(rb,t)

        IMP.isd.gmm_tools.decorate_gmm_from_text("model_gmm.txt",gaussians,self.m)

    def _compare_rmf_repr_names(self, rmfname, reprname, component_name):
        """Print a warning if particle names in RMF and model don't match"""
        def match_any_suffix():
            # Handle common mismatches like 743 != Nup85_743_pdb
            suffixes = ["pdb", "bead_floppy_body_rigid_body_member_floppy_body",
                        "bead_floppy_body_rigid_body_member",
                        "bead_floppy_body"]
            for s in suffixes:
                if "%s_%s_%s" % (component_name, rmfname, s) == reprname:
                    return True
        if rmfname != reprname and not match_any_suffix():
            print("set_coordinates_from_rmf: WARNING rmf particle and "
                  "representation particle names don't match %s %s"
                  % (rmfname, reprname))

    def set_coordinates_from_rmf(self, component_name, rmf_file_name,
                                 rmf_frame_number,
                                 rmf_component_name=None,
                                 check_number_particles=True,
                                 representation_name_to_rmf_name_map=None,
                                 state_number=0,
                                 skip_gaussian_in_rmf=False,
                                 skip_gaussian_in_representation=False,
                                 save_file=False,
                                 force_rigid_update=False):
        '''Read and replace coordinates from an RMF file.
        Replace the coordinates of particles with the same name.
        It assumes that the RMF and the representation have the particles
        in the same order.
        @param component_name Component name
        @param rmf_component_name Name of the component in the RMF file
                (if not specified, use `component_name`)
        @param representation_name_to_rmf_name_map a dictionary that map
                the original rmf particle name to the recipient particle component name
        @param save_file: save a file with the names of particles of the component
        @param force_rigid_update: update the coordinates of rigid bodies
               (normally this should be called before rigid bodies are set up)
        '''
        import IMP.pmi.analysis

        tempm = IMP.Model()
        prots = IMP.pmi.analysis.get_hiers_from_rmf(
            tempm,
            rmf_frame_number,
            rmf_file_name)

        if not prots:
            raise ValueError("cannot read hierarchy from rmf")

        prot=prots[0]
        # Make sure coordinates of rigid body members in the RMF are correct
        if force_rigid_update:
            tempm.update()

        # if len(self.rigid_bodies)!=0:
        #   print "set_coordinates_from_rmf: cannot proceed if rigid bodies were initialized. Use the function before defining the rigid bodies"
        #   exit()

        allpsrmf = IMP.atom.get_leaves(prot)
        psrmf = []
        for p in allpsrmf:
            (protname, is_a_bead) = IMP.pmi.tools.get_prot_name_from_particle(
                p, self.hier_dict.keys())
            if (protname is None) and (rmf_component_name is not None):
                (protname, is_a_bead) = IMP.pmi.tools.get_prot_name_from_particle(
                    p, rmf_component_name)
            if (skip_gaussian_in_rmf):
                if (IMP.core.Gaussian.get_is_setup(p)) and not (IMP.atom.Fragment.get_is_setup(p) or IMP.atom.Residue.get_is_setup(p)):
                    continue
            if (rmf_component_name is not None) and (protname == rmf_component_name):
                psrmf.append(p)
            elif (rmf_component_name is None) and (protname == component_name):
                psrmf.append(p)

        psrepr = IMP.atom.get_leaves(self.hier_dict[component_name])
        if (skip_gaussian_in_representation):
            allpsrepr = psrepr
            psrepr = []
            for p in allpsrepr:
                #(protname, is_a_bead) = IMP.pmi.tools.get_prot_name_from_particle(
                #    p, self.hier_dict.keys())
                if (IMP.core.Gaussian.get_is_setup(p)) and not (IMP.atom.Fragment.get_is_setup(p) or IMP.atom.Residue.get_is_setup(p)):
                    continue
                psrepr.append(p)

        import itertools
        reprnames=[p.get_name() for p in psrepr]
        rmfnames=[p.get_name() for p in psrmf]

        if save_file:
            fl=open(component_name+".txt","w")
            for i in itertools.izip_longest(reprnames,rmfnames): fl.write(str(i[0])+","+str(i[1])+"\n")


        if check_number_particles and not representation_name_to_rmf_name_map:
            if len(psrmf) != len(psrepr):
                fl=open(component_name+".txt","w")
                for i in itertools.izip_longest(reprnames,rmfnames): fl.write(str(i[0])+","+str(i[1])+"\n")
                raise ValueError("%s cannot proceed the rmf and the representation don't have the same number of particles; "
                           "particles in rmf: %s particles in the representation: %s" % (str(component_name), str(len(psrmf)), str(len(psrepr))))


        if not representation_name_to_rmf_name_map:
            for n, prmf in enumerate(psrmf):

                prmfname = prmf.get_name()
                preprname = psrepr[n].get_name()
                if force_rigid_update:
                    if IMP.core.RigidBody.get_is_setup(psrepr[n]) \
                       and not IMP.core.RigidBodyMember.get_is_setup(psrepr[n]):
                        continue
                else:
                    if IMP.core.RigidBodyMember.get_is_setup(psrepr[n]):
                        raise ValueError("component %s cannot proceed if rigid bodies were initialized. Use the function before defining the rigid bodies" % component_name)

                self._compare_rmf_repr_names(prmfname, preprname,
                                             component_name)
                if IMP.core.XYZ.get_is_setup(prmf) and IMP.core.XYZ.get_is_setup(psrepr[n]):
                    xyz = IMP.core.XYZ(prmf).get_coordinates()
                    IMP.core.XYZ(psrepr[n]).set_coordinates(xyz)
                    if IMP.core.RigidBodyMember.get_is_setup(psrepr[n]):
                        # Set rigid body so that coordinates are preserved
                        # on future model updates
                        rbm = IMP.core.RigidBodyMember(psrepr[n])
                        rbm.set_internal_coordinates(xyz)
                        tr = IMP.algebra.ReferenceFrame3D()
                        rb = rbm.get_rigid_body()
                        if IMP.core.RigidBodyMember.get_is_setup(rb):
                            raise ValueError("Cannot handle nested "
                                             "rigid bodies yet")
                        rb.set_reference_frame_lazy(tr)
                else:
                    print("set_coordinates_from_rmf: WARNING particles are not XYZ decorated %s %s " % (str(IMP.core.XYZ.get_is_setup(prmf)), str(IMP.core.XYZ.get_is_setup(psrepr[n]))))

                if IMP.core.Gaussian.get_is_setup(prmf) and IMP.core.Gaussian.get_is_setup(psrepr[n]):
                    gprmf=IMP.core.Gaussian(prmf)
                    grepr=IMP.core.Gaussian(psrepr[n])
                    g=gprmf.get_gaussian()
                    grepr.set_gaussian(g)

        else:
            repr_name_particle_map={}
            rmf_name_particle_map={}
            for p in psrmf:
                rmf_name_particle_map[p.get_name()]=p
            #for p in psrepr:
            #    repr_name_particle_map[p.get_name()]=p

            for prepr in psrepr:
                try:
                    prmf=rmf_name_particle_map[representation_name_to_rmf_name_map[prepr.get_name()]]
                except KeyError:
                    print("set_coordinates_from_rmf: WARNING missing particle name in representation_name_to_rmf_name_map, skipping...")
                    continue
                xyz = IMP.core.XYZ(prmf).get_coordinates()
                IMP.core.XYZ(prepr).set_coordinates(xyz)



    def check_root(self, name, protein_h, resolution):
        '''
        If the root hierarchy does not exist, construct it.
        '''
        if "Res:" + str(int(resolution)) not in self.hier_representation[name]:
            root = IMP.atom.Hierarchy.setup_particle(IMP.Particle(self.m))
            root.set_name(name + "_Res:" + str(int(resolution)))
            self.hier_representation[name][
                "Res:" + str(int(resolution))] = root
            protein_h.add_child(root)

    def coarse_hierarchy(self, name, start, end, resolutions, isnucleicacid,
                         input_hierarchy, protein_h, type, color):
        '''
        Generate all needed coarse grained layers.

        @param name name of the protein
        @param resolutions list of resolutions
        @param protein_h root hierarchy
        @param input_hierarchy hierarchy to coarse grain
        @param type a string, typically "pdb" or "helix"
        '''
        outhiers = []

        if (1 in resolutions) or (0 in resolutions):
            # in that case create residues and append atoms

            if 1 in resolutions:
                self.check_root(name, protein_h, 1)
                s1 = IMP.atom.Fragment.setup_particle(IMP.Particle(self.m))
                s1.set_name('%s_%i-%i_%s' % (name, start, end, type))
                # s1.set_residue_indexes(range(start,end+1))
                self.hier_representation[name]["Res:1"].add_child(s1)
                outhiers += [s1]
            if 0 in resolutions:
                self.check_root(name, protein_h, 0)
                s0 = IMP.atom.Fragment.setup_particle(IMP.Particle(self.m))
                s0.set_name('%s_%i-%i_%s' % (name, start, end, type))
                # s0.set_residue_indexes(range(start,end+1))
                self.hier_representation[name]["Res:0"].add_child(s0)
                outhiers += [s0]

            if not isnucleicacid:
                sel = IMP.atom.Selection(
                    input_hierarchy,
                    atom_type=IMP.atom.AT_CA)
            else:
                sel = IMP.atom.Selection(
                    input_hierarchy,
                    atom_type=IMP.atom.AT_P)

            for p in sel.get_selected_particles():
                resobject = IMP.atom.Residue(IMP.atom.Atom(p).get_parent())
                if 0 in resolutions:
                    # if you ask for atoms
                    resclone0 = IMP.atom.create_clone(resobject)
                    resindex = IMP.atom.Residue(resclone0).get_index()
                    s0.add_child(resclone0)
                    self.hier_db.add_particles(
                        name,
                        resindex,
                        0,
                        resclone0.get_children())

                    chil = resclone0.get_children()
                    for ch in chil:
                        IMP.pmi.Resolution.setup_particle(ch, 0)
                        try:
                            clr = IMP.display.get_rgb_color(color)
                        except:
                            clr = IMP.display.get_rgb_color(1.0)
                        IMP.display.Colored.setup_particle(ch, clr)

                if 1 in resolutions:
                    # else clone the residue
                    resclone1 = IMP.atom.create_clone_one(resobject)
                    resindex = IMP.atom.Residue(resclone1).get_index()
                    s1.add_child(resclone1)
                    self.hier_db.add_particles(name, resindex, 1, [resclone1])

                    rt = IMP.atom.Residue(resclone1).get_residue_type()
                    xyz = IMP.core.XYZ(p).get_coordinates()
                    prt = resclone1.get_particle()
                    prt.set_name('%s_%i_%s' % (name, resindex, type))
                    IMP.core.XYZ.setup_particle(prt).set_coordinates(xyz)

                    try:
                        vol = IMP.atom.get_volume_from_residue_type(rt)
                        # mass=IMP.atom.get_mass_from_residue_type(rt)
                    except IMP.ValueException:
                        vol = IMP.atom.get_volume_from_residue_type(
                            IMP.atom.ResidueType("ALA"))
                        # mass=IMP.atom.get_mass_from_residue_type(IMP.atom.ResidueType("ALA"))
                    radius = IMP.algebra.get_ball_radius_from_volume_3d(vol)
                    IMP.core.XYZR.setup_particle(prt).set_radius(radius)
                    IMP.atom.Mass.setup_particle(prt, 100)
                    IMP.pmi.Uncertainty.setup_particle(prt, radius)
                    IMP.pmi.Symmetric.setup_particle(prt, 0)
                    IMP.pmi.Resolution.setup_particle(prt, 1)
                    try:
                        clr = IMP.display.get_rgb_color(color)
                    except:
                        clr = IMP.display.get_rgb_color(1.0)
                    IMP.display.Colored.setup_particle(prt, clr)

        for r in resolutions:
            if r != 0 and r != 1:
                self.check_root(name, protein_h, r)
                s = IMP.atom.create_simplified_along_backbone(
                    input_hierarchy,
                    r)

                chil = s.get_children()
                s0 = IMP.atom.Fragment.setup_particle(IMP.Particle(self.m))
                s0.set_name('%s_%i-%i_%s' % (name, start, end, type))
                # Move all children from s to s0
                for ch in chil:
                    s.remove_child(ch)
                    s0.add_child(ch)
                self.hier_representation[name][
                    "Res:" + str(int(r))].add_child(s0)
                outhiers += [s0]
                IMP.atom.destroy(s)

                for prt in IMP.atom.get_leaves(s0):
                    ri = IMP.atom.Fragment(prt).get_residue_indexes()
                    first = ri[0]
                    last = ri[-1]
                    if first == last:
                        prt.set_name('%s_%i_%s' % (name, first, type))
                    else:
                        prt.set_name('%s_%i_%i_%s' % (name, first, last, type))
                    for kk in ri:
                        self.hier_db.add_particles(name, kk, r, [prt])

                    radius = IMP.core.XYZR(prt).get_radius()
                    IMP.pmi.Uncertainty.setup_particle(prt, radius)
                    IMP.pmi.Symmetric.setup_particle(prt, 0)
                    IMP.pmi.Resolution.setup_particle(prt, r)

                    # setting up color for each particle in the
                    # hierarchy, if colors missing in the colors list set it to
                    # red
                    try:
                        clr = IMP.display.get_rgb_color(color)
                    except:
                        colors.append(1.0)
                        clr = IMP.display.get_rgb_color(colors[pdb_part_count])
                    IMP.display.Colored.setup_particle(prt, clr)

        return outhiers

    def get_hierarchies_at_given_resolution(self, resolution):
        '''
        Get the hierarchies at the given resolution.

        The map between resolution and hierarchies is cached to accelerate
        the selection algorithm. The cache is invalidated when the
        representation was changed by any add_component_xxx.
        '''

        if self.representation_is_modified:
            rhbr = self.hier_db.get_all_root_hierarchies_by_resolution(
                resolution)
            self.hier_resolution[resolution] = rhbr
            self.representation_is_modified = False
            return rhbr
        else:
            if resolution in self.hier_resolution:
                return self.hier_resolution[resolution]
            else:
                rhbr = self.hier_db.get_all_root_hierarchies_by_resolution(
                    resolution)
                self.hier_resolution[resolution] = rhbr
                return rhbr

    def shuffle_configuration(
        self, max_translation=300., max_rotation=2.0 * pi,
        avoidcollision=True, cutoff=10.0, niterations=100,
        bounding_box=None,
        excluded_rigid_bodies=None,
        ignore_initial_coordinates=False,
        hierarchies_excluded_from_collision=None):
        '''
        Shuffle configuration; used to restart the optimization.

        The configuration of the system is initialized by placing each
        rigid body and each bead randomly in a box with a side of
        `max_translation` angstroms, and far enough from each other to
        prevent any steric clashes. The rigid bodies are also randomly rotated.

        @param avoidcollision check if the particle/rigid body was
                  placed close to another particle; uses the optional
                  arguments cutoff and niterations
        @param bounding_box defined by ((x1,y1,z1),(x2,y2,z2))
        '''

        if excluded_rigid_bodies is None:
            excluded_rigid_bodies = []
        if hierarchies_excluded_from_collision is None:
            hierarchies_excluded_from_collision = []

        if len(self.rigid_bodies) == 0:
            print("shuffle_configuration: rigid bodies were not intialized")

        gcpf = IMP.core.GridClosePairsFinder()
        gcpf.set_distance(cutoff)
        ps = []
        hierarchies_excluded_from_collision_indexes = []
        for p in IMP.atom.get_leaves(self.prot):
            if IMP.core.XYZ.get_is_setup(p):
                ps.append(p)
            if IMP.core.Gaussian.get_is_setup(p):
                # remove the densities particles out of the calculation
                hierarchies_excluded_from_collision_indexes += IMP.get_indexes([p])
        allparticleindexes = IMP.get_indexes(ps)

        if bounding_box is not None:
            ((x1, y1, z1), (x2, y2, z2)) = bounding_box
            lb = IMP.algebra.Vector3D(x1, y1, z1)
            ub = IMP.algebra.Vector3D(x2, y2, z2)
            bb = IMP.algebra.BoundingBox3D(lb, ub)

        for h in hierarchies_excluded_from_collision:
            hierarchies_excluded_from_collision_indexes += IMP.get_indexes(IMP.atom.get_leaves(h))


        allparticleindexes = list(
            set(allparticleindexes) - set(hierarchies_excluded_from_collision_indexes))

        print(hierarchies_excluded_from_collision)
        print(len(allparticleindexes),len(hierarchies_excluded_from_collision_indexes))

        print('shuffling', len(self.rigid_bodies) - len(excluded_rigid_bodies), 'rigid bodies')
        for rb in self.rigid_bodies:
            if rb not in excluded_rigid_bodies:
                if avoidcollision:
                    rbindexes = rb.get_member_particle_indexes()
                    rbindexes = list(
                        set(rbindexes) - set(hierarchies_excluded_from_collision_indexes))
                    otherparticleindexes = list(
                        set(allparticleindexes) - set(rbindexes))

                    if len(otherparticleindexes) is None:
                        continue

                niter = 0
                while niter < niterations:
                    if (ignore_initial_coordinates):
                        # Move the particle to the origin
                        transformation = IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(), -IMP.core.XYZ(rb).get_coordinates())
                        IMP.core.transform(rb, transformation)
                    rbxyz = IMP.core.XYZ(rb).get_coordinates()

                    if bounding_box is not None:
                        # overrides the perturbation
                        translation = IMP.algebra.get_random_vector_in(bb)
                        rotation = IMP.algebra.get_random_rotation_3d()
                        transformation = IMP.algebra.Transformation3D(rotation, translation-rbxyz)
                    else:
                        transformation = IMP.algebra.get_random_local_transformation(
                            rbxyz,
                            max_translation,
                            max_rotation)

                    IMP.core.transform(rb, transformation)

                    if avoidcollision:
                        self.m.update()
                        npairs = len(
                            gcpf.get_close_pairs(
                                self.m,
                                otherparticleindexes,
                                rbindexes))
                        if npairs == 0:
                            niter = niterations
                            if (ignore_initial_coordinates):
                                print (rb.get_name(), IMP.core.XYZ(rb).get_coordinates())
                        else:
                            niter += 1
                            print("shuffle_configuration: rigid body placed close to other %d particles, trying again..." % npairs)
                            print("shuffle_configuration: rigid body name: " + rb.get_name())
                            if niter == niterations:
                                raise ValueError("tried the maximum number of iterations to avoid collisions, increase the distance cutoff")
                    else:
                        break

        print('shuffling', len(self.floppy_bodies), 'floppy bodies')
        for fb in self.floppy_bodies:
            if (avoidcollision):
                rm = not IMP.core.RigidMember.get_is_setup(fb)
                nrm = not IMP.core.NonRigidMember.get_is_setup(fb)
                if rm and nrm:
                    fbindexes = IMP.get_indexes([fb])
                    otherparticleindexes = list(
                        set(allparticleindexes) - set(fbindexes))
                    if len(otherparticleindexes) is None:
                        continue
                else:
                    continue
            else:
                rm = IMP.core.RigidMember.get_is_setup(fb)
                nrm = IMP.core.NonRigidMember.get_is_setup(fb)
                if (rm or nrm):
                    continue

            if IMP.core.RigidBodyMember.get_is_setup(fb):
                d=IMP.core.RigidBodyMember(fb).get_rigid_body()
            elif IMP.core.RigidBody.get_is_setup(fb):
                d=IMP.core.RigidBody(fb)
            elif IMP.core.XYZ.get_is_setup(fb):
                d=IMP.core.XYZ(fb)

            niter = 0
            while niter < niterations:
                if (ignore_initial_coordinates):
                    # Move the particle to the origin
                    transformation = IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(), -IMP.core.XYZ(fb).get_coordinates())
                    IMP.core.transform(d, transformation)
                fbxyz = IMP.core.XYZ(fb).get_coordinates()

                if bounding_box is not None:
                    # overrides the perturbation
                    translation = IMP.algebra.get_random_vector_in(bb)
                    transformation = IMP.algebra.Transformation3D(translation-fbxyz)
                else:
                    transformation = IMP.algebra.get_random_local_transformation(
                        fbxyz,
                        max_translation,
                        max_rotation)

                IMP.core.transform(d, transformation)

                if (avoidcollision):
                    self.m.update()
                    npairs = len(
                        gcpf.get_close_pairs(
                            self.m,
                            otherparticleindexes,
                            fbindexes))
                    if npairs == 0:
                        niter = niterations
                        if (ignore_initial_coordinates):
                            print (fb.get_name(), IMP.core.XYZ(fb).get_coordinates())
                    else:
                        niter += 1
                        print("shuffle_configuration: floppy body placed close to other %d particles, trying again..." % npairs)
                        print("shuffle_configuration: floppy body name: " + fb.get_name())
                        if niter == niterations:
                            raise ValueError("tried the maximum number of iterations to avoid collisions, increase the distance cutoff")
                else:
                    break

    def set_current_coordinates_as_reference_for_rmsd(self, label="None"):
        # getting only coordinates from pdb
        ps = IMP.pmi.tools.select(self, resolution=1.0)
        # storing the reference coordinates and the particles
        self.reference_structures[label] = (
            [IMP.core.XYZ(p).get_coordinates() for p in ps],
            ps)

    def get_all_rmsds(self):
        rmsds = {}

        for label in self.reference_structures:

            current_coordinates = [IMP.core.XYZ(p).get_coordinates()
                                   for p in self.reference_structures[label][1]]
            reference_coordinates = self.reference_structures[label][0]
            if len(reference_coordinates) != len(current_coordinates):
                print("calculate_all_rmsds: reference and actual coordinates are not the same")
                continue
            transformation = IMP.algebra.get_transformation_aligning_first_to_second(
                current_coordinates,
                reference_coordinates)
            rmsd_global = IMP.algebra.get_rmsd(
                reference_coordinates,
                current_coordinates)
            # warning: temporary we are calculating the drms, and not the rmsd,
            # for the relative distance
            rmsd_relative = IMP.atom.get_drms(
                reference_coordinates,
                current_coordinates)
            rmsds[label + "_GlobalRMSD"] = rmsd_global
            rmsds[label + "_RelativeDRMS"] = rmsd_relative
        return rmsds

    def setup_component_geometry(self, name, color=None, resolution=1.0):
        if color is None:
            color = self.color_dict[name]
        # this function stores all particle pairs
        # ordered by residue number, to be used
        # to construct backbone traces
        self.hier_geometry_pairs[name] = []
        protein_h = self.hier_dict[name]
        pbr = IMP.pmi.tools.select(self, name=name, resolution=resolution)
        pbr = IMP.pmi.tools.sort_by_residues(pbr)

        for n in range(len(pbr) - 1):
            self.hier_geometry_pairs[name].append((pbr[n], pbr[n + 1], color))

    def setup_component_sequence_connectivity(
        self, name, resolution=10, scale=1.0):
        '''
        Generate restraints between contiguous fragments in the hierarchy.
        The linkers are generated at resolution 10 by default.

        '''

        unmodeledregions_cr = IMP.RestraintSet(self.m, "unmodeledregions")
        sortedsegments_cr = IMP.RestraintSet(self.m, "sortedsegments")

        protein_h = self.hier_dict[name]
        SortedSegments = []
        frs = self.hier_db.get_preroot_fragments_by_resolution(
            name,
            resolution)

        for fr in frs:
            try:
                start = fr.get_children()[0]
            except:
                start = fr

            try:
                end = fr.get_children()[-1]
            except:
                end = fr

            startres = IMP.pmi.tools.get_residue_indexes(start)[0]
            endres = IMP.pmi.tools.get_residue_indexes(end)[-1]
            SortedSegments.append((start, end, startres))
        SortedSegments = sorted(SortedSegments, key=itemgetter(2))

        # connect the particles
        for x in range(len(SortedSegments) - 1):
            last = SortedSegments[x][1]
            first = SortedSegments[x + 1][0]

            nreslast = len(IMP.pmi.tools.get_residue_indexes(last))
            lastresn = IMP.pmi.tools.get_residue_indexes(last)[-1]
            nresfirst = len(IMP.pmi.tools.get_residue_indexes(first))
            firstresn = IMP.pmi.tools.get_residue_indexes(first)[0]

            residuegap = firstresn - lastresn - 1
            if self.disorderedlength and (nreslast / 2 + nresfirst / 2 + residuegap) > 20.0:
                # calculate the distance between the sphere centers using Kohn
                # PNAS 2004
                optdist = sqrt(5 / 3) * 1.93 * \
                    (nreslast / 2 + nresfirst / 2 + residuegap) ** 0.6
                # optdist2=sqrt(5/3)*1.93*((nreslast)**0.6+(nresfirst)**0.6)/2
                if self.upperharmonic:
                    hu = IMP.core.HarmonicUpperBound(optdist, self.kappa)
                else:
                    hu = IMP.core.Harmonic(optdist, self.kappa)
                dps = IMP.core.DistancePairScore(hu)
            else:  # default
                optdist = (0.0 + (float(residuegap) + 1.0) * 3.6) * scale
                if self.upperharmonic:  # default
                    hu = IMP.core.HarmonicUpperBound(optdist, self.kappa)
                else:
                    hu = IMP.core.Harmonic(optdist, self.kappa)
                dps = IMP.core.SphereDistancePairScore(hu)

            pt0 = last.get_particle()
            pt1 = first.get_particle()
            r = IMP.core.PairRestraint(self.m, dps, (pt0.get_index(), pt1.get_index()))

            print("Adding sequence connectivity restraint between", pt0.get_name(), " and ", pt1.get_name(), 'of distance', optdist)
            sortedsegments_cr.add_restraint(r)
            self.linker_restraints_dict[
                "LinkerRestraint-" + pt0.get_name() + "-" + pt1.get_name()] = r
            self.connected_intra_pairs.append((pt0, pt1))
            self.connected_intra_pairs.append((pt1, pt0))

        self.linker_restraints.add_restraint(sortedsegments_cr)
        self.linker_restraints.add_restraint(unmodeledregions_cr)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.linker_restraints)
        self.sortedsegments_cr_dict[name] = sortedsegments_cr
        self.unmodeledregions_cr_dict[name] = unmodeledregions_cr

    def optimize_floppy_bodies(self, nsteps, temperature=1.0):
        import IMP.pmi.samplers
        pts = IMP.pmi.tools.ParticleToSampleList()
        for n, fb in enumerate(self.floppy_bodies):
            pts.add_particle(fb, "Floppy_Bodies", 1.0, "Floppy_Body_" + str(n))
        if len(pts.get_particles_to_sample()) > 0:
            mc = IMP.pmi.samplers.MonteCarlo(self.m, [pts], temperature)
            print("optimize_floppy_bodies: optimizing %i floppy bodies" % len(self.floppy_bodies))
            mc.optimize(nsteps)
        else:
            print("optimize_floppy_bodies: no particle to optimize")

    def create_rotational_symmetry(self, maincopy, copies, rotational_axis=IMP.algebra.Vector3D(0, 0, 1.0),
                                   nSymmetry=None, skip_gaussian_in_clones=False):
        '''
        The copies must not contain rigid bodies.
        The symmetry restraints are applied at each leaf.
        '''

        from math import pi
        self.representation_is_modified = True
        ncopies = len(copies) + 1
        main_hiers = IMP.atom.get_leaves(self.hier_dict[maincopy])

        for k in range(len(copies)):
            if (nSymmetry is None):
                rotation_angle = 2.0 * pi / float(ncopies) * float(k + 1)
            else:
                if ( k % 2 == 0 ):
                    rotation_angle = 2.0 * pi / float(nSymmetry) * float((k + 2) / 2)
                else:
                    rotation_angle = -2.0 * pi / float(nSymmetry) * float((k + 1) / 2)

            rotation3D = IMP.algebra.get_rotation_about_axis(rotational_axis, rotation_angle)

            sm = IMP.core.TransformationSymmetry(rotation3D)
            clone_hiers = IMP.atom.get_leaves(self.hier_dict[copies[k]])

            lc = IMP.container.ListSingletonContainer(self.m)
            for n, p in enumerate(main_hiers):
                if (skip_gaussian_in_clones):
                    if (IMP.core.Gaussian.get_is_setup(p)) and not (IMP.atom.Fragment.get_is_setup(p) or IMP.atom.Residue.get_is_setup(p)):
                        continue
                pc = clone_hiers[n]
                #print("setting " + p.get_name() + " as reference for " + pc.get_name())
                IMP.core.Reference.setup_particle(pc.get_particle(), p.get_particle())
                lc.add(pc.get_particle().get_index())

            c = IMP.container.SingletonsConstraint(sm, None, lc)
            self.m.add_score_state(c)
            print("Completed setting " + str(maincopy) + " as a reference for " + str(copies[k]) \
                   + " by rotating it in " + str(rotation_angle / 2.0 / pi * 360) + " degree around the " + str(rotational_axis) + " axis.")
        self.m.update()

    def create_rigid_body_symmetry(self, particles_reference, particles_copy,label="None",
                    initial_transformation=IMP.algebra.get_identity_transformation_3d()):
        from math import pi
        self.representation_is_modified = True

        mainparticles = particles_reference

        t=initial_transformation
        p=IMP.Particle(self.m)
        p.set_name("RigidBody_Symmetry")
        rb=IMP.core.RigidBody.setup_particle(p,IMP.algebra.ReferenceFrame3D(t))

        sm = IMP.core.TransformationSymmetry(rb)


        copyparticles = particles_copy

        mainpurged = []
        copypurged = []
        for n, p in enumerate(mainparticles):
            print(p.get_name())
            pc = copyparticles[n]

            mainpurged.append(p)
            if not IMP.pmi.Symmetric.get_is_setup(p):
                IMP.pmi.Symmetric.setup_particle(p, 0)
            else:
                IMP.pmi.Symmetric(p).set_symmetric(0)

            copypurged.append(pc)
            if not IMP.pmi.Symmetric.get_is_setup(pc):
                IMP.pmi.Symmetric.setup_particle(pc, 1)
            else:
                IMP.pmi.Symmetric(pc).set_symmetric(1)

        lc = IMP.container.ListSingletonContainer(self.m)
        for n, p in enumerate(mainpurged):

            pc = copypurged[n]
            print("setting " + p.get_name() + " as reference for " + pc.get_name())

            IMP.core.Reference.setup_particle(pc, p)
            lc.add(pc.get_index())

        c = IMP.container.SingletonsConstraint(sm, None, lc)
        self.m.add_score_state(c)

        self.m.update()
        self.rigid_bodies.append(rb)
        self.rigid_body_symmetries.append(rb)
        rb.set_name(label+".rigid_body_symmetry."+str(len(self.rigid_body_symmetries)))


    def create_amyloid_fibril_symmetry(self, maincopy, axial_copies,
                                       longitudinal_copies, axis=(0, 0, 1), translation_value=4.8):

        from math import pi
        self.representation_is_modified = True

        outhiers = []
        protein_h = self.hier_dict[maincopy]
        mainparts = IMP.atom.get_leaves(protein_h)

        for ilc in range(-longitudinal_copies, longitudinal_copies + 1):
            for iac in range(axial_copies):
                copyname = maincopy + "_a" + str(ilc) + "_l" + str(iac)
                self.create_component(copyname, 0.0)
                for hier in protein_h.get_children():
                    self.add_component_hierarchy_clone(copyname, hier)
                copyhier = self.hier_dict[copyname]
                outhiers.append(copyhier)
                copyparts = IMP.atom.get_leaves(copyhier)
                rotation3D = IMP.algebra.get_rotation_about_axis(
                    IMP.algebra.Vector3D(axis),
                    2 * pi / axial_copies * (float(iac)))
                translation_vector = tuple(
                    [translation_value * float(ilc) * x for x in axis])
                print(translation_vector)
                translation = IMP.algebra.Vector3D(translation_vector)
                sm = IMP.core.TransformationSymmetry(
                    IMP.algebra.Transformation3D(rotation3D, translation))
                lc = IMP.container.ListSingletonContainer(self.m)
                for n, p in enumerate(mainparts):
                    pc = copyparts[n]
                    if not IMP.pmi.Symmetric.get_is_setup(p):
                        IMP.pmi.Symmetric.setup_particle(p, 0)
                    if not IMP.pmi.Symmetric.get_is_setup(pc):
                        IMP.pmi.Symmetric.setup_particle(pc, 1)
                    IMP.core.Reference.setup_particle(pc, p)
                    lc.add(pc.get_index())
                c = IMP.container.SingletonsConstraint(sm, None, lc)
                self.m.add_score_state(c)
                self.m.update()
        return outhiers

    def link_components_to_rmf(self, rmfname, frameindex):
        '''
        Load coordinates in the current representation.
        This should be done only if the hierarchy self.prot is identical
        to the one as stored in the rmf i.e. all components were added.
        '''
        import IMP.rmf
        import RMF

        rh = RMF.open_rmf_file_read_only(rmfname)
        IMP.rmf.link_hierarchies(rh, [self.prot])
        IMP.rmf.load_frame(rh, RMF.FrameID(frameindex))
        del rh

    def create_components_from_rmf(self, rmfname, frameindex):
        '''
        still not working.
        create the representation (i.e. hierarchies) from the rmf file.
        it will be stored in self.prot, which will be overwritten.
        load the coordinates from the rmf file at frameindex.
        '''
        rh = RMF.open_rmf_file_read_only(rmfname)
        self.prot = IMP.rmf.create_hierarchies(rh, self.m)[0]
        IMP.atom.show_molecular_hierarchy(self.prot)
        IMP.rmf.link_hierarchies(rh, [self.prot])
        IMP.rmf.load_frame(rh, RMF.FrameID(frameindex))
        del rh
        for p in self.prot.get_children():
            self.create_component(p.get_name())
            self.hier_dict[p.get_name()] = p
        '''
        still missing: save rigid bodies contained in the rmf in self.rigid_bodies
        save floppy bodies in self.floppy_bodies
        get the connectivity restraints
        '''

    def set_rigid_body_from_hierarchies(self, hiers, particles=None):
        '''
        Construct a rigid body from hierarchies (and optionally particles).

        @param hiers list of hierarchies to make rigid
        @param particles list of particles to add to the rigid body
        '''

        if particles is None:
            rigid_parts = set()
        else:
            rigid_parts = set(particles)

        name = ""
        print("set_rigid_body_from_hierarchies> setting up a new rigid body")
        for hier in hiers:
            ps = IMP.atom.get_leaves(hier)

            for p in ps:
                if IMP.core.RigidMember.get_is_setup(p):
                    rb = IMP.core.RigidMember(p).get_rigid_body()
                    print("set_rigid_body_from_hierarchies> WARNING particle %s already belongs to rigid body %s" % (p.get_name(), rb.get_name()))
                else:
                    rigid_parts.add(p)
            name += hier.get_name() + "-"
            print("set_rigid_body_from_hierarchies> adding %s to the rigid body" % hier.get_name())

        if len(list(rigid_parts)) != 0:
            rb = IMP.atom.create_rigid_body(list(rigid_parts))
            rb.set_coordinates_are_optimized(True)
            rb.set_name(name + "rigid_body")
            self.rigid_bodies.append(rb)
            return rb

        else:
            print("set_rigid_body_from_hierarchies> rigid body could not be setup")

    def set_rigid_bodies(self, subunits):
        '''
        Construct a rigid body from a list of subunits.

        Each subunit is a tuple that identifies the residue ranges and the
        component name (as used in create_component()).

        subunits: [(name_1,(first_residue_1,last_residue_1)),(name_2,(first_residue_2,last_residue_2)),.....]
                  or
                  [name_1,name_2,(name_3,(first_residue_3,last_residue_3)),.....]

                   example: ["prot1","prot2",("prot3",(1,10))]

        sometimes, we know about structure of an interaction
        and here we make such PPIs rigid
        '''

        rigid_parts = set()
        for s in subunits:
            if type(s) == type(tuple()) and len(s) == 2:
                sel = IMP.atom.Selection(
                    self.prot,
                    molecule=s[0],
                    residue_indexes=list(range(s[1][0],
                                          s[1][1] + 1)))
                if len(sel.get_selected_particles()) == 0:
                    print("set_rigid_bodies: selected particle does not exist")
                for p in sel.get_selected_particles():
                    # if not p in self.floppy_bodies:
                    if IMP.core.RigidMember.get_is_setup(p):
                        rb = IMP.core.RigidMember(p).get_rigid_body()
                        print("set_rigid_body_from_hierarchies> WARNING particle %s already belongs to rigid body %s" % (p.get_name(), rb.get_name()))
                    else:
                        rigid_parts.add(p)

            elif type(s) == type(str()):
                sel = IMP.atom.Selection(self.prot, molecule=s)
                if len(sel.get_selected_particles()) == 0:
                    print("set_rigid_bodies: selected particle does not exist")
                for p in sel.get_selected_particles():
                    # if not p in self.floppy_bodies:
                    if IMP.core.RigidMember.get_is_setup(p):
                        rb = IMP.core.RigidMember(p).get_rigid_body()
                        print("set_rigid_body_from_hierarchies> WARNING particle %s already belongs to rigid body %s" % (p.get_name(), rb.get_name()))
                    else:
                        rigid_parts.add(p)

        rb = IMP.atom.create_rigid_body(list(rigid_parts))
        rb.set_coordinates_are_optimized(True)
        rb.set_name(''.join(str(subunits)) + "_rigid_body")
        self.rigid_bodies.append(rb)
        return rb

    def set_super_rigid_body_from_hierarchies(
        self,
        hiers,
        axis=None,
        min_size=1):
        # axis is the rotation axis for 2D rotation
        super_rigid_xyzs = set()
        super_rigid_rbs = set()
        name = ""
        print("set_super_rigid_body_from_hierarchies> setting up a new SUPER rigid body")

        for hier in hiers:
            ps = IMP.atom.get_leaves(hier)
            for p in ps:
                if IMP.core.RigidMember.get_is_setup(p):
                    rb = IMP.core.RigidMember(p).get_rigid_body()
                    super_rigid_rbs.add(rb)
                else:
                    super_rigid_xyzs.add(p)
            print("set_rigid_body_from_hierarchies> adding %s to the rigid body" % hier.get_name())
        if len(super_rigid_rbs|super_rigid_xyzs) < min_size:
            return
        if axis is None:
            self.super_rigid_bodies.append((super_rigid_xyzs, super_rigid_rbs))
        else:
            # these will be 2D rotation SRB or a bond rotamer (axis can be a IMP.algebra.Vector3D or particle Pair)
            self.super_rigid_bodies.append(
                (super_rigid_xyzs, super_rigid_rbs, axis))

    def fix_rigid_bodies(self, rigid_bodies):
        self.fixed_rigid_bodies += rigid_bodies


    def set_chain_of_super_rigid_bodies(
        self, hiers, min_length=None, max_length=None, axis=None):
        '''
        Make a chain of super rigid bodies from a list of hierarchies.

        Takes a linear list of hierarchies (they are supposed to be
        sequence-contiguous) and produces a chain of super rigid bodies
        with given length range, specified by min_length and max_length.
        '''
        try:
            hiers = IMP.pmi.tools.flatten_list(hiers)
        except:
            pass
        for hs in IMP.pmi.tools.sublist_iterator(hiers, min_length, max_length):
            self.set_super_rigid_body_from_hierarchies(hs, axis, min_length)

    def set_super_rigid_bodies(self, subunits, coords=None):
        super_rigid_xyzs = set()
        super_rigid_rbs = set()

        for s in subunits:
            if type(s) == type(tuple()) and len(s) == 3:
                sel = IMP.atom.Selection(
                    self.prot,
                    molecule=s[2],
                    residue_indexes=list(range(s[0],
                                          s[1] + 1)))
                if len(sel.get_selected_particles()) == 0:
                    print("set_rigid_bodies: selected particle does not exist")
                for p in sel.get_selected_particles():
                    if IMP.core.RigidMember.get_is_setup(p):
                        rb = IMP.core.RigidMember(p).get_rigid_body()
                        super_rigid_rbs.add(rb)
                    else:
                        super_rigid_xyzs.add(p)
            elif type(s) == type(str()):
                sel = IMP.atom.Selection(self.prot, molecule=s)
                if len(sel.get_selected_particles()) == 0:
                    print("set_rigid_bodies: selected particle does not exist")
                for p in sel.get_selected_particles():
                    # if not p in self.floppy_bodies:
                    if IMP.core.RigidMember.get_is_setup(p):
                        rb = IMP.core.RigidMember(p).get_rigid_body()
                        super_rigid_rbs.add(rb)
                    else:
                        super_rigid_xyzs.add(p)
        self.super_rigid_bodies.append((super_rigid_xyzs, super_rigid_rbs))

    def remove_floppy_bodies_from_component(self, component_name):
        '''
        Remove leaves of hierarchies from the floppy bodies list based
        on the component name
        '''
        hs=IMP.atom.get_leaves(self.hier_dict[component_name])
        ps=[h.get_particle() for h in hs]
        for p in self.floppy_bodies:
            try:
                if p in ps: self.floppy_bodies.remove(p)
                if p in hs: self.floppy_bodies.remove(p)
            except:
                continue


    def remove_floppy_bodies(self, hierarchies):
        '''
        Remove leaves of hierarchies from the floppy bodies list.

        Given a list of hierarchies, get the leaves and remove the
        corresponding particles from the floppy bodies list. We need this
        function because sometimes
        we want to constrain the floppy bodies in a rigid body - for instance
        when you want to associate a bead with a density particle.
        '''
        for h in hierarchies:
            ps = IMP.atom.get_leaves(h)
            for p in ps:
                if p in self.floppy_bodies:
                    print("remove_floppy_bodies: removing %s from floppy body list" % p.get_name())
                    self.floppy_bodies.remove(p)


    def set_floppy_bodies(self):
        for p in self.floppy_bodies:
            name = p.get_name()
            p.set_name(name + "_floppy_body")
            if IMP.core.RigidMember.get_is_setup(p):
                print("I'm trying to make this particle flexible although it was assigned to a rigid body", p.get_name())
                rb = IMP.core.RigidMember(p).get_rigid_body()
                try:
                    rb.set_is_rigid_member(p.get_index(), False)
                except:
                    # some IMP versions still work with that
                    rb.set_is_rigid_member(p.get_particle_index(), False)
                p.set_name(p.get_name() + "_rigid_body_member")

    def set_floppy_bodies_from_hierarchies(self, hiers):
        for hier in hiers:
            ps = IMP.atom.get_leaves(hier)
            for p in ps:
                IMP.core.XYZ(p).set_coordinates_are_optimized(True)
                self.floppy_bodies.append(p)

    def get_particles_from_selection_tuples(
        self,
        selection_tuples,
            resolution=None):
        '''
        selection tuples must be [(r1,r2,"name1"),(r1,r2,"name2"),....]
        @return the particles
        '''
        particles = []
        print(selection_tuples)
        for s in selection_tuples:
            ps = IMP.pmi.tools.select_by_tuple(
                representation=self, tupleselection=tuple(s),
                resolution=None, name_is_ambiguous=False)
            particles += ps
        return particles

    def get_connected_intra_pairs(self):
        return self.connected_intra_pairs

    def set_rigid_bodies_max_trans(self, maxtrans):
        self.maxtrans_rb = maxtrans

    def set_rigid_bodies_max_rot(self, maxrot):
        self.maxrot_rb = maxrot

    def set_super_rigid_bodies_max_trans(self, maxtrans):
        self.maxtrans_srb = maxtrans

    def set_super_rigid_bodies_max_rot(self, maxrot):
        self.maxrot_srb = maxrot

    def set_floppy_bodies_max_trans(self, maxtrans):
        self.maxtrans_fb = maxtrans

    def set_rigid_bodies_as_fixed(self, rigidbodiesarefixed=True):
        '''
        Fix rigid bodies in their actual position.
        The get_particles_to_sample() function will return
        just the floppy bodies (if they are not fixed).
        '''
        self.rigidbodiesarefixed = rigidbodiesarefixed

    def set_floppy_bodies_as_fixed(self, floppybodiesarefixed=True):
        '''
        Fix floppy bodies in their actual position.
        The get_particles_to_sample() function will return
        just the rigid bodies (if they are not fixed).
        '''
        self.floppybodiesarefixed=floppybodiesarefixed

    def draw_hierarchy_graph(self):
        for c in IMP.atom.Hierarchy(self.prot).get_children():
            print("Drawing hierarchy graph for " + c.get_name())
            IMP.pmi.output.get_graph_from_hierarchy(c)

    def get_geometries(self):
        # create segments at the lowest resolution
        seggeos = []
        for name in self.hier_geometry_pairs:
            for pt in self.hier_geometry_pairs[name]:
                p1 = pt[0]
                p2 = pt[1]
                color = pt[2]
                try:
                    clr = IMP.display.get_rgb_color(color)
                except:
                    clr = IMP.display.get_rgb_color(1.0)
                coor1 = IMP.core.XYZ(p1).get_coordinates()
                coor2 = IMP.core.XYZ(p2).get_coordinates()
                seg = IMP.algebra.Segment3D(coor1, coor2)
                seggeos.append(IMP.display.SegmentGeometry(seg, clr))
        return seggeos

    def setup_bonds(self):
        # create segments at the lowest resolution
        seggeos = []
        for name in self.hier_geometry_pairs:
            for pt in self.hier_geometry_pairs[name]:

                p1 = pt[0]
                p2 = pt[1]
                if not IMP.atom.Bonded.get_is_setup(p1):
                    IMP.atom.Bonded.setup_particle(p1)
                if not IMP.atom.Bonded.get_is_setup(p2):
                    IMP.atom.Bonded.setup_particle(p2)

                if not IMP.atom.get_bond(IMP.atom.Bonded(p1),IMP.atom.Bonded(p2)):
                    IMP.atom.create_bond(
                        IMP.atom.Bonded(p1),
                        IMP.atom.Bonded(p2),
                        1)

    def show_component_table(self, name):
        if name in self.sequence_dict:
            lastresn = len(self.sequence_dict[name])
            firstresn = 1
        else:
            residues = self.hier_db.get_residue_numbers(name)
            firstresn = min(residues)
            lastresn = max(residues)

        for nres in range(firstresn, lastresn + 1):
            try:
                resolutions = self.hier_db.get_residue_resolutions(name, nres)
                ps = []
                for r in resolutions:
                    ps += self.hier_db.get_particles(name, nres, r)
                print("%20s %7s" % (name, nres), " ".join(["%20s %7s" % (str(p.get_name()),
                                                                         str(IMP.pmi.Resolution(p).get_resolution())) for p in ps]))
            except:
                print("%20s %20s" % (name, nres), "**** not represented ****")

    def draw_hierarchy_composition(self):

        ks = list(self.elements.keys())
        ks.sort()

        max = 0
        for k in self.elements:
            for l in self.elements[k]:
                if l[1] > max:
                    max = l[1]

        for k in ks:
            self.draw_component_composition(k, max)

    def draw_component_composition(self, name, max=1000, draw_pdb_names=False):
        from matplotlib import pyplot
        import matplotlib as mpl
        k = name
        tmplist = sorted(self.elements[k], key=itemgetter(0))
        try:
            endres = tmplist[-1][1]
        except:
            print("draw_component_composition: missing information for component %s" % name)
            return
        fig = pyplot.figure(figsize=(26.0 * float(endres) / max + 2, 2))
        ax = fig.add_axes([0.05, 0.475, 0.9, 0.15])

        # Set the colormap and norm to correspond to the data for which
        # the colorbar will be used.
        cmap = mpl.cm.cool
        norm = mpl.colors.Normalize(vmin=5, vmax=10)
        bounds = [1]
        colors = []

        for n, l in enumerate(tmplist):
            firstres = l[0]
            lastres = l[1]
            if l[3] != "end":
                if bounds[-1] != l[0]:
                    colors.append("white")
                    bounds.append(l[0])
                    if l[3] == "pdb":
                        colors.append("#99CCFF")
                    if l[3] == "bead":
                        colors.append("#FFFF99")
                    if l[3] == "helix":
                        colors.append("#33CCCC")
                    if l[3] != "end":
                        bounds.append(l[1] + 1)
                else:
                    if l[3] == "pdb":
                        colors.append("#99CCFF")
                    if l[3] == "bead":
                        colors.append("#FFFF99")
                    if l[3] == "helix":
                        colors.append("#33CCCC")
                    if l[3] != "end":
                        bounds.append(l[1] + 1)
            else:
                if bounds[-1] - 1 == l[0]:
                    bounds.pop()
                    bounds.append(l[0])
                else:
                    colors.append("white")
                    bounds.append(l[0])

        bounds.append(bounds[-1])
        colors.append("white")
        cmap = mpl.colors.ListedColormap(colors)
        cmap.set_over('0.25')
        cmap.set_under('0.75')

        norm = mpl.colors.BoundaryNorm(bounds, cmap.N)
        cb2 = mpl.colorbar.ColorbarBase(ax, cmap=cmap,
                                        norm=norm,
                                        # to use 'extend', you must
                                        # specify two extra boundaries:
                                        boundaries=bounds,
                                        ticks=bounds,  # optional
                                        spacing='proportional',
                                        orientation='horizontal')

        extra_artists = []
        npdb = 0

        if draw_pdb_names:
            for l in tmplist:
                if l[3] == "pdb":
                    npdb += 1
                    mid = 1.0 / endres * float(l[0])
                    # t =ax.text(mid, float(npdb-1)/2.0+1.5, l[2], ha="left", va="center", rotation=0,
                    # size=10)
                    # t=ax.annotate(l[0],2)
                    t = ax.annotate(
                        l[2], xy=(mid, 1),  xycoords='axes fraction',
                        xytext=(mid + 0.025, float(npdb - 1) / 2.0 + 1.5), textcoords='axes fraction',
                        arrowprops=dict(arrowstyle="->",
                                        connectionstyle="angle,angleA=0,angleB=90,rad=10"),
                    )
                    extra_artists.append(t)

        # set the title of the bar
        title = ax.text(-0.005, 0.5, k, ha="right", va="center", rotation=90,
                        size=20)

        extra_artists.append(title)
        # changing the xticks labels
        labels = len(bounds) * [" "]
        ax.set_xticklabels(labels)
        mid = 1.0 / endres * float(bounds[0])
        t = ax.annotate(bounds[0], xy=(mid, 0),  xycoords='axes fraction',
                        xytext=(mid - 0.01, -0.5), textcoords='axes fraction',)
        extra_artists.append(t)
        offsets = [0, -0.5, -1.0]
        nclashes = 0
        for n in range(1, len(bounds)):
            if bounds[n] == bounds[n - 1]:
                continue
            mid = 1.0 / endres * float(bounds[n])
            if (float(bounds[n]) - float(bounds[n - 1])) / max <= 0.01:
                nclashes += 1
                offset = offsets[nclashes % 3]
            else:
                nclashes = 0
                offset = offsets[0]
            if offset > -0.75:
                t = ax.annotate(
                    bounds[n], xy=(mid, 0),  xycoords='axes fraction',
                    xytext=(mid, -0.5 + offset), textcoords='axes fraction')
            else:
                t = ax.annotate(
                    bounds[n], xy=(mid, 0),  xycoords='axes fraction',
                    xytext=(mid, -0.5 + offset), textcoords='axes fraction', arrowprops=dict(arrowstyle="-"))
            extra_artists.append(t)

        cb2.add_lines(bounds, ["black"] * len(bounds), [1] * len(bounds))
        # cb2.set_label(k)

        pyplot.savefig(
            k + "structure.pdf",
            dpi=150,
            transparent="True",
            bbox_extra_artists=(extra_artists),
            bbox_inches='tight')
        pyplot.show()

    def draw_coordinates_projection(self):
        import matplotlib.pyplot as pp
        xpairs = []
        ypairs = []
        for name in self.hier_geometry_pairs:
            for pt in self.hier_geometry_pairs[name]:
                p1 = pt[0]
                p2 = pt[1]
                color = pt[2]
                coor1 = IMP.core.XYZ(p1).get_coordinates()
                coor2 = IMP.core.XYZ(p2).get_coordinates()
                x1 = coor1[0]
                x2 = coor2[0]
                y1 = coor1[1]
                y2 = coor2[1]
                xpairs.append([x1, x2])
                ypairs.append([y1, y2])
        xlist = []
        ylist = []
        for xends, yends in zip(xpairs, ypairs):
            xlist.extend(xends)
            xlist.append(None)
            ylist.extend(yends)
            ylist.append(None)
        pp.plot(xlist, ylist, 'b-', alpha=0.1)
        pp.show()

    def get_prot_name_from_particle(self, particle):
        names = self.get_component_names()
        particle0 = particle
        name = None
        while not name in names:
            h = IMP.atom.Hierarchy(particle0).get_parent()
            name = h.get_name()
            particle0 = h.get_particle()
        return name


    def get_particles_to_sample(self):
        # get the list of samplable particles with their type
        # and the mover displacement. Everything wrapped in a dictionary,
        # to be used by samplers modules
        ps = {}

        # remove symmetric particles: they are not sampled
        rbtmp = []
        fbtmp = []
        srbtmp = []
        if not self.rigidbodiesarefixed:
            for rb in self.rigid_bodies:
                if IMP.pmi.Symmetric.get_is_setup(rb):
                    if IMP.pmi.Symmetric(rb).get_symmetric() != 1:
                        rbtmp.append(rb)
                else:
                    if rb not in self.fixed_rigid_bodies:
                        rbtmp.append(rb)

        if not self.floppybodiesarefixed:
            for fb in self.floppy_bodies:
                if IMP.pmi.Symmetric.get_is_setup(fb):
                    if IMP.pmi.Symmetric(fb).get_symmetric() != 1:
                        fbtmp.append(fb)
                else:
                    fbtmp.append(fb)

        for srb in self.super_rigid_bodies:
            # going to prune the fixed rigid bodies out
            # of the super rigid body list
            rigid_bodies = list(srb[1])
            filtered_rigid_bodies = []
            for rb in rigid_bodies:
                if rb not in self.fixed_rigid_bodies:
                    filtered_rigid_bodies.append(rb)
            srbtmp.append((srb[0], filtered_rigid_bodies))

        self.rigid_bodies = rbtmp
        self.floppy_bodies = fbtmp
        self.super_rigid_bodies = srbtmp

        ps["Rigid_Bodies_SimplifiedModel"] = (
            self.rigid_bodies,
            self.maxtrans_rb,
            self.maxrot_rb)
        ps["Floppy_Bodies_SimplifiedModel"] = (
            self.floppy_bodies,
            self.maxtrans_fb)
        ps["SR_Bodies_SimplifiedModel"] = (
            self.super_rigid_bodies,
            self.maxtrans_srb,
            self.maxrot_srb)
        return ps

    def set_output_level(self, level):
        self.output_level = level

    def _evaluate(self, deriv):
        """Evaluate the total score of all added restraints"""
        r = IMP.pmi.tools.get_restraint_set(self.m)
        return r.evaluate(deriv)

    def get_output(self):
        output = {}
        score = 0.0

        output["SimplifiedModel_Total_Score_" +
               self.label] = str(self._evaluate(False))
        output["SimplifiedModel_Linker_Score_" +
               self.label] = str(self.linker_restraints.unprotected_evaluate(None))
        for name in self.sortedsegments_cr_dict:
            partialscore = self.sortedsegments_cr_dict[name].evaluate(False)
            score += partialscore
            output[
                "SimplifiedModel_Link_SortedSegments_" +
                name +
                "_" +
                self.label] = str(
                partialscore)
            partialscore = self.unmodeledregions_cr_dict[name].evaluate(False)
            score += partialscore
            output[
                "SimplifiedModel_Link_UnmodeledRegions_" +
                name +
                "_" +
                self.label] = str(
                partialscore)
        for rb in self.rigid_body_symmetries:
            name=rb.get_name()
            output[name +"_" +self.label]=str(rb.get_reference_frame().get_transformation_to())
        for name in self.linker_restraints_dict:
            output[
                name +
                "_" +
                self.label] = str(
                self.linker_restraints_dict[
                    name].unprotected_evaluate(
                    None))

        if len(self.reference_structures.keys()) != 0:
            rmsds = self.get_all_rmsds()
            for label in rmsds:
                output[
                    "SimplifiedModel_" +
                    label +
                    "_" +
                    self.label] = rmsds[
                    label]

        if self.output_level == "high":
            for p in IMP.atom.get_leaves(self.prot):
                d = IMP.core.XYZR(p)
                output["Coordinates_" +
                       p.get_name() + "_" + self.label] = str(d)

        output["_TotalScore"] = str(score)
        return output

    def get_test_output(self):
        # this method is called by test functions and return an enriched output
        output = self.get_output()
        for n, p in enumerate(self.get_particles_to_sample()):
            output["Particle_to_sample_" + str(n)] = str(p)
        output["Number_of_particles"] = len(IMP.atom.get_leaves(self.prot))
        output["Hierarchy_Dictionary"] = list(self.hier_dict.keys())
        output["Number_of_floppy_bodies"] = len(self.floppy_bodies)
        output["Number_of_rigid_bodies"] = len(self.rigid_bodies)
        output["Number_of_super_bodies"] = len(self.super_rigid_bodies)
        output["Selection_resolution_1"] = len(
            IMP.pmi.tools.select(self, resolution=1))
        output["Selection_resolution_5"] = len(
            IMP.pmi.tools.select(self, resolution=5))
        output["Selection_resolution_7"] = len(
            IMP.pmi.tools.select(self, resolution=5))
        output["Selection_resolution_10"] = len(
            IMP.pmi.tools.select(self, resolution=10))
        output["Selection_resolution_100"] = len(
            IMP.pmi.tools.select(self, resolution=100))
        output["Selection_All"] = len(IMP.pmi.tools.select(self))
        output["Selection_resolution=1"] = len(
            IMP.pmi.tools.select(self, resolution=1))
        output["Selection_resolution=1,resid=10"] = len(
            IMP.pmi.tools.select(self, resolution=1, residue=10))
        for resolution in self.hier_resolution:
            output["Hier_resolution_dictionary" +
                   str(resolution)] = len(self.hier_resolution[resolution])
        for name in self.hier_dict:
            output[
                "Selection_resolution=1,resid=10,name=" +
                name] = len(
                IMP.pmi.tools.select(
                    self,
                    resolution=1,
                    name=name,
                    residue=10))
            output[
                "Selection_resolution=1,resid=10,name=" +
                name +
                ",ambiguous"] = len(
                IMP.pmi.tools.select(
                    self,
                    resolution=1,
                    name=name,
                    name_is_ambiguous=True,
                    residue=10))
            output[
                "Selection_resolution=1,resid=10,name=" +
                name +
                ",ambiguous"] = len(
                IMP.pmi.tools.select(
                    self,
                    resolution=1,
                    name=name,
                    name_is_ambiguous=True,
                    residue=10))
            output[
                "Selection_resolution=1,resrange=(10,20),name=" +
                name] = len(
                IMP.pmi.tools.select(
                    self,
                    resolution=1,
                    name=name,
                    first_residue=10,
                    last_residue=20))
            output[
                "Selection_resolution=1,resrange=(10,20),name=" +
                name +
                ",ambiguous"] = len(
                IMP.pmi.tools.select(
                    self,
                    resolution=1,
                    name=name,
                    name_is_ambiguous=True,
                    first_residue=10,
                    last_residue=20))
            output[
                "Selection_resolution=10,resrange=(10,20),name=" +
                name] = len(
                IMP.pmi.tools.select(
                    self,
                    resolution=10,
                    name=name,
                    first_residue=10,
                    last_residue=20))
            output[
                "Selection_resolution=10,resrange=(10,20),name=" +
                name +
                ",ambiguous"] = len(
                IMP.pmi.tools.select(
                    self,
                    resolution=10,
                    name=name,
                    name_is_ambiguous=True,
                    first_residue=10,
                    last_residue=20))
            output[
                "Selection_resolution=100,resrange=(10,20),name=" +
                name] = len(
                IMP.pmi.tools.select(
                    self,
                    resolution=100,
                    name=name,
                    first_residue=10,
                    last_residue=20))
            output[
                "Selection_resolution=100,resrange=(10,20),name=" +
                name +
                ",ambiguous"] = len(
                IMP.pmi.tools.select(
                    self,
                    resolution=100,
                    name=name,
                    name_is_ambiguous=True,
                    first_residue=10,
                    last_residue=20))

        return output
