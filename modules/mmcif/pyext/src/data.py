"""@namespace IMP.mmcif.data
   @brief Classes to represent data structures used in mmCIF.
"""

from __future__ import print_function
import IMP.atom
import ihm.location
import ihm.metadata
import ihm.startmodel
import ihm.analysis
import ihm.protocol
import ihm.model

def get_molecule(h):
    """Given a Hierarchy, walk up and find the parent Molecule"""
    while h:
        if IMP.atom.Molecule.get_is_setup(h):
            return IMP.atom.Molecule(h)
        h = h.get_parent()
    return None


class _EntityMapper(dict):
    """Handle mapping from IMP chains to CIF entities.
       Multiple components may map to the same entity if they share sequence."""
    def __init__(self, system):
        self.system = system
        super(_EntityMapper, self).__init__()
        self._sequence_dict = {}
        self._entities = []

    def add(self, chain):
        # todo: handle non-protein sequences
        sequence = chain.get_sequence()
        if sequence == '':
            raise ValueError("Chain %s has no sequence" % chain)
        if sequence not in self._sequence_dict:
            entity = ihm.Entity(sequence)
            self.system.entities.append(entity)
            self._entities.append(entity)
            self._sequence_dict[sequence] = entity
        self[chain] = self._sequence_dict[sequence]
        return self[chain]

    def get_all(self):
        """Yield all entities"""
        return self._entities


def _assign_id(obj, seen_objs, obj_by_id):
    """Assign a unique ID to obj, and track all ids in obj_by_id."""
    if obj not in seen_objs:
        if not hasattr(obj, 'id'):
            obj_by_id.append(obj)
            obj.id = len(obj_by_id)
        seen_objs[obj] = obj.id
    else:
        obj.id = seen_objs[obj]


class _Component(object):
    """An mmCIF component. This is an instance of an _Entity. Multiple
       _Components may map to the same _Entity but must have unique
       asym_ids. A _Component is similar to an IMP Chain but multiple
       Chains may map to the same _Component (the Chains represent the
       same structure, just in different states, and potentially in
       different IMP Models). A _Component may also represent something
       that is described by an experiment but was not modeled by IMP, and
       so no Chains map to it but a string name does."""
    def __init__(self, entity, asym_id, name):
        self.entity, self.asym_id, self.name = entity, asym_id, name


class _ComponentMapper(object):
    """Handle mapping from chains to CIF components."""
    def __init__(self, system):
        super(_ComponentMapper, self).__init__()
        self.system = system
        self._used_entities = set()
        self._all_components = []
        self._all_modeled_components = []
        self._map = {}

    def __getitem__(self, chain):
        modeled, asym_id, map_key, name = self._handle_chain(chain)
        return self._map[map_key]

    def _handle_chain(self, chain):
        if isinstance(chain, IMP.atom.Chain):
            modeled = True
            mol = get_molecule(chain)
            asym_id = map_key = chain.get_id()
            name = mol.get_name() if mol else None
        else:
            modeled = False
            asym_id = None
            name = map_key = chain.name
        return modeled, asym_id, map_key, name

    def add(self, chain, entity):
        """Add a chain (either an IMP Chain object for a modeled component,
           or a NonModeledChain object for a non-modeled component)"""
        modeled, asym_id, map_key, name = self._handle_chain(chain)
        if map_key not in self._map:
            component = _Component(entity, asym_id, name)
            if entity not in self._used_entities:
                self._used_entities.add(entity)
                # Assign entity name from the component; strip out anything
                # after a @ or .
                entity.description = component.name.split("@")[0].split(".")[0]
            self._all_components.append(component)
            if modeled:
                asym = ihm.AsymUnit(entity, name, id=asym_id)
                self.system.asym_units.append(asym)
                component.asym_unit = asym
                self._all_modeled_components.append(component)
            self._map[map_key] = component
        else:
            component = self._map[map_key]
            if component.entity != entity:
                raise ValueError("Two chains have the same ID (%s) but "
                                 "different sequences - rename one of the "
                                 "chains" % map_key)
        return component

    def get_all(self):
        """Get all components"""
        return self._all_components

    def get_all_modeled(self):
        """Get all modeled components"""
        return self._all_modeled_components


class _Representation(object):
    """Group a set of contiguous particles with the same representation"""
    def __init__(self):
        self.particles = []
        self.residue_range = () # inclusive range
        self.starting_model = None

    def add(self, particle, starting_model):
        """Potentially add a new particle to this representation.
           Iff the particle could be added, return True."""
        resrange, rigid_body, primitive = self._get_particle_info(particle)
        if not self.particles:
            self.particles.append(particle)
            self.residue_range = resrange
            self.rigid_body = rigid_body
            self.primitive = primitive
            self.starting_model = starting_model
            return True
        elif type(particle) == type(self.particles[0]) \
             and resrange[0] == self.residue_range[1] + 1 \
             and starting_model == self.starting_model \
             and self._same_rigid_body(rigid_body):
            self.particles.append(particle)
            self.residue_range = (self.residue_range[0], resrange[1])
            return True

    def _same_rigid_body(self, rigid_body):
        # Note: can't just use self.rigid_body == rigid_body as IMP may
        # crash when comparing a RigidBody object against None
        if self.rigid_body is None and rigid_body is None:
            return True
        elif self.rigid_body is None or rigid_body is None:
            return False
        else:
            return self.rigid_body == rigid_body

    def _get_particle_info(self, p):
        # Note that we consider nonrigid members to not be rigid here
        if IMP.core.RigidMember.get_is_setup(p):
            rigid_body = IMP.core.RigidMember(p).get_rigid_body()
        else:
            rigid_body = None
        if isinstance(p, IMP.atom.Residue):
            return (p.get_index(), p.get_index()), rigid_body, 'sphere'
        elif isinstance(p, IMP.atom.Fragment):
            resinds = p.get_residue_indexes()
            return (resinds[0], resinds[-1]), rigid_body, 'sphere'
        raise TypeError("Unknown particle ", p)

    def __bool__(self):
        return len(self.particles) > 0

    __nonzero__ = __bool__ # Python 2 compatibility


def _get_all_structure_provenance(p):
    """Yield all StructureProvenance decorators for the given particle."""
    return IMP.core.get_all_provenance(p, types=[IMP.core.StructureProvenance])


class _MutantHandler(object):
    def __init__(self, templates, asym):
        self._seq_dif = []
        self._last_res_index = None
        self.templates = templates
        self.asym = asym

    def _residue_first_atom(self, res):
        """Return True iff we're looking at the first atom in this residue"""
        # Only add one seq_dif record per residue
        ind = res.get_index()
        if ind != self._last_res_index:
            self._last_res_index = ind
            return True

    def handle_residue(self, res, comp_id, seq_id, offset):
        res_name = res.get_residue_type().get_string()
        # MSE in the original PDB is automatically mutated
        # by IMP to MET, so reflect that in the output,
        # and pass back to populate the seq_dif category.
        if res_name == 'MSE' and comp_id == 'MET':
            if self._residue_first_atom(res):
                # This should only happen when we're using
                # a crystal structure as the source (a
                # comparative model would use MET in
                # the sequence)
                assert(len(self.templates) == 0)
                self._seq_dif.append(ihm.startmodel.MSESeqDif(
                            res.get_index(), seq_id))
        elif res_name != comp_id:
            if self._residue_first_atom(res):
                print("WARNING: Starting model residue %s does not match "
                      "that in the output model (%s) for chain %s residue %d. "
                      "Check offset (currently %d)."
                      % (res_name, comp_id, self.asym._id, seq_id, offset))
                self._seq_dif.append(ihm.startmodel.SeqDif(
                         db_seq_id=res.get_index(), seq_id=seq_id,
                         db_comp_id=res_name,
                         details="Mutation of %s to %s" % (res_name, comp_id)))


class _StartingModel(ihm.startmodel.StartingModel):
    _eq_keys = ['filename', 'asym_id', 'offset']

    def __init__(self, asym_unit, struc_prov):
        self.filename = struc_prov[0].get_filename()
        super(_StartingModel, self).__init__(
                asym_unit=asym_unit(0,0), # will update in _add_residue()
                dataset=None, # will fill in later with _set_sources_datasets()
                asym_id=struc_prov[0].get_chain_id(),
                offset=struc_prov[0].get_residue_offset())

    def _add_residue(self, resind):
        # Update seq_id_range to accommodate this residue
        seq_id_end = resind + self.offset
        seq_id_begin = self.asym_unit.seq_id_range[0]
        if seq_id_begin == 0:
            seq_id_begin = seq_id_end
        self.asym_unit = self.asym_unit.asym(seq_id_begin, seq_id_end)

    # Two starting models with same filename, chain ID, and offset
    # compare identical
    # note: this results in separate starting models if only the offset differs;
    # maybe consolidate into one?
    def _eq_vals(self):
        return tuple([self.__class__]
                     + [getattr(self, x) for x in self._eq_keys])
    def __eq__(self, other):
        return other is not None and self._eq_vals() == other._eq_vals()
    def __hash__(self):
        return hash(self._eq_vals())

    def _set_sources_datasets(self, system):
        # Attempt to identify PDB file vs. comparative model
        p = ihm.metadata.PDBParser()
        r = p.parse_file(self.filename)
        system.system.software.extend(r['software'])
        dataset = system.datasets.add(r['dataset'])
        # We only want the templates that model the starting model chain
        templates = r['templates'].get(self.asym_id, [])
        for t in templates:
            if t.alignment_file:
                system.system.locations.append(t.alignment_file)
            if t.dataset:
                system.datasets.add(t.dataset)
        self.dataset = dataset
        self.templates = templates
        self.metadata = r['metadata']

    def _read_coords(self):
        """Read the coordinates for this starting model"""
        m = IMP.Model()
        # todo: support reading other subsets of the atoms (e.g. CA/CB)
        slt = IMP.atom.ChainPDBSelector(self.asym_id) \
                   & IMP.atom.NonWaterNonHydrogenPDBSelector()
        hier = IMP.atom.read_pdb(self.filename, m, slt)
        rng = self.asym_unit.seq_id_range
        sel = IMP.atom.Selection(hier,
                residue_indexes=list(range(rng[0] - self.offset,
                                           rng[1] + 1 - self.offset)))
        return m, sel

    def get_seq_dif(self):
        return self._seq_dif  # filled in by get_atoms()

    def get_atoms(self):
        mh = _MutantHandler(self.templates, self.asym_unit)
        m, sel = self._read_coords()
        for a in sel.get_selected_particles():
            coord = IMP.core.XYZ(a).get_coordinates()
            atom = IMP.atom.Atom(a)
            element = atom.get_element()
            element = IMP.atom.get_element_table().get_name(element)
            atom_name = atom.get_atom_type().get_string()
            het = atom_name.startswith('HET:')
            if het:
                atom_name = atom_name[4:]
            res = IMP.atom.get_residue(atom)

            seq_id = res.get_index() + self.offset
            comp_id = self.asym_unit.entity.sequence[seq_id-1].id
            mh.handle_residue(res, comp_id, seq_id, self.offset)
            yield ihm.model.Atom(asym_unit=self.asym_unit,
                                 seq_id=seq_id,
                                 atom_id=atom_name, type_symbol=element,
                                 x=coord[0], y=coord[1], z=coord[2],
                                 het=het, biso=atom.get_temperature_factor())
        self._seq_dif = mh._seq_dif


class _StartingModelFinder(object):
    """Map IMP particles to starting model objects"""
    def __init__(self, component, existing_starting_models):
        self._seen_particles = {}
        self._component = component
        self._seen_starting_models = dict.fromkeys(existing_starting_models)

    def find(self, particle, system):
        """Return a StartingModel object, or None, for this particle"""
        def _get_starting_model(sp, resind):
            s = _StartingModel(self._component.asym_unit, sp)
            if s not in self._seen_starting_models:
                self._seen_starting_models[s] = s
                s._set_sources_datasets(system)
                system.system.orphan_starting_models.append(s)
            s = self._seen_starting_models[s]
            if s:
                s._add_residue(resind)
            return s
        resind = None
        if IMP.atom.Residue.get_is_setup(particle):
            resind = IMP.atom.Residue(particle).get_index()
        sp = list(_get_all_structure_provenance(particle))
        if sp:
            return _get_starting_model(sp, resind)
        elif IMP.atom.Hierarchy.get_is_setup(particle):
            h = IMP.atom.Hierarchy(particle).get_parent()
            # Remember all nodes we inspect
            seen_parents = []
            while h:
                if IMP.atom.Residue.get_is_setup(h):
                    resind = IMP.atom.Residue(h).get_index()
                pi = h.get_particle_index()
                seen_parents.append(pi)
                # If we inspected this node before, return the cached result
                if pi in self._seen_particles:
                    sp = self._seen_particles[pi]
                    if sp and sp[0] and resind is not None:
                        sp[0]._add_residue(resind)
                    return sp[0] if sp else None
                else:
                    sp = list(_get_all_structure_provenance(h))
                    self._seen_particles[pi] = []
                    if sp:
                        s = _get_starting_model(sp, resind)
                        # Set cache for this node and all the children we
                        # inspected on the way up
                        for spi in seen_parents:
                            self._seen_particles[spi].append(s)
                        return s
                h = h.get_parent()


class _Datasets(object):
    """Store all datasets used."""
    def __init__(self, system):
        super(_Datasets, self).__init__()
        self._datasets = {}
        self.system = system

    def add(self, d):
        """Add and return a new dataset."""
        if d not in self._datasets:
            self._datasets[d] = d
            self.system.orphan_datasets.append(d)
        return self._datasets[d]

    def get_all(self):
        """Yield all datasets"""
        return self._datasets.keys()


class _AllSoftware(object):
    """Keep track of all Software objects."""
    def __init__(self, system):
        self.system = system
        super(_AllSoftware, self).__init__()

    def add_hierarchy(self, h):
        # todo: if no SoftwareProvenance available, use RMF producer field
        for p in IMP.core.get_all_provenance(h,
                                        types=[IMP.core.SoftwareProvenance]):
            self.system.software.append(
                    ihm.Software(name=p.get_software_name(),
                                 classification='integrative model building',
                                 description=None,
                                 version=p.get_version(),
                                 location=p.get_location()))


class _ExternalFiles(object):
    """Track all externally-referenced files
       (i.e. anything that refers to a Location that isn't
       a DatabaseLocation)."""
    def __init__(self, system):
        self.system = system

    def add(self, location):
        """Add a new externally-referenced file.
           Note that ids are assigned later."""
        self.system.locations.append(location)

    def add_hierarchy(self, h):
        # Add all Python scripts that were used in the modeling
        for p in IMP.core.get_all_provenance(h,
                                     types=[IMP.core.ScriptProvenance]):
            # todo: set details
            l = ihm.location.WorkflowFileLocation(path=p.get_filename(),
                               details='Integrative modeling Python script')
            self.add(l)


class _ProtocolStep(ihm.protocol.Step):
    """A single step (e.g. sampling, refinement) in a protocol."""
    def __init__(self, prov, num_models_begin, assembly):
        method = prov.get_method()
        if prov.get_number_of_replicas() > 1:
            method = "Replica exchange " + method
        super(_ProtocolStep, self).__init__(
                assembly=assembly,
                # todo: fill in useful value for dataset_group
                dataset_group=None,
                method=method, name='Sampling',
                num_models_begin=num_models_begin,
                num_models_end=prov.get_number_of_frames(),
                # todo: support multiple states, time ordered
                multi_state=False, ordered=False,
                # todo: revisit assumption all models are multiscale
                multi_scale=True)

    def add_combine(self, prov):
        self.num_models_end = prov.get_number_of_frames()
        return self.num_models_end


class _Protocol(ihm.protocol.Protocol):
    """A modeling protocol.
       Each protocol consists of a number of protocol steps (e.g. sampling,
       refinement) followed by a number of postprocessing steps (e.g.
       filtering, rescoring, clustering)"""

    def add_step(self, prov, num_models, assembly):
        if isinstance(prov, IMP.core.CombineProvenance):
            # Fold CombineProvenance into a previous sampling step
            if len(self.steps) == 0:
                raise ValueError("CombineProvenance with no previous sampling")
            return self.steps[-1].add_combine(prov)
        else:
            ps = _ProtocolStep(prov, num_models, assembly)
            self.steps.append(ps)
            return ps.num_models_end

    def add_postproc(self, prov, num_models, assembly):
        if not self.analyses:
            self.analyses.append(ihm.analysis.Analysis())
        if isinstance(prov, IMP.core.FilterProvenance):
            pp = ihm.analysis.FilterStep(feature='energy/score',
                    assembly=assembly, num_models_begin=num_models,
                    num_models_end = prov.get_number_of_frames())
        elif isinstance(prov, IMP.core.ClusterProvenance):
            # Assume clustering uses all models
            pp = ihm.analysis.ClusterStep(feature='RMSD',
                    assembly=assembly, num_models_begin=num_models,
                    num_models_end=num_models)
        else:
            raise ValueError("Unhandled provenance", prov)
        self.analyses[-1].steps.append(pp)
        return pp.num_models_end


class _Protocols(object):
    """Track all modeling protocols used."""
    def __init__(self, system):
        self.system = system

    def _add_protocol(self, prot):
        self.system.orphan_protocols.append(prot)

    def _add_hierarchy(self, h, modeled_assembly):
        num_models = 0 # assume we always start with no models
        prot_types = (IMP.core.SampleProvenance, IMP.core.CombineProvenance)
        pp_types = (IMP.core.FilterProvenance, IMP.core.ClusterProvenance)
        in_postproc = False
        prot = _Protocol()
        for p in reversed(list(IMP.core.get_all_provenance(
                                          h, types=prot_types + pp_types))):
            if isinstance(p, pp_types):
                num_models = prot.add_postproc(p, num_models, modeled_assembly)
                in_postproc = True
            else:
                if in_postproc:
                    # Start a new protocol
                    self._add_protocol(prot)
                    prot = _Protocol()
                num_models = prot.add_step(p, num_models, modeled_assembly)
                in_postproc = False
        if len(prot.steps) > 0:
            self._add_protocol(prot)
