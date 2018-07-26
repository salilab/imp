"""Utility classes to read in information in mmCIF format"""

import ihm.format
import ihm.location
import ihm.dataset
import ihm.representation
import ihm.startmodel
import ihm.protocol
import ihm.analysis
import ihm.model
import ihm.restraint
import ihm.geometry
import inspect

def _make_new_entity():
    """Make a new Entity object"""
    e = ihm.Entity([])
    # make sequence mutable
    e.sequence = list(e.sequence)
    return e

def _get_lower(d, key):
    """Return lowercase d[key] or None if key is not in d"""
    return d[key].lower() if key in d else None

def _get_int(d, key):
    """Return int(d[key]) or None if key is not in d"""
    return int(d[key]) if key in d else None

def _get_int_or_string(d, key):
    """Return d[key] as an int or str as appropriate,
       or None if key is not in d"""
    if key in d:
        val = d[key]
        return int(val) if val.isdigit() else val

def _get_float(d, key):
    """Return float(d[key]) or None if key is not in d"""
    return float(d[key]) if key in d else None

def _get_vector3(d, key):
    """Return a 3D vector (as a list) from d[key+[1..3]] or None if key
       is not in d"""
    if key+'[1]' in d:
        # Assume if one element is present, all are
        return [float(d[key+"[%d]" % k]) for k in (1,2,3)]

def _get_matrix33(d, key):
    """Return a 3x3 matrix (as a list of lists) from d[key+[1..3][1..3]]
       or None if key is not in d"""
    if key+'[1][1]' in d:
        # Assume if one element is present, all are
        return [[float(d[key+"[%d][%d]" % (i,j)]) for j in (1,2,3)]
                for i in (1,2,3)]

_boolmap = {'YES':True, 'NO':False}
def _get_bool(d, key):
    """Convert d[key] to bool and return, or None if key is not in d"""
    return _boolmap.get(d[key].upper(), None) if key in d else None

class _IDMapper(object):
    """Handle mapping from mmCIF IDs to Python objects.

       :param list system_list: The list in :class:`ihm.System` that keeps
              track of these objects.
       :param class cls: The base class for the Python objects.
    """

    # The attribute in the class used to store the ID
    id_attr = '_id'

    def __init__(self, system_list, cls, *cls_args, **cls_keys):
        self.system_list = system_list
        self._obj_by_id = {}
        self._cls = cls
        self._cls_args = cls_args
        self._cls_keys = cls_keys

    def get_all(self):
        """Yield all objects seen so far (unordered)"""
        return self._obj_by_id.values()

    def _make_new_object(self, newcls=None):
        if newcls is None:
            newcls = self._cls
        return newcls(*self._cls_args, **self._cls_keys)

    def _update_old_object(self, obj, newcls=None):
        # If this object was referenced by another table before it was
        # created, it may have the wrong class - fix that retroactively
        # (need to be careful that old and new classes are compatible)
        if newcls:
            obj.__class__ = newcls

    def get_by_id(self, objid, newcls=None):
        """Get the object with given ID, creating it if it doesn't already
           exist."""
        if objid in self._obj_by_id:
            obj = self._obj_by_id[objid]
            self._update_old_object(obj, newcls)
            return obj
        else:
            newobj = self._make_new_object(newcls)
            if self.id_attr is not None:
                setattr(newobj, self.id_attr, objid)
            self._obj_by_id[objid] = newobj
            if self.system_list is not None:
                self.system_list.append(newobj)
            return newobj

    def get_by_id_or_none(self, d, key, newcls=None):
        """Get the object with ID d[key], creating it if it doesn't already
           exist. If key is not in d, return None instead."""
        return self.get_by_id(d[key], newcls) if key in d else None


class _ChemCompIDMapper(_IDMapper):
    """Add extra handling to _IDMapper for the chem_comp category"""

    id_attr = 'id'

    def __init__(self, *args, **keys):
        super(_ChemCompIDMapper, self).__init__(*args, **keys)
        # get standard residue types
        alphabets = [x[1] for x in inspect.getmembers(ihm, inspect.isclass)
                     if issubclass(x[1], ihm.Alphabet)
                     and x[1] is not ihm.Alphabet]
        self._standard_by_id = {}
        for alphabet in alphabets:
            self._standard_by_id.update((item[1].id, item[1])
                                        for item in alphabet._comps.items())

    def get_by_id(self, objid, newcls=None):
        # Don't modify class of standard residue types
        if objid in self._standard_by_id:
            return self._standard_by_id[objid]
        else:
            return super(_ChemCompIDMapper, self).get_by_id(objid, newcls)


class _AnalysisIDMapper(_IDMapper):
    """Add extra handling to _IDMapper for the post processing category"""

    def _make_new_object(self, newcls=None):
        if newcls is None:
            newcls = self._cls
        if newcls is ihm.analysis.EmptyStep:
            return newcls()
        else:
            return newcls(*self._cls_args, **self._cls_keys)


class _FeatureIDMapper(_IDMapper):
    """Add extra handling to _IDMapper for restraint features"""

    def _make_new_object(self, newcls=None):
        if newcls is None:
            # Make Feature base class (takes no args)
            return self._cls()
        else:
            # Make subclass (takes one ranges/atoms argument)
            return newcls([])

    def _update_old_object(self, obj, newcls=None):
        super(_FeatureIDMapper, self)._update_old_object(obj, newcls)
        # Add missing members if the base class was originally instantianted
        if newcls is ihm.restraint.PolyResidueFeature \
           and not hasattr(obj, 'ranges'):
            obj.ranges = []
        elif newcls is ihm.restraint.PolyAtomFeature \
           and not hasattr(obj, 'atoms'):
            obj.atoms = []


class _GeometryIDMapper(_IDMapper):
    """Add extra handling to _IDMapper for geometric objects"""

    _members = {ihm.geometry.Sphere: ('center', 'radius', 'transformation'),
                ihm.geometry.Torus: ('center', 'transformation',
                                     'major_radius', 'minor_radius'),
                ihm.geometry.HalfTorus: ('center', 'transformation',
                                         'major_radius', 'minor_radius',
                                         'thickness'),
                ihm.geometry.XAxis: ('transformation',),
                ihm.geometry.YAxis: ('transformation',),
                ihm.geometry.ZAxis: ('transformation',),
                ihm.geometry.XYPlane: ('transformation',),
                ihm.geometry.YZPlane: ('transformation',),
                ihm.geometry.XZPlane: ('transformation',)}

    def _make_new_object(self, newcls=None):
        if newcls is None:
            # Make GeometricObject base class (takes no args)
            return self._cls()
        else:
            # Make subclass (takes variable number of args)
            len_args = {ihm.geometry.Sphere: 2,
                        ihm.geometry.Torus: 3,
                        ihm.geometry.HalfTorus: 4}.get(newcls, 0)
            return newcls(*(None,)*len_args)

    def _update_old_object(self, obj, newcls=None):
        # Don't revert a HalfTorus back to a Torus
        if newcls is ihm.geometry.Torus \
           and isinstance(obj, ihm.geometry.HalfTorus):
            return
        # Don't revert a derived class back to a base class
        elif newcls and isinstance(obj, newcls):
            return
        super(_GeometryIDMapper, self)._update_old_object(obj, newcls)
        # Add missing members if the base class was originally instantianted
        for member in self._members.get(newcls, ()):
           if not hasattr(obj, member):
               setattr(obj, member, None)


class _CrossLinkIDMapper(_IDMapper):
    """Add extra handling to _IDMapper for cross links"""

    def _make_new_object(self, newcls=None):
        if newcls is None:
            # Make base class (takes no args)
            obj = self._cls()
            # Need fits in case we never decide on a type
            obj.fits = {}
            return obj
        elif newcls is ihm.restraint.AtomCrossLink:
            return newcls(*(None,)*6)
        else:
            return newcls(*(None,)*4)


class _DatasetIDMapper(object):
    """Handle mapping from mmCIF dataset IDs to Python objects.

       This is similar to _IDMapper but is intended for objects like restraints
       that don't have their own IDs but instead use the dataset ID.

       :param list system_list: The list in :class:`ihm.System` that keeps
              track of these objects.
       :param datasets: Mapping from IDs to Dataset objects.
       :param class cls: The base class for the Python objects. Its constructor
              is expected to take a Dataset object as the first argument.
    """
    def __init__(self, system_list, datasets, cls, *cls_args, **cls_keys):
        self.system_list = system_list
        self.datasets = datasets
        self._obj_by_id = {}
        self._cls = cls
        self._cls_args = cls_args
        self._cls_keys = cls_keys

    def get_by_dataset(self, dataset_id):
        dataset = self.datasets.get_by_id(dataset_id)
        if dataset._id not in self._obj_by_id:
            r = self._cls(dataset, *self._cls_args, **self._cls_keys)
            self.system_list.append(r)
            self._obj_by_id[dataset._id] = r
        else:
            r = self._obj_by_id[dataset._id]
        return r


class _XLRestraintMapper(object):
    """Map entries to CrossLinkRestraint"""

    def __init__(self, system_list):
        self.system_list = system_list
        self._seen_rsrs = {}

    def get_by_attrs(self, dataset, linker_type):
        """Group all crosslinks with same dataset and linker type in one
           CrossLinkRestraint object"""
        k = (dataset._id, linker_type)
        if k not in self._seen_rsrs:
            r = ihm.restraint.CrossLinkRestraint(dataset, linker_type)
            self.system_list.append(r)
            self._seen_rsrs[k] = r
        return self._seen_rsrs[k]

    def get_all(self):
        """Yield all objects seen so far (unordered)"""
        return self._seen_rsrs.values()



class _SystemReader(object):
    """Track global information for a System being read from a file, such
       as the mapping from IDs to objects."""
    def __init__(self, model_class):
        self.system = ihm.System()
        self.software = _IDMapper(self.system.software, ihm.Software,
                                  *(None,)*4)
        self.citations = _IDMapper(self.system.citations, ihm.Citation,
                                   *(None,)*8)
        self.entities = _IDMapper(self.system.entities, _make_new_entity)
        self.asym_units = _IDMapper(self.system.asym_units, ihm.AsymUnit, None)
        self.chem_comps = _ChemCompIDMapper(None, ihm.ChemComp, *(None,)*3)
        self.assemblies = _IDMapper(self.system.orphan_assemblies, ihm.Assembly)
        self.repos = _IDMapper(None, ihm.location.Repository, None)
        self.external_files = _IDMapper(self.system.locations,
                                 ihm.location.FileLocation,
                                 '/') # should always exist?
        self.db_locations = _IDMapper(self.system.locations,
                                 ihm.location.DatabaseLocation, None, None)
        self.datasets = _IDMapper(self.system.orphan_datasets,
                                  ihm.dataset.Dataset, None)
        self.dataset_groups = _IDMapper(self.system.orphan_dataset_groups,
                                  ihm.dataset.DatasetGroup)
        self.starting_models = _IDMapper(self.system.orphan_starting_models,
                                  ihm.startmodel.StartingModel, *(None,)*3)
        self.representations = _IDMapper(self.system.orphan_representations,
                                  ihm.representation.Representation)
        self.protocols = _IDMapper(self.system.orphan_protocols,
                                  ihm.protocol.Protocol)
        self.analysis_steps = _AnalysisIDMapper(None, ihm.analysis.Step,
                                  *(None,)*3)
        self.analyses = _IDMapper(None, ihm.analysis.Analysis)
        self.models = _IDMapper(None, model_class, *(None,)*3)
        self.model_groups = _IDMapper(None, ihm.model.ModelGroup)
        self.states = _IDMapper(None, ihm.model.State)
        self.state_groups = _IDMapper(self.system.state_groups,
                                  ihm.model.StateGroup)
        self.ensembles = _IDMapper(self.system.ensembles,
                                   ihm.model.Ensemble, *(None,)*2)
        self.densities = _IDMapper(None,
                                   ihm.model.LocalizationDensity, *(None,)*2)
        self.em3d_restraints = _DatasetIDMapper(self.system.restraints,
                                   self.datasets,
                                   ihm.restraint.EM3DRestraint, None)
        self.em2d_restraints = _IDMapper(self.system.restraints,
                                   ihm.restraint.EM2DRestraint, *(None,)*2)
        self.sas_restraints = _DatasetIDMapper(self.system.restraints,
                                   self.datasets,
                                   ihm.restraint.SASRestraint, None)
        self.features = _FeatureIDMapper(self.system.orphan_features,
                                   ihm.restraint.Feature)
        self.dist_restraints = _IDMapper(self.system.restraints,
                                   ihm.restraint.DerivedDistanceRestraint,
                                   *(None,)*4)
        self.geometries = _GeometryIDMapper(
                                self.system.orphan_geometric_objects,
                                ihm.geometry.GeometricObject)
        self.centers = _IDMapper(None, ihm.geometry.Center, *(None,)*3)
        self.transformations = _IDMapper(None, ihm.geometry.Transformation,
                                         *(None,)*2)
        self.geom_restraints = _IDMapper(self.system.restraints,
                                   ihm.restraint.GeometricRestraint,
                                   *(None,)*4)
        self.xl_restraints = _XLRestraintMapper(self.system.restraints)
        self.experimental_xl_groups = _IDMapper(None, list)
        self.experimental_xl_groups.id_attr = None
        self.experimental_xls = _IDMapper(None,
                                    ihm.restraint.ExperimentalCrossLink,
                                    *(None,)*2)
        self.cross_links = _CrossLinkIDMapper(None,
                                ihm.restraint.CrossLink)
        self.ordered_procs = _IDMapper(self.system.ordered_processes,
                                    ihm.model.OrderedProcess, None)
        self.ordered_steps = _IDMapper(None, ihm.model.ProcessStep)


class _Handler(object):
    """Base class for all handlers of mmCIF data."""
    def __init__(self, sysr):
        self.sysr = sysr

    def finalize(self):
        pass

    def _copy_if_present(self, obj, data, keys=[], mapkeys={}):
        """Set obj.x from data['x'] for each x in keys if present in data.
           The dict mapkeys is handled similarly except that its keys are looked
           up in data and the corresponding value used to set obj."""
        for key in keys:
            if key in data:
                setattr(obj, key, data[key])
        for key, val in mapkeys.items():
            if key in data:
                setattr(obj, val, data[key])

    system = property(lambda self: self.sysr.system)


class _StructHandler(_Handler):
    category = '_struct'

    def __call__(self, d):
        self._copy_if_present(self.system, d, keys=('title',),
                              mapkeys={'entry_id': 'id'})


class _SoftwareHandler(_Handler):
    category = '_software'

    def __call__(self, d):
        s = self.sysr.software.get_by_id(d['pdbx_ordinal'])
        self._copy_if_present(s, d,
                keys=('name', 'classification', 'description', 'version',
                      'type', 'location'))


class _CitationHandler(_Handler):
    category = '_citation'

    def __call__(self, d):
        s = self.sysr.citations.get_by_id(d['id'])
        self._copy_if_present(s, d,
                keys=('title', 'year'),
                mapkeys={'pdbx_database_id_pubmed':'pmid',
                         'journal_abbrev':'journal',
                         'journal_volume':'volume',
                         'pdbx_database_id_doi':'doi'})
        if 'page_first' in d:
            if 'page_last' in d:
                s.page_range = (d['page_first'], d['page_last'])
            else:
                s.page_range = d['page_first']


class _CitationAuthorHandler(_Handler):
    category = '_citation_author'

    def __call__(self, d):
        s = self.sysr.citations.get_by_id(d['citation_id'])
        if 'name' in d:
            s.authors.append(d['name'])


class _ChemCompHandler(_Handler):
    category = '_chem_comp'

    def __init__(self, *args):
        super(_ChemCompHandler, self).__init__(*args)
        # Map _chem_comp.type to corresponding subclass of ihm.ChemComp
        self.type_map = dict((x[1].type.lower(), x[1])
                             for x in inspect.getmembers(ihm, inspect.isclass)
                             if issubclass(x[1], ihm.ChemComp))

    def __call__(self, d):
        typ = d.get('type', 'other').lower()
        s = self.sysr.chem_comps.get_by_id(d['id'],
                                           self.type_map.get(typ, ihm.ChemComp))


class _EntityHandler(_Handler):
    category = '_entity'

    def __call__(self, d):
        s = self.sysr.entities.get_by_id(d['id'])
        self._copy_if_present(s, d,
                keys=('details', 'type', 'src_method', 'formula_weight'),
                mapkeys={'pdbx_description':'description',
                         'pdbx_number_of_molecules':'number_of_molecules'})


class _EntityPolySeqHandler(_Handler):
    category = '_entity_poly_seq'

    def __call__(self, d):
        s = self.sysr.entities.get_by_id(d['entity_id'])
        seq_id = int(d['num'])
        if seq_id > len(s.sequence):
            s.sequence.extend([None]*(seq_id-len(s.sequence)))
        s.sequence[seq_id-1] = self.sysr.chem_comps.get_by_id(d['mon_id'])


class _StructAsymHandler(_Handler):
    category = '_struct_asym'

    def __call__(self, d):
        s = self.sysr.asym_units.get_by_id(d['id'])
        s.entity = self.sysr.entities.get_by_id(d['entity_id'])
        self._copy_if_present(s, d, keys=('details',))


class _AssemblyDetailsHandler(_Handler):
    category = '_ihm_struct_assembly_details'

    def __call__(self, d):
        s = self.sysr.assemblies.get_by_id(d['assembly_id'])
        self._copy_if_present(s, d,
                mapkeys={'assembly_name':'name',
                         'assembly_description':'description'})


class _AssemblyHandler(_Handler):
    # todo: figure out how to populate System.complete_assembly
    category = '_ihm_struct_assembly'

    def __call__(self, d):
        a_id = d['assembly_id']
        a = self.sysr.assemblies.get_by_id(a_id)
        parent_id = d.get('parent_assembly_id', None)
        if parent_id and parent_id != a_id and not a.parent:
            a.parent = self.sysr.assemblies.get_by_id(parent_id)
        seqrng = (int(d['seq_id_begin']), int(d['seq_id_end']))
        asym_id = d.get('asym_id', None)
        if asym_id:
            asym = self.sysr.asym_units.get_by_id(asym_id)
            a.append(asym(*seqrng))
        else:
            entity = self.sysr.entities.get_by_id(d['entity_id'])
            a.append(entity(*seqrng))


class _LocalFiles(ihm.location.Repository):
    """Placeholder for files stored locally"""
    reference_provider = None
    reference_type = 'Supplementary Files'
    reference = None
    refers_to = 'Other'
    url = None


class _ExtRefHandler(_Handler):
    category = '_ihm_external_reference_info'

    def __init__(self, *args):
        super(_ExtRefHandler, self).__init__(*args)
        self.type_map = {'doi':ihm.location.Repository,
                         'supplementary files':_LocalFiles}

    def __call__(self, d):
        ref_id = d['reference_id']
        typ = d.get('reference_type', 'DOI').lower()
        repo = self.sysr.repos.get_by_id(ref_id,
                             self.type_map.get(typ, ihm.location.Repository))
        self._copy_if_present(repo, d,
                    mapkeys={'reference':'doi', 'associated_url':'url'})

    def finalize(self):
        # Map use of placeholder _LocalFiles repository to repo=None
        for location in self.system.locations:
            if hasattr(location, 'repo') \
                    and isinstance(location.repo, _LocalFiles):
                location.repo = None


class _ExtFileHandler(_Handler):
    category = '_ihm_external_files'

    def __init__(self, *args):
        super(_ExtFileHandler, self).__init__(*args)
        # Map _ihm_external_files.content_type to corresponding
        # subclass of ihm.location.FileLocation
        self.type_map = dict(
                (x[1].content_type.lower(), x[1])
                for x in inspect.getmembers(ihm.location, inspect.isclass)
                if issubclass(x[1], ihm.location.FileLocation)
                and x[1] is not ihm.location.FileLocation)

    def __call__(self, d):
        if 'content_type' in d:
            typ = d['content_type'].lower()
        else:
            typ = None
        f = self.sysr.external_files.get_by_id(d['id'],
                             self.type_map.get(typ, ihm.location.FileLocation))
        f.repo = self.sysr.repos.get_by_id(d['reference_id'])
        self._copy_if_present(f, d,
                    keys=['details'],
                    mapkeys={'file_path':'path'})
        # Handle DOI that is itself a file
        if 'file_path' not in d:
            f.path = '.'


class _DatasetListHandler(_Handler):
    category = '_ihm_dataset_list'

    def __init__(self, *args):
        super(_DatasetListHandler, self).__init__(*args)
        # Map data_type to corresponding
        # subclass of ihm.dataset.Dataset
        self.type_map = dict(
                (x[1].data_type.lower(), x[1])
                for x in inspect.getmembers(ihm.dataset, inspect.isclass)
                if issubclass(x[1], ihm.dataset.Dataset))

    def __call__(self, d):
        if 'data_type' in d:
            typ = d['data_type'].lower()
        else:
            typ = None
        f = self.sysr.datasets.get_by_id(d['id'],
                             self.type_map.get(typ, ihm.dataset.Dataset))


class _DatasetGroupHandler(_Handler):
    category = '_ihm_dataset_group'

    def __call__(self, d):
        g = self.sysr.dataset_groups.get_by_id(d['group_id'])
        ds = self.sysr.datasets.get_by_id(d['dataset_list_id'])
        g.append(ds)


class _DatasetExtRefHandler(_Handler):
    category = '_ihm_dataset_external_reference'

    def __call__(self, d):
        ds = self.sysr.datasets.get_by_id(d['dataset_list_id'])
        f = self.sysr.external_files.get_by_id(d['file_id'])
        ds.location = f


class _DatasetDBRefHandler(_Handler):
    category = '_ihm_dataset_related_db_reference'

    def __init__(self, *args):
        super(_DatasetDBRefHandler, self).__init__(*args)
        # Map data_type to corresponding
        # subclass of ihm.location.DatabaseLocation
        self.type_map = dict(
                (x[1]._db_name.lower(), x[1])
                for x in inspect.getmembers(ihm.location, inspect.isclass)
                if issubclass(x[1], ihm.location.DatabaseLocation)
                and x[1] is not ihm.location.DatabaseLocation)

    def __call__(self, d):
        ds = self.sysr.datasets.get_by_id(d['dataset_list_id'])
        if 'db_name' in d:
            typ = d['db_name'].lower()
        else:
            typ = None
        dbloc = self.sysr.db_locations.get_by_id(d['id'],
                                                 self.type_map.get(typ, None))
        ds.location = dbloc
        self._copy_if_present(dbloc, d,
                    keys=['version', 'details'],
                    mapkeys={'accession_code':'access_code'})


class _RelatedDatasetsHandler(_Handler):
    category = '_ihm_related_datasets'

    def __call__(self, d):
        derived = self.sysr.datasets.get_by_id(d['dataset_list_id_derived'])
        primary = self.sysr.datasets.get_by_id(d['dataset_list_id_primary'])
        derived.parents.append(primary)


def _make_atom_segment(asym, rigid, primitive, count, smodel):
    return ihm.representation.AtomicSegment(
                asym_unit=asym, rigid=rigid, starting_model=smodel)

def _make_residue_segment(asym, rigid, primitive, count, smodel):
    return ihm.representation.ResidueSegment(
                asym_unit=asym, rigid=rigid, primitive=primitive,
                starting_model=smodel)

def _make_multi_residue_segment(asym, rigid, primitive, count, smodel):
    return ihm.representation.MultiResidueSegment(
                asym_unit=asym, rigid=rigid, primitive=primitive,
                starting_model=smodel)

def _make_feature_segment(asym, rigid, primitive, count, smodel):
    return ihm.representation.FeatureSegment(
                asym_unit=asym, rigid=rigid, primitive=primitive,
                count=count, starting_model=smodel)

class _ModelRepresentationHandler(_Handler):
    category = '_ihm_model_representation'

    _rigid_map = {'rigid': True, 'flexible': False, '': None}
    _segment_factory = {'by-atom': _make_atom_segment,
                        'by-residue': _make_residue_segment,
                        'multi-residue': _make_multi_residue_segment,
                        'by-feature': _make_feature_segment}

    def __call__(self, d):
        asym = self.sysr.asym_units.get_by_id(d['entity_asym_id'])
        if 'seq_id_begin' in d and 'seq_id_end' in d:
            asym = asym(int(d['seq_id_begin']), int(d['seq_id_end']))
        rep = self.sysr.representations.get_by_id(d['representation_id'])
        smodel = self.sysr.starting_models.get_by_id_or_none(
                                            d, 'starting_model_id')
        primitive = _get_lower(d, 'model_object_primitive')
        gran = _get_lower(d, 'model_granularity')
        primitive = _get_lower(d, 'model_object_primitive')
        count = _get_int(d, 'model_object_count')
        rigid = self._rigid_map[d.get('model_mode', '').lower()]
        segment = self._segment_factory[gran](asym, rigid, primitive,
                                              count, smodel)
        rep.append(segment)


# todo: support user subclass of StartingModel, pass it coordinates, seqdif
class _StartingModelDetailsHandler(_Handler):
    category = '_ihm_starting_model_details'

    def __call__(self, d):
        m = self.sysr.starting_models.get_by_id(d['starting_model_id'])
        asym = self.sysr.asym_units.get_by_id(d['asym_id'])
        if 'seq_id_begin' in d and 'seq_id_end' in d:
            asym = asym(int(d['seq_id_begin']), int(d['seq_id_end']))
        m.asym_unit = asym
        m.dataset = self.sysr.datasets.get_by_id(d['dataset_list_id'])
        self._copy_if_present(m, d,
                    mapkeys={'starting_model_auth_asym_id':'asym_id'})
        if 'starting_model_sequence_offset' in d:
            m.offset = int(d['starting_model_sequence_offset'])


class _StartingComputationalModelsHandler(_Handler):
    category = '_ihm_starting_computational_models'

    def __call__(self, d):
        m = self.sysr.starting_models.get_by_id(d['starting_model_id'])
        if 'script_file_id' in d:
            m.script_file = self.sysr.external_files.get_by_id(
                                                      d['script_file_id'])
        if 'software_id' in d:
            m.software = self.sysr.software.get_by_id(d['software_id'])


class _StartingComparativeModelsHandler(_Handler):
    category = '_ihm_starting_comparative_models'

    def __call__(self, d):
        m = self.sysr.starting_models.get_by_id(d['starting_model_id'])
        dataset = self.sysr.datasets.get_by_id(d['template_dataset_list_id'])
        aln = self.sysr.external_files.get_by_id_or_none(
                                            d, 'alignment_file_id')
        asym_id = d.get('template_auth_asym_id', None)
        seq_id_range = (int(d['starting_model_seq_id_begin']),
                        int(d['starting_model_seq_id_end']))
        template_seq_id_range = (int(d['template_seq_id_begin']),
                                 int(d['template_seq_id_end']))
        identity = ihm.startmodel.SequenceIdentity(
                      _get_float(d, 'template_sequence_identity'),
                      _get_int(d, 'template_sequence_identity_denominator'))
        t = ihm.startmodel.Template(dataset, asym_id, seq_id_range,
                        template_seq_id_range, identity, aln)
        m.templates.append(t)


class _ProtocolHandler(_Handler):
    category = '_ihm_modeling_protocol'

    def __call__(self, d):
        p = self.sysr.protocols.get_by_id(d['protocol_id'])
        self._copy_if_present(p, d, mapkeys={'protocol_name':'name'})
        nbegin = _get_int(d, 'num_models_begin')
        nend = _get_int(d, 'num_models_end')
        mscale = _get_bool(d, 'multi_scale_flag')
        mstate = _get_bool(d, 'multi_state_flag')
        ordered = _get_bool(d, 'ordered_flag')
        assembly = self.sysr.assemblies.get_by_id_or_none(
                                            d, 'struct_assembly_id')
        dg = self.sysr.dataset_groups.get_by_id_or_none(d, 'dataset_group_id')
        software = self.sysr.software.get_by_id_or_none(d, 'software_id')
        script = self.sysr.external_files.get_by_id_or_none(d, 'script_file_id')
        s = ihm.protocol.Step(assembly=assembly, dataset_group=dg,
                              method=None, num_models_begin=nbegin,
                              num_models_end=nend, multi_scale=mscale,
                              multi_state=mstate, ordered=ordered,
                              software=software, script_file=script)
        self._copy_if_present(s, d,
                mapkeys={'step_name':'name', 'step_method':'method'})
        p.steps.append(s)


class _PostProcessHandler(_Handler):
    category = '_ihm_modeling_post_process'

    def __init__(self, *args):
        super(_PostProcessHandler, self).__init__(*args)
        # Map _ihm_modeling_post_process.type to corresponding subclass
        # of ihm.analysis.Step
        self.type_map = dict((x[1].type.lower(), x[1])
                             for x in inspect.getmembers(ihm.analysis,
                                                         inspect.isclass)
                             if issubclass(x[1], ihm.analysis.Step)
                             and x[1] is not ihm.analysis.Step)

    def __call__(self, d):
        protocol = self.sysr.protocols.get_by_id(d['protocol_id'])
        analysis = self.sysr.analyses.get_by_id(d['analysis_id'])
        if analysis._id not in [a._id for a in protocol.analyses]:
            protocol.analyses.append(analysis)

        typ = d.get('type', 'other').lower()
        step = self.sysr.analysis_steps.get_by_id(d['id'],
                                self.type_map.get(typ, ihm.analysis.Step))
        analysis.steps.append(step)

        if typ == 'none':
            # If this step was forward referenced, feature will have been set
            # to Python None - set it to explicit 'none' instead
            step.feature = 'none'
        else:
            step.num_models_begin = _get_int(d, 'num_models_begin')
            step.num_models_end = _get_int(d, 'num_models_end')
            step.assembly = self.sysr.assemblies.get_by_id_or_none(
                                            d, 'struct_assembly_id')
            step.dataset_group = self.sysr.dataset_groups.get_by_id_or_none(
                                            d, 'dataset_group_id')
            step.software = self.sysr.software.get_by_id_or_none(
                                            d, 'software_id')
            step.script_file = self.sysr.external_files.get_by_id_or_none(
                                            d, 'script_file_id')
            self._copy_if_present(step, d, keys=['feature'])


class _ModelListHandler(_Handler):
    category = '_ihm_model_list'

    def __call__(self, d):
        model_group = self.sysr.model_groups.get_by_id(d['model_group_id'])
        self._copy_if_present(model_group, d,
                              mapkeys={'model_group_name':'name'})

        model = self.sysr.models.get_by_id(d['model_id'])

        assert model._id not in (m._id for m in model_group)
        model_group.append(model)

        self._copy_if_present(model, d, mapkeys={'model_name':'name'})
        model.assembly = self.sysr.assemblies.get_by_id_or_none(
                                            d, 'assembly_id')
        model.representation = self.sysr.representations.get_by_id_or_none(
                                            d, 'representation_id')
        model.protocol = self.sysr.protocols.get_by_id_or_none(
                                            d, 'protocol_id')

    def finalize(self):
        # Put all model groups not assigned to a state in their own state
        # todo: handle models not in model groups too?
        model_groups_in_states = set()
        for sg in self.system.state_groups:
            for state in sg:
                for model_group in state:
                    model_groups_in_states.add(model_group._id)
        mgs = [mg for mgid, mg in self.sysr.model_groups._obj_by_id.items()
               if mgid not in model_groups_in_states]
        if mgs:
            s = ihm.model.State(mgs)
            self.system.state_groups.append(ihm.model.StateGroup([s]))


class _MultiStateHandler(_Handler):
    category = '_ihm_multi_state_modeling'

    def __call__(self, d):
        state_group = self.sysr.state_groups.get_by_id(d['state_group_id'])
        state = self.sysr.states.get_by_id(d['state_id'])

        if state._id not in [s._id for s in state_group]:
            state_group.append(state)

        model_group = self.sysr.model_groups.get_by_id(d['model_group_id'])
        state.append(model_group)

        state.population_fraction = _get_float(d, 'population_fraction')
        self._copy_if_present(state, d,
                keys=['experiment_type', 'details'],
                mapkeys={'state_name':'name', 'state_type':'type'})


class _EnsembleHandler(_Handler):
    category = '_ihm_ensemble_info'

    def __call__(self, d):
        ensemble = self.sysr.ensembles.get_by_id(d['ensemble_id'])
        mg = self.sysr.model_groups.get_by_id(d['model_group_id'])
        pp = self.sysr.analysis_steps.get_by_id_or_none(d, 'post_process_id')
        f = self.sysr.external_files.get_by_id_or_none(d, 'ensemble_file_id')

        ensemble.model_group = mg
        ensemble.num_models = _get_int(d, 'num_ensemble_models')
        ensemble.precision = _get_float(d, 'ensemble_precision_value')
        # note that num_ensemble_models_deposited is ignored (should be size of
        # model group anyway)
        ensemble.post_process = pp
        ensemble.file = f
        self._copy_if_present(ensemble, d,
                mapkeys={'ensemble_name':'name',
                         'ensemble_clustering_method':'clustering_method',
                         'ensemble_clustering_feature':'clustering_feature'})


class _DensityHandler(_Handler):
    category = '_ihm_localization_density_files'

    def __call__(self, d):
        density = self.sysr.densities.get_by_id(d['id'])
        ensemble = self.sysr.ensembles.get_by_id(d['ensemble_id'])
        f = self.sysr.external_files.get_by_id(d['file_id'])

        asym = self.sysr.asym_units.get_by_id(d['asym_id'])
        if 'seq_id_begin' in d and 'seq_id_end' in d:
            asym = asym(int(d['seq_id_begin']), int(d['seq_id_end']))

        density.asym_unit = asym
        density.file = f
        ensemble.densities.append(density)


class _EM3DRestraintHandler(_Handler):
    category = '_ihm_3dem_restraint'

    def __call__(self, d):
        # EM3D restraints don't have their own IDs - they use the dataset id
        r = self.sysr.em3d_restraints.get_by_dataset(d['dataset_list_id'])
        r.assembly = self.sysr.assemblies.get_by_id_or_none(
                                            d, 'struct_assembly_id')
        r.fitting_method_citation = self.sysr.citations.get_by_id_or_none(
                                            d, 'fitting_method_citation_id')
        self._copy_if_present(r, d, keys=('fitting_method',))
        r.number_of_gaussians = _get_int(d, 'number_of_gaussians')

        model = self.sysr.models.get_by_id(d['model_id'])
        ccc = _get_float(d, 'cross_correlation_coefficient')
        r.fits[model] = ihm.restraint.EM3DRestraintFit(ccc)


class _EM2DRestraintHandler(_Handler):
    category = '_ihm_2dem_class_average_restraint'

    def __call__(self, d):
        r = self.sysr.em2d_restraints.get_by_id(d['id'])
        r.dataset = self.sysr.datasets.get_by_id(d['dataset_list_id'])
        r.number_raw_micrographs = _get_int(d, 'number_raw_micrographs')
        r.pixel_size_width = _get_float(d, 'pixel_size_width')
        r.pixel_size_height = _get_float(d, 'pixel_size_height')
        r.image_resolution = _get_float(d, 'image_resolution')
        r.segment = _get_bool(d, 'image_segment_flag')
        r.number_of_projections = _get_int(d, 'number_of_projections')
        r.assembly = self.sysr.assemblies.get_by_id_or_none(
                                            d, 'struct_assembly_id')
        self._copy_if_present(r, d, keys=('details',))


class _EM2DFittingHandler(_Handler):
    category = '_ihm_2dem_class_average_fitting'

    def __call__(self, d):
        r = self.sysr.em2d_restraints.get_by_id(d['restraint_id'])
        model = self.sysr.models.get_by_id(d['model_id'])
        ccc = _get_float(d, 'cross_correlation_coefficient')
        tr_vector = _get_vector3(d, 'tr_vector')
        rot_matrix = _get_matrix33(d, 'rot_matrix')
        r.fits[model] = ihm.restraint.EM2DRestraintFit(
                                  cross_correlation_coefficient=ccc,
                                  rot_matrix=rot_matrix, tr_vector=tr_vector)


class _SASRestraintHandler(_Handler):
    category = '_ihm_sas_restraint'

    def __call__(self, d):
        # SAS restraints don't have their own IDs - they use the dataset id
        r = self.sysr.sas_restraints.get_by_dataset(d['dataset_list_id'])
        r.assembly = self.sysr.assemblies.get_by_id_or_none(
                                            d, 'struct_assembly_id')
        r.segment = _get_bool(d, 'profile_segment_flag')
        self._copy_if_present(r, d,
                keys=('fitting_atom_type', 'fitting_method', 'details'))
        r.multi_state = d.get('fitting_state', 'Single') != 'Single'
        r.radius_of_gyration = _get_float(d, 'radius_of_gyration')
        r.number_of_gaussians = _get_int(d, 'number_of_gaussians')

        model = self.sysr.models.get_by_id(d['model_id'])
        r.fits[model] = ihm.restraint.SASRestraintFit(
                                 chi_value=_get_float(d, 'chi_value'))


class _SphereObjSiteHandler(_Handler):
    category = '_ihm_sphere_obj_site'

    def __call__(self, d):
        model = self.sysr.models.get_by_id(d['model_id'])
        asym = self.sysr.asym_units.get_by_id(d['asym_id'])
        rmsf = float(d['rmsf']) if 'rmsf' in d else None
        s = ihm.model.Sphere(asym_unit=asym,
                seq_id_range=(int(d['seq_id_begin']), int(d['seq_id_end'])),
                x=float(d['cartn_x']), y=float(d['cartn_y']),
                z=float(d['cartn_z']), radius=float(d['object_radius']),
                rmsf=rmsf)
        model.add_sphere(s)


class _AtomSiteHandler(_Handler):
    category = '_atom_site'

    def __call__(self, d):
        # todo: handle fields other than those output by us
        # todo: handle insertion codes
        model = self.sysr.models.get_by_id(d['pdbx_pdb_model_num'])
        asym = self.sysr.asym_units.get_by_id(d['label_asym_id'])
        biso = float(d['b_iso_or_equiv']) if 'b_iso_or_equiv' in d else None
        # seq_id can be None for non-polymers (HETATM)
        seq_id = _get_int(d, 'label_seq_id')
        a = ihm.model.Atom(asym_unit=asym,
                seq_id=seq_id,
                atom_id=d['label_atom_id'],
                type_symbol=d['type_symbol'],
                x=float(d['cartn_x']), y=float(d['cartn_y']),
                z=float(d['cartn_z']), het=d.get('group_pdb', 'ATOM') != 'ATOM',
                biso=biso)
        model.add_atom(a)

        auth_seq_id = _get_int_or_string(d, 'auth_seq_id')
        # Note any residues that have different seq_id and auth_seq_id
        if auth_seq_id is not None and seq_id != auth_seq_id:
            if asym.auth_seq_id_map == 0:
                asym.auth_seq_id_map = {}
            asym.auth_seq_id_map[seq_id] = auth_seq_id


class _PolyResidueFeatureHandler(_Handler):
    category = '_ihm_poly_residue_feature'

    def __call__(self, d):
        f = self.sysr.features.get_by_id(
                           d['feature_id'], ihm.restraint.PolyResidueFeature)
        asym = self.sysr.asym_units.get_by_id(d['asym_id'])
        r1 = int(d['seq_id_begin'])
        r2 = int(d['seq_id_end'])
        f.ranges.append(asym(r1,r2))


class _PolyAtomFeatureHandler(_Handler):
    category = '_ihm_poly_atom_feature'

    def __call__(self, d):
        f = self.sysr.features.get_by_id(
                           d['feature_id'], ihm.restraint.PolyAtomFeature)
        asym = self.sysr.asym_units.get_by_id(d['asym_id'])
        seq_id = int(d['seq_id'])
        atom = asym.residue(seq_id).atom(d['atom_id'])
        f.atoms.append(atom)


def _make_harmonic(d):
    low = _get_float(d, 'distance_lower_limit')
    up = _get_float(d, 'distance_upper_limit')
    return ihm.restraint.HarmonicDistanceRestraint(up if low is None else low)

def _make_upper_bound(d):
    up = _get_float(d, 'distance_upper_limit')
    return ihm.restraint.UpperBoundDistanceRestraint(up)

def _make_lower_bound(d):
    low = _get_float(d, 'distance_lower_limit')
    return ihm.restraint.LowerBoundDistanceRestraint(low)

def _make_lower_upper_bound(d):
    low = _get_float(d, 'distance_lower_limit')
    up = _get_float(d, 'distance_upper_limit')
    return ihm.restraint.LowerUpperBoundDistanceRestraint(
                    distance_lower_limit=low, distance_upper_limit=up)

# todo: add a handler for an unknown restraint_type
_handle_distance = {'harmonic': _make_harmonic,
                    'upper bound': _make_upper_bound,
                    'lower bound': _make_lower_bound,
                    'lower and upper bound': _make_lower_upper_bound}

class _DerivedDistanceRestraintHandler(_Handler):
    category = '_ihm_derived_distance_restraint'
    _cond_map = {'ALL': True, 'ANY': False, None: None}

    def __call__(self, d):
        r = self.sysr.dist_restraints.get_by_id(d['id'])
        r.dataset = self.sysr.datasets.get_by_id_or_none(d, 'dataset_list_id')
        r.feature1 = self.sysr.features.get_by_id(d['feature_id_1'])
        r.feature2 = self.sysr.features.get_by_id(d['feature_id_2'])
        r.distance = _handle_distance[d['restraint_type']](d)
        r.restrain_all = self._cond_map[d.get('group_conditionality', None)]
        r.probability = _get_float(d, 'probability')


class _CenterHandler(_Handler):
    category = '_ihm_geometric_object_center'

    def __call__(self, d):
        c = self.sysr.centers.get_by_id(d['id'])
        c.x = _get_float(d, 'xcoord')
        c.y = _get_float(d, 'ycoord')
        c.z = _get_float(d, 'zcoord')


class _TransformationHandler(_Handler):
    category = '_ihm_geometric_object_transformation'

    def __call__(self, d):
        t = self.sysr.transformations.get_by_id(d['id'])
        t.rot_matrix = _get_matrix33(d, 'rot_matrix')
        t.tr_vector = _get_vector3(d, 'tr_vector')


class _GeometricObjectHandler(_Handler):
    category = '_ihm_geometric_object_list'

    # Map object_type to corresponding subclass (but not subsubclasses such
    # as XYPlane)
    _type_map = dict((x[1].type.lower(), x[1])
                     for x in inspect.getmembers(ihm.geometry, inspect.isclass)
                     if issubclass(x[1], ihm.geometry.GeometricObject)
                        and ihm.geometry.GeometricObject in x[1].__bases__)

    def __call__(self, d):
        typ = d.get('object_type', 'other').lower()
        g = self.sysr.geometries.get_by_id(d['object_id'],
                          self._type_map.get(typ, ihm.geometry.GeometricObject))
        self._copy_if_present(g, d,
                              mapkeys={'object_name': 'name',
                                       'object_description': 'description',
                                       'other_details': 'details'})


class _SphereHandler(_Handler):
    category = '_ihm_geometric_object_sphere'

    def __call__(self, d):
        s = self.sysr.geometries.get_by_id(d['object_id'], ihm.geometry.Sphere)
        s.center = self.sysr.centers.get_by_id_or_none(d, 'center_id')
        s.transformation = self.sysr.transformations.get_by_id_or_none(
                                                  d, 'transformation_id')
        s.radius = _get_float(d, 'radius_r')


class _TorusHandler(_Handler):
    category = '_ihm_geometric_object_torus'

    def __call__(self, d):
        t = self.sysr.geometries.get_by_id(d['object_id'], ihm.geometry.Torus)
        t.center = self.sysr.centers.get_by_id_or_none(d, 'center_id')
        t.transformation = self.sysr.transformations.get_by_id_or_none(
                                                  d, 'transformation_id')
        t.major_radius = _get_float(d, 'major_radius_r')
        t.minor_radius = _get_float(d, 'minor_radius_r')


class _HalfTorusHandler(_Handler):
    category = '_ihm_geometric_object_half_torus'

    _inner_map = {'inner half': True, 'outer half': False}

    def __call__(self, d):
        t = self.sysr.geometries.get_by_id(d['object_id'],
                                           ihm.geometry.HalfTorus)
        t.thickness = _get_float(d, 'thickness_th')
        t.inner = self._inner_map.get(d.get('section', '').lower(), None)


class _AxisHandler(_Handler):
    category = '_ihm_geometric_object_axis'

    # Map axis_type to corresponding subclass
    _type_map = dict((x[1].axis_type.lower(), x[1])
                     for x in inspect.getmembers(ihm.geometry, inspect.isclass)
                     if issubclass(x[1], ihm.geometry.Axis)
                        and x[1] is not ihm.geometry.Axis)

    def __call__(self, d):
        typ = d.get('axis_type', 'other').lower()
        a = self.sysr.geometries.get_by_id(d['object_id'],
                          self._type_map.get(typ, ihm.geometry.Axis))
        a.transformation = self.sysr.transformations.get_by_id_or_none(
                                                  d, 'transformation_id')

class _PlaneHandler(_Handler):
    category = '_ihm_geometric_object_plane'

    # Map plane_type to corresponding subclass
    _type_map = dict((x[1].plane_type.lower(), x[1])
                     for x in inspect.getmembers(ihm.geometry, inspect.isclass)
                     if issubclass(x[1], ihm.geometry.Plane)
                        and x[1] is not ihm.geometry.Plane)

    def __call__(self, d):
        typ = d.get('plane_type', 'other').lower()
        a = self.sysr.geometries.get_by_id(d['object_id'],
                          self._type_map.get(typ, ihm.geometry.Plane))
        a.transformation = self.sysr.transformations.get_by_id_or_none(
                                                  d, 'transformation_id')


class _GeometricRestraintHandler(_Handler):
    category = '_ihm_geometric_object_distance_restraint'

    _cond_map = {'ALL': True, 'ANY': False, None: None}

    # Map object_characteristic to corresponding subclass
    _type_map = dict((x[1].object_characteristic.lower(), x[1])
                     for x in inspect.getmembers(ihm.restraint, inspect.isclass)
                     if issubclass(x[1], ihm.restraint.GeometricRestraint))

    def __call__(self, d):
        typ = d.get('object_characteristic', 'other').lower()
        r = self.sysr.geom_restraints.get_by_id(d['id'],
                      self._type_map.get(typ, ihm.restraint.GeometricRestraint))
        r.dataset = self.sysr.datasets.get_by_id_or_none(d, 'dataset_list_id')
        r.geometric_object = self.sysr.geometries.get_by_id(d['object_id'])
        r.feature = self.sysr.features.get_by_id(d['feature_id'])
        r.distance = _handle_distance[d['restraint_type']](d)
        r.harmonic_force_constant = _get_float(d, 'harmonic_force_constant')
        r.restrain_all = self._cond_map[d.get('group_conditionality', None)]


class _PolySeqSchemeHandler(_Handler):
    category = '_pdbx_poly_seq_scheme'

    def __call__(self, d):
        asym = self.sysr.asym_units.get_by_id(d['asym_id'])
        seq_id = _get_int(d, 'seq_id')
        auth_seq_num = _get_int_or_string(d, 'auth_seq_num')
        # Note any residues that have different seq_id and auth_seq_id
        if seq_id is not None and auth_seq_num is not None \
           and seq_id != auth_seq_num:
            if asym.auth_seq_id_map == 0:
                asym.auth_seq_id_map = {}
            asym.auth_seq_id_map[seq_id] = auth_seq_num

    def finalize(self):
        for asym in self.sysr.system.asym_units:
            # If every residue in auth_seq_id_map is offset by the same
            # amount, replace the map with a simple offset
            offset = self._get_auth_seq_id_offset(asym)
            if offset is not None:
                asym.auth_seq_id_map = offset

    def _get_auth_seq_id_offset(self, asym):
        """Get the offset from seq_id to auth_seq_id. Return None if no
           consistent offset exists."""
        # Do nothing if no map exists
        if asym.auth_seq_id_map == 0:
            return
        rng = asym.seq_id_range
        offset = None
        for seq_id in range(rng[0], rng[1]+1):
            # If a residue isn't in the map, it has an effective offset of 0,
            # which has to be inconsistent (since everything in the map has
            # a nonzero offset by construction)
            if seq_id not in asym.auth_seq_id_map:
                return
            auth_seq_id = asym.auth_seq_id_map[seq_id]
            # If auth_seq_id is a string, we can't use any offset
            if not isinstance(auth_seq_id, int):
                return
            this_offset = auth_seq_id - seq_id
            if offset is None:
                offset = this_offset
            elif offset != this_offset:
                # Offset is inconsistent
                return
        return offset


class _CrossLinkListHandler(_Handler):
    category = '_ihm_cross_link_list'

    def __init__(self, *args):
        super(_CrossLinkListHandler, self).__init__(*args)
        self._seen_group_ids = set()

    def __call__(self, d):
        dataset = self.sysr.datasets.get_by_id_or_none(d, 'dataset_list_id')
        linker_type = d['linker_type'].upper()
        # Group all crosslinks with same dataset and linker type in one
        # CrossLinkRestraint object
        r = self.sysr.xl_restraints.get_by_attrs(dataset, linker_type)

        group_id = d['group_id']
        xl_group = self.sysr.experimental_xl_groups.get_by_id(group_id)
        xl = self.sysr.experimental_xls.get_by_id(d['id'])

        if group_id not in self._seen_group_ids:
            self._seen_group_ids.add(group_id)
            r.experimental_cross_links.append(xl_group)
        xl_group.append(xl)
        xl.residue1 = self._get_entity_residue(d, '_1')
        xl.residue2 = self._get_entity_residue(d, '_2')

    def _get_entity_residue(self, d, suffix):
        entity = self.sysr.entities.get_by_id(d['entity_id' + suffix])
        seq_id = int(d['seq_id' + suffix])
        return entity.residue(seq_id)


class _CrossLinkRestraintHandler(_Handler):
    category = '_ihm_cross_link_restraint'

    _cond_map = {'ALL': True, 'ANY': False, None: None}
    _distance_map = {'harmonic': ihm.restraint.HarmonicDistanceRestraint,
                     'lower bound': ihm.restraint.LowerBoundDistanceRestraint,
                     'upper bound': ihm.restraint.UpperBoundDistanceRestraint}

    # Map granularity to corresponding subclass
    _type_map = dict((x[1].granularity.lower(), x[1])
                     for x in inspect.getmembers(ihm.restraint, inspect.isclass)
                     if issubclass(x[1], ihm.restraint.CrossLink)
                     and x[1] is not ihm.restraint.CrossLink)

    def __call__(self, d):
        typ = d.get('model_granularity', 'other').lower()
        xl = self.sysr.cross_links.get_by_id(d['id'],
                      self._type_map.get(typ, ihm.restraint.ResidueCrossLink))
        ex_xl = self.sysr.experimental_xls.get_by_id(d['group_id'])

        xl.experimental_cross_link = ex_xl
        xl.asym1 = self.sysr.asym_units.get_by_id(d['asym_id_1'])
        xl.asym2 = self.sysr.asym_units.get_by_id(d['asym_id_2'])
        # todo: handle unknown restraint type
        _distcls = self._distance_map[d['restraint_type'].lower()]
        xl.distance = _distcls(float(d['distance_threshold']))
        xl.restrain_all = self._cond_map[d.get('conditional_crosslink_flag',
                                               None)]
        if isinstance(xl, ihm.restraint.AtomCrossLink):
            xl.atom1 = d['atom_id_1']
            xl.atom2 = d['atom_id_2']
        xl.psi = _get_float(d, 'psi')
        xl.sigma1 = _get_float(d, 'sigma_1')
        xl.sigma2 = _get_float(d, 'sigma_2')

    def finalize(self):
        # Put each cross link in the restraint that owns its experimental xl
        rsr_for_ex_xl = {}
        for r in self.sysr.xl_restraints.get_all():
            for ex_xl_group in r.experimental_cross_links:
                for ex_xl in ex_xl_group:
                    rsr_for_ex_xl[ex_xl] = r
        for xl in self.sysr.cross_links.get_all():
            r = rsr_for_ex_xl[xl.experimental_cross_link]
            r.cross_links.append(xl)


class _CrossLinkResultHandler(_Handler):
    category = '_ihm_cross_link_result_parameters'

    def __call__(self, d):
        xl = self.sysr.cross_links.get_by_id(d['restraint_id'])
        model = self.sysr.models.get_by_id(d['model_id'])
        xl.fits[model] = ihm.restraint.CrossLinkFit(
                                psi=_get_float(d, 'psi'),
                                sigma1=_get_float(d, 'sigma_1'),
                                sigma2=_get_float(d, 'sigma_2'))


class _OrderedEnsembleHandler(_Handler):
    category = '_ihm_ordered_ensemble'

    def __call__(self, d):
        proc = self.sysr.ordered_procs.get_by_id(d['process_id'])
        # todo: will this work with multiple processes?
        step = self.sysr.ordered_steps.get_by_id(d['step_id'])
        edge = ihm.model.ProcessEdge(
                   self.sysr.model_groups.get_by_id(d['model_group_id_begin']),
                   self.sysr.model_groups.get_by_id(d['model_group_id_end']))
        self._copy_if_present(edge, d,
                mapkeys={'edge_description':'description'})
        step.append(edge)

        if d['step_id'] not in [s._id for s in proc.steps]:
            proc.steps.append(step)

        self._copy_if_present(proc, d,
                keys=('ordered_by',),
                mapkeys={'process_description':'description'})
        self._copy_if_present(step, d,
                mapkeys={'step_description':'description'})


def read(fh, model_class=ihm.model.Model):
    """Read data from the mmCIF file handle `fh`.
    
       Note that the reader currently expects to see an mmCIF file compliant
       with the PDBx and/or IHM dictionaries. It is not particularly tolerant
       of noncompliant or incomplete files, and will probably throw an
       exception rather than warning about and trying to handle such files.
       Please `open an issue <https://github.com/ihmwg/python-ihm/issues>`_
       if you encounter such a problem.

       :param file fh: The file handle to read from.
       :param model_class: The class to use to store model coordinates.
              For use with other software, it is recommended to subclass
              :class:`ihm.model.Model` and override
              :meth:`~ihm.model.Model.add_sphere` and/or
              :meth:`~ihm.model.Model.add_atom`, and provide that subclass
              here. See :meth:`ihm.model.Model.get_spheres` for more
              information.
       :return: A list of :class:`ihm.System` objects.
    """
    systems = []

    while True:
        s = _SystemReader(model_class)
        handlers = [_StructHandler(s), _SoftwareHandler(s), _CitationHandler(s),
                    _CitationAuthorHandler(s), _ChemCompHandler(s),
                    _EntityHandler(s), _EntityPolySeqHandler(s),
                    _StructAsymHandler(s), _AssemblyDetailsHandler(s),
                    _AssemblyHandler(s), _ExtRefHandler(s), _ExtFileHandler(s),
                    _DatasetListHandler(s), _DatasetGroupHandler(s),
                    _DatasetExtRefHandler(s), _DatasetDBRefHandler(s),
                    _RelatedDatasetsHandler(s),
                    _ModelRepresentationHandler(s),
                    _StartingModelDetailsHandler(s),
                    _StartingComputationalModelsHandler(s),
                    _StartingComparativeModelsHandler(s),
                    _ProtocolHandler(s), _PostProcessHandler(s),
                    _ModelListHandler(s), _MultiStateHandler(s),
                    _EnsembleHandler(s), _DensityHandler(s),
                    _EM3DRestraintHandler(s), _EM2DRestraintHandler(s),
                    _EM2DFittingHandler(s), _SASRestraintHandler(s),
                    _SphereObjSiteHandler(s), _AtomSiteHandler(s),
                    _PolyResidueFeatureHandler(s), _PolyAtomFeatureHandler(s),
                    _DerivedDistanceRestraintHandler(s),
                    _CenterHandler(s), _TransformationHandler(s),
                    _GeometricObjectHandler(s), _SphereHandler(s),
                    _TorusHandler(s), _HalfTorusHandler(s),
                    _AxisHandler(s), _PlaneHandler(s),
                    _GeometricRestraintHandler(s), _PolySeqSchemeHandler(s),
                    _CrossLinkListHandler(s), _CrossLinkRestraintHandler(s),
                    _CrossLinkResultHandler(s),
                    _OrderedEnsembleHandler(s)]
        r = ihm.format.CifReader(fh, dict((h.category, h) for h in handlers))
        more_data = r.read_file()
        for h in handlers:
            h.finalize()
        systems.append(s.system)
        if not more_data:
            break

    return systems
