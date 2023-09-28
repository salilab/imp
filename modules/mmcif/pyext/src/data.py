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
import ihm.citations
import ihm.reference
import operator
import inspect


# Map from IMP ResidueType to ihm ChemComp
_imp_to_ihm = {}


def _fill_imp_to_ihm():
    d = dict(x for x in inspect.getmembers(IMP.atom)
             if isinstance(x[1], IMP.atom.ResidueType))
    # Handle standard amino acids plus some extras like MSE, UNK
    for comp in ihm.LPeptideAlphabet._comps.values():
        if comp.id in d:
            _imp_to_ihm[d[comp.id]] = comp
    # Handle RNA and DNA
    alpha = ihm.RNAAlphabet()
    for code in ['ADE', 'CYT', 'GUA', 'URA']:
        _imp_to_ihm[d[code]] = alpha[d[code].get_string()]
    alpha = ihm.DNAAlphabet()
    for code in ['DADE', 'DCYT', 'DGUA', 'DTHY']:
        _imp_to_ihm[d[code]] = alpha[d[code].get_string()]
    # Pass through missing IMP residue
    _imp_to_ihm[None] = None


_fill_imp_to_ihm()


def get_molecule(h):
    """Given a Hierarchy, walk up and find the parent Molecule"""
    while h:
        if IMP.atom.Molecule.get_is_setup(h):
            return IMP.atom.Molecule(h)
        h = h.get_parent()
    return None


def _check_sequential(fragment, resinds):
    for i in range(1, len(resinds)):
        if resinds[i - 1] + 1 != resinds[i]:
            raise ValueError(
                "%s: non-sequential residue indices are not supported"
                % str(fragment))


def _get_all_state_provenance(state_h, top_h, types):
    """Yield all provenance information for the given state.
       If the given State Hierarchy node contains no provenance information,
       fall back to any provenance information for the top-level node
       (if provided)."""
    count = 0
    for p in IMP.core.get_all_provenance(state_h, types=types):
        count += 1
        yield p
    if count == 0 and top_h is not None:
        for p in IMP.core.get_all_provenance(top_h, types=types):
            yield p


class _CustomDNAAlphabet(ihm.Alphabet):
    """Custom DNA alphabet that maps A,C,G,T (rather than DA,DC,DG,DT
       as in python-ihm)"""
    _comps = dict([cc.code_canonical, cc]
                  for cc in ihm.DNAAlphabet._comps.values())


class _EntityMapper(dict):
    """Handle mapping from IMP chains to CIF entities.
       Multiple components may map to the same entity if they
       share sequence."""
    def __init__(self, system):
        self.system = system
        super(_EntityMapper, self).__init__()
        self._sequence_dict = {}
        self._entities = []
        self._alphabet_map = {
            IMP.atom.UnknownChainType: ihm.LPeptideAlphabet,
            IMP.atom.Protein: ihm.LPeptideAlphabet,
            IMP.atom.RNA: ihm.RNAAlphabet,
            IMP.atom.DNA: _CustomDNAAlphabet}

    def _get_sequence_from_residues(self, chain, seq_from_res):
        seq_id_begin, seq = seq_from_res
        if not seq:
            raise ValueError("Chain %s has no sequence and no residues"
                             % chain)
        missing_seq = [ind + seq_id_begin
                       for (ind, res) in enumerate(seq) if res is None]
        if missing_seq:
            raise ValueError(
                "Chain %s has no declared sequence; tried to determine the "
                "sequence from Residues, but the following residue indices "
                "have no residue type (perhaps covered only by Fragments): %s"
                % (chain, str(missing_seq)))
        return seq_id_begin - 1, tuple(seq)

    def add(self, chain, seq_from_res=None):
        sequence = chain.get_sequence()
        offset = chain.get_sequence_offset()
        if sequence == '':
            if seq_from_res is not None:
                offset, sequence = self._get_sequence_from_residues(
                    chain, seq_from_res)
            else:
                raise ValueError("Chain %s has no sequence" % chain)
        else:
            # Map one-letter codes to ihm.ChemComp
            alphabet = self._alphabet_map[chain.get_chain_type()]()
            sequence = tuple(alphabet[x] for x in sequence)
        if sequence not in self._sequence_dict:
            entity = ihm.Entity(sequence)
            self.system.entities.append(entity)
            self._entities.append(entity)
            self._sequence_dict[sequence] = entity
            uniprot = chain.get_uniprot_accession()
            if uniprot:
                up = ihm.reference.UniProtSequence.from_accession(uniprot)
                entity.references.append(up)
        self[chain] = self._sequence_dict[sequence]
        return self[chain], offset

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
    """Handle mapping from IMP Chains to CIF AsymUnits."""
    def __init__(self, system):
        super(_ComponentMapper, self).__init__()
        self.system = system
        self._used_entities = set()
        self._all_components = []
        self._map = {}

    def __getitem__(self, chain):
        asym_id, map_key, name = self._handle_chain(chain)
        return self._map[map_key]

    def _handle_chain(self, chain):
        mol = get_molecule(chain)
        asym_id = chain.get_id()
        name = mol.get_name() if mol else None
        # Avoid conflict between name="A" and asym_id="A"
        if name:
            map_key = "name", name
        else:
            map_key = "asym_id", asym_id
        return asym_id, map_key, name

    def add(self, chain, entity, offset):
        """Add a chain (an IMP Chain object)"""
        asym_id, map_key, name = self._handle_chain(chain)
        if map_key not in self._map:
            component = _Component(entity, asym_id, name)
            if entity not in self._used_entities:
                self._used_entities.add(entity)
                # Assign entity name from the component; strip out anything
                # after a @ or .
                if component.name:
                    entity.description = \
                        component.name.split("@")[0].split(".")[0]
            self._all_components.append(component)
            asym = ihm.AsymUnit(entity, name, id=asym_id,
                                auth_seq_id_map=offset)
            self.system.asym_units.append(asym)
            component.asym_unit = asym
            self._map[map_key] = component
        else:
            component = self._map[map_key]
            if component.entity != entity:
                raise ValueError("Two chains have the same ID (%s) but "
                                 "different sequences - rename one of the "
                                 "chains" % component.asym_unit.id)
            if component.asym_unit.auth_seq_id_map != offset:
                raise ValueError(
                    "Two chains have the same ID (%s) but different offsets "
                    "(%d, %d) - this is not supported"
                    % (component.asym_unit.id,
                       component.asym_unit.auth_seq_id_map, offset))
        return component

    def get_all(self):
        """Get all components"""
        return self._all_components


class _RepSegmentFactory(object):
    """Make ihm.representation.Segment objects for each set of contiguous
       particles with the same representation"""
    def __init__(self, asym):
        self.asym = asym
        # Offset from IHM to IMP numbering
        self.offset = asym.auth_seq_id_map
        self.particles = []
        self.imp_residue_range = ()  # inclusive range, using IMP numbering

    def add(self, particle, starting_model):
        """Add a new particle to the last segment (and return None).
           Iff the particle could not be added, return the segment and start
           a new one."""
        (resrange, rigid_body,
         is_res, is_atom) = self._get_particle_info(particle)

        def start_new_segment():
            self.particles = [particle]
            self.imp_residue_range = resrange
            self.rigid_body = rigid_body
            self.is_res = is_res
            self.is_atom = is_atom
            self.starting_model = starting_model
        if not self.particles:
            # First particle in a segment
            start_new_segment()
        elif (type(particle) == type(self.particles[0])  # noqa: E721
              and is_res == self.is_res
              and is_atom == self.is_atom
              and resrange[0] <= self.imp_residue_range[1] + 1
              and starting_model == self.starting_model
              and self._same_rigid_body(rigid_body)):
            # Continue an existing segment
            self.particles.append(particle)
            self.imp_residue_range = (self.imp_residue_range[0], resrange[1])
        else:
            # Make a new segment
            seg = self.get_last()
            start_new_segment()
            return seg

    def get_last(self):
        """Return the last segment, or None"""
        if self.particles:
            # Convert residue_range from IMP to IHM
            asym = self.asym(self.imp_residue_range[0] - self.offset,
                             self.imp_residue_range[1] - self.offset)
            if self.is_atom:
                return ihm.representation.AtomicSegment(
                        asym_unit=asym, rigid=self.rigid_body is not None,
                        starting_model=self.starting_model)
            elif self.is_res:
                return ihm.representation.ResidueSegment(
                        asym_unit=asym,
                        rigid=self.rigid_body is not None, primitive='sphere',
                        starting_model=self.starting_model)
            else:
                return ihm.representation.FeatureSegment(
                        asym_unit=asym,
                        rigid=self.rigid_body is not None, primitive='sphere',
                        count=len(self.particles),
                        starting_model=self.starting_model)

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
            return (p.get_index(), p.get_index()), rigid_body, True, False
        elif isinstance(p, IMP.atom.Atom):
            res = IMP.atom.get_residue(p)
            return (res.get_index(), res.get_index()), rigid_body, False, True
        elif isinstance(p, IMP.atom.Fragment):
            resinds = p.get_residue_indexes()
            return (resinds[0], resinds[-1]), rigid_body, False, False
        raise TypeError("Unknown particle ", p)


def _get_all_structure_provenance(p):
    """Yield all StructureProvenance decorators for the given particle."""
    return IMP.core.get_all_provenance(p, types=[IMP.core.StructureProvenance])


class _StartingModelAtomHandler(object):
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
                assert len(self.templates) == 0
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

    def get_ihm_atoms(self, particles, offset):
        for a in particles:
            coord = IMP.core.XYZ(a).get_coordinates()
            atom = IMP.atom.Atom(a)
            element = atom.get_element()
            element = IMP.atom.get_element_table().get_name(element)
            atom_name = atom.get_atom_type().get_string()
            het = atom_name.startswith('HET:')
            if het:
                atom_name = atom_name[4:]
            res = IMP.atom.get_residue(atom)

            seq_id = res.get_index() + offset
            comp_id = self.asym.entity.sequence[seq_id-1].id
            self.handle_residue(res, comp_id, seq_id, offset)
            yield ihm.model.Atom(asym_unit=self.asym, seq_id=seq_id,
                                 atom_id=atom_name, type_symbol=element,
                                 x=coord[0], y=coord[1], z=coord[2],
                                 het=het, biso=atom.get_temperature_factor())


class _StartingModel(ihm.startmodel.StartingModel):
    _eq_keys = ['filename', 'asym_id', 'offset']

    def __init__(self, asym_unit, struc_prov):
        self.filename = struc_prov[0].get_filename()
        super(_StartingModel, self).__init__(
                asym_unit=asym_unit(0, 0),  # will update in _add_residue()
                # will fill in later with _set_sources_datasets()
                dataset=None,
                asym_id=struc_prov[0].get_chain_id(),
                offset=struc_prov[0].get_residue_offset())

    def _add_residue(self, resind):
        # Update seq_id_range to accommodate this residue
        seq_id_end = resind
        seq_id_begin = self.asym_unit.seq_id_range[0]
        if seq_id_begin == 0:
            seq_id_begin = seq_id_end
        self.asym_unit = self.asym_unit.asym(seq_id_begin, seq_id_end)

    # Two starting models with same filename, chain ID, and offset
    # compare identical
    # note: this results in separate starting models if only the
    # offset differs; maybe consolidate into one?
    def _eq_vals(self):
        return tuple([self.__class__]
                     + [getattr(self, x) for x in self._eq_keys])

    def __eq__(self, other):
        return other is not None and self._eq_vals() == other._eq_vals()

    def __hash__(self):
        return hash(self._eq_vals())

    def _set_sources_datasets(self, system, datasets):
        # Attempt to identify PDB file vs. comparative model
        p = ihm.metadata.PDBParser()
        r = p.parse_file(self.filename)
        system.software.extend(r['software'])
        dataset = datasets.add(r['dataset'])
        # We only want the templates that model the starting model chain
        templates = r['templates'].get(self.asym_id, [])
        for t in templates:
            if t.alignment_file:
                system.locations.append(t.alignment_file)
            if t.dataset:
                datasets.add(t.dataset)
        self.dataset = dataset
        self.templates = templates
        self.metadata = r['metadata']

    def _read_coords(self):
        """Read the coordinates for this starting model"""
        m = IMP.Model()
        # todo: support reading other subsets of the atoms (e.g. CA/CB)
        slt = IMP.atom.ChainPDBSelector([self.asym_id]) \
            & IMP.atom.NonWaterNonHydrogenPDBSelector()
        hier = IMP.atom.read_pdb(self.filename, m, slt)
        rng = self.asym_unit.seq_id_range
        sel = IMP.atom.Selection(
            hier, residue_indexes=list(range(rng[0] - self.offset,
                                             rng[1] + 1 - self.offset)))
        return m, sel

    def get_seq_dif(self):
        return self._seq_dif  # filled in by get_atoms()

    def get_atoms(self):
        mh = _StartingModelAtomHandler(self.templates, self.asym_unit)
        m, sel = self._read_coords()
        for a in mh.get_ihm_atoms(sel.get_selected_particles(), self.offset):
            yield a
        self._seq_dif = mh._seq_dif


class _StartingModelFinder(object):
    """Map IMP particles to starting model objects"""
    def __init__(self, asym, existing_starting_models, system, datasets):
        self._seen_particles = {}
        self._asym = asym
        self._seen_starting_models = {}
        for sm in existing_starting_models:
            self._seen_starting_models[sm] = sm
        self._system = system
        self._datasets = datasets

    def find(self, particle):
        """Return a StartingModel object, or None, for this particle"""
        def _get_starting_model(sp, resind):
            s = _StartingModel(self._asym, sp)
            if s not in self._seen_starting_models:
                self._seen_starting_models[s] = s
                s._set_sources_datasets(self._system, self._datasets)
                self._system.orphan_starting_models.append(s)
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
        self._groups = {}
        self.system = system

    def add(self, d):
        """Add and return a new dataset."""
        if d not in self._datasets:
            self._datasets[d] = d
            self.system.orphan_datasets.append(d)
        return self._datasets[d]

    def add_group(self, datasets, name):
        """Add and return a new group of datasets"""
        seen = set()
        # Remove duplicates
        d = []
        for dataset in datasets:
            if dataset not in seen:
                d.append(dataset)
                seen.add(dataset)
        d = tuple(d)
        if d not in self._groups:
            g = ihm.dataset.DatasetGroup(d, name=name)
            self._groups[d] = g
            self.system.orphan_dataset_groups.append(g)
        return self._groups[d]

    def get_all(self):
        """Yield all datasets"""
        return self._datasets.keys()


class _AllSoftware(object):
    """Keep track of all Software objects."""

    # IMP/RMF doesn't store citation info for software, so provide it
    # for known software packages
    cites = {'Integrative Modeling Platform (IMP)': ihm.citations.imp,
             'IMP PMI module': ihm.citations.pmi}

    def __init__(self, system):
        self.system = system
        self._by_namever = {}
        super(_AllSoftware, self).__init__()

    def add_hierarchy(self, h, top_h=None):
        # todo: if no SoftwareProvenance available, use RMF producer field
        for p in _get_all_state_provenance(
                h, top_h, types=[IMP.core.SoftwareProvenance]):
            self._add_provenance(p)

    def _add_provenance(self, p):
        """Add Software from SoftwareProvenance"""
        # Only reference the same version of a given software package once
        name = p.get_software_name()
        version = p.get_version()
        if (name, version) not in self._by_namever:
            s = ihm.Software(name=name,
                             classification='integrative model building',
                             description=None, version=version,
                             location=p.get_location(),
                             citation=self.cites.get(name))
            self.system.software.append(s)
            self._by_namever[name, version] = s
        return self._by_namever[name, version]

    def _add_previous_provenance(self, prov):
        """Add Software from a previous SoftwareProvenance, if any"""
        while prov:
            if IMP.core.SoftwareProvenance.get_is_setup(prov):
                return self._add_provenance(IMP.core.SoftwareProvenance(prov))
            prov = prov.get_previous()


class _ExternalFiles(object):
    """Track all externally-referenced files
       (i.e. anything that refers to a Location that isn't
       a DatabaseLocation)."""
    def __init__(self, system):
        self.system = system
        self._by_path = {}

    def add_hierarchy(self, h, top_h=None):
        # Add all Python scripts that were used in the modeling
        for p in _get_all_state_provenance(
                h, top_h, types=[IMP.core.ScriptProvenance]):
            self._add_provenance(p)

    def _add_provenance(self, p):
        """Add external file from ScriptProvenance"""
        # Only reference the same path once
        path = p.get_filename()
        if path not in self._by_path:
            loc = ihm.location.WorkflowFileLocation(
                path=p.get_filename(),
                details='Integrative modeling Python script')
            self.system.locations.append(loc)
            self._by_path[path] = loc
        return self._by_path[path]


class _ProtocolStep(ihm.protocol.Step):
    """A single step (e.g. sampling, refinement) in a protocol."""
    def __init__(self, prov, num_models_begin, assembly, all_software):
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
                multi_scale=True,
                software=all_software._add_previous_provenance(prov))

    def add_combine(self, prov):
        self.num_models_end = prov.get_number_of_frames()
        return self.num_models_end


class _Protocol(ihm.protocol.Protocol):
    """A modeling protocol.
       Each protocol consists of a number of protocol steps (e.g. sampling,
       refinement) followed by a number of postprocessing steps (e.g.
       filtering, rescoring, clustering)"""

    def add_step(self, prov, num_models, assembly, all_software):
        if isinstance(prov, IMP.core.CombineProvenance):
            # Fold CombineProvenance into a previous sampling step
            if len(self.steps) == 0:
                raise ValueError("CombineProvenance with no previous sampling")
            return self.steps[-1].add_combine(prov)
        else:
            ps = _ProtocolStep(prov, num_models, assembly, all_software)
            self.steps.append(ps)
            return ps.num_models_end

    def add_postproc(self, prov, num_models, assembly):
        if not self.analyses:
            self.analyses.append(ihm.analysis.Analysis())
        if isinstance(prov, IMP.core.FilterProvenance):
            pp = ihm.analysis.FilterStep(
                feature='energy/score', assembly=assembly,
                num_models_begin=num_models,
                num_models_end=prov.get_number_of_frames())
        elif isinstance(prov, IMP.core.ClusterProvenance):
            # Assume clustering uses all models
            pp = ihm.analysis.ClusterStep(
                feature='RMSD', assembly=assembly, num_models_begin=num_models,
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
        # Protocol isn't hashable or sortable, so just compare dicts
        # with existing protocols. This should still be performant as
        # we generally don't have more than one or two protocols.
        # We exclude dataset_group from the comparison as this is typically
        # filled in later.
        def step_equal(x, y):
            def get_dict(d):
                return {x: y for x, y in d.__dict__.items()
                        if x != 'dataset_group'}

            return (type(x) == type(y)  # noqa: E721
                    and get_dict(x) == get_dict(y))

        def analysis_equal(x, y):
            return (len(x.steps) == len(y.steps)
                    and all(step_equal(a, b)
                            for (a, b) in zip(x.steps, y.steps)))

        for existing in self.system.orphan_protocols:
            if (len(existing.steps) == len(prot.steps)
                and len(existing.analyses) == len(prot.analyses)
                and all(step_equal(x, y)
                        for (x, y) in zip(existing.steps, prot.steps))
                and all(analysis_equal(x, y)
                        for (x, y) in zip(existing.analyses, prot.analyses))):
                return existing
        self.system.orphan_protocols.append(prot)
        return prot

    def _add_hierarchy(self, h, top_h, modeled_assembly, all_software):
        num_models = 0  # assume we always start with no models
        prot_types = (IMP.core.SampleProvenance, IMP.core.CombineProvenance)
        pp_types = (IMP.core.FilterProvenance, IMP.core.ClusterProvenance)
        in_postproc = False
        prot = _Protocol()
        for p in reversed(list(_get_all_state_provenance(
                h, top_h, types=prot_types + pp_types))):
            if isinstance(p, pp_types):
                num_models = prot.add_postproc(p, num_models, modeled_assembly)
                in_postproc = True
            else:
                if in_postproc:
                    # Start a new protocol
                    self._add_protocol(prot)
                    prot = _Protocol()
                num_models = prot.add_step(p, num_models, modeled_assembly,
                                           all_software)
                in_postproc = False
        if len(prot.steps) > 0:
            return self._add_protocol(prot)


class _CoordinateHandler(object):
    def __init__(self, system, datasets):
        self._system = system
        self._datasets = datasets
        self._representation = ihm.representation.Representation()
        # IHM atoms/spheres corresponding to IMP beads/residues/atoms
        # We build them up front (rather than on the fly) as the original
        # IMP objects may have been destroyed or changed (e.g. if we read
        # multiple frames from an RMF file) by the time we write the mmCIF.
        self._atoms = []
        self._spheres = []

    def get_residue_sequence(self, ps):
        """Determine the primary sequence based on Residue particles.
           Return the index of the first residue and the sequence, as a list
           of ihm.ChemComp objects (or None)"""
        restyp = {}
        for p in ps:
            if isinstance(p, IMP.atom.Atom):
                residue = IMP.atom.get_residue(p)
                restyp[residue.get_index()] = residue.get_residue_type()
            elif isinstance(p, IMP.atom.Residue):
                restyp[p.get_index()] = p.get_residue_type()
            else:  # fragment
                resinds = p.get_residue_indexes()
                for ri in resinds:
                    if ri not in restyp:   # don't overwrite residue/atom info
                        restyp[ri] = None
        if not restyp:
            return 1, []
        seq_id_begin = min(restyp.keys())
        seq_id_end = max(restyp.keys())
        return (seq_id_begin,
                [_imp_to_ihm[restyp.get(x)]
                 for x in range(seq_id_begin, seq_id_end + 1)])

    def add_chain(self, ps, asym):
        def matches_asym(s):
            # Match AsymUnit or AsymUnitRange
            return s == asym or hasattr(s, 'asym') and s.asym == asym

        # Consolidate starting models if the same model was used for this
        # asym in a different state or for a different model_id
        smf = _StartingModelFinder(
            asym, [s for s in self._system.orphan_starting_models
                   if matches_asym(s.asym_unit)],
            self._system, self._datasets)
        segfactory = _RepSegmentFactory(asym)
        offset = asym.auth_seq_id_map
        for p in ps:
            starting_model = smf.find(p)
            seg = segfactory.add(p, starting_model)
            if seg:
                self._representation.append(seg)
            self._add_atom_or_sphere(p, asym, offset)
        last = segfactory.get_last()
        if last:
            self._representation.append(last)

    def _add_atom_or_sphere(self, p, asym, offset):
        if isinstance(p, IMP.atom.Atom):
            residue = IMP.atom.get_residue(p)
            xyz = IMP.core.XYZ(p).get_coordinates()
            element = p.get_element()
            element = IMP.atom.get_element_table().get_name(element)
            atom_name = p.get_atom_type().get_string()
            het = atom_name.startswith('HET:')
            if het:
                atom_name = atom_name[4:]
            self._atoms.append(ihm.model.Atom(
                asym_unit=asym, seq_id=residue.get_index() - offset,
                atom_id=atom_name, type_symbol=element,
                x=xyz[0], y=xyz[1], z=xyz[2], het=het,
                biso=p.get_temperature_factor(),
                occupancy=p.get_occupancy()))
        else:
            if isinstance(p, IMP.atom.Fragment):
                resinds = p.get_residue_indexes()
                sbegin = resinds[0]
                send = resinds[-1]
            else:  # residue
                sbegin = send = p.get_index()
            xyzr = IMP.core.XYZR(p)
            xyz = xyzr.get_coordinates()
            self._spheres.append(ihm.model.Sphere(
                asym_unit=asym, seq_id_range=(sbegin - offset, send - offset),
                x=xyz[0], y=xyz[1], z=xyz[2], radius=xyzr.get_radius()))

    def get_structure_particles(self, h):
        """Return particles sorted by residue index"""
        ps = []
        if h.get_number_of_children() == 0:
            return []
        if not h.get_is_valid():
            raise ValueError("Invalid hierarchy as input")
        for p in IMP.atom.Selection(
                hierarchy=h, resolution=0.).get_selected_particles():
            if IMP.atom.Residue.get_is_setup(p):
                residue = IMP.atom.Residue(p)
                ps.append((residue.get_index(), residue))
            elif IMP.atom.Fragment.get_is_setup(p):
                fragment = IMP.atom.Fragment(p)
                resinds = fragment.get_residue_indexes()
                _check_sequential(fragment, resinds)
                resind = resinds[len(resinds) // 2]
                ps.append((resind, fragment))
            elif IMP.atom.Atom.get_is_setup(p):
                atom = IMP.atom.Atom(p)
                residue = IMP.atom.get_residue(atom)
                ps.append((residue.get_index(), atom))
        return [p[1] for p in sorted(ps, key=operator.itemgetter(0))]


class _ModelAssemblies(object):
    def __init__(self, system):
        self.system = system
        self._seen_assemblies = {}

    def add(self, asyms):
        # list isn't hashable but tuple is
        asyms = tuple(asyms)
        if asyms not in self._seen_assemblies:
            assembly = ihm.Assembly(
                asyms, name="Modeled assembly",
                description="All components modeled by IMP")
            self.system.orphan_assemblies.append(assembly)
            self._seen_assemblies[asyms] = assembly
        return self._seen_assemblies[asyms]


class _Representations(object):
    def __init__(self, system):
        self.system = system

    def add(self, rep):
        # Representation isn't hashable or sortable, so just compare dicts
        # with existing representations. This should still be performant as
        # we generally don't have more than one or two representations.
        for existing in self.system.orphan_representations:
            if (len(existing) == len(rep)
                and all(type(x) == type(y)  # noqa: E721
                        and x.__dict__ == y.__dict__
                        for (x, y) in zip(existing, rep))):
                return existing
        self.system.orphan_representations.append(rep)
        return rep
