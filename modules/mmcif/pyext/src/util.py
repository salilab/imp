"""@namespace IMP.mmcif.util
   @brief Utility functions for IMP.mmcif.
"""

import ihm.location
import ihm.dumper
import IMP.mmcif.data
import IMP.mmcif.restraint
import IMP.rmf
import IMP.atom
import RMF
import ihm.format
import ihm.representation
import string
import weakref
import operator
import json
import sys
from IMP.mmcif.data import _get_all_state_provenance


if sys.version_info[0] == 2:
    # basestring is base for both str and unicode (used by json)
    _string_type = basestring  # noqa: F821
else:
    _string_type = str


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

    def get_sequence(self):
        return self.sequence


class System(object):
    def __init__(self):
        self.system = ihm.System()
        self._states = []
        self._ensembles = []
        self._frames = []

        self.entities = IMP.mmcif.data._EntityMapper(self.system)
        self.components = IMP.mmcif.data._ComponentMapper(self.system)
        self._software = IMP.mmcif.data._AllSoftware(self.system)
        self._external_files = IMP.mmcif.data._ExternalFiles(self.system)
        self.datasets = IMP.mmcif.data._Datasets(self.system)
        # All modeling protocols
        self.protocols = IMP.mmcif.data._Protocols(self.system)
        self.representation = ihm.representation.Representation()
        self.system.orphan_representations.append(self.representation)

    def _update_location(self, fileloc):
        """Update FileLocation to point to a parent repository, if any"""
        ihm.location.Repository._update_in_repos(fileloc,
                                                 self._external_files._repos)

    def add_repository(self, doi, root=None, url=None, top_directory=None):
        """Add a repository containing one or more modeling files."""
        self._external_files.add_repo(ihm.location.Repository(
                                            doi, root, url, top_directory))

    def report(self, fh=sys.stdout):
        """Use python-ihm to print a summary report of this system."""
        self.system.report(fh)

    def _parse_sel_tuple(self, t):
        """Convert a PMI-style selection tuple into an IHM object"""
        asym_map = {}
        for asym in self.system.asym_units:
            asym_map[asym.details] = asym
        if isinstance(t, _string_type):
            return asym_map[t]
        elif isinstance(t, (list, tuple)) and len(t) == 3:
            return asym_map[t[2]](t[0], t[1])
        else:
            raise TypeError("Cannot handle selection tuple: %s" % t)

    def _add_state(self, state):
        if not self.system.state_groups:
            self.system.state_groups.append(ihm.model.StateGroup())
        self.system.state_groups[-1].append(state)
        self._states.append(state)

    def _add_ensemble(self, ensemble):
        self._ensembles.append(ensemble)
        self.system.ensembles.append(ensemble)

    def _add_frame(self, frame):
        self._frames.append(frame)
        frame.id = len(self._frames)

    def _add_hierarchy(self, h, state):
        if self.system.title is None:
            self.system.title = h.get_name()
        chains = [IMP.atom.Chain(c)
                  for c in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)]
        if len(chains) == 0:
            raise ValueError("No chains found in %s" % h)
        # todo: handle same chain in multiple states
        for c in chains:
            component = self._add_chain(c)
            state._all_modeled_components.append(component)
            if hasattr(component, 'asym_unit'):
                state.modeled_assembly.append(component.asym_unit)
            else:
                state.modeled_assembly.append(component.entity)
            state.repsegments[component] = \
                list(self._get_repsegments(
                    c, component, self._get_all_starting_models(component)))
            # Number of states that have representation for this component
            num_state_reps = len([s for s in self._states
                                  if component in s.repsegments])
            # Assume representation for a given component is the same in all
            # states, so we only need one copy of it in the mmCIF file
            if num_state_reps == 1:
                self.representation.extend(state.repsegments[component])
        self._software.add_hierarchy(h)
        self.protocols._add_hierarchy(h, None, state.modeled_assembly,
                                      self._software)
        self._external_files.add_hierarchy(h)

    def _get_all_starting_models(self, comp):
        """Get all starting models (in all states) for the given component"""
        for state in self._states:
            for seg in state.repsegments.get(comp, []):
                if seg.starting_model:
                    yield seg.starting_model

    def _get_repsegments(self, chain, component, existing_starting_models):
        """Yield groups of particles under chain with same representation"""
        smf = IMP.mmcif.data._StartingModelFinder(component.asym_unit,
                                                  existing_starting_models,
                                                  self.system, self.datasets)
        segfactory = IMP.mmcif.data._RepSegmentFactory(component.asym_unit)

        for sp in self._get_structure_particles(chain):
            starting_model = smf.find(sp)
            seg = segfactory.add(sp, starting_model)
            if seg:
                yield seg
        last = segfactory.get_last()
        if last:
            yield last

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
        return component

    def write(self, fname):
        with open(fname, 'w') as fh:
            ihm.dumper.write(fh, [self.system])


class State(ihm.model.State):
    """Represent a single IMP state."""
    def __init__(self, system):
        super(State, self).__init__()
        self.system = weakref.proxy(system)
        system._add_state(self)
        self.model = IMP.Model()
        self.hiers = None
        self._wrapped_restraints = []
        # The assembly of all components modeled by IMP in this state.
        # This may be smaller than the complete assembly.
        self.modeled_assembly = ihm.Assembly(
            name="Modeled assembly",
            description="All components modeled by IMP")
        system.system.orphan_assemblies.append(self.modeled_assembly)
        # A list of ihm.representation.Segment objects for each Component
        self.repsegments = {}
        self._frames = []

        self._all_modeled_components = []

    def _add_frame(self, f, model):
        self._frames.append(f)
        self.system._add_frame(f)
        if self._load_frame(f):
            for h in self.hiers:
                self._add_hierarchy(h)
            self._add_restraints(self.restraints, model)
        else:
            self._update_restraints(model)

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
            chains.extend(
                IMP.atom.Chain(c)
                for c in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE))
        if new_hiers:
            self._assigned_chain_ids = []
            chain_ids = [c.get_id() for c in chains]
            if len(set(chain_ids)) < len(chain_ids):
                print("Duplicate chain IDs detected - reassigning "
                      "alphabetically")
                for chain, cid in zip(chains, _ChainIDs()):
                    self._assigned_chain_ids.append(cid)
                    chain.set_id(cid)
        else:
            for chain, cid in zip(chains, self._assigned_chain_ids):
                chain.set_id(cid)

    def _add_hierarchy(self, h):
        self.system._add_hierarchy(h, self)

    def _add_restraints(self, rs, model):
        mapper = IMP.mmcif.restraint._RestraintMapper(self.system)
        for r in rs:
            self._handle_restraint(mapper, r, model)

    def _handle_restraint(self, mapper, r, model):
        rw = mapper.handle(r, model, self.modeled_assembly)
        if rw:
            self._wrapped_restraints.append(rw)
            self.system.system.restraints.append(rw)
        else:
            try:
                rs = IMP.RestraintSet.get_from(r)
            except ValueError:
                rs = None
            if rs:
                for child in rs.restraints:
                    self._handle_restraint(mapper, child, model)

    def _update_restraints(self, model):
        for rw in self._wrapped_restraints:
            rw.add_model_fit(model)


class Ensemble(ihm.model.Ensemble):
    """Represent a set of similar models in a state."""
    def __init__(self, state, name):
        self.state = weakref.proxy(state)
        state.system._add_ensemble(self)
        self._frames = []
        mg = ihm.model.ModelGroup(name=name)
        state.append(mg)
        super(Ensemble, self).__init__(model_group=mg, num_models=0, name=name)

    def add_frame(self, frame):
        """Add a frame from a custom source"""
        self._frames.append(frame)
        self.num_models += 1
        model = IMP.mmcif.data._Model(frame, self.state)
        self.model_group.append(model)
        self.state._add_frame(frame, model)
        for h in self.state.hiers:
            self._add_hierarchy(h)

    def _add_hierarchy(self, h):
        """Add ensemble-specific information from the given hierarchy"""
        for prov in reversed(list(IMP.core.get_all_provenance(
                h, types=[IMP.core.ClusterProvenance]))):
            self.num_models = prov.get_number_of_members()
            self.precision = prov.get_precision()
            self.name = prov.get_name()
            d = prov.get_density()
            if d and d.endswith('.json'):
                with open(d) as fh:
                    j = json.load(fh)
                self._parse_json_density(j, d)

    def _parse_json_density(self, j, json_fname):
        """Add information from cluster density JSON file"""
        if j.get('producer') and j['producer'].get('name') == 'IMP.sampcon':
            self._parse_sampcon(j, json_fname)

    def _parse_sampcon(self, j, json_fname):
        """Add information from IMP.sampcon-generated JSON file"""
        ranges = j['density_custom_ranges']
        for cluster in j['clusters']:
            if cluster['name'] == self.name:
                for range_name, mrc in cluster['density'].items():
                    sel_tuple = ranges[range_name]
                    if len(sel_tuple) > 1:
                        raise ValueError("Cannot handle sel tuple")
                    asym = self.state.system._parse_sel_tuple(sel_tuple[0])
                    # Path is relative to that of the JSON file
                    # With Python 2, covert mrc from unicode to str
                    full_mrc = IMP.get_relative_path(json_fname, str(mrc))
                    density = ihm.model.LocalizationDensity(
                        file=ihm.location.OutputFileLocation(
                            path=full_mrc, details='Localization density'),
                        asym_unit=asym)
                    self.densities.append(density)

    def add_rmf(self, fname, name, frame=0):
        """Add a frame from an RMF file"""
        self.add_frame(RMFFrame(fname, frame, name))

    def add_model(self, hiers, restraints, name):
        """Add hierarchies and restraints from an IMP.Model"""
        self.add_frame(_ModelFrame(hiers, restraints, name))


class Convert(object):
    """Convert one or more IMP Models and/or RMF frames to mmCIF
       or BinaryCIF.

       Models can be added by calling `add_model`. The final output
       is collected in a python-ihm `ihm.System` object, as the `system`
       attribute, and can be written out using the python-ihm API or
       with the convenience `write` method.

       This class is still in development and does not yet convert all
       IMP data to IHM mmCIF/BinaryCIF.
    """

    def __init__(self):
        self.system = ihm.System()
        self._state_by_name = {}
        self._entities = IMP.mmcif.data._EntityMapper(self.system)
        self._components = IMP.mmcif.data._ComponentMapper(self.system)
        self._software = IMP.mmcif.data._AllSoftware(self.system)
        self._external_files = IMP.mmcif.data._ExternalFiles(self.system)
        self._datasets = IMP.mmcif.data._Datasets(self.system)
        self._model_assemblies = IMP.mmcif.data._ModelAssemblies(self.system)
        self._representations = IMP.mmcif.data._Representations(self.system)
        self._protocols = IMP.mmcif.data._Protocols(self.system)
        self._restraints = IMP.mmcif.restraint._AllRestraints(
            self.system, self._components)

    def add_model(self, hiers, restraints, name=None, states=None,
                  ensembles=None):
        """Add information to the system from an IMP Model.
           Multiple IHM Models (coordinate sets) may be created (one per
           state per hierarchy). All IHM models in a state are given the
           same name and are added to the given IHM Ensemble for that state
           (or placed in a new Ensemble if not given). Only coordinates
           for the given state names (or all states, if not given) are added.
           The IHM Ensembles containing the new models are returned."""
        if ensembles is None:
            ensembles = {}
        if self.system.title is None and len(hiers) > 0:
            self.system.title = hiers[0].get_name()
        ihm_models = []
        for state_obj, state_hier, top_hier in self._get_states(hiers, states):
            e, model = self._add_hierarchy(
                state_hier, top_hier, state_obj, name,
                ensembles.get(state_obj.name))
            ihm_models.append(model)
            ensembles[state_obj.name] = e
        self._add_restraints(restraints, ihm_models)
        self._remove_duplicate_chain_ids()
        return ensembles

    def _remove_duplicate_chain_ids(self):
        chain_ids = [a.id for a in self.system.asym_units]
        if len(set(chain_ids)) < len(chain_ids):
            print("Duplicate chain IDs detected - reassigning alphabetically")
            for chain, cid in zip(self.system.asym_units, _ChainIDs()):
                chain.id = cid

    def _add_restraints(self, restraints, ihm_models):
        all_rsr = []
        for r in restraints:
            all_rsr.extend(self._handle_restraint(r, ihm_models))
        # IMP provenance doesn't currently record which restraints were
        # used in modeling, so assume all of them were
        dsg = self._datasets.add_group(
            [r.dataset for r in all_rsr if r.dataset is not None],
            "All datasets used in modeling")
        for m in ihm_models:
            if not m.protocol:
                continue
            for step in m.protocol.steps:
                step.dataset_group = dsg
            for analysis in m.protocol.analyses:
                for step in analysis.steps:
                    step.dataset_group = dsg

    def _handle_restraint(self, r, ihm_models):
        num_cif_r = 0
        for cif_r in self._restraints.handle(r, ihm_models):
            num_cif_r += 1
            yield cif_r
        if num_cif_r == 0:
            try:
                rs = IMP.RestraintSet.get_from(r)
            except ValueError:
                rs = None
            if rs:
                for child in rs.restraints:
                    for r in self._handle_restraint(child, ihm_models):
                        yield r

    def _add_hierarchy(self, h, top_h, state, name, ensemble):
        if ensemble is None:
            mg = ihm.model.ModelGroup()
            state.append(mg)
            ensemble = ihm.model.Ensemble(model_group=mg, num_models=0)
            self.system.ensembles.append(ensemble)
        chains = [IMP.atom.Chain(c)
                  for c in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)]
        if len(chains) == 0:
            raise ValueError("No chains found in %s" % h)
        asyms = []
        ch = IMP.mmcif.data._CoordinateHandler(self.system, self._datasets)
        for c in chains:
            comp = self._add_chain(c)
            asyms.append(comp.asym_unit)
            ch.add_chain(c, comp.asym_unit)
        representation = self._representations.add(ch._representation)
        assembly = self._model_assemblies.add(asyms)
        self._add_hierarchy_ensemble_info(h, top_h, ensemble)
        self._software.add_hierarchy(h, top_h)
        protocol = self._protocols._add_hierarchy(h, top_h, assembly,
                                                  self._software)
        self._external_files.add_hierarchy(h, top_h)
        model = ihm.model.Model(assembly=assembly, protocol=protocol,
                                representation=representation, name=name)
        model._atoms = ch._atoms
        model._spheres = ch._spheres
        ensemble.model_group.append(model)
        ensemble.num_models += 1
        return ensemble, model

    def _add_hierarchy_ensemble_info(self, h, top_h, ensemble):
        """Add ensemble-specific information from the given hierarchy"""
        for prov in reversed(list(_get_all_state_provenance(
                h, top_h, types=[IMP.core.ClusterProvenance]))):
            ensemble.num_models = prov.get_number_of_members()
            ensemble.precision = prov.get_precision()
            ensemble.name = prov.get_name()
            d = prov.get_density()
            if d and d.endswith('.json'):
                with open(d) as fh:
                    j = json.load(fh)
                self._parse_json_density(j, d, ensemble)

    def _parse_json_density(self, j, json_fname, ensemble):
        """Add information from cluster density JSON file"""
        if j.get('producer') and j['producer'].get('name') == 'IMP.sampcon':
            self._parse_sampcon(j, json_fname, ensemble)

    def _parse_sampcon(self, j, json_fname, ensemble):
        """Add information from IMP.sampcon-generated JSON file"""
        ranges = j['density_custom_ranges']
        for cluster in j['clusters']:
            if cluster['name'] == ensemble.name:
                for range_name, mrc in cluster['density'].items():
                    sel_tuple = ranges[range_name]
                    if len(sel_tuple) > 1:
                        raise ValueError("Cannot handle sel tuple")
                    asym = self._parse_sel_tuple(sel_tuple[0])
                    # Path is relative to that of the JSON file
                    # With Python 2, covert mrc from unicode to str
                    full_mrc = IMP.get_relative_path(json_fname, str(mrc))
                    density = ihm.model.LocalizationDensity(
                        file=ihm.location.OutputFileLocation(
                            path=full_mrc, details='Localization density'),
                        asym_unit=asym)
                    ensemble.densities.append(density)

    def _parse_sel_tuple(self, t):
        """Convert a PMI-style selection tuple into an IHM object"""
        asym_map = {}
        for asym in self.system.asym_units:
            asym_map[asym.details] = asym
        if isinstance(t, _string_type):
            return asym_map[t]
        elif isinstance(t, (list, tuple)) and len(t) == 3:
            return asym_map[t[2]](t[0], t[1])
        else:
            raise TypeError("Cannot handle selection tuple: %s" % t)

    def _get_states(self, hiers, states):
        def get_state_by_name(name):
            if states is not None and name not in states:
                return None
            if name not in self._state_by_name:
                self._add_state(ihm.model.State(name=name))
            return self._state_by_name[name]

        for h in hiers:
            state_hiers = IMP.atom.get_by_type(h, IMP.atom.STATE_TYPE)
            for state_hier in state_hiers:
                state_obj = get_state_by_name(state_hier.get_name())
                if state_obj is not None:
                    yield state_obj, state_hier, h
            # If no state nodes, treat everything as in a single unnamed state
            if len(state_hiers) == 0:
                state_obj = get_state_by_name(None)
                if state_obj is not None:
                    yield state_obj, h, None

    def _add_state(self, state):
        if not self.system.state_groups:
            self.system.state_groups.append(ihm.model.StateGroup())
        self.system.state_groups[-1].append(state)
        self._state_by_name[state.name] = state

    def _add_chain(self, chain):
        entity = self._entities.add(chain)
        component = self._components.add(chain, entity)
        return component

    def add_rmf(self, filename, name=None, frame=0, states=None,
                ensembles=None):
        """Add information to the system from a single frame in an RMF file."""
        m = IMP.Model()
        rh = RMF.open_rmf_file_read_only(filename)
        hiers = IMP.rmf.create_hierarchies(rh, m)
        restraints = IMP.rmf.create_restraints(rh, m)
        if frame != 0:
            IMP.rmf.load_frame(rh, RMF.FrameID(frame))
        return self.add_model(hiers, restraints, name=name, states=states,
                              ensembles=ensembles)

    def write(self, filename):
        """Write out the IHM system to a named mmCIF or BinaryCIF file."""
        if filename.endswith('.bcif'):
            with open(filename, 'wb') as fh:
                ihm.dumper.write(fh, [self.system], format='BCIF')
        else:
            with open(filename, 'w') as fh:
                ihm.dumper.write(fh, [self.system])

    def report(self, fh=sys.stdout):
        """Use python-ihm to print a summary report of the IHM system."""
        self.system.report(fh)
