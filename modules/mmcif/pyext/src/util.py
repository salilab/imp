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
import itertools
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
        return ensembles

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

    def _ensure_unique_molecule_names(self, chains):
        """Make sure that the input IMP molecule names are unique.
           We assume that a given molecule name maps to an IHM asym,
           so we need these names to be unique. Rename molecules if
           necessary."""
        def _get_unique_name(old_name, seen_names):
            for copy_num in itertools.count(1):
                name = old_name + " copy %d" % copy_num
                if name not in seen_names:
                    return name

        seen_names = set()
        for c in chains:
            mol = IMP.mmcif.data.get_molecule(c)
            if mol and mol.get_name():
                name = mol.get_name()
                if name in seen_names:
                    old_name = name
                    name = _get_unique_name(old_name, seen_names)
                    mol.set_name(name)
                    print("Duplicate molecule name detected - reassigning "
                          "from %r to %r" % (old_name, name))
                seen_names.add(name)

    def _ensure_unique_chain_ids(self, chains):
        """Make sure that IMP chain IDs are unique, since they are mapped to
           IHM asym IDs, which need to be unique. Reassign them alphabetically
           if duplicates are found."""
        chain_ids = [c.get_id() for c in chains]
        if len(set(chain_ids)) < len(chain_ids):
            for chain, cid in zip(chains, _ChainIDs()):
                chain.set_id(cid)
            print("Duplicate chain IDs detected - reassigning from %s to %s"
                  % (chain_ids, [c.get_id() for c in chains]))

    def _add_hierarchy(self, h, top_h, state, name, ensemble):
        if ensemble is None:
            mg = ihm.model.ModelGroup()
            state.append(mg)
            ensemble = ihm.model.Ensemble(model_group=mg, num_models=0)
            self.system.ensembles.append(ensemble)
        chains = [IMP.atom.Chain(c)
                  for c in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)]
        self._ensure_unique_molecule_names(chains)
        self._ensure_unique_chain_ids(chains)
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
