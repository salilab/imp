"""@namespace IMP.pmi.output
   Classes for writing output files and processing them.
"""

from __future__ import print_function, division
import IMP
import IMP.atom
import IMP.core
import IMP.pmi
import IMP.pmi.tools
import IMP.pmi.io
import os
import sys
import ast
import RMF
import numpy as np
import operator
import itertools
import warnings
import string
import ihm.format
import collections
try:
    import cPickle as pickle
except ImportError:
    import pickle


class _ChainIDs(object):
    """Map indices to multi-character chain IDs.
       We label the first 26 chains A-Z, then we move to two-letter
       chain IDs: AA through AZ, then BA through BZ, through to ZZ.
       This continues with longer chain IDs."""
    def __getitem__(self, ind):
        chars = string.ascii_uppercase
        lc = len(chars)
        ids = []
        while ind >= lc:
            ids.append(chars[ind % lc])
            ind = ind // lc - 1
        ids.append(chars[ind])
        return "".join(reversed(ids))


class ProtocolOutput(object):
    """Base class for capturing a modeling protocol.
       Unlike simple output of model coordinates, a complete
       protocol includes the input data used, details on the restraints,
       sampling, and clustering, as well as output models.
       Use via IMP.pmi.topology.System.add_protocol_output().

       @see IMP.pmi.mmcif.ProtocolOutput for a concrete subclass that outputs
            mmCIF files.
    """
    pass


def _flatten(seq):
    for elt in seq:
        if isinstance(elt, (tuple, list)):
            for elt2 in _flatten(elt):
                yield elt2
        else:
            yield elt


def _disambiguate_chain(chid, seen_chains):
    """Make sure that the chain ID is unique; warn and correct if it isn't"""
    # Handle null chain IDs
    if chid == '\0':
        chid = ' '

    if chid in seen_chains:
        warnings.warn("Duplicate chain ID '%s' encountered" % chid,
                      IMP.pmi.StructureWarning)

        for suffix in itertools.count(1):
            new_chid = chid + "%d" % suffix
            if new_chid not in seen_chains:
                seen_chains.add(new_chid)
                return new_chid
    seen_chains.add(chid)
    return chid


def _write_pdb_internal(flpdb, particle_infos_for_pdb, geometric_center,
                        write_all_residues_per_bead):
    for n, tupl in enumerate(particle_infos_for_pdb):
        (xyz, atom_type, residue_type,
         chain_id, residue_index, all_indexes, radius) = tupl
        if atom_type is None:
            atom_type = IMP.atom.AT_CA
        if write_all_residues_per_bead and all_indexes is not None:
            for residue_number in all_indexes:
                flpdb.write(
                    IMP.atom.get_pdb_string((xyz[0] - geometric_center[0],
                                             xyz[1] - geometric_center[1],
                                             xyz[2] - geometric_center[2]),
                                            n+1, atom_type, residue_type,
                                            chain_id[:1], residue_number, ' ',
                                            1.00, radius))
        else:
            flpdb.write(
                IMP.atom.get_pdb_string((xyz[0] - geometric_center[0],
                                         xyz[1] - geometric_center[1],
                                         xyz[2] - geometric_center[2]),
                                        n+1, atom_type, residue_type,
                                        chain_id[:1], residue_index, ' ',
                                        1.00, radius))
    flpdb.write("ENDMDL\n")


_Entity = collections.namedtuple('_Entity', ('id', 'seq'))
_ChainInfo = collections.namedtuple('_ChainInfo', ('entity', 'name'))


def _get_chain_info(chains, root_hier):
    chain_info = {}
    entities = {}
    all_entities = []
    for mol in IMP.atom.get_by_type(root_hier, IMP.atom.MOLECULE_TYPE):
        molname = IMP.pmi.get_molecule_name_and_copy(mol)
        chain_id = chains[molname]
        chain = IMP.atom.Chain(mol)
        seq = chain.get_sequence()
        if seq not in entities:
            entities[seq] = e = _Entity(id=len(entities)+1, seq=seq)
            all_entities.append(e)
        entity = entities[seq]
        info = _ChainInfo(entity=entity, name=molname)
        chain_info[chain_id] = info
    return chain_info, all_entities


def _write_mmcif_internal(flpdb, particle_infos_for_pdb, geometric_center,
                          write_all_residues_per_bead, chains, root_hier):
    # get dict with keys=chain IDs, values=chain info
    chain_info, entities = _get_chain_info(chains, root_hier)

    writer = ihm.format.CifWriter(flpdb)
    writer.start_block('model')
    with writer.category("_entry") as lp:
        lp.write(id='model')

    with writer.loop("_entity", ["id"]) as lp:
        for e in entities:
            lp.write(id=e.id)

    with writer.loop("_entity_poly",
                     ["entity_id", "pdbx_seq_one_letter_code"]) as lp:
        for e in entities:
            lp.write(entity_id=e.id, pdbx_seq_one_letter_code=e.seq)

    with writer.loop("_struct_asym", ["id", "entity_id", "details"]) as lp:
        for chid in sorted(chains.values()):
            ci = chain_info[chid]
            lp.write(id=chid, entity_id=ci.entity.id, details=ci.name)

    with writer.loop("_atom_site",
                     ["group_PDB", "type_symbol", "label_atom_id",
                      "label_comp_id", "label_asym_id", "label_seq_id",
                      "auth_seq_id",
                      "Cartn_x", "Cartn_y", "Cartn_z", "label_entity_id",
                      "pdbx_pdb_model_num",
                      "id"]) as lp:
        ordinal = 1
        for n, tupl in enumerate(particle_infos_for_pdb):
            (xyz, atom_type, residue_type,
             chain_id, residue_index, all_indexes, radius) = tupl
            ci = chain_info[chain_id]
            if atom_type is None:
                atom_type = IMP.atom.AT_CA
            c = xyz - geometric_center
            if write_all_residues_per_bead and all_indexes is not None:
                for residue_number in all_indexes:
                    lp.write(group_PDB='ATOM',
                             type_symbol='C',
                             label_atom_id=atom_type.get_string(),
                             label_comp_id=residue_type.get_string(),
                             label_asym_id=chain_id,
                             label_seq_id=residue_index,
                             auth_seq_id=residue_index, Cartn_x=c[0],
                             Cartn_y=c[1], Cartn_z=c[2], id=ordinal,
                             pdbx_pdb_model_num=1,
                             label_entity_id=ci.entity.id)
                    ordinal += 1
            else:
                lp.write(group_PDB='ATOM', type_symbol='C',
                         label_atom_id=atom_type.get_string(),
                         label_comp_id=residue_type.get_string(),
                         label_asym_id=chain_id,
                         label_seq_id=residue_index,
                         auth_seq_id=residue_index, Cartn_x=c[0],
                         Cartn_y=c[1], Cartn_z=c[2], id=ordinal,
                         pdbx_pdb_model_num=1,
                         label_entity_id=ci.entity.id)
                ordinal += 1


class Output(object):
    """Class for easy writing of PDBs, RMFs, and stat files

    @note Model should be updated prior to writing outputs.
    """
    def __init__(self, ascii=True, atomistic=False):
        self.dictionary_pdbs = {}
        self._pdb_mmcif = {}
        self.dictionary_rmfs = {}
        self.dictionary_stats = {}
        self.dictionary_stats2 = {}
        self.best_score_list = None
        self.nbestscoring = None
        self.suffixes = []
        self.replica_exchange = False
        self.ascii = ascii
        self.initoutput = {}
        self.residuetypekey = IMP.StringKey("ResidueName")
        # 1-character chain IDs, suitable for PDB output
        self.chainids = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
            "abcdefghijklmnopqrstuvwxyz0123456789"
        # Multi-character chain IDs, suitable for mmCIF output
        self.multi_chainids = _ChainIDs()
        self.dictchain = {}  # keys are molecule names, values are chain ids
        self.particle_infos_for_pdb = {}
        self.atomistic = atomistic
        self.use_pmi2 = False

    def get_pdb_names(self):
        """Get a list of all PDB files being output by this instance"""
        return list(self.dictionary_pdbs.keys())

    def get_rmf_names(self):
        return list(self.dictionary_rmfs.keys())

    def get_stat_names(self):
        return list(self.dictionary_stats.keys())

    def _init_dictchain(self, name, prot, multichar_chain=False):
        self.dictchain[name] = {}
        self.use_pmi2 = False
        seen_chains = set()

        # attempt to find PMI objects.
        if IMP.pmi.get_is_canonical(prot):
            self.use_pmi2 = True
            self.atomistic = True  # detects automatically
            for n, mol in enumerate(IMP.atom.get_by_type(
                    prot, IMP.atom.MOLECULE_TYPE)):
                chid = _disambiguate_chain(IMP.atom.Chain(mol).get_id(),
                                           seen_chains)
                molname = IMP.pmi.get_molecule_name_and_copy(mol)
                self.dictchain[name][molname] = chid
        else:
            chainids = self.multi_chainids if multichar_chain \
                else self.chainids
            for n, i in enumerate(self.dictionary_pdbs[name].get_children()):
                self.dictchain[name][i.get_name()] = chainids[n]

    def init_pdb(self, name, prot, mmcif=False):
        """Init PDB Writing.
        @param name The PDB filename
        @param prot The hierarchy to write to this pdb file
        @param mmcif If True, write PDBs in mmCIF format
        @note if the PDB name is 'System' then will use Selection
              to get molecules
        """
        flpdb = open(name, 'w')
        flpdb.close()
        self.dictionary_pdbs[name] = prot
        self._pdb_mmcif[name] = mmcif
        self._init_dictchain(name, prot)

    def write_psf(self, filename, name):
        flpsf = open(filename, 'w')
        flpsf.write("PSF CMAP CHEQ" + "\n")
        index_residue_pair_list = {}
        (particle_infos_for_pdb, geometric_center) = \
            self.get_particle_infos_for_pdb_writing(name)
        nparticles = len(particle_infos_for_pdb)
        flpsf.write(str(nparticles) + " !NATOM" + "\n")
        for n, p in enumerate(particle_infos_for_pdb):
            atom_index = n+1
            residue_type = p[2]
            chain = p[3]
            resid = p[4]
            flpsf.write('{0:8d}{1:1s}{2:4s}{3:1s}{4:4s}{5:1s}{6:4s}{7:1s}'
                        '{8:4s}{9:1s}{10:4s}{11:14.6f}{12:14.6f}{13:8d}'
                        '{14:14.6f}{15:14.6f}'.format(
                            atom_index, " ", chain, " ", str(resid), " ",
                            '"'+residue_type.get_string()+'"', " ", "C",
                            " ", "C", 1.0, 0.0, 0, 0.0, 0.0))
            flpsf.write('\n')
            if chain not in index_residue_pair_list:
                index_residue_pair_list[chain] = [(atom_index, resid)]
            else:
                index_residue_pair_list[chain].append((atom_index, resid))

        # now write the connectivity
        indexes_pairs = []
        for chain in sorted(index_residue_pair_list.keys()):

            ls = index_residue_pair_list[chain]
            # sort by residue
            ls = sorted(ls, key=lambda tup: tup[1])
            # get the index list
            indexes = [x[0] for x in ls]
            # get the contiguous pairs
            indexes_pairs += list(IMP.pmi.tools.sublist_iterator(
                indexes, lmin=2, lmax=2))
        nbonds = len(indexes_pairs)
        flpsf.write(str(nbonds)+" !NBOND: bonds"+"\n")

        # save bonds in fixed column format
        for i in range(0, len(indexes_pairs), 4):
            for bond in indexes_pairs[i:i+4]:
                flpsf.write('{0:8d}{1:8d}'.format(*bond))
            flpsf.write('\n')

        del particle_infos_for_pdb
        flpsf.close()

    def write_pdb(self, name, appendmode=True,
                  translate_to_geometric_center=False,
                  write_all_residues_per_bead=False):

        (particle_infos_for_pdb,
         geometric_center) = self.get_particle_infos_for_pdb_writing(name)

        if not translate_to_geometric_center:
            geometric_center = (0, 0, 0)

        filemode = 'a' if appendmode else 'w'
        with open(name, filemode) as flpdb:
            if self._pdb_mmcif[name]:
                _write_mmcif_internal(flpdb, particle_infos_for_pdb,
                                      geometric_center,
                                      write_all_residues_per_bead,
                                      self.dictchain[name],
                                      self.dictionary_pdbs[name])
            else:
                _write_pdb_internal(flpdb, particle_infos_for_pdb,
                                    geometric_center,
                                    write_all_residues_per_bead)

    def get_prot_name_from_particle(self, name, p):
        """Get the protein name from the particle.
           This is done by traversing the hierarchy."""
        if self.use_pmi2:
            return IMP.pmi.get_molecule_name_and_copy(p), True
        else:
            return IMP.pmi.tools.get_prot_name_from_particle(
                p, self.dictchain[name])

    def get_particle_infos_for_pdb_writing(self, name):
        # index_residue_pair_list={}

        # the resindexes dictionary keep track of residues that have
        # been already added to avoid duplication
        # highest resolution have highest priority
        resindexes_dict = {}

        # this dictionary dill contain the sequence of tuples needed to
        # write the pdb
        particle_infos_for_pdb = []

        geometric_center = [0, 0, 0]
        atom_count = 0

        if self.use_pmi2:
            # select highest resolution
            sel = IMP.atom.Selection(self.dictionary_pdbs[name], resolution=0)
            ps = sel.get_selected_particles()
        else:
            ps = IMP.atom.get_leaves(self.dictionary_pdbs[name])

        for n, p in enumerate(ps):
            protname, is_a_bead = self.get_prot_name_from_particle(name, p)

            if protname not in resindexes_dict:
                resindexes_dict[protname] = []

            if IMP.atom.Atom.get_is_setup(p) and self.atomistic:
                residue = IMP.atom.Residue(IMP.atom.Atom(p).get_parent())
                rt = residue.get_residue_type()
                resind = residue.get_index()
                atomtype = IMP.atom.Atom(p).get_atom_type()
                xyz = list(IMP.core.XYZ(p).get_coordinates())
                radius = IMP.core.XYZR(p).get_radius()
                geometric_center[0] += xyz[0]
                geometric_center[1] += xyz[1]
                geometric_center[2] += xyz[2]
                atom_count += 1
                particle_infos_for_pdb.append(
                    (xyz, atomtype, rt, self.dictchain[name][protname],
                     resind, None, radius))
                resindexes_dict[protname].append(resind)

            elif IMP.atom.Residue.get_is_setup(p):

                residue = IMP.atom.Residue(p)
                resind = residue.get_index()
                # skip if the residue was already added by atomistic resolution
                # 0
                if resind in resindexes_dict[protname]:
                    continue
                else:
                    resindexes_dict[protname].append(resind)
                rt = residue.get_residue_type()
                xyz = IMP.core.XYZ(p).get_coordinates()
                radius = IMP.core.XYZR(p).get_radius()
                geometric_center[0] += xyz[0]
                geometric_center[1] += xyz[1]
                geometric_center[2] += xyz[2]
                atom_count += 1
                particle_infos_for_pdb.append(
                    (xyz, None, rt, self.dictchain[name][protname], resind,
                     None, radius))

            elif IMP.atom.Fragment.get_is_setup(p) and not is_a_bead:
                resindexes = IMP.pmi.tools.get_residue_indexes(p)
                resind = resindexes[len(resindexes) // 2]
                if resind in resindexes_dict[protname]:
                    continue
                else:
                    resindexes_dict[protname].append(resind)
                rt = IMP.atom.ResidueType('BEA')
                xyz = IMP.core.XYZ(p).get_coordinates()
                radius = IMP.core.XYZR(p).get_radius()
                geometric_center[0] += xyz[0]
                geometric_center[1] += xyz[1]
                geometric_center[2] += xyz[2]
                atom_count += 1
                particle_infos_for_pdb.append(
                    (xyz, None, rt, self.dictchain[name][protname], resind,
                     resindexes, radius))

            else:
                if is_a_bead:
                    rt = IMP.atom.ResidueType('BEA')
                    resindexes = IMP.pmi.tools.get_residue_indexes(p)
                    if len(resindexes) > 0:
                        resind = resindexes[len(resindexes) // 2]
                        xyz = IMP.core.XYZ(p).get_coordinates()
                        radius = IMP.core.XYZR(p).get_radius()
                        geometric_center[0] += xyz[0]
                        geometric_center[1] += xyz[1]
                        geometric_center[2] += xyz[2]
                        atom_count += 1
                        particle_infos_for_pdb.append(
                            (xyz, None, rt, self.dictchain[name][protname],
                             resind, resindexes, radius))

        if atom_count > 0:
            geometric_center = (geometric_center[0] / atom_count,
                                geometric_center[1] / atom_count,
                                geometric_center[2] / atom_count)

        # sort by chain ID, then residue index. Longer chain IDs (e.g. AA)
        # should always come after shorter (e.g. Z)
        particle_infos_for_pdb = sorted(particle_infos_for_pdb,
                                        key=lambda x: (len(x[3]), x[3], x[4]))

        return (particle_infos_for_pdb, geometric_center)

    def write_pdbs(self, appendmode=True, mmcif=False):
        for pdb in self.dictionary_pdbs.keys():
            self.write_pdb(pdb, appendmode)

    def init_pdb_best_scoring(self,
                              suffix,
                              prot,
                              nbestscoring,
                              replica_exchange=False, mmcif=False):
        # save only the nbestscoring conformations
        # create as many pdbs as needed

        self._pdb_best_scoring_mmcif = mmcif
        fileext = '.cif' if mmcif else '.pdb'
        self.suffixes.append(suffix)
        self.replica_exchange = replica_exchange
        if not self.replica_exchange:
            # common usage
            # if you are not in replica exchange mode
            # initialize the array of scores internally
            self.best_score_list = []
        else:
            # otherwise the replicas must communicate
            # through a common file to know what are the best scores
            self.best_score_file_name = "best.scores.rex.py"
            self.best_score_list = []
            with open(self.best_score_file_name, "w") as best_score_file:
                best_score_file.write(
                    "self.best_score_list=" + str(self.best_score_list) + "\n")

        self.nbestscoring = nbestscoring
        for i in range(self.nbestscoring):
            name = suffix + "." + str(i) + fileext
            flpdb = open(name, 'w')
            flpdb.close()
            self.dictionary_pdbs[name] = prot
            self._pdb_mmcif[name] = mmcif
            self._init_dictchain(name, prot)

    def write_pdb_best_scoring(self, score):
        if self.nbestscoring is None:
            print("Output.write_pdb_best_scoring: init_pdb_best_scoring "
                  "not run")

        mmcif = self._pdb_best_scoring_mmcif
        fileext = '.cif' if mmcif else '.pdb'
        # update the score list
        if self.replica_exchange:
            # read the self.best_score_list from the file
            with open(self.best_score_file_name) as fh:
                exec(fh.read())

        if len(self.best_score_list) < self.nbestscoring:
            self.best_score_list.append(score)
            self.best_score_list.sort()
            index = self.best_score_list.index(score)
            for suffix in self.suffixes:
                for i in range(len(self.best_score_list) - 2, index - 1, -1):
                    oldname = suffix + "." + str(i) + fileext
                    newname = suffix + "." + str(i + 1) + fileext
                    # rename on Windows fails if newname already exists
                    if os.path.exists(newname):
                        os.unlink(newname)
                    os.rename(oldname, newname)
                filetoadd = suffix + "." + str(index) + fileext
                self.write_pdb(filetoadd, appendmode=False)

        else:
            if score < self.best_score_list[-1]:
                self.best_score_list.append(score)
                self.best_score_list.sort()
                self.best_score_list.pop(-1)
                index = self.best_score_list.index(score)
                for suffix in self.suffixes:
                    for i in range(len(self.best_score_list) - 1,
                                   index - 1, -1):
                        oldname = suffix + "." + str(i) + fileext
                        newname = suffix + "." + str(i + 1) + fileext
                        os.rename(oldname, newname)
                    filenametoremove = suffix + \
                        "." + str(self.nbestscoring) + fileext
                    os.remove(filenametoremove)
                    filetoadd = suffix + "." + str(index) + fileext
                    self.write_pdb(filetoadd, appendmode=False)

        if self.replica_exchange:
            # write the self.best_score_list to the file
            with open(self.best_score_file_name, "w") as best_score_file:
                best_score_file.write(
                    "self.best_score_list=" + str(self.best_score_list) + '\n')

    def init_rmf(self, name, hierarchies, rs=None, geometries=None,
                 listofobjects=None):
        """
        Initialize an RMF file

        @param name          the name of the RMF file
        @param hierarchies   the hierarchies to be included (it is a list)
        @param rs            optional, the restraint sets (it is a list)
        @param geometries    optional, the geometries (it is a list)
        @param listofobjects optional, the list of objects for the stat
               (it is a list)
        """
        rh = RMF.create_rmf_file(name)
        IMP.rmf.add_hierarchies(rh, hierarchies)
        cat = None
        outputkey_rmfkey = None

        if rs is not None:
            IMP.rmf.add_restraints(rh, rs)
        if geometries is not None:
            IMP.rmf.add_geometries(rh, geometries)
        if listofobjects is not None:
            cat = rh.get_category("stat")
            outputkey_rmfkey = {}
            for o in listofobjects:
                if "get_output" not in dir(o):
                    raise ValueError(
                        "Output: object %s doesn't have get_output() method"
                        % str(o))
                output = o.get_output()
                for outputkey in output:
                    rmftag = RMF.string_tag
                    if isinstance(output[outputkey], float):
                        rmftag = RMF.float_tag
                    elif isinstance(output[outputkey], int):
                        rmftag = RMF.int_tag
                    elif isinstance(output[outputkey], str):
                        rmftag = RMF.string_tag
                    else:
                        rmftag = RMF.string_tag
                    rmfkey = rh.get_key(cat, outputkey, rmftag)
                    outputkey_rmfkey[outputkey] = rmfkey
            outputkey_rmfkey["rmf_file"] = \
                rh.get_key(cat, "rmf_file", RMF.string_tag)
            outputkey_rmfkey["rmf_frame_index"] = \
                rh.get_key(cat, "rmf_frame_index", RMF.int_tag)

        self.dictionary_rmfs[name] = (rh, cat, outputkey_rmfkey, listofobjects)

    def add_restraints_to_rmf(self, name, objectlist):
        for o in _flatten(objectlist):
            try:
                rs = o.get_restraint_for_rmf()
                if not isinstance(rs, (list, tuple)):
                    rs = [rs]
            except:  # noqa: E722
                rs = [o.get_restraint()]
            IMP.rmf.add_restraints(
                self.dictionary_rmfs[name][0], rs)

    def add_geometries_to_rmf(self, name, objectlist):
        for o in objectlist:
            geos = o.get_geometries()
            IMP.rmf.add_geometries(self.dictionary_rmfs[name][0], geos)

    def add_particle_pair_from_restraints_to_rmf(self, name, objectlist):
        for o in objectlist:

            pps = o.get_particle_pairs()
            for pp in pps:
                IMP.rmf.add_geometry(
                    self.dictionary_rmfs[name][0],
                    IMP.core.EdgePairGeometry(pp))

    def write_rmf(self, name):
        IMP.rmf.save_frame(self.dictionary_rmfs[name][0])
        if self.dictionary_rmfs[name][1] is not None:
            outputkey_rmfkey = self.dictionary_rmfs[name][2]
            listofobjects = self.dictionary_rmfs[name][3]
            for o in listofobjects:
                output = o.get_output()
                for outputkey in output:
                    rmfkey = outputkey_rmfkey[outputkey]
                    try:
                        n = self.dictionary_rmfs[name][0].get_root_node()
                        n.set_value(rmfkey, output[outputkey])
                    except NotImplementedError:
                        continue
            rmfkey = outputkey_rmfkey["rmf_file"]
            self.dictionary_rmfs[name][0].get_root_node().set_value(
                rmfkey, name)
            rmfkey = outputkey_rmfkey["rmf_frame_index"]
            nframes = self.dictionary_rmfs[name][0].get_number_of_frames()
            self.dictionary_rmfs[name][0].get_root_node().set_value(
                rmfkey, nframes-1)
        self.dictionary_rmfs[name][0].flush()

    def close_rmf(self, name):
        rh = self.dictionary_rmfs[name][0]
        del self.dictionary_rmfs[name]
        del rh

    def write_rmfs(self):
        for rmfinfo in self.dictionary_rmfs.keys():
            self.write_rmf(rmfinfo[0])

    def init_stat(self, name, listofobjects):
        if self.ascii:
            flstat = open(name, 'w')
            flstat.close()
        else:
            flstat = open(name, 'wb')
            flstat.close()

        # check that all objects in listofobjects have a get_output method
        for o in listofobjects:
            if "get_output" not in dir(o):
                raise ValueError(
                    "Output: object %s doesn't have get_output() method"
                    % str(o))
        self.dictionary_stats[name] = listofobjects

    def set_output_entry(self, key, value):
        self.initoutput.update({key: value})

    def write_stat(self, name, appendmode=True):
        output = self.initoutput
        for obj in self.dictionary_stats[name]:
            d = obj.get_output()
            # remove all entries that begin with _ (private entries)
            dfiltered = dict((k, v) for k, v in d.items() if k[0] != "_")
            output.update(dfiltered)

        if appendmode:
            writeflag = 'a'
        else:
            writeflag = 'w'

        if self.ascii:
            flstat = open(name, writeflag)
            flstat.write("%s \n" % output)
            flstat.close()
        else:
            flstat = open(name, writeflag + 'b')
            pickle.dump(output, flstat, 2)
            flstat.close()

    def write_stats(self):
        for stat in self.dictionary_stats.keys():
            self.write_stat(stat)

    def get_stat(self, name):
        output = {}
        for obj in self.dictionary_stats[name]:
            output.update(obj.get_output())
        return output

    def write_test(self, name, listofobjects):
        flstat = open(name, 'w')
        output = self.initoutput
        for o in listofobjects:
            if "get_test_output" not in dir(o) and "get_output" not in dir(o):
                raise ValueError(
                    "Output: object %s doesn't have get_output() or "
                    "get_test_output() method" % str(o))
        self.dictionary_stats[name] = listofobjects

        for obj in self.dictionary_stats[name]:
            try:
                d = obj.get_test_output()
            except:  # noqa: E722
                d = obj.get_output()
            # remove all entries that begin with _ (private entries)
            dfiltered = dict((k, v) for k, v in d.items() if k[0] != "_")
            output.update(dfiltered)
        flstat.write("%s \n" % output)
        flstat.close()

    def test(self, name, listofobjects, tolerance=1e-5):
        output = self.initoutput
        for o in listofobjects:
            if "get_test_output" not in dir(o) and "get_output" not in dir(o):
                raise ValueError(
                    "Output: object %s doesn't have get_output() or "
                    "get_test_output() method" % str(o))
        for obj in listofobjects:
            try:
                output.update(obj.get_test_output())
            except:  # noqa: E722
                output.update(obj.get_output())

        flstat = open(name, 'r')

        passed = True
        for fl in flstat:
            test_dict = ast.literal_eval(fl)
        for k in test_dict:
            if k in output:
                old_value = str(test_dict[k])
                new_value = str(output[k])
                try:
                    float(old_value)
                    is_float = True
                except ValueError:
                    is_float = False

                if is_float:
                    fold = float(old_value)
                    fnew = float(new_value)
                    diff = abs(fold - fnew)
                    if diff > tolerance:
                        print("%s: test failed, old value: %s new value %s; "
                              "diff %f > %f" % (str(k), str(old_value),
                                                str(new_value), diff,
                                                tolerance), file=sys.stderr)
                        passed = False
                elif test_dict[k] != output[k]:
                    if len(old_value) < 50 and len(new_value) < 50:
                        print("%s: test failed, old value: %s new value %s"
                              % (str(k), old_value, new_value),
                              file=sys.stderr)
                        passed = False
                    else:
                        print("%s: test failed, omitting results (too long)"
                              % str(k), file=sys.stderr)
                        passed = False

            else:
                print("%s from old objects (file %s) not in new objects"
                      % (str(k), str(name)), file=sys.stderr)
        flstat.close()
        return passed

    def get_environment_variables(self):
        import os
        return str(os.environ)

    def get_versions_of_relevant_modules(self):
        import IMP
        versions = {}
        versions["IMP_VERSION"] = IMP.get_module_version()
        versions["PMI_VERSION"] = IMP.pmi.get_module_version()
        try:
            import IMP.isd2
            versions["ISD2_VERSION"] = IMP.isd2.get_module_version()
        except ImportError:
            pass
        try:
            import IMP.isd_emxl
            versions["ISD_EMXL_VERSION"] = IMP.isd_emxl.get_module_version()
        except ImportError:
            pass
        return versions

    def init_stat2(self, name, listofobjects, extralabels=None,
                   listofsummedobjects=None):
        # this is a new stat file that should be less
        # space greedy!
        # listofsummedobjects must be in the form
        # [([obj1,obj2,obj3,obj4...],label)]
        # extralabels

        if listofsummedobjects is None:
            listofsummedobjects = []
        if extralabels is None:
            extralabels = []
        flstat = open(name, 'w')
        output = {}
        stat2_keywords = {"STAT2HEADER": "STAT2HEADER"}
        stat2_keywords.update(
            {"STAT2HEADER_ENVIRON": str(self.get_environment_variables())})
        stat2_keywords.update(
            {"STAT2HEADER_IMP_VERSIONS":
             str(self.get_versions_of_relevant_modules())})
        stat2_inverse = {}

        for obj in listofobjects:
            if "get_output" not in dir(obj):
                raise ValueError(
                    "Output: object %s doesn't have get_output() method"
                    % str(obj))
            else:
                d = obj.get_output()
                # remove all entries that begin with _ (private entries)
                dfiltered = dict((k, v)
                                 for k, v in d.items() if k[0] != "_")
                output.update(dfiltered)

        # check for customizable entries
        for obj in listofsummedobjects:
            for t in obj[0]:
                if "get_output" not in dir(t):
                    raise ValueError(
                        "Output: object %s doesn't have get_output() method"
                        % str(t))
                else:
                    if "_TotalScore" not in t.get_output():
                        raise ValueError(
                            "Output: object %s doesn't have _TotalScore "
                            "entry to be summed" % str(t))
                    else:
                        output.update({obj[1]: 0.0})

        for k in extralabels:
            output.update({k: 0.0})

        for n, k in enumerate(output):
            stat2_keywords.update({n: k})
            stat2_inverse.update({k: n})

        flstat.write("%s \n" % stat2_keywords)
        flstat.close()
        self.dictionary_stats2[name] = (
            listofobjects,
            stat2_inverse,
            listofsummedobjects,
            extralabels)

    def write_stat2(self, name, appendmode=True):
        output = {}
        (listofobjects, stat2_inverse, listofsummedobjects,
         extralabels) = self.dictionary_stats2[name]

        # writing objects
        for obj in listofobjects:
            od = obj.get_output()
            dfiltered = dict((k, v) for k, v in od.items() if k[0] != "_")
            for k in dfiltered:
                output.update({stat2_inverse[k]: od[k]})

        # writing summedobjects
        for so in listofsummedobjects:
            partial_score = 0.0
            for t in so[0]:
                d = t.get_output()
                partial_score += float(d["_TotalScore"])
            output.update({stat2_inverse[so[1]]: str(partial_score)})

        # writing extralabels
        for k in extralabels:
            if k in self.initoutput:
                output.update({stat2_inverse[k]: self.initoutput[k]})
            else:
                output.update({stat2_inverse[k]: "None"})

        if appendmode:
            writeflag = 'a'
        else:
            writeflag = 'w'

        flstat = open(name, writeflag)
        flstat.write("%s \n" % output)
        flstat.close()

    def write_stats2(self):
        for stat in self.dictionary_stats2.keys():
            self.write_stat2(stat)


class OutputStatistics(object):
    """Collect statistics from ProcessOutput.get_fields().
       Counters of the total number of frames read, plus the models that
       passed the various filters used in get_fields(), are provided."""
    def __init__(self):
        self.total = 0
        self.passed_get_every = 0
        self.passed_filterout = 0
        self.passed_filtertuple = 0


class ProcessOutput(object):
    """A class for reading stat files (either rmf or ascii v1 and v2)"""
    def __init__(self, filename):
        self.filename = filename
        self.isstat1 = False
        self.isstat2 = False
        self.isrmf = False

        if self.filename is None:
            raise ValueError("No file name provided. Use -h for help")

        try:
            # let's see if that is an rmf file
            rh = RMF.open_rmf_file_read_only(self.filename)
            self.isrmf = True
            cat = rh.get_category('stat')
            rmf_klist = rh.get_keys(cat)
            self.rmf_names_keys = dict([(rh.get_name(k), k)
                                       for k in rmf_klist])
            del rh

        except IOError:
            f = open(self.filename, "r")
            # try with an ascii stat file
            # get the keys from the first line
            for line in f.readlines():
                d = ast.literal_eval(line)
                self.klist = list(d.keys())
                # check if it is a stat2 file
                if "STAT2HEADER" in self.klist:
                    self.isstat2 = True
                    for k in self.klist:
                        if "STAT2HEADER" in str(k):
                            # if print_header: print k, d[k]
                            del d[k]
                    stat2_dict = d
                    # get the list of keys sorted by value
                    kkeys = [k[0]
                             for k in sorted(stat2_dict.items(),
                                             key=operator.itemgetter(1))]
                    self.klist = [k[1]
                                  for k in sorted(stat2_dict.items(),
                                                  key=operator.itemgetter(1))]
                    self.invstat2_dict = {}
                    for k in kkeys:
                        self.invstat2_dict.update({stat2_dict[k]: k})
                else:
                    IMP.handle_use_deprecated(
                        "statfile v1 is deprecated. "
                        "Please convert to statfile v2.\n")
                    self.isstat1 = True
                    self.klist.sort()

                break
            f.close()

    def get_keys(self):
        if self.isrmf:
            return sorted(self.rmf_names_keys.keys())
        else:
            return self.klist

    def show_keys(self, ncolumns=2, truncate=65):
        IMP.pmi.tools.print_multicolumn(self.get_keys(), ncolumns, truncate)

    def get_fields(self, fields, filtertuple=None, filterout=None, get_every=1,
                   statistics=None):
        '''
        Get the desired field names, and return a dictionary.
        Namely, "fields" are the queried keys in the stat file
        (eg. ["Total_Score",...])
        The returned data structure is a dictionary, where each key is
        a field and the value is the time series (ie, frame ordered series)
        of that field (ie, {"Total_Score":[Score_0,Score_1,Score_2,,...],....})

        @param fields (list of strings) queried keys in the stat file
               (eg. "Total_Score"....)
        @param filterout specify if you want to "grep" out something from
                         the file, so that it is faster
        @param filtertuple a tuple that contains
                     ("TheKeyToBeFiltered",relationship,value)
                     where relationship = "<", "==", or ">"
        @param get_every only read every Nth line from the file
        @param statistics if provided, accumulate statistics in an
                          OutputStatistics object
        '''

        if statistics is None:
            statistics = OutputStatistics()
        outdict = {}
        for field in fields:
            outdict[field] = []

        # print fields values
        if self.isrmf:
            rh = RMF.open_rmf_file_read_only(self.filename)
            nframes = rh.get_number_of_frames()
            for i in range(nframes):
                statistics.total += 1
                # "get_every" and "filterout" not enforced for RMF
                statistics.passed_get_every += 1
                statistics.passed_filterout += 1
                IMP.rmf.load_frame(rh, RMF.FrameID(i))
                if filtertuple is not None:
                    keytobefiltered = filtertuple[0]
                    relationship = filtertuple[1]
                    value = filtertuple[2]
                    datavalue = rh.get_root_node().get_value(
                        self.rmf_names_keys[keytobefiltered])
                    if self.isfiltered(datavalue, relationship, value):
                        continue

                statistics.passed_filtertuple += 1
                for field in fields:
                    outdict[field].append(rh.get_root_node().get_value(
                        self.rmf_names_keys[field]))

        else:
            f = open(self.filename, "r")
            line_number = 0

            for line in f.readlines():
                statistics.total += 1
                if filterout is not None:
                    if filterout in line:
                        continue
                statistics.passed_filterout += 1
                line_number += 1

                if line_number % get_every != 0:
                    if line_number == 1 and self.isstat2:
                        statistics.total -= 1
                        statistics.passed_filterout -= 1
                    continue
                statistics.passed_get_every += 1
                try:
                    d = ast.literal_eval(line)
                except:  # noqa: E722
                    print("# Warning: skipped line number " + str(line_number)
                          + " not a valid line")
                    continue

                if self.isstat1:

                    if filtertuple is not None:
                        keytobefiltered = filtertuple[0]
                        relationship = filtertuple[1]
                        value = filtertuple[2]
                        datavalue = d[keytobefiltered]
                        if self.isfiltered(datavalue, relationship, value):
                            continue

                    statistics.passed_filtertuple += 1
                    [outdict[field].append(d[field]) for field in fields]

                elif self.isstat2:
                    if line_number == 1:
                        statistics.total -= 1
                        statistics.passed_filterout -= 1
                        statistics.passed_get_every -= 1
                        continue

                    if filtertuple is not None:
                        keytobefiltered = filtertuple[0]
                        relationship = filtertuple[1]
                        value = filtertuple[2]
                        datavalue = d[self.invstat2_dict[keytobefiltered]]
                        if self.isfiltered(datavalue, relationship, value):
                            continue

                    statistics.passed_filtertuple += 1
                    [outdict[field].append(d[self.invstat2_dict[field]])
                     for field in fields]

            f.close()

        return outdict

    def isfiltered(self, datavalue, relationship, refvalue):
        dofilter = False
        try:
            _ = float(datavalue)
        except ValueError:
            raise ValueError("ProcessOutput.filter: datavalue cannot be "
                             "converted into a float")

        if relationship == "<":
            if float(datavalue) >= refvalue:
                dofilter = True
        if relationship == ">":
            if float(datavalue) <= refvalue:
                dofilter = True
        if relationship == "==":
            if float(datavalue) != refvalue:
                dofilter = True
        return dofilter


class RMFHierarchyHandler(IMP.atom.Hierarchy):
    """ class to allow more advanced handling of RMF files.
    It is both a container and a IMP.atom.Hierarchy.
     - it is iterable (while loading the corresponding frame)
     - Item brackets [] load the corresponding frame
     - slice create an iterator
     - can relink to another RMF file
     """
    def __init__(self, model, rmf_file_name):
        """
        @param model: the IMP.Model()
        @param rmf_file_name: str, path of the rmf file
        """
        self.model = model
        try:
            self.rh_ref = RMF.open_rmf_file_read_only(rmf_file_name)
        except TypeError:
            raise TypeError("Wrong rmf file name or type: %s"
                            % str(rmf_file_name))
        hs = IMP.rmf.create_hierarchies(self.rh_ref, self.model)
        IMP.rmf.load_frame(self.rh_ref, RMF.FrameID(0))
        self.root_hier_ref = hs[0]
        IMP.atom.Hierarchy.__init__(self, self.root_hier_ref)
        self.model.update()
        self.ColorHierarchy = None

    def link_to_rmf(self, rmf_file_name):
        """
        Link to another RMF file
        """
        self.rh_ref = RMF.open_rmf_file_read_only(rmf_file_name)
        IMP.rmf.link_hierarchies(self.rh_ref, [self])
        if self.ColorHierarchy:
            self.ColorHierarchy.method()
        RMFHierarchyHandler.set_frame(self, 0)

    def set_frame(self, index):
        try:
            IMP.rmf.load_frame(self.rh_ref, RMF.FrameID(index))
        except:  # noqa: E722
            print("skipping frame %s:%d\n" % (self.current_rmf, index))
        self.model.update()

    def get_number_of_frames(self):
        return self.rh_ref.get_number_of_frames()

    def __getitem__(self, int_slice_adaptor):
        if isinstance(int_slice_adaptor, int):
            self.set_frame(int_slice_adaptor)
            return int_slice_adaptor
        elif isinstance(int_slice_adaptor, slice):
            return self.__iter__(int_slice_adaptor)
        else:
            raise TypeError("Unknown Type")

    def __len__(self):
        return self.get_number_of_frames()

    def __iter__(self, slice_key=None):
        if slice_key is None:
            for nframe in range(len(self)):
                yield self[nframe]
        else:
            for nframe in list(range(len(self)))[slice_key]:
                yield self[nframe]


class CacheHierarchyCoordinates(object):
    def __init__(self, StatHierarchyHandler):
        self.xyzs = []
        self.nrms = []
        self.rbs = []
        self.nrm_coors = {}
        self.xyz_coors = {}
        self.rb_trans = {}
        self.current_index = None
        self.rmfh = StatHierarchyHandler
        rbs, xyzs = IMP.pmi.tools.get_rbs_and_beads([self.rmfh])
        self.model = self.rmfh.get_model()
        self.rbs = rbs
        for xyz in xyzs:
            if IMP.core.NonRigidMember.get_is_setup(xyz):
                nrm = IMP.core.NonRigidMember(xyz)
                self.nrms.append(nrm)
            else:
                fb = IMP.core.XYZ(xyz)
                self.xyzs.append(fb)

    def do_store(self, index):
        self.rb_trans[index] = {}
        self.nrm_coors[index] = {}
        self.xyz_coors[index] = {}
        for rb in self.rbs:
            self.rb_trans[index][rb] = rb.get_reference_frame()
        for nrm in self.nrms:
            self.nrm_coors[index][nrm] = nrm.get_internal_coordinates()
        for xyz in self.xyzs:
            self.xyz_coors[index][xyz] = xyz.get_coordinates()
        self.current_index = index

    def do_update(self, index):
        if self.current_index != index:
            for rb in self.rbs:
                rb.set_reference_frame(self.rb_trans[index][rb])
            for nrm in self.nrms:
                nrm.set_internal_coordinates(self.nrm_coors[index][nrm])
            for xyz in self.xyzs:
                xyz.set_coordinates(self.xyz_coors[index][xyz])
            self.current_index = index
            self.model.update()

    def get_number_of_frames(self):
        return len(self.rb_trans.keys())

    def __getitem__(self, index):
        if isinstance(index, int):
            return index in self.rb_trans.keys()
        else:
            raise TypeError("Unknown Type")

    def __len__(self):
        return self.get_number_of_frames()


class StatHierarchyHandler(RMFHierarchyHandler):
    """ class to link stat files to several rmf files """
    def __init__(self, model=None, stat_file=None,
                 number_best_scoring_models=None, score_key=None,
                 StatHierarchyHandler=None, cache=None):
        """

        @param model: IMP.Model()
        @param stat_file: either 1) a list or 2) a single stat file names
               (either rmfs or ascii, or pickled data or pickled cluster),
               3) a dictionary containing an rmf/ascii
               stat file name as key and a list of frames as values
        @param number_best_scoring_models:
        @param StatHierarchyHandler: copy constructor input object
        @param cache: cache coordinates and rigid body transformations.
        """

        if StatHierarchyHandler is not None:
            # overrides all other arguments
            # copy constructor: create a copy with
            # different RMFHierarchyHandler
            self.model = StatHierarchyHandler.model
            self.data = StatHierarchyHandler.data
            self.number_best_scoring_models = \
                StatHierarchyHandler.number_best_scoring_models
            self.is_setup = True
            self.current_rmf = StatHierarchyHandler.current_rmf
            self.current_frame = None
            self.current_index = None
            self.score_threshold = StatHierarchyHandler.score_threshold
            self.score_key = StatHierarchyHandler.score_key
            self.cache = StatHierarchyHandler.cache
            RMFHierarchyHandler.__init__(self, self.model,
                                         self.current_rmf)
            if self.cache:
                self.cache = CacheHierarchyCoordinates(self)
            else:
                self.cache = None
            self.set_frame(0)

        else:
            # standard constructor
            self.model = model
            self.data = []
            self.number_best_scoring_models = number_best_scoring_models
            self.cache = cache

            if score_key is None:
                self.score_key = "Total_Score"
            else:
                self.score_key = score_key
            self.is_setup = None
            self.current_rmf = None
            self.current_frame = None
            self.current_index = None
            self.score_threshold = None

            if isinstance(stat_file, str):
                self.add_stat_file(stat_file)
            elif isinstance(stat_file, list):
                for f in stat_file:
                    self.add_stat_file(f)

    def add_stat_file(self, stat_file):
        try:
            '''check that it is not a pickle file with saved data
               from a previous calculation'''
            self.load_data(stat_file)

            if self.number_best_scoring_models:
                scores = self.get_scores()
                max_score = sorted(scores)[
                    0:min(len(self), self.number_best_scoring_models)][-1]
                self.do_filter_by_score(max_score)

        except pickle.UnpicklingError:
            '''alternatively read the ascii stat files'''
            try:
                scores, rmf_files, rmf_frame_indexes, features = \
                    self.get_info_from_stat_file(stat_file,
                                                 self.score_threshold)
            except (KeyError, SyntaxError):
                # in this case check that is it an rmf file, probably
                # without stat stored in
                try:
                    # let's see if that is an rmf file
                    rh = RMF.open_rmf_file_read_only(stat_file)
                    nframes = rh.get_number_of_frames()
                    scores = [0.0]*nframes
                    rmf_files = [stat_file]*nframes
                    rmf_frame_indexes = range(nframes)
                    features = {}
                except:  # noqa: E722
                    return

            if len(set(rmf_files)) > 1:
                raise ("Multiple RMF files found")

            if not rmf_files:
                print("StatHierarchyHandler: Error: Trying to set none as "
                      "rmf_file (probably empty stat file), aborting")
                return

            for n, index in enumerate(rmf_frame_indexes):
                featn_dict = dict([(k, features[k][n]) for k in features])
                self.data.append(IMP.pmi.output.DataEntry(
                    stat_file, rmf_files[n], index, scores[n], featn_dict))

            if self.number_best_scoring_models:
                scores = self.get_scores()
                max_score = sorted(scores)[
                    0:min(len(self), self.number_best_scoring_models)][-1]
                self.do_filter_by_score(max_score)

        if not self.is_setup:
            RMFHierarchyHandler.__init__(
                self, self.model, self.get_rmf_names()[0])
            if self.cache:
                self.cache = CacheHierarchyCoordinates(self)
            else:
                self.cache = None
            self.is_setup = True
            self.current_rmf = self.get_rmf_names()[0]

        self.set_frame(0)

    def save_data(self, filename='data.pkl'):
        with open(filename, 'wb') as fl:
            pickle.dump(self.data, fl)

    def load_data(self, filename='data.pkl'):
        with open(filename, 'rb') as fl:
            data_structure = pickle.load(fl)
        # first check that it is a list
        if not isinstance(data_structure, list):
            raise TypeError(
                "%filename should contain a list of IMP.pmi.output.DataEntry "
                "or IMP.pmi.output.Cluster" % filename)
        # second check the types
        if all(isinstance(item, IMP.pmi.output.DataEntry)
               for item in data_structure):
            self.data = data_structure
        elif all(isinstance(item, IMP.pmi.output.Cluster)
                 for item in data_structure):
            nmodels = 0
            for cluster in data_structure:
                nmodels += len(cluster)
            self.data = [None]*nmodels
            for cluster in data_structure:
                for n, data in enumerate(cluster):
                    index = cluster.members[n]
                    self.data[index] = data
        else:
            raise TypeError(
                "%filename should contain a list of IMP.pmi.output.DataEntry "
                "or IMP.pmi.output.Cluster" % filename)

    def set_frame(self, index):
        if self.cache is not None and self.cache[index]:
            self.cache.do_update(index)
        else:
            nm = self.data[index].rmf_name
            fidx = self.data[index].rmf_index
            if nm != self.current_rmf:
                self.link_to_rmf(nm)
                self.current_rmf = nm
                self.current_frame = -1
            if fidx != self.current_frame:
                RMFHierarchyHandler.set_frame(self, fidx)
                self.current_frame = fidx
            if self.cache is not None:
                self.cache.do_store(index)

        self.current_index = index

    def __getitem__(self, int_slice_adaptor):
        if isinstance(int_slice_adaptor, int):
            self.set_frame(int_slice_adaptor)
            return self.data[int_slice_adaptor]
        elif isinstance(int_slice_adaptor, slice):
            return self.__iter__(int_slice_adaptor)
        else:
            raise TypeError("Unknown Type")

    def __len__(self):
        return len(self.data)

    def __iter__(self, slice_key=None):
        if slice_key is None:
            for i in range(len(self)):
                yield self[i]
        else:
            for i in range(len(self))[slice_key]:
                yield self[i]

    def do_filter_by_score(self, maximum_score):
        self.data = [d for d in self.data if d.score <= maximum_score]

    def get_scores(self):
        return [d.score for d in self.data]

    def get_feature_series(self, feature_name):
        return [d.features[feature_name] for d in self.data]

    def get_feature_names(self):
        return self.data[0].features.keys()

    def get_rmf_names(self):
        return [d.rmf_name for d in self.data]

    def get_stat_files_names(self):
        return [d.stat_file for d in self.data]

    def get_rmf_indexes(self):
        return [d.rmf_index for d in self.data]

    def get_info_from_stat_file(self, stat_file, score_threshold=None):
        po = ProcessOutput(stat_file)
        fs = po.get_keys()
        models = IMP.pmi.io.get_best_models(
            [stat_file], score_key=self.score_key, feature_keys=fs,
            rmf_file_key="rmf_file", rmf_file_frame_key="rmf_frame_index",
            prefiltervalue=score_threshold, get_every=1)

        scores = [float(y) for y in models[2]]
        rmf_files = models[0]
        rmf_frame_indexes = models[1]
        features = models[3]
        return scores, rmf_files, rmf_frame_indexes, features


class DataEntry(object):
    '''
    A class to store data associated to a model
    '''
    def __init__(self, stat_file=None, rmf_name=None, rmf_index=None,
                 score=None, features=None):
        self.rmf_name = rmf_name
        self.rmf_index = rmf_index
        self.score = score
        self.features = features
        self.stat_file = stat_file

    def __repr__(self):
        s = "IMP.pmi.output.DataEntry\n"
        s += "---- stat file %s \n" % (self.stat_file)
        s += "---- rmf file %s \n" % (self.rmf_name)
        s += "---- rmf index %s \n" % (str(self.rmf_index))
        s += "---- score %s \n" % (str(self.score))
        s += "---- number of features %s \n" % (str(len(self.features.keys())))
        return s


class Cluster(object):
    '''
    A container for models organized into clusters
    '''
    def __init__(self, cid=None):
        self.cluster_id = cid
        self.members = []
        self.precision = None
        self.center_index = None
        self.members_data = {}

    def add_member(self, index, data=None):
        self.members.append(index)
        self.members_data[index] = data
        self.average_score = self.compute_score()

    def compute_score(self):
        try:
            score = sum([d.score for d in self])/len(self)
        except AttributeError:
            score = None
        return score

    def __repr__(self):
        s = "IMP.pmi.output.Cluster\n"
        s += "---- cluster_id %s \n" % str(self.cluster_id)
        s += "---- precision %s \n" % str(self.precision)
        s += "---- average score %s \n" % str(self.average_score)
        s += "---- number of members %s \n" % str(len(self.members))
        s += "---- center index %s \n" % str(self.center_index)
        return s

    def __getitem__(self, int_slice_adaptor):
        if isinstance(int_slice_adaptor, int):
            index = self.members[int_slice_adaptor]
            return self.members_data[index]
        elif isinstance(int_slice_adaptor, slice):
            return self.__iter__(int_slice_adaptor)
        else:
            raise TypeError("Unknown Type")

    def __len__(self):
        return len(self.members)

    def __iter__(self, slice_key=None):
        if slice_key is None:
            for i in range(len(self)):
                yield self[i]
        else:
            for i in range(len(self))[slice_key]:
                yield self[i]

    def __add__(self, other):
        self.members += other.members
        self.members_data.update(other.members_data)
        self.average_score = self.compute_score()
        self.precision = None
        self.center_index = None
        return self


def plot_clusters_populations(clusters):
    indexes = []
    populations = []
    for cluster in clusters:
        indexes.append(cluster.cluster_id)
        populations.append(len(cluster))

    import matplotlib.pyplot as plt
    fig, ax = plt.subplots()
    ax.bar(indexes, populations, 0.5, color='r')
    ax.set_ylabel('Population')
    ax.set_xlabel(('Cluster index'))
    plt.show()


def plot_clusters_precisions(clusters):
    indexes = []
    precisions = []
    for cluster in clusters:
        indexes.append(cluster.cluster_id)

        prec = cluster.precision
        print(cluster.cluster_id, prec)
        if prec is None:
            prec = 0.0
        precisions.append(prec)

    import matplotlib.pyplot as plt
    fig, ax = plt.subplots()
    ax.bar(indexes, precisions, 0.5, color='r')
    ax.set_ylabel('Precision [A]')
    ax.set_xlabel(('Cluster index'))
    plt.show()


def plot_clusters_scores(clusters):
    indexes = []
    values = []
    for cluster in clusters:
        indexes.append(cluster.cluster_id)
        values.append([])
        for data in cluster:
            values[-1].append(data.score)

    plot_fields_box_plots("scores.pdf", values, indexes, frequencies=None,
                          valuename="Scores", positionname="Cluster index",
                          xlabels=None, scale_plot_length=1.0)


class CrossLinkIdentifierDatabase(object):
    def __init__(self):
        self.clidb = dict()

    def check_key(self, key):
        if key not in self.clidb:
            self.clidb[key] = {}

    def set_unique_id(self, key, value):
        self.check_key(key)
        self.clidb[key]["XLUniqueID"] = str(value)

    def set_protein1(self, key, value):
        self.check_key(key)
        self.clidb[key]["Protein1"] = str(value)

    def set_protein2(self, key, value):
        self.check_key(key)
        self.clidb[key]["Protein2"] = str(value)

    def set_residue1(self, key, value):
        self.check_key(key)
        self.clidb[key]["Residue1"] = int(value)

    def set_residue2(self, key, value):
        self.check_key(key)
        self.clidb[key]["Residue2"] = int(value)

    def set_idscore(self, key, value):
        self.check_key(key)
        self.clidb[key]["IDScore"] = float(value)

    def set_state(self, key, value):
        self.check_key(key)
        self.clidb[key]["State"] = int(value)

    def set_sigma1(self, key, value):
        self.check_key(key)
        self.clidb[key]["Sigma1"] = str(value)

    def set_sigma2(self, key, value):
        self.check_key(key)
        self.clidb[key]["Sigma2"] = str(value)

    def set_psi(self, key, value):
        self.check_key(key)
        self.clidb[key]["Psi"] = str(value)

    def get_unique_id(self, key):
        return self.clidb[key]["XLUniqueID"]

    def get_protein1(self, key):
        return self.clidb[key]["Protein1"]

    def get_protein2(self, key):
        return self.clidb[key]["Protein2"]

    def get_residue1(self, key):
        return self.clidb[key]["Residue1"]

    def get_residue2(self, key):
        return self.clidb[key]["Residue2"]

    def get_idscore(self, key):
        return self.clidb[key]["IDScore"]

    def get_state(self, key):
        return self.clidb[key]["State"]

    def get_sigma1(self, key):
        return self.clidb[key]["Sigma1"]

    def get_sigma2(self, key):
        return self.clidb[key]["Sigma2"]

    def get_psi(self, key):
        return self.clidb[key]["Psi"]

    def set_float_feature(self, key, value, feature_name):
        self.check_key(key)
        self.clidb[key][feature_name] = float(value)

    def set_int_feature(self, key, value, feature_name):
        self.check_key(key)
        self.clidb[key][feature_name] = int(value)

    def set_string_feature(self, key, value, feature_name):
        self.check_key(key)
        self.clidb[key][feature_name] = str(value)

    def get_feature(self, key, feature_name):
        return self.clidb[key][feature_name]

    def write(self, filename):
        with open(filename, 'wb') as handle:
            pickle.dump(self.clidb, handle)

    def load(self, filename):
        with open(filename, 'rb') as handle:
            self.clidb = pickle.load(handle)


def plot_fields(fields, output, framemin=None, framemax=None):
    """Plot the given fields and save a figure as `output`.
       The fields generally are extracted from a stat file
       using ProcessOutput.get_fields()."""
    import matplotlib as mpl
    mpl.use('Agg')
    import matplotlib.pyplot as plt

    plt.rc('lines', linewidth=4)
    fig, axs = plt.subplots(nrows=len(fields))
    fig.set_size_inches(10.5, 5.5 * len(fields))
    plt.rc('axes')

    n = 0
    for key in fields:
        if framemin is None:
            framemin = 0
        if framemax is None:
            framemax = len(fields[key])
        x = list(range(framemin, framemax))
        y = [float(y) for y in fields[key][framemin:framemax]]
        if len(fields) > 1:
            axs[n].plot(x, y)
            axs[n].set_title(key, size="xx-large")
            axs[n].tick_params(labelsize=18, pad=10)
        else:
            axs.plot(x, y)
            axs.set_title(key, size="xx-large")
            axs.tick_params(labelsize=18, pad=10)
        n += 1

    # Tweak spacing between subplots to prevent labels from overlapping
    plt.subplots_adjust(hspace=0.3)
    plt.savefig(output)


def plot_field_histogram(name, values_lists, valuename=None, bins=40,
                         colors=None, format="png", reference_xline=None,
                         yplotrange=None, xplotrange=None, normalized=True,
                         leg_names=None):
    '''Plot a list of histograms from a value list.
    @param name the name of the plot
    @param value_lists the list of list of values eg: [[...],[...],[...]]
    @param valuename the y-label
    @param bins the number of bins
    @param colors If None, will use rainbow. Else will use specific list
    @param format output format
    @param reference_xline plot a reference line parallel to the y-axis
    @param yplotrange the range for the y-axis
    @param normalized whether the histogram is normalized or not
    @param leg_names names for the legend
    '''

    import matplotlib as mpl
    mpl.use('Agg')
    import matplotlib.pyplot as plt
    import matplotlib.cm as cm
    plt.figure(figsize=(18.0, 9.0))

    if colors is None:
        colors = cm.rainbow(np.linspace(0, 1, len(values_lists)))
    for nv, values in enumerate(values_lists):
        col = colors[nv]
        if leg_names is not None:
            label = leg_names[nv]
        else:
            label = str(nv)
        plt.hist(
            [float(y) for y in values], bins=bins, color=col,
            density=normalized, histtype='step', lw=4, label=label)

    # plt.title(name,size="xx-large")
    plt.tick_params(labelsize=12, pad=10)
    if valuename is None:
        plt.xlabel(name, size="xx-large")
    else:
        plt.xlabel(valuename, size="xx-large")
    plt.ylabel("Frequency", size="xx-large")

    if yplotrange is not None:
        plt.ylim()
    if xplotrange is not None:
        plt.xlim(xplotrange)

    plt.legend(loc=2)

    if reference_xline is not None:
        plt.axvline(
            reference_xline,
            color='red',
            linestyle='dashed',
            linewidth=1)

    plt.savefig(name + "." + format, dpi=150, transparent=True)


def plot_fields_box_plots(name, values, positions, frequencies=None,
                          valuename="None", positionname="None",
                          xlabels=None, scale_plot_length=1.0):
    '''
    Plot time series as boxplots.
    fields is a list of time series, positions are the x-values
    valuename is the y-label, positionname is the x-label
    '''

    import matplotlib as mpl
    mpl.use('Agg')
    import matplotlib.pyplot as plt

    bps = []
    fig = plt.figure(figsize=(float(len(positions))*scale_plot_length, 5.0))
    fig.canvas.set_window_title(name)

    ax1 = fig.add_subplot(111)

    plt.subplots_adjust(left=0.1, right=0.990, top=0.95, bottom=0.4)

    bps.append(plt.boxplot(values, notch=0, sym='', vert=1,
                           whis=1.5, positions=positions))

    plt.setp(bps[-1]['boxes'], color='black', lw=1.5)
    plt.setp(bps[-1]['whiskers'], color='black', ls=":", lw=1.5)

    if frequencies is not None:
        for n, v in enumerate(values):
            plist = [positions[n]]*len(v)
            ax1.plot(plist, v, 'gx', alpha=0.7, markersize=7)

    # print ax1.xaxis.get_majorticklocs()
    if xlabels is not None:
        ax1.set_xticklabels(xlabels)
    plt.xticks(rotation=90)
    plt.xlabel(positionname)
    plt.ylabel(valuename)

    plt.savefig(name + ".pdf", dpi=150)
    plt.show()


def plot_xy_data(x, y, title=None, out_fn=None, display=True,
                 set_plot_yaxis_range=None, xlabel=None, ylabel=None):
    import matplotlib as mpl
    mpl.use('Agg')
    import matplotlib.pyplot as plt
    plt.rc('lines', linewidth=2)

    fig, ax = plt.subplots(nrows=1)
    fig.set_size_inches(8, 4.5)
    if title is not None:
        fig.canvas.set_window_title(title)

    ax.plot(x, y, color='r')
    if set_plot_yaxis_range is not None:
        x1, x2, y1, y2 = plt.axis()
        y1 = set_plot_yaxis_range[0]
        y2 = set_plot_yaxis_range[1]
        plt.axis((x1, x2, y1, y2))
    if title is not None:
        ax.set_title(title)
    if xlabel is not None:
        ax.set_xlabel(xlabel)
    if ylabel is not None:
        ax.set_ylabel(ylabel)
    if out_fn is not None:
        plt.savefig(out_fn + ".pdf")
    if display:
        plt.show()
    plt.close(fig)


def plot_scatter_xy_data(x, y, labelx="None", labely="None",
                         xmin=None, xmax=None, ymin=None, ymax=None,
                         savefile=False, filename="None.eps", alpha=0.75):

    import matplotlib as mpl
    mpl.use('Agg')
    import matplotlib.pyplot as plt
    from matplotlib import rc
    rc('font', **{'family': 'sans-serif', 'sans-serif': ['Helvetica']})

    fig, axs = plt.subplots(1)

    axs0 = axs

    axs0.set_xlabel(labelx, size="xx-large")
    axs0.set_ylabel(labely, size="xx-large")
    axs0.tick_params(labelsize=18, pad=10)

    plot2 = []

    plot2.append(axs0.plot(x, y, 'o', color='k', lw=2, ms=0.1, alpha=alpha,
                           c="w"))

    axs0.legend(
        loc=0,
        frameon=False,
        scatterpoints=1,
        numpoints=1,
        columnspacing=1)

    fig.set_size_inches(8.0, 8.0)
    fig.subplots_adjust(left=0.161, right=0.850, top=0.95, bottom=0.11)
    if (ymin is not None) and (ymax is not None):
        axs0.set_ylim(ymin, ymax)
    if (xmin is not None) and (xmax is not None):
        axs0.set_xlim(xmin, xmax)

    if savefile:
        fig.savefig(filename, dpi=300)


def get_graph_from_hierarchy(hier):
    graph = []
    depth_dict = {}
    depth = 0
    (graph, depth, depth_dict) = recursive_graph(
        hier, graph, depth, depth_dict)

    # filters node labels according to depth_dict
    node_labels_dict = {}
    for key in depth_dict:
        if depth_dict[key] < 3:
            node_labels_dict[key] = key
        else:
            node_labels_dict[key] = ""
    draw_graph(graph, labels_dict=node_labels_dict)


def recursive_graph(hier, graph, depth, depth_dict):
    depth = depth + 1
    nameh = IMP.atom.Hierarchy(hier).get_name()
    index = str(hier.get_particle().get_index())
    name1 = nameh + "|#" + index
    depth_dict[name1] = depth

    children = IMP.atom.Hierarchy(hier).get_children()

    if len(children) == 1 or children is None:
        depth = depth - 1
        return (graph, depth, depth_dict)

    else:
        for c in children:
            (graph, depth, depth_dict) = recursive_graph(
                c, graph, depth, depth_dict)
            nameh = IMP.atom.Hierarchy(c).get_name()
            index = str(c.get_particle().get_index())
            namec = nameh + "|#" + index
            graph.append((name1, namec))

        depth = depth - 1
        return (graph, depth, depth_dict)


def draw_graph(graph, labels_dict=None, graph_layout='spring',
               node_size=5, node_color=None, node_alpha=0.3,
               node_text_size=11, fixed=None, pos=None,
               edge_color='blue', edge_alpha=0.3, edge_thickness=1,
               edge_text_pos=0.3,
               validation_edges=None,
               text_font='sans-serif',
               out_filename=None):

    import matplotlib as mpl
    mpl.use('Agg')
    import networkx as nx
    import matplotlib.pyplot as plt
    from math import sqrt, pi

    # create networkx graph
    G = nx.Graph()

    # add edges
    if isinstance(edge_thickness, list):
        for edge, weight in zip(graph, edge_thickness):
            G.add_edge(edge[0], edge[1], weight=weight)
    else:
        for edge in graph:
            G.add_edge(edge[0], edge[1])

    if node_color is None:
        node_color_rgb = (0, 0, 0)
        node_color_hex = "000000"
    else:
        cc = IMP.pmi.tools.ColorChange()
        tmpcolor_rgb = []
        tmpcolor_hex = []
        for node in G.nodes():
            cctuple = cc.rgb(node_color[node])
            tmpcolor_rgb.append((cctuple[0]/255,
                                 cctuple[1]/255,
                                 cctuple[2]/255))
            tmpcolor_hex.append(node_color[node])
        node_color_rgb = tmpcolor_rgb
        node_color_hex = tmpcolor_hex

    # get node sizes if dictionary
    if isinstance(node_size, dict):
        tmpsize = []
        for node in G.nodes():
            size = sqrt(node_size[node])/pi*10.0
            tmpsize.append(size)
        node_size = tmpsize

    for n, node in enumerate(G.nodes()):
        color = node_color_hex[n]
        size = node_size[n]
        nx.set_node_attributes(
            G, "graphics",
            {node: {'type': 'ellipse', 'w': size, 'h': size,
                    'fill': '#' + color, 'label': node}})
        nx.set_node_attributes(
            G, "LabelGraphics",
            {node: {'type': 'text', 'text': node, 'color': '#000000',
                    'visible': 'true'}})

    for edge in G.edges():
        nx.set_edge_attributes(
            G, "graphics",
            {edge: {'width': 1, 'fill': '#000000'}})

    for ve in validation_edges:
        print(ve)
        if (ve[0], ve[1]) in G.edges():
            print("found forward")
            nx.set_edge_attributes(
                G, "graphics",
                {ve: {'width': 1, 'fill': '#00FF00'}})
        elif (ve[1], ve[0]) in G.edges():
            print("found backward")
            nx.set_edge_attributes(
                G, "graphics",
                {(ve[1], ve[0]): {'width': 1, 'fill': '#00FF00'}})
        else:
            G.add_edge(ve[0], ve[1])
            print("not found")
            nx.set_edge_attributes(
                G, "graphics",
                {ve: {'width': 1, 'fill': '#FF0000'}})

    # these are different layouts for the network you may try
    # shell seems to work best
    if graph_layout == 'spring':
        print(fixed, pos)
        graph_pos = nx.spring_layout(G, k=1.0/8.0, fixed=fixed, pos=pos)
    elif graph_layout == 'spectral':
        graph_pos = nx.spectral_layout(G)
    elif graph_layout == 'random':
        graph_pos = nx.random_layout(G)
    else:
        graph_pos = nx.shell_layout(G)

    # draw graph
    nx.draw_networkx_nodes(G, graph_pos, node_size=node_size,
                           alpha=node_alpha, node_color=node_color_rgb,
                           linewidths=0)
    nx.draw_networkx_edges(G, graph_pos, width=edge_thickness,
                           alpha=edge_alpha, edge_color=edge_color)
    nx.draw_networkx_labels(
        G, graph_pos, labels=labels_dict, font_size=node_text_size,
        font_family=text_font)
    if out_filename:
        plt.savefig(out_filename)
        nx.write_gml(G, 'out.gml')
    plt.show()


def draw_table():

    # still an example!

    from ipyD3 import d3object
    from IPython.display import display

    d3 = d3object(width=800,
                  height=400,
                  style='JFTable',
                  number=1,
                  d3=None,
                  title='Example table with d3js',
                  desc='An example table created created with d3js with '
                       'data generated with Python.')
    data = [[1277.0, 654.0, 288.0, 1976.0, 3281.0, 3089.0, 10336.0, 4650.0,
             4441.0, 4670.0, 944.0, 110.0],
            [1318.0, 664.0, 418.0, 1952.0, 3581.0, 4574.0, 11457.0, 6139.0,
             7078.0, 6561.0, 2354.0, 710.0],
            [1783.0, 774.0, 564.0, 1470.0, 3571.0, 3103.0, 9392.0, 5532.0,
             5661.0, 4991.0, 2032.0, 680.0],
            [1301.0, 604.0, 286.0, 2152.0, 3282.0, 3369.0, 10490.0, 5406.0,
             4727.0, 3428.0, 1559.0, 620.0],
            [1537.0, 1714.0, 724.0, 4824.0, 5551.0, 8096.0, 16589.0, 13650.0,
             9552.0, 13709.0, 2460.0, 720.0],
            [5691.0, 2995.0, 1680.0, 11741.0, 16232.0, 14731.0, 43522.0,
             32794.0, 26634.0, 31400.0, 7350.0, 3010.0],
            [1650.0, 2096.0, 60.0, 50.0, 1180.0, 5602.0, 15728.0, 6874.0,
             5115.0, 3510.0, 1390.0, 170.0],
            [72.0, 60.0, 60.0, 10.0, 120.0, 172.0, 1092.0, 675.0, 408.0,
             360.0, 156.0, 100.0]]
    data = [list(i) for i in zip(*data)]
    sRows = [['January',
              'February',
              'March',
              'April',
              'May',
              'June',
              'July',
              'August',
              'September',
              'October',
              'November',
              'Deecember']]
    sColumns = [['Prod {0}'.format(i) for i in range(1, 9)],
                [None, '', None, None, 'Group 1', None, None, 'Group 2']]
    d3.addSimpleTable(data,
                      fontSizeCells=[12, ],
                      sRows=sRows,
                      sColumns=sColumns,
                      sRowsMargins=[5, 50, 0],
                      sColsMargins=[5, 20, 10],
                      spacing=0,
                      addBorders=1,
                      addOutsideBorders=-1,
                      rectWidth=45,
                      rectHeight=0
                      )
    html = d3.render(mode=['html', 'show'])
    display(html)
