"""@namespace IMP.mmcif.data
   @brief Classes to represent data structures used in mmCIF.
"""

from __future__ import print_function
import IMP.atom
import IMP.mmcif.dataset
import IMP.mmcif.metadata

def get_molecule(h):
    """Given a Hierarchy, walk up and find the parent Molecule"""
    while h:
        if IMP.atom.Molecule.get_is_setup(h):
            return IMP.atom.Molecule(h)
        h = h.get_parent()
    return None

class _Entity(object):
    """Represent a CIF entity (a component with a unique sequence)"""
    def __init__(self, seq):
        self.sequence = seq
        self.first_component = None
    # Use the name of the first component, stripped of any copy number,
    # as the description of the entity
    def __get_description(self):
        n = self.first_component.name
        # Strip out anything after a @ or .
        return n.split("@")[0].split(".")[0]
    description = property(__get_description)


class _EntityMapper(dict):
    """Handle mapping from IMP chains to CIF entities.
       Multiple components may map to the same entity if they share sequence."""
    def __init__(self):
        super(_EntityMapper, self).__init__()
        self._sequence_dict = {}
        self._entities = []

    def add(self, chain):
        # todo: handle non-protein sequences
        sequence = chain.get_sequence()
        if sequence == '':
            raise ValueError("Chain %s has no sequence" % chain)
        if sequence not in self._sequence_dict:
            entity = _Entity(sequence)
            self._entities.append(entity)
            entity.id = len(self._entities)
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
    def __init__(self):
        super(_ComponentMapper, self).__init__()
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
            if entity.first_component is None:
                entity.first_component = component
            self._all_components.append(component)
            if modeled:
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


class _Assembly(list):
    """A collection of components. Currently simply implemented as a list of
       the _Component objects. These must be in creation order."""
    def __hash__(self):
        # allow putting assemblies in a dict. 'list' isn't hashable
        # but 'tuple' is
        return hash(tuple(self))


class _Assemblies(object):
    """Track all assemblies used in the modeling."""
    def __init__(self):
        self._assemblies = []

    def add(self, a):
        """Add a new assembly. The first such assembly is assumed to contain
           all components. Duplicate assemblies will be pruned at the end."""
        self._assemblies.append(a)
        return a

    def get_subassembly(self, compdict):
        """Get an _Assembly consisting of the given components."""
        # Put components in creation order
        newa = _Assembly(c for c in self._assemblies[0] if c in compdict)
        return self.add(newa)

    def get_all(self):
        """Get all assemblies"""
        return self._assemblies


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

class _StartingModel(object):
    _eq_keys = ['filename', 'chain_id', 'offset']

    def __init__(self, struc_prov):
        self.filename = struc_prov[0].get_filename()
        self.chain_id = struc_prov[0].get_chain_id()
        self.offset = struc_prov[0].get_residue_offset()

    def _add_residue(self, resind):
        self.seq_id_end = resind + self.offset
        if not hasattr(self, 'seq_id_begin'):
            self.seq_id_begin = self.seq_id_end

    def get_seq_id_range_all_sources(self):
        source0 = self.sources[0]
        # Where there are multiple sources (to date, this can only
        # mean multiple templates for a comparative model) consolidate
        # them; template info is given in starting_comparative_models.
        seq_id_begin, seq_id_end = source0.get_seq_id_range(self)
        for source in self.sources[1:]:
            this_begin, this_end = source.get_seq_id_range(self)
            seq_id_begin = min(seq_id_begin, this_begin)
            seq_id_end = max(seq_id_end, this_end)
        return seq_id_begin, seq_id_end

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
        p = IMP.mmcif.metadata._PDBMetadataParser()
        p.parse_file(self.filename, self.chain_id, system)
        self.dataset = p.dataset
        self.sources = p.sources
        self.alignment_file = p.alignment_file


class _StartingModelFinder(object):
    """Map IMP particles to starting model objects"""
    def __init__(self, existing_starting_models):
        self._seen_particles = {}
        self._seen_starting_models = dict.fromkeys(existing_starting_models)

    def find(self, particle, system):
        """Return a StartingModel object, or None, for this particle"""
        def _get_starting_model(sp, resind):
            s = _StartingModel(sp)
            if s not in self._seen_starting_models:
                self._seen_starting_models[s] = s
                s._set_sources_datasets(system)
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
    def __init__(self, external_files):
        super(_Datasets, self).__init__()
        self._datasets = {}
        self._external_files = external_files

    def add(self, d):
        """Add and return a new dataset."""
        if d not in self._datasets:
            self._datasets[d] = d
            d.id = len(self._datasets)
            self._external_files.add_input(d.location)
        return self._datasets[d]

    def get_all(self):
        """Yield all datasets"""
        return self._datasets.keys()


class _Citation(object):
    """A publication that describes the modeling."""
    def __init__(self, pmid, title, journal, volume, page_range, year, authors,
                 doi):
        self.title, self.journal, self.volume = title, journal, volume
        self.page_range, self.year = page_range, year
        self.pmid, self.authors, self.doi = pmid, authors, doi


class _Software(object):
    """Software (other than IMP) used as part of the modeling protocol."""
    def __init__(self, name, classification, description, url,
                 type='program', version=None):
        self.name = name
        self.classification = classification
        self.description = description
        self.url = url
        self.type = type
        self.version = version


class _AllSoftware(list):
    """Keep track of all _Software objects."""
    def __init__(self):
        super(_AllSoftware, self).__init__()
        self.modeller_used = self.phyre2_used = False

    def set_modeller_used(self, version, date):
        if self.modeller_used:
            return
        self.modeller_used = True
        self.append(_Software(
                name='MODELLER', classification='comparative modeling',
                description='Comparative modeling by satisfaction '
                            'of spatial restraints, build ' + date,
                url='https://salilab.org/modeller/',
                version=version))

    def set_phyre2_used(self):
        if self.phyre2_used:
            return
        self.phyre2_used = True
        self.append(_Software(
               name='Phyre2', classification='protein homology modeling',
               description='Protein Homology/analogY Recognition '
                           'Engine V 2.0',
               version='2.0', url='http://www.sbg.bio.ic.ac.uk/~phyre2/'))

    def add_hierarchy(self, h):
        # todo: if no SoftwareProvenance available, use RMF producer field
        for p in IMP.core.get_all_provenance(h,
                                        types=[IMP.core.SoftwareProvenance]):
            self.append(_Software(name=p.get_software_name(),
                                  classification='integrative model building',
                                  description=None,
                                  version=p.get_version(),
                                  url=p.get_location()))


class _ExternalFile(object):
    """A single externally-referenced file"""

    # All valid content types
    INPUT_DATA = "Input data or restraints"
    MODELING_OUTPUT = "Modeling or post-processing output"
    WORKFLOW = "Modeling workflow or script"

    def __init__(self, location, content_type):
        self.location, self.content_type = location, content_type

    # Pass id through to location
    def __set_id(self, i):
        self.location.id = i
    id = property(lambda x: x.location.id, __set_id)
    file_size = property(lambda x: x.location.file_size)

    def __eq__(self, other):
        return self.location == other.location
    def __hash__(self):
        return hash(self.location)


class _ExternalFiles(object):
    """Track all externally-referenced files
       (i.e. anything that refers to a Location that isn't
       a DatabaseLocation)."""
    def __init__(self):
        self._refs = []
        self._repos = []

    def add_repo(self, repo):
        """Add a repository containing modeling files."""
        self._repos.append(repo)

    def _add(self, location, content_type):
        """Add a new externally-referenced file.
           Note that ids are assigned later."""
        self._refs.append(_ExternalFile(location, content_type))

    def add_input(self, location):
        """Add a new externally-referenced file used as input."""
        return self._add(location, _ExternalFile.INPUT_DATA)

    def add_output(self, location):
        """Add a new externally-referenced file produced as output."""
        return self._add(location, _ExternalFile.MODELING_OUTPUT)

    def add_workflow(self, location):
        """Add a new externally-referenced file that's part of the workflow."""
        return self._add(location, _ExternalFile.WORKFLOW)

    def add_hierarchy(self, h):
        # Add all Python scripts that were used in the modeling
        for p in IMP.core.get_all_provenance(h,
                                     types=[IMP.core.ScriptProvenance]):
            # todo: set details
            l = IMP.mmcif.dataset.FileLocation(path=p.get_filename(),
                               details='Integrative modeling Python script')
            self.add_workflow(l)

    def get_all_nondb(self):
        """Yield all external files that are not database hosted"""
        for x in self._refs:
            if not isinstance(x.location, IMP.mmcif.dataset.DatabaseLocation):
                yield x


class _ProtocolStep(object):
    """A single step (e.g. sampling, refinement) in a protocol."""
    def __init__(self, prov, num_models_begin):
        self.num_models_begin = num_models_begin
        self._prov = [prov]
        self.num_models_end = prov.get_number_of_frames()

    def add_combine(self, prov):
        self._prov.append(prov)
        self.num_models_end = prov.get_number_of_frames()
        return self.num_models_end


class _PostProcessing(object):
    """A single postprocessing step (e.g. filtering) in a protocol."""
    def __init__(self, prov, num_models_begin):
        self.num_models_begin = num_models_begin
        self._prov = prov
        if isinstance(prov, IMP.core.FilterProvenance):
            self.num_models_end = prov.get_number_of_frames()
        elif isinstance(prov, IMP.core.ClusterProvenance):
            # Assume clustering uses all models
            self.num_models_end = self.num_models_begin
        else:
            raise ValueError("Unhandled provenance", prov)


class _Protocol(object):
    """A modeling protocol.
       Each protocol consists of a number of protocol steps (e.g. sampling,
       refinement) followed by a number of postprocessing steps (e.g.
       filtering, rescoring, clustering)"""
    def __init__(self):
        self._steps = []
        self._postprocs = []

    def add_step(self, prov, num_models):
        if isinstance(prov, IMP.core.CombineProvenance):
            # Fold CombineProvenance into a previous sampling step
            if len(self._steps) == 0:
                raise ValueError("CombineProvenance with no previous sampling")
            return self._steps[-1].add_combine(prov)
        else:
            ps = _ProtocolStep(prov, num_models)
            self._steps.append(ps)
            ps.id = len(self._steps)
            return ps.num_models_end

    def add_postproc(self, prov, num_models):
        pp = _PostProcessing(prov, num_models)
        self._postprocs.append(pp)
        pp.id = len(self._postprocs)
        return pp.num_models_end


class _Protocols(object):
    """Track all modeling protocols used."""
    def __init__(self):
        self._protocols = []

    def get_all(self):
        return self._protocols

    def _add_protocol(self, prot, modeled_assembly):
        prot.modeled_assembly = modeled_assembly
        self._protocols.append(prot)
        prot.id = len(self._protocols)

    def _add_hierarchy(self, h, modeled_assembly):
        num_models = 0 # assume we always start with no models
        prot_types = (IMP.core.SampleProvenance, IMP.core.CombineProvenance)
        pp_types = (IMP.core.FilterProvenance, IMP.core.ClusterProvenance)
        in_postproc = False
        prot = _Protocol()
        for p in reversed(list(IMP.core.get_all_provenance(
                                          h, types=prot_types + pp_types))):
            if isinstance(p, pp_types):
                num_models = prot.add_postproc(p, num_models)
                in_postproc = True
            else:
                if in_postproc:
                    # Start a new protocol
                    self._add_protocol(prot, modeled_assembly)
                    prot = _Protocol()
                num_models = prot.add_step(p, num_models)
                in_postproc = False
        if len(prot._steps) > 0:
            self._add_protocol(prot, modeled_assembly)
