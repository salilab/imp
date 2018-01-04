"""@namespace IMP.mmcif.util
   @brief Utility functions for IMP.mmcif.
"""

import IMP.mmcif.dumper
import IMP.mmcif.dataset
import IMP.mmcif.data
import IMP.mmcif.format
import IMP.mmcif.restraint
import IMP.rmf
import IMP.atom
import RMF
import string
import weakref
import operator

class RMFFrame(object):
    """An individual state conformation read from a PDB file"""
    def __init__(self, filename, frame, name):
        self.filename, self.frame = filename, frame
        self.name = name

    def create(self, model):
        rmf = RMF.open_rmf_file_read_only(self.filename)
        # todo: support frame!=0
        hiers = IMP.rmf.create_hierarchies(rmf, model)
        restraints = IMP.rmf.create_restraints(rmf, model)
        return hiers, restraints

    def link(self, hiers, restraints):
        rmf = RMF.open_rmf_file_read_only(self.filename)
        IMP.rmf.link_hierarchies(rmf, hiers)
        IMP.rmf.link_restraints(rmf, restraints)
        IMP.rmf.load_frame(rmf, RMF.FrameID(self.frame))


class _ModelFrame(object):
    """An individual state conformation read from an IMP.Model"""
    def __init__(self, hiers, restraints, name):
        self.hiers, self.restraints = hiers, restraints
        self.name = name

    def create(self, model):
        return self.hiers, self.restraints

    def link(self, hiers, restraints):
        if len(hiers) != len(self.hiers) \
           or len(restraints) != len(self.restraints):
            raise ValueError("Frames do not match")
        hiers[:] = self.hiers
        # todo: this won't work currently because the Restraint objects
        # will change
        restraints[:] = self.restraints


class _NonModeledChain(object):
    """Represent a chain that was experimentally characterized but not modeled.
       Such a chain resembles an IMP.atom.Chain, but has no associated
       structure, and belongs to no state."""
    def __init__(self, name, sequence, chain_type):
        self.name = name
        self.sequence = sequence
        self.chain_type = chain_type

    get_sequence = lambda self: self.sequence


class System(object):
    def __init__(self):
        self._states = {}
        self._ensembles = []
        self._frames = []
        self._assemblies = IMP.mmcif.data._Assemblies()
        # The assembly of all known components.
        self.complete_assembly = IMP.mmcif.data._Assembly()
        self._assemblies.add(self.complete_assembly)

        self._dumpers = [IMP.mmcif.dumper._EntryDumper(), # must be first
                         IMP.mmcif.dumper._SoftwareDumper(),
                         IMP.mmcif.dumper._CitationDumper(),
                         IMP.mmcif.dumper._ChemCompDumper(),
                         IMP.mmcif.dumper._EntityDumper(),
                         IMP.mmcif.dumper._EntityPolyDumper(),
                         IMP.mmcif.dumper._EntityPolySeqDumper(),
                         IMP.mmcif.dumper._StructAsymDumper(),
                         IMP.mmcif.dumper._AssemblyDumper(),
                         IMP.mmcif.dumper._ModelRepresentationDumper(),
                         IMP.mmcif.dumper._ExternalReferenceDumper(),
                         IMP.mmcif.dumper._DatasetDumper(),
                         IMP.mmcif.dumper._StartingModelDumper(),
                         IMP.mmcif.dumper._ProtocolDumper(),
                         IMP.mmcif.dumper._PostProcessDumper(),
                         IMP.mmcif.dumper._EnsembleDumper(),
                         IMP.mmcif.dumper._ModelListDumper(),
                         IMP.mmcif.dumper._EM3DDumper(),
                         IMP.mmcif.dumper._SiteDumper()]
        self.entities = IMP.mmcif.data._EntityMapper()
        self.components = IMP.mmcif.data._ComponentMapper()
        self._citations = []
        self._software = IMP.mmcif.data._AllSoftware()
        self._external_files = IMP.mmcif.data._ExternalFiles()
        self.datasets = IMP.mmcif.data._Datasets(self._external_files)
        # All modeling protocols
        self.protocols = IMP.mmcif.data._Protocols()

    def _update_location(self, fileloc):
        """Update FileLocation to point to a parent repository, if any"""
        IMP.mmcif.dataset.Repository.update_in_repos(fileloc,
                                                   self._external_files._repos)

    def add_repository(self, doi, root=None, url=None, top_directory=None):
        """Add a repository containing one or more modeling files."""
        self._external_files.add_repo(IMP.mmcif.dataset.Repository(
                                            doi, root, url, top_directory))

    def add_citation(self, pmid, title, journal, volume, page_range, year,
                     authors, doi):
        """Add a publication that describes the modeling"""
        self._citations.append(IMP.mmcif.data._Citation(
                    pmid, title, journal, volume, page_range, year,
                    authors, doi))

    def add_software(self, name, classification, description, url,
                     type='program', version=None):
        "Add software (other than IMP) used as part of the modeling protocol."
        self._software.append(IMP.mmcif.data._Software(
                    name, classification, description, url, type, version))

    def _add_state(self, state):
        self._states[state] = None
        state.id = len(self._states)

    def _add_ensemble(self, ensemble):
        self._ensembles.append(ensemble)
        ensemble.id = len(self._ensembles)

    def _add_frame(self, frame):
        self._frames.append(frame)
        frame.id = len(self._frames)

    def _add_hierarchy(self, h, state):
        chains = [IMP.atom.Chain(c)
                  for c in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)]
        if len(chains) == 0:
            raise ValueError("No chains found in %s" % h)
        # todo: handle same chain in multiple states
        for c in chains:
            component = self._add_chain(c)
            state._all_modeled_components.append(component)
            state.modeled_assembly.append(component)
            state.representation[component] \
                = list(self._get_representation(c,
                                     self._get_all_starting_models(component)))
        self.protocols._add_hierarchy(h, state.modeled_assembly)
        self._external_files.add_hierarchy(h)
        self._software.add_hierarchy(h)

    def _get_all_starting_models(self, comp):
        """Get all starting models (in all states) for the given component"""
        for state in self._states:
            for rep in state.representation.get(comp, []):
                if rep.starting_model:
                    yield rep.starting_model

    def _get_representation(self, chain, existing_starting_models):
        """Yield groups of particles under chain with same representation"""
        smf = IMP.mmcif.data._StartingModelFinder(existing_starting_models)
        rep = IMP.mmcif.data._Representation()
        for sp in self._get_structure_particles(chain):
            starting_model = smf.find(sp, self)
            if not rep.add(sp, starting_model):
                yield rep
                rep = IMP.mmcif.data._Representation()
                rep.add(sp, starting_model)
        if rep:
            yield rep

    def _get_structure_particles(self, chain):
        """Yield all particles under chain with coordinates.
           They are sorted by residue index."""
        # todo: handle Representation decorators for non-PMI-1 models
        ps = IMP.atom.get_leaves(chain)
        resind_dict = {}
        for p in ps:
            if IMP.atom.Residue.get_is_setup(p):
                residue = IMP.atom.Residue(p)
                resind = residue.get_index()
                if resind in resind_dict:
                    continue
                resind_dict[resind] = residue
            elif IMP.atom.Fragment.get_is_setup(p):
                fragment = IMP.atom.Fragment(p)
                # todo: handle non-contiguous fragments
                resinds = fragment.get_residue_indexes()
                resind = resinds[len(resinds) // 2]
                if resind in resind_dict:
                    continue
                resind_dict[resind] = fragment
        # Return values sorted by key (residue index)
        for item in sorted(resind_dict.items(), key=operator.itemgetter(0)):
            yield item[1]

    def add_non_modeled_chain(self, name, sequence,
                              chain_type=IMP.atom.UnknownChainType):
        """Add a chain that wasn't modeled by IMP."""
        c = _NonModeledChain(name, sequence, chain_type)
        self._add_chain(c)

    def _add_chain(self, c):
        entity = self.entities.add(c)
        component = self.components.add(c, entity)
        if component not in self.complete_assembly:
            self.complete_assembly.append(component)
        return component

    def write(self, fname):
        with open(fname, 'w') as fh:
            writer = IMP.mmcif.format.CifWriter(fh)
            for dumper in self._dumpers:
                dumper.finalize_metadata(self)
            for dumper in self._dumpers:
                dumper.finalize(self)
            for dumper in self._dumpers:
                dumper.dump(self, writer)

class State(object):
    """Represent a single IMP state."""
    def __init__(self, system):
        self.system = weakref.proxy(system)
        system._add_state(self)
        self.model = IMP.Model()
        self.hiers = None
        self._wrapped_restraints = []
        # The assembly of all components modeled by IMP in this state.
        # This may be smaller than the complete assembly.
        self.modeled_assembly = IMP.mmcif.data._Assembly()
        system._assemblies.add(self.modeled_assembly)
        # A list of Representation objects for each Component
        self.representation = {}
        self._frames = []

        self._all_modeled_components = []

    def _add_frame(self, f):
        self._frames.append(f)
        self.system._add_frame(f)
        if self._load_frame(f):
            for h in self.hiers:
                self._add_hierarchy(h)
            self._add_restraints(self.restraints, f)
        else:
            self._update_restraints(f)

    def _load_frame(self, f):
        """Load hierarchies and restraints from a frame.
           Return True if this results in making new hierarchies."""
        if self.hiers is None:
            self.hiers, self.restraints = f.create(self.model)
            self._remove_duplicate_chain_ids(True)
            return True
        else:
            f.link(self.hiers, self.restraints)
            self._remove_duplicate_chain_ids(False)
            return False

    def _remove_duplicate_chain_ids(self, new_hiers):
        chains = []
        for h in self.hiers:
            chains.extend(IMP.atom.Chain(c)
                          for c in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE))
        if new_hiers:
            self._assigned_chain_ids = []
            chain_ids = [c.get_id() for c in chains]
            if len(set(chain_ids)) < len(chain_ids):
                print("Duplicate chain IDs detected - reassigning "
                      "alphabetically")
                for chain, cid in zip(chains, self._get_alpha_chain_ids()):
                    self._assigned_chain_ids.append(cid)
                    chain.set_id(cid)
        else:
            for chain, cid in zip(chains, self._assigned_chain_ids):
                chain.set_id(cid)

    def _get_alpha_chain_ids(self):
        """Yield alphabetical chain IDs.
           We label the first 26 chains A-Z, then we move to two-letter
           chain IDs: AA through AZ, then BA through BZ, through to ZZ."""
        for cid in string.uppercase:
            yield cid
        for al1 in string.uppercase:
            for al2 in string.uppercase:
                yield al1 + al2
        raise ValueError("Out of chain IDs")

    def _add_hierarchy(self, h):
        self.system._add_hierarchy(h, self)

    def _add_restraints(self, rs, frame):
        m = IMP.mmcif.restraint._RestraintMapper(self.system)
        for r in rs:
            rw = m.handle(r, frame)
            if rw:
                self._wrapped_restraints.append(rw)

    def _update_restraints(self, frame):
        for rw in self._wrapped_restraints:
            rw._get_frame_info(frame)


class Ensemble(object):
    """Represent a set of similar models in a state."""
    def __init__(self, state, name):
        self.state = weakref.proxy(state)
        state.system._add_ensemble(self)
        self.name = name
        self._frames = []

    def add_frame(self, frame):
        """Add a frame from a custom source"""
        self._frames.append(frame)
        self.state._add_frame(frame)

    def add_rmf(self, fname, name, frame=0):
        """Add a frame from an RMF file"""
        self.add_frame(RMFFrame(fname, frame, name))

    def add_model(self, hiers, restraints, name):
        """Add hierarchies and restraints from an IMP.Model"""
        self.add_frame(_ModelFrame(hiers, restraints, name))
