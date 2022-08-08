"""Utility classes to read in information in mmCIF or BinaryCIF format"""

import ihm.format
import ihm.format_bcif
import ihm.location
import ihm.dataset
import ihm.representation
import ihm.reference
import ihm.startmodel
import ihm.protocol
import ihm.analysis
import ihm.model
import ihm.restraint
import ihm.geometry
import ihm.source
import ihm.cross_linkers
import ihm.flr
import inspect
import warnings
import collections
try:
    from . import _format
except ImportError:
    _format = None


class OldFileError(Exception):
    """Exception raised if a file conforms to too old a version of the
       IHM extension dictionary. See :func:`read`."""
    pass


def _make_new_entity():
    """Make a new Entity object"""
    e = ihm.Entity([])
    # make sequence mutable (see also SystemReader.finalize)
    e.sequence = list(e.sequence)
    return e


def _get_vector3(d, key):
    """Return a 3D vector (as a list) from d[key+[1..3]]
       or leave as is if None or ihm.unknown"""
    if d[key + '1'] in (None, ihm.unknown):
        return d[key + '1']
    else:
        # Assume if one element is present, all are
        return [float(d[key + "%d" % k]) for k in (1, 2, 3)]


def _get_matrix33(d, key):
    """Return a 3x3 matrix (as a list of lists) from d[key+[1..3][1..3]]]
       or leave as is if None or ihm.unknown"""
    if d[key + '11'] in (None, ihm.unknown):
        return d[key + '11']
    else:
        # Assume if one element is present, all are
        return [[float(d[key + "%d%d" % (i, j)]) for j in (1, 2, 3)]
                for i in (1, 2, 3)]


class IDMapper(object):
    """Utility class to handle mapping from mmCIF IDs to Python objects.

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
           exist. If `newcls` is specified, the object will be an instance
           of that class (this is commonly used when different subclasses
           are employed depending on a type specified in the mmCIF file, such
           as the various subclasses of :class:`ihm.dataset.Dataset`)."""
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

    def get_by_id_or_none(self, objid, newcls=None):
        """Get the object with given ID, creating it if it doesn't already
           exist. If ID is None or ihm.unknown, return None instead."""
        return (None if objid in (None, ihm.unknown)
                else self.get_by_id(objid, newcls))


class _ChemCompIDMapper(IDMapper):
    """Add extra handling to IDMapper for the chem_comp category"""

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
            # Assign nonpolymer class based on the ID
            if newcls is ihm.NonPolymerChemComp or newcls is ihm.WaterChemComp:
                newcls = (ihm.WaterChemComp if objid == 'HOH'
                          else ihm.NonPolymerChemComp)
            return super(_ChemCompIDMapper, self).get_by_id(objid, newcls)

    def _make_new_object(self, newcls=None):
        if newcls is None:
            newcls = self._cls
        if newcls is ihm.NonPolymerChemComp:
            return newcls(None)
        elif newcls is ihm.WaterChemComp:
            return newcls()
        else:
            return newcls(*self._cls_args, **self._cls_keys)


class RangeIDMapper(object):
    """Utility class to handle mapping from mmCIF IDs to
       :class:`ihm.AsymUnitRange` or :class:`EntityRange` objects."""

    def __init__(self):
        self._id_map = {}

    def set(self, range_id, seq_id_begin, seq_id_end):
        """Add a range.

           :param str range_id: mmCIF ID
           :param int seq_id_begin: Index of the start of the range
           :param int seq_id_end: Index of the end of the range
        """
        self._id_map[range_id] = (seq_id_begin, seq_id_end)

    def get(self, asym_or_entity, range_id):
        """Get a range from an ID.

           :param asym_or_entity: An :class:`ihm.Entity` or
                  :class:`ihm.AsymUnit` object representing the part of
                  the system to which the range will be applied.
           :param str range_id: mmCIF ID
           :return: A range as a :class:`ihm.Entity`, :class:`ihm.AsymUnit`,
                    :class:`ihm.EntityRange` or :class:`ihm.AsymUnitRange`
                    object.
        """
        # range_id can be None if the entire asym/entity should be selected
        # (e.g. for a non-polymer)
        if range_id is None:
            return asym_or_entity
        else:
            return asym_or_entity(*self._id_map[range_id])


class _AnalysisIDMapper(IDMapper):
    """Add extra handling to IDMapper for the post processing category"""

    def _make_new_object(self, newcls=None):
        if newcls is None:
            newcls = self._cls
        if newcls is ihm.analysis.EmptyStep:
            return newcls()
        else:
            return newcls(*self._cls_args, **self._cls_keys)


class _FeatureIDMapper(IDMapper):
    """Add extra handling to IDMapper for restraint features"""

    def _make_new_object(self, newcls=None):
        if newcls is None:
            # Make Feature base class (takes no args)
            return self._cls()
        elif newcls is ihm.restraint.PseudoSiteFeature:
            # Pseudo site constructor needs "site" argument
            return newcls(None)
        else:
            # Make subclass (takes one ranges/atoms argument)
            return newcls([])

    def _update_old_object(self, obj, newcls=None):
        super(_FeatureIDMapper, self)._update_old_object(obj, newcls)
        # Add missing members if the base class was originally instantianted
        if (newcls is ihm.restraint.ResidueFeature
                and not hasattr(obj, 'ranges')):
            obj.ranges = []
        elif (newcls is ihm.restraint.AtomFeature
              and not hasattr(obj, 'atoms')):
            obj.atoms = []
        elif (newcls is ihm.restraint.NonPolyFeature
              and not hasattr(obj, 'objs')):
            obj.objs = []
        elif (newcls is ihm.restraint.PseudoSiteFeature
              and not hasattr(obj, 'site')):
            obj.site = None


class _GeometryIDMapper(IDMapper):
    """Add extra handling to IDMapper for geometric objects"""

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
            return newcls(*(None,) * len_args)

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


class _CrossLinkIDMapper(IDMapper):
    """Add extra handling to IDMapper for cross links"""

    def _make_new_object(self, newcls=None):
        if newcls is None:
            # Make base class (takes no args)
            obj = self._cls()
            # Need fits in case we never decide on a type
            obj.fits = {}
            return obj
        elif newcls is ihm.restraint.AtomCrossLink:
            return newcls(*(None,) * 6)
        else:
            return newcls(*(None,) * 4)


class _ReferenceIDMapper(IDMapper):
    """Add extra handling to IDMapper for ihm.reference.Reference objects"""

    def _make_new_object(self, newcls=None):
        if newcls is None or newcls is ihm.reference.Sequence:
            return self._cls(*(None,) * 4)
        else:
            return newcls(*(None,) * 3)


class _FLRListAdapter(object):
    """Take objects from IDMapper and place them in objects in FLRData."""

    def __init__(self, collection_dict, collection_list, flr_data):
        self.collection_dict, self.flr_data = collection_dict, flr_data
        self.collection_list = collection_list

    def append(self, obj):
        # We generally only have a single FLRData object, id=1
        d = self.flr_data.get_by_id(1)

        # Store in collection dict (by ID) in FLRData rather than a
        # list in System
        collection_dict = getattr(d, self.collection_dict)
        collection_dict[obj._id] = obj
        # Also store in list in FLRData if applicable
        if self.collection_list is not None:
            collection_list = getattr(d, self.collection_list)
            collection_list.append(obj)


class _FLRIDMapper(IDMapper):
    """Handle mapping from mmCIF IDs to FLR Python objects.
       This differs from the base IDMapper class in that created objects
       are stored in the FLRData object, not in the System."""

    def __init__(self, collection_dict, collection_list, flr_data, cls,
                 *args, **keys):
        system_list = _FLRListAdapter(collection_dict, collection_list,
                                      flr_data)
        super(_FLRIDMapper, self).__init__(system_list, cls, *args, **keys)


class _DatasetIDMapper(object):
    """Handle mapping from mmCIF dataset IDs to Python objects.

       This is similar to IDMapper but is intended for objects like restraints
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

    def get_by_attrs(self, dataset, linker):
        """Group all crosslinks with same dataset and linker in one
           CrossLinkRestraint object"""
        k = (dataset._id, linker)
        if k not in self._seen_rsrs:
            r = ihm.restraint.CrossLinkRestraint(dataset, linker)
            self.system_list.append(r)
            self._seen_rsrs[k] = r
        return self._seen_rsrs[k]

    def get_all(self):
        """Yield all objects seen so far (unordered)"""
        return self._seen_rsrs.values()


class SystemReader(object):
    """Utility class to track global information for a :class:`ihm.System`
       being read from a file, such as the mapping from IDs to objects
       (as :class:`IDMapper` objects). This can be used by :class:`Handler`
       subclasses."""
    def __init__(self, model_class, starting_model_class):
        #: The :class:`ihm.System` object being read in
        self.system = ihm.System()

        #: Mapping from ID to :class:`ihm.Software` objects
        self.software = IDMapper(self.system.software, ihm.Software,
                                 *(None,) * 4)

        #: Mapping from ID to :class:`ihm.Citation` objects
        self.citations = IDMapper(self.system.citations, ihm.Citation,
                                  *(None,) * 8)

        #: Mapping from ID to :class:`ihm.Entity` objects
        self.entities = IDMapper(self.system.entities, _make_new_entity)

        #: Mapping from ID to :class:`ihm.source.Manipulated` objects
        self.src_gens = IDMapper(None, ihm.source.Manipulated)

        #: Mapping from ID to :class:`ihm.source.Natural` objects
        self.src_nats = IDMapper(None, ihm.source.Natural)

        #: Mapping from ID to :class:`ihm.source.Synthetic` objects
        self.src_syns = IDMapper(None, ihm.source.Synthetic)

        #: Mapping from ID to :class:`ihm.AsymUnit` objects
        self.asym_units = IDMapper(self.system.asym_units, ihm.AsymUnit, None)

        #: Mapping from ID to :class:`ihm.ChemComp` objects
        self.chem_comps = _ChemCompIDMapper(None, ihm.ChemComp, *(None,) * 3)

        #: Mapping from ID to :class:`ihm.reference.Alignment` objects
        self.alignments = IDMapper(None, ihm.reference.Alignment)

        #: Mapping from ID to :class:`ihm.reference.Reference` objects
        self.references = _ReferenceIDMapper(None, ihm.reference.Sequence)

        #: Mapping from ID to :class:`ihm.ChemDescriptor` objects
        self.chem_descriptors = IDMapper(self.system.orphan_chem_descriptors,
                                         ihm.ChemDescriptor, None)

        #: Mapping from ID to :class:`ihm.Assembly` objects
        self.assemblies = IDMapper(self.system.orphan_assemblies, ihm.Assembly)

        #: Mapping from ID to :class:`ihm.AsymUnitRange`
        #: or :class:`EntityRange` objects
        self.ranges = RangeIDMapper()

        #: Mapping from ID to :class:`ihm.location.Repository` objects
        self.repos = IDMapper(None, ihm.location.Repository, None)

        #: Mapping from ID to :class:`ihm.location.FileLocation` objects
        self.external_files = IDMapper(self.system.locations,
                                       ihm.location.FileLocation,
                                       '/')  # should always exist?

        #: Mapping from ID to :class:`ihm.location.DatabaseLocation` objects
        self.db_locations = IDMapper(self.system.locations,
                                     ihm.location.DatabaseLocation, None, None)

        #: Mapping from ID to :class:`ihm.dataset.Dataset` objects
        self.datasets = IDMapper(self.system.orphan_datasets,
                                 ihm.dataset.Dataset, None)

        #: Mapping from ID to :class:`ihm.dataset.DatasetGroup` objects
        self.dataset_groups = IDMapper(self.system.orphan_dataset_groups,
                                       ihm.dataset.DatasetGroup)

        #: Mapping from ID to :class:`ihm.startmodel.StartingModel` objects
        self.starting_models = IDMapper(self.system.orphan_starting_models,
                                        starting_model_class, *(None,) * 3)

        #: Mapping from ID to :class:`ihm.representation.Representation`
        #: objects
        self.representations = IDMapper(self.system.orphan_representations,
                                        ihm.representation.Representation)

        #: Mapping from ID to :class:`ihm.protocol.Protocol` objects
        self.protocols = IDMapper(self.system.orphan_protocols,
                                  ihm.protocol.Protocol)

        #: Mapping from ID to :class:`ihm.analysis.Step` objects
        self.analysis_steps = _AnalysisIDMapper(None, ihm.analysis.Step,
                                                *(None,) * 3)

        #: Mapping from ID to :class:`ihm.analysis.Analysis` objects
        self.analyses = IDMapper(None, ihm.analysis.Analysis)

        #: Mapping from ID to :class:`ihm.model.Model` objects
        self.models = IDMapper(None, model_class, *(None,) * 3)

        #: Mapping from ID to :class:`ihm.model.ModelGroup` objects
        self.model_groups = IDMapper(None, ihm.model.ModelGroup)

        #: Mapping from ID to :class:`ihm.model.State` objects
        self.states = IDMapper(None, ihm.model.State)

        #: Mapping from ID to :class:`ihm.model.StateGroup` objects
        self.state_groups = IDMapper(self.system.state_groups,
                                     ihm.model.StateGroup)

        #: Mapping from ID to :class:`ihm.model.Ensemble` objects
        self.ensembles = IDMapper(self.system.ensembles,
                                  ihm.model.Ensemble, *(None,) * 2)

        #: Mapping from ID to :class:`ihm.model.LocalizationDensity` objects
        self.densities = IDMapper(None,
                                  ihm.model.LocalizationDensity, *(None,) * 2)

        #: Mapping from ID to :class:`ihm.restraint.EM3DRestraint` objects
        self.em3d_restraints = _DatasetIDMapper(self.system.restraints,
                                                self.datasets,
                                                ihm.restraint.EM3DRestraint,
                                                None)

        #: Mapping from ID to :class:`ihm.restraint.EM2DRestraint` objects
        self.em2d_restraints = IDMapper(self.system.restraints,
                                        ihm.restraint.EM2DRestraint,
                                        *(None,) * 2)

        #: Mapping from ID to :class:`ihm.restraint.SASRestraint` objects
        self.sas_restraints = _DatasetIDMapper(self.system.restraints,
                                               self.datasets,
                                               ihm.restraint.SASRestraint,
                                               None)

        #: Mapping from ID to :class:`ihm.restraint.Feature` objects
        self.features = _FeatureIDMapper(self.system.orphan_features,
                                         ihm.restraint.Feature)

        #: Mapping from ID to :class:`ihm.restraint.PseudoSite` objects
        self.pseudo_sites = IDMapper(self.system.orphan_pseudo_sites,
                                     ihm.restraint.PseudoSite, *(None,) * 3)

        #: Mapping from ID to :class:`ihm.restraint.DerivedDistanceRestraint`
        #: objects
        self.dist_restraints = IDMapper(
            self.system.restraints, ihm.restraint.DerivedDistanceRestraint,
            *(None,) * 4)

        #: Mapping from ID to :class:`ihm.restraint.PredictedContactRestraint`
        #: objects
        self.pred_cont_restraints = IDMapper(
            self.system.restraints, ihm.restraint.PredictedContactRestraint,
            *(None,) * 5)

        #: Mapping from ID to :class:`ihm.restraint.RestraintGroup` of
        #: :class:`ihm.restraint.DerivedDistanceRestraint` objects
        self.dist_restraint_groups = IDMapper(
            self.system.restraint_groups, ihm.restraint.RestraintGroup)

        #: Mapping from ID to :class:`ihm.restraint.RestraintGroup` of
        #: :class:`ihm.restraint.PredictedContactRestraint` objects
        self.pred_cont_restraint_groups = IDMapper(
            self.system.restraint_groups, ihm.restraint.RestraintGroup)

        #: Mapping from ID to :class:`ihm.geometry.GeometricObject` objects
        self.geometries = _GeometryIDMapper(
            self.system.orphan_geometric_objects, ihm.geometry.GeometricObject)

        #: Mapping from ID to :class:`ihm.geometry.Center` objects
        self.centers = IDMapper(None, ihm.geometry.Center, *(None,) * 3)

        #: Mapping from ID to :class:`ihm.geometry.Transformation` objects
        self.transformations = IDMapper(None, ihm.geometry.Transformation,
                                        *(None,) * 2)

        #: Mapping from ID to :class:`ihm.geometry.Transformation` objects
        #: used by :class:`ihm.dataset.TransformedDataset` objects (this is
        #: distinct from :attr:`transformations` since they are stored in
        #: separate tables, with different IDs, in the mmCIF file).
        self.data_transformations = IDMapper(
            None, ihm.geometry.Transformation, *(None,) * 2)

        #: Mapping from ID to :class:`ihm.restraint.GeometricRestraint` objects
        self.geom_restraints = IDMapper(
            self.system.restraints, ihm.restraint.GeometricRestraint,
            *(None,) * 4)

        #: Mapping from ID to :class:`ihm.restraint.CrossLinkRestraint` objects
        self.xl_restraints = _XLRestraintMapper(self.system.restraints)

        #: Mapping from ID to groups of
        #: :class:`ihm.restraint.ExperimentalCrossLink` objects
        self.experimental_xl_groups = IDMapper(None, list)
        self.experimental_xl_groups.id_attr = None

        #: Mapping from ID to :class:`ihm.restraint.ExperimentalCrossLink`
        #: objects
        self.experimental_xls = IDMapper(
            None, ihm.restraint.ExperimentalCrossLink, *(None,) * 2)

        #: Mapping from ID to :class:`ihm.restraint.CrossLink`
        self.cross_links = _CrossLinkIDMapper(
            None, ihm.restraint.CrossLink)

        #: Mapping from ID to :class:`ihm.restraint.CrossLinkPseudoSite`
        self.cross_link_pseudo_sites = IDMapper(
            None, ihm.restraint.CrossLinkPseudoSite, None)

        #: Mapping from ID to :class:`ihm.model.OrderedProcess` objects
        self.ordered_procs = IDMapper(self.system.ordered_processes,
                                      ihm.model.OrderedProcess, None)

        #: Mapping from ID to :class:`ihm.model.ProcessStep` objects
        self.ordered_steps = IDMapper(None, ihm.model.ProcessStep)

        # FLR part

        #: Mapping from ID to :class:`ihm.flr.FLRData` objects
        self.flr_data = IDMapper(self.system.flr_data, ihm.flr.FLRData)

        #: Mapping from ID to :class:`ihm.flr.InstSetting` objects
        self.flr_inst_settings = _FLRIDMapper('_collection_flr_inst_setting',
                                              None, self.flr_data,
                                              ihm.flr.InstSetting)

        #: Mapping from ID to :class:`ihm.flr.ExpCondition` objects
        self.flr_exp_conditions = _FLRIDMapper('_collection_flr_exp_condition',
                                               None, self.flr_data,
                                               ihm.flr.ExpCondition)

        #: Mapping from ID to :class:`ihm.flr.Instrument` objects
        self.flr_instruments = _FLRIDMapper('_collection_flr_instrument',
                                            None, self.flr_data,
                                            ihm.flr.Instrument)

        #: Mapping from ID to :class:`ihm.flr.EntityAssembly` objects
        self.flr_entity_assemblies = _FLRIDMapper(
            '_collection_flr_entity_assembly', None, self.flr_data,
            ihm.flr.EntityAssembly)

        #: Mapping from ID to :class:`ihm.flr.SampleCondition` objects
        self.flr_sample_conditions = _FLRIDMapper(
            '_collection_flr_sample_condition', None, self.flr_data,
            ihm.flr.SampleCondition)

        #: Mapping from ID to :class:`ihm.flr.Sample` objects
        self.flr_samples = _FLRIDMapper('_collection_flr_sample', None,
                                        self.flr_data, ihm.flr.Sample,
                                        *(None,) * 6)

        #: Mapping from ID to :class:`ihm.flr.Experiment` objects
        self.flr_experiments = _FLRIDMapper('_collection_flr_experiment', None,
                                            self.flr_data, ihm.flr.Experiment)

        #: Mapping from ID to :class:`ihm.flr.Probe` objects
        self.flr_probes = _FLRIDMapper('_collection_flr_probe', None,
                                       self.flr_data, ihm.flr.Probe)

        #: Mapping from ID to :class:`ihm.flr.PolyProbePosition` objects
        self.flr_poly_probe_positions = _FLRIDMapper(
            '_collection_flr_poly_probe_position', None, self.flr_data,
            ihm.flr.PolyProbePosition, None)

        #: Mapping from ID to :class:`ihm.flr.SampleProbeDetails` objects
        self.flr_sample_probe_details = _FLRIDMapper(
            '_collection_flr_sample_probe_details', None, self.flr_data,
            ihm.flr.SampleProbeDetails, *(None,) * 5)

        #: Mapping from ID to :class:`ihm.flr.PolyProbeConjugate` objects
        self.flr_poly_probe_conjugates = _FLRIDMapper(
            '_collection_flr_poly_probe_conjugate', 'poly_probe_conjugates',
            self.flr_data, ihm.flr.PolyProbeConjugate, *(None,) * 4)

        #: Mapping from ID to :class:`ihm.flr.FRETForsterRadius` objects
        self.flr_fret_forster_radius = _FLRIDMapper(
            '_collection_flr_fret_forster_radius', None, self.flr_data,
            ihm.flr.FRETForsterRadius, *(None,) * 4)

        #: Mapping from ID to :class:`ihm.flr.FRETCalibrationParameters`
        #: objects
        self.flr_fret_calibration_parameters = _FLRIDMapper(
            '_collection_flr_fret_calibration_parameters', None, self.flr_data,
            ihm.flr.FRETCalibrationParameters, *(None,) * 8)

        #: Mapping from ID to :class:`ihm.flr.FRETAnalysis` objects
        self.flr_fret_analyses = _FLRIDMapper(
            '_collection_flr_fret_analysis', None, self.flr_data,
            ihm.flr.FRETAnalysis, *(None,) * 9)

        #: Mapping from ID to :class:`ihm.flr.LifetimeFitModel` objects
        self.flr_lifetime_fit_models = _FLRIDMapper(
            '_collection_flr_lifetime_fit_model', None, self.flr_data,
            ihm.flr.LifetimeFitModel, *(None,) * 4)

        #: Mapping from ID to :class:`ihm.flr.RefMeasurementGroup` objects
        self.flr_ref_measurement_groups = _FLRIDMapper(
            '_collection_flr_ref_measurement_group', None, self.flr_data,
            ihm.flr.RefMeasurementGroup, *(None,))

        #: Mapping from ID to :class:`ihm.flr.RefMeasurement` objects
        self.flr_ref_measurements = _FLRIDMapper(
            '_collection_flr_ref_measurement', None, self.flr_data,
            ihm.flr.RefMeasurement, *(None,) * 3)

        #: Mapping from ID to :class:`ihm.flr.RefMeasurementLifetime` objects
        self.flr_ref_measurement_lifetimes = _FLRIDMapper(
            '_collection_flr_ref_measurement_lifetime', None, self.flr_data,
            ihm.flr.RefMeasurementLifetime, *(None,) * 3)

        #: Mapping from ID to :class:`ihm.flr.PeakAssignment` objects
        self.flr_peak_assignments = _FLRIDMapper(
            '_collection_flr_peak_assignment', None,
            self.flr_data, ihm.flr.PeakAssignment, *(None,) * 2)

        #: Mapping from ID to :class:`ihm.flr.FRETDistanceRestraint` objects
        self.flr_fret_distance_restraints = _FLRIDMapper(
            '_collection_flr_fret_distance_restraint', None,
            self.flr_data, ihm.flr.FRETDistanceRestraint, *(None,) * 10)

        #: Mapping from ID to :class:`ihm.flr.FRETDistanceRestraintGroup`
        #: objects
        self.flr_fret_distance_restraint_groups = _FLRIDMapper(
            '_collection_flr_fret_distance_restraint_group',
            'distance_restraint_groups', self.flr_data,
            ihm.flr.FRETDistanceRestraintGroup)

        #: Mapping from ID to :class:`ihm.flr.FRETModelQuality` objects
        self.flr_fret_model_qualities = _FLRIDMapper(
            '_collection_flr_fret_model_quality', 'fret_model_qualities',
            self.flr_data, ihm.flr.FRETModelQuality, *(None,) * 5)

        #: Mapping from ID to :class:`ihm.flr.FRETModelDistance` objects
        self.flr_fret_model_distances = _FLRIDMapper(
            '_collection_flr_fret_model_distance', 'fret_model_distances',
            self.flr_data, ihm.flr.FRETModelDistance, *(None,) * 4)

        #: Mapping from ID to :class:`ihm.flr.FPSModeling` objects
        self.flr_fps_modeling = _FLRIDMapper(
            '_collection_flr_fps_modeling', None, self.flr_data,
            ihm.flr.FPSModeling, *(None,) * 5)

        #: Mapping from ID to :class:`ihm.flr.FPSGlobalParameters` objects
        self.flr_fps_global_parameters = _FLRIDMapper(
            '_collection_flr_fps_global_parameters', None,
            self.flr_data, ihm.flr.FPSGlobalParameters, *(None,) * 20)

        #: Mapping from ID to :class:`ihm.flr.FPSAVParameter` objects
        self.flr_fps_av_parameters = _FLRIDMapper(
            '_collection_flr_fps_av_parameter', None,
            self.flr_data, ihm.flr.FPSAVParameter, *(None,) * 6)

        #: Mapping from ID to :class:`ihm.flr.FPSAVModeling` objects
        self.flr_fps_av_modeling = _FLRIDMapper(
            '_collection_flr_fps_av_modeling', 'fps_modeling',
            self.flr_data, ihm.flr.FPSAVModeling, *(None,) * 3)

        #: Mapping from ID to :class:`ihm.flr.FPSMeanProbePosition` objects
        self.flr_fps_mean_probe_positions = _FLRIDMapper(
            '_collection_flr_fps_mean_probe_position', None,
            self.flr_data, ihm.flr.FPSMeanProbePosition, *(None,) * 4)

        #: Mapping from ID to :class:`ihm.flr.FPSMPPAtomPositionGroup` objects
        self.flr_fps_mpp_atom_position_groups = IDMapper(
            None, ihm.flr.FPSMPPAtomPositionGroup)

        #: Mapping from ID to :class:`ihm.flr.FPSMPPAtomPosition` objects
        self.flr_fps_mpp_atom_positions = _FLRIDMapper(
            '_collection_flr_fps_mpp_atom_position', None,
            self.flr_data, ihm.flr.FPSMPPAtomPosition, *(None,) * 4)

        #: Mapping from ID to :class:`ihm.flr.FPSMPPModeling` objects
        self.flr_fps_mpp_modeling = _FLRIDMapper(
            '_collection_flr_fps_mpp_modeling', 'fps_modeling',
            self.flr_data, ihm.flr.FPSMPPModeling, *(None,) * 3)

    def finalize(self):
        # make sequence immutable (see also _make_new_entity)
        for e in self.system.entities:
            e.sequence = tuple(e.sequence)


class Handler(object):
    """Base class for all handlers of mmCIF data.
       Each class handles a single category in the mmCIF or BinaryCIF file.
       To add a new handler (for example to handle a custom category)
       make a subclass and set the class attribute
       `category` to the mmCIF category name (e.g. `_struct`). Provide
       a `__call__` method. This will be called for each category (multiple
       times for loop constructs) with the parameters to `__call__` filled in
       with the same-named mmCIF keywords. For example the class::

           class CustomHandler(Handler):
               category = "_custom"
               def __call__(self, key1, key2):
                   pass

       will be called with arguments `"x", "y"` when given the mmCIF input::

           _custom.key1 x
           _custom.key2 y

       Note that the arguments will always be strings when reading an mmCIF
       file. To convert to integer, floating point, or boolean, use the utility
       methods :meth:`get_int`, :meth:`get_float` or :meth:`get_bool`
       respectively.
       """

    #: Value passed to `__call__` for keywords not in the file
    not_in_file = None

    #: Value passed to `__call__` for data marked as omitted ('.') in the file
    omitted = None

    #: Value passed to `__call__` for data marked as unknown ('?') in the file
    unknown = ihm.unknown

    #: Keywords which are explicitly ignored (read() will not warn about their
    #: presence in the file). These are usually things like ordinal fields
    #: which we don't use.
    ignored_keywords = []

    def __init__(self, sysr):
        #: Utility class to map IDs to Python objects.
        self.sysr = sysr

    def get_int(self, val):
        """Return int(val) or leave as is if None or ihm.unknown"""
        return int(val) if val is not None and val is not ihm.unknown else val

    def get_int_or_string(self, val):
        """Return val as an int or str as appropriate,
           or leave as is if None or ihm.unknown"""
        if val is None or val is ihm.unknown:
            return val
        else:
            return int(val) if isinstance(val, int) or val.isdigit() else val

    def get_float(self, val):
        """Return float(val) or leave as is if None or ihm.unknown"""
        return (float(val) if val is not None
                and val is not ihm.unknown else val)

    _boolmap = {'YES': True, 'NO': False}

    def get_bool(self, val):
        """Convert val to bool and return, or leave as is if None
           or ihm.unknown"""
        return (self._boolmap.get(val.upper(), None)
                if val is not None and val is not ihm.unknown else val)

    def get_lower(self, val):
        """Return lowercase string val or leave as is if None or ihm.unknown"""
        return (val.lower()
                if val is not None and val is not ihm.unknown else val)

    def finalize(self):
        """Called at the end of each data block."""
        pass

    def end_save_frame(self):
        """Called at the end of each save frame."""
        pass

    def _get_asym_or_entity(self, asym_id, entity_id):
        """Return an :class:`AsymUnit`, or an :class:`Entity`
           if asym_id is omitted"""
        asym = self.sysr.asym_units.get_by_id_or_none(asym_id)
        return asym if asym else self.sysr.entities.get_by_id(entity_id)

    def copy_if_present(self, obj, data, keys=[], mapkeys={}):
        """Set obj.x from data['x'] for each x in keys if present in data.
           The dict mapkeys is handled similarly except that its keys are
           looked up in data and the corresponding value used to set obj."""
        for key in keys:
            d = data.get(key)
            if d is not None:
                setattr(obj, key, d)
        for key, val in mapkeys.items():
            d = data.get(key)
            if d is not None:
                setattr(obj, val, d)

    system = property(lambda self: self.sysr.system,
                      doc="The :class:`ihm.System` object to read into")


class _StructHandler(Handler):
    category = '_struct'

    def __call__(self, title, entry_id, pdbx_model_details):
        self.copy_if_present(self.system, locals(), keys=('title',),
                             mapkeys={'entry_id': 'id',
                                      'pdbx_model_details': 'model_details'})


class _AuditConformHandler(Handler):
    category = '_audit_conform'

    def __call__(self, dict_name, dict_version):
        # Reject old file versions if we can parse the version
        if dict_name == 'ihm-extension.dic':
            try:
                major, minor = [int(x) for x in dict_version.split('.')]
                if (major, minor) < (1, 0):
                    raise OldFileError(
                        "This version of python-ihm only supports reading "
                        "files that conform to version 1.0 or later of the "
                        "IHM extension dictionary. This file conforms to "
                        "version %s." % dict_version)
            except ValueError:
                pass


class _SoftwareHandler(Handler):
    category = '_software'

    def __call__(self, pdbx_ordinal, name, classification, description,
                 version, type, location, citation_id):
        s = self.sysr.software.get_by_id(pdbx_ordinal)
        self.copy_if_present(
            s, locals(),
            keys=('name', 'classification', 'description', 'version',
                  'type', 'location'))
        s.citation = self.sysr.citations.get_by_id_or_none(citation_id)


class _CitationHandler(Handler):
    category = '_citation'

    def __call__(self, id, title, year, pdbx_database_id_pubmed,
                 journal_abbrev, journal_volume, pdbx_database_id_doi,
                 page_first, page_last):
        s = self.sysr.citations.get_by_id(id)
        s.is_primary = (id == 'primary')
        self.copy_if_present(
            s, locals(), keys=('title', 'year'),
            mapkeys={'pdbx_database_id_pubmed': 'pmid',
                     'journal_abbrev': 'journal',
                     'journal_volume': 'volume',
                     'pdbx_database_id_doi': 'doi'})
        if page_first is not None:
            if page_last is not None:
                s.page_range = (page_first, page_last)
            else:
                s.page_range = page_first


class _AuditAuthorHandler(Handler):
    category = '_audit_author'
    ignored_keywords = ['pdbx_ordinal']

    def __call__(self, name):
        self.system.authors.append(name)


class _GrantHandler(Handler):
    category = '_pdbx_audit_support'

    def __call__(self, funding_organization, country, grant_number):
        g = ihm.Grant(funding_organization=funding_organization,
                      country=country, grant_number=grant_number)
        self.system.grants.append(g)


class _CitationAuthorHandler(Handler):
    category = '_citation_author'
    ignored_keywords = ['ordinal']

    def __call__(self, citation_id, name):
        s = self.sysr.citations.get_by_id(citation_id)
        if name is not None:
            s.authors.append(name)


class _ChemCompHandler(Handler):
    category = '_chem_comp'

    def __init__(self, *args):
        super(_ChemCompHandler, self).__init__(*args)
        # Map _chem_comp.type to corresponding subclass of ihm.ChemComp
        self.type_map = dict((x[1].type.lower(), x[1])
                             for x in inspect.getmembers(ihm, inspect.isclass)
                             if issubclass(x[1], ihm.ChemComp))

    def __call__(self, type, id, name, formula):
        typ = 'other' if type is None else type.lower()
        s = self.sysr.chem_comps.get_by_id(
            id, self.type_map.get(typ, ihm.ChemComp))
        self.copy_if_present(s, locals(), keys=('name', 'formula'))


class _ChemDescriptorHandler(Handler):
    category = '_ihm_chemical_component_descriptor'

    def __call__(self, id, auth_name, chemical_name, common_name,
                 smiles, smiles_canonical, inchi, inchi_key):
        d = self.sysr.chem_descriptors.get_by_id(id)
        self.copy_if_present(
            d, locals(),
            keys=('auth_name', 'chemical_name',
                  'common_name', 'smiles', 'smiles_canonical', 'inchi',
                  'inchi_key'))


class _EntityHandler(Handler):
    category = '_entity'

    def __init__(self, *args):
        super(_EntityHandler, self).__init__(*args)
        self.src_map = dict(
            (x[1].src_method.lower(), x[1])
            for x in inspect.getmembers(ihm.source, inspect.isclass)
            if issubclass(x[1], ihm.source.Source)
            and x[1] is not ihm.source.Source)

    def __call__(self, id, details, type, src_method, formula_weight,
                 pdbx_description, pdbx_number_of_molecules):
        s = self.sysr.entities.get_by_id(id)
        self.copy_if_present(
            s, locals(), keys=('details',),
            mapkeys={'pdbx_description': 'description',
                     'pdbx_number_of_molecules': 'number_of_molecules'})
        if src_method:
            source_cls = self.src_map.get(src_method.lower(), None)
            if source_cls and s.source is None:
                s.source = source_cls()
        # Force polymer if _entity.type says so, even if it doesn't look like
        # one (e.g. just a single amino acid)
        if type and type.lower() == 'polymer':
            s._force_polymer = True


class _EntitySrcNatHandler(Handler):
    category = '_entity_src_nat'

    def __call__(self, entity_id, pdbx_src_id, pdbx_ncbi_taxonomy_id,
                 pdbx_organism_scientific, common_name, strain):
        e = self.sysr.entities.get_by_id(entity_id)
        s = self.sysr.src_nats.get_by_id(pdbx_src_id)
        s.ncbi_taxonomy_id = pdbx_ncbi_taxonomy_id
        s.scientific_name = pdbx_organism_scientific
        s.common_name = common_name
        s.strain = strain
        e.source = s


class _EntitySrcSynHandler(Handler):
    category = '_pdbx_entity_src_syn'

    # Note that _pdbx_entity_src_syn.strain is not used in current PDB entries
    def __call__(self, entity_id, pdbx_src_id, ncbi_taxonomy_id,
                 organism_scientific, organism_common_name):
        e = self.sysr.entities.get_by_id(entity_id)
        s = self.sysr.src_syns.get_by_id(pdbx_src_id)
        s.ncbi_taxonomy_id = ncbi_taxonomy_id
        s.scientific_name = organism_scientific
        s.common_name = organism_common_name
        e.source = s


class _StructRefHandler(Handler):
    category = '_struct_ref'

    def __init__(self, *args):
        super(_StructRefHandler, self).__init__(*args)
        # Map db_name to subclass of ihm.reference.Sequence
        self.type_map = dict(
            (x[1]._db_name.lower(), x[1])
            for x in inspect.getmembers(ihm.reference, inspect.isclass)
            if issubclass(x[1], ihm.reference.Sequence)
            and x[1] is not ihm.reference.Sequence)

    def __call__(self, id, entity_id, db_name, db_code, pdbx_db_accession,
                 pdbx_seq_one_letter_code, details):
        # todo: handle things that aren't sequences
        e = self.sysr.entities.get_by_id(entity_id)
        typ = self.type_map.get(db_name.lower())
        ref = self.sysr.references.get_by_id(id, typ)
        # Strip newlines and whitespace from code
        if pdbx_seq_one_letter_code not in (None, ihm.unknown):
            pdbx_seq_one_letter_code \
                = pdbx_seq_one_letter_code.replace('\n', '').replace(' ', '')
        self.copy_if_present(
            ref, locals(), keys=('db_name', 'db_code', 'details'),
            mapkeys={'pdbx_db_accession': 'accession',
                     'pdbx_seq_one_letter_code': '_partial_sequence'})
        e.references.append(ref)

    def finalize(self):
        # The mmCIF file only contains the subset of the sequence that
        # overlaps with our entities, but we need the full sequence. Pad it
        # out with gaps if necessary so that indexing works correctly.
        for e in self.system.entities:
            for r in e.references:
                if hasattr(r, '_partial_sequence'):
                    if r._partial_sequence in (None, ihm.unknown):
                        r.sequence = r._partial_sequence
                    else:
                        db_begin = min(a.db_begin for a in r._get_alignments())
                        r.sequence = '-' * (db_begin - 1) + r._partial_sequence
                    del r._partial_sequence


class _StructRefSeqHandler(Handler):
    category = '_struct_ref_seq'

    def __call__(self, align_id, ref_id, seq_align_beg, seq_align_end,
                 db_align_beg, db_align_end):
        ref = self.sysr.references.get_by_id(ref_id)
        align = self.sysr.alignments.get_by_id(align_id)
        align.db_begin = self.get_int(db_align_beg)
        align.db_end = self.get_int(db_align_end)
        align.entity_begin = self.get_int(seq_align_beg)
        align.entity_end = self.get_int(seq_align_end)
        ref.alignments.append(align)


class _StructRefSeqDifHandler(Handler):
    category = '_struct_ref_seq_dif'

    def __call__(self, align_id, seq_num, db_mon_id, mon_id, details):
        align = self.sysr.alignments.get_by_id(align_id)
        db_monomer = self.sysr.chem_comps.get_by_id_or_none(db_mon_id)
        monomer = self.sysr.chem_comps.get_by_id(mon_id)
        sd = ihm.reference.SeqDif(self.get_int(seq_num), db_monomer, monomer,
                                  details)
        align.seq_dif.append(sd)


class _EntitySrcGenHandler(Handler):
    category = '_entity_src_gen'

    def __call__(self, entity_id, pdbx_src_id, pdbx_gene_src_ncbi_taxonomy_id,
                 pdbx_gene_src_scientific_name, gene_src_common_name,
                 gene_src_strain, pdbx_host_org_ncbi_taxonomy_id,
                 pdbx_host_org_scientific_name, host_org_common_name,
                 pdbx_host_org_strain):
        e = self.sysr.entities.get_by_id(entity_id)
        s = self.sysr.src_gens.get_by_id(pdbx_src_id)
        s.gene = ihm.source.Details(
            ncbi_taxonomy_id=pdbx_gene_src_ncbi_taxonomy_id,
            scientific_name=pdbx_gene_src_scientific_name,
            common_name=gene_src_common_name, strain=gene_src_strain)
        s.host = ihm.source.Details(
            ncbi_taxonomy_id=pdbx_host_org_ncbi_taxonomy_id,
            scientific_name=pdbx_host_org_scientific_name,
            common_name=host_org_common_name,
            strain=pdbx_host_org_strain)
        e.source = s


class _EntityPolySeqHandler(Handler):
    category = '_entity_poly_seq'

    def __call__(self, entity_id, num, mon_id):
        s = self.sysr.entities.get_by_id(entity_id)
        seq_id = int(num)
        if seq_id > len(s.sequence):
            s.sequence.extend([None] * (seq_id - len(s.sequence)))
        s.sequence[seq_id - 1] = self.sysr.chem_comps.get_by_id(mon_id)


class _EntityPolyHandler(Handler):
    category = '_entity_poly'

    def __init__(self, *args):
        super(_EntityPolyHandler, self).__init__(*args)
        self._entity_info = {}

    def _get_codes(self, codestr):
        """Convert a one-letter-code string into a sequence of individual
           codes"""
        if codestr is None:
            return
        i = 0
        while i < len(codestr):
            # Strip out linebreaks
            if codestr[i] == '\n':
                pass
            elif codestr[i] == '(':
                end = codestr.index(')', i)
                yield codestr[i + 1:end]
                i = end
            else:
                yield codestr[i]
            i += 1

    def __call__(self, entity_id, type, pdbx_seq_one_letter_code,
                 pdbx_seq_one_letter_code_can):
        class EntityInfo(object):
            pass
        e = EntityInfo()
        e.one_letter = tuple(self._get_codes(pdbx_seq_one_letter_code))
        e.one_letter_can = tuple(self._get_codes(pdbx_seq_one_letter_code_can))
        e.sequence_type = type
        self._entity_info[entity_id] = e

    def finalize(self):
        for e in self.system.entities:
            ei = self._entity_info.get(e._id, None)
            if ei is None:
                continue
            # Fill in missing information (one-letter codes) for nonstandard
            # residues
            # todo: also add info for residues that aren't in entity_poly_seq
            # at all
            for i, comp in enumerate(e.sequence):
                if comp.code is None and i < len(ei.one_letter):
                    comp.code = ei.one_letter[i]
                if (comp.code_canonical is None
                        and i < len(ei.one_letter_can)):
                    comp.code_canonical = ei.one_letter_can[i]


class _EntityPolySegmentHandler(Handler):
    category = '_ihm_entity_poly_segment'

    def __call__(self, id, seq_id_begin, seq_id_end):
        self.sysr.ranges.set(id, self.get_int(seq_id_begin),
                             self.get_int(seq_id_end))


class _EntityNonPolyHandler(Handler):
    category = '_pdbx_entity_nonpoly'

    def __call__(self, entity_id, comp_id):
        s = self.sysr.entities.get_by_id(entity_id)
        s.sequence = (self.sysr.chem_comps.get_by_id(comp_id),)


class _StructAsymHandler(Handler):
    category = '_struct_asym'

    def __call__(self, id, entity_id, details):
        s = self.sysr.asym_units.get_by_id(id)
        # Keep this ID (like a user-assigned ID); don't reassign it on output
        s.id = id
        s.entity = self.sysr.entities.get_by_id(entity_id)
        self.copy_if_present(s, locals(), keys=('details',))


class _AssemblyHandler(Handler):
    category = '_ihm_struct_assembly'

    def __call__(self, id, name, description):
        s = self.sysr.assemblies.get_by_id(id)
        self.copy_if_present(s, locals(), keys=('name', 'description'))


class _AssemblyDetailsHandler(Handler):
    category = '_ihm_struct_assembly_details'
    ignored_keywords = ['ordinal_id', 'entity_description']

    def __init__(self, *args):
        super(_AssemblyDetailsHandler, self).__init__(*args)
        self._read_args = []

    def __call__(self, assembly_id, parent_assembly_id, entity_poly_segment_id,
                 asym_id, entity_id):
        a_id = assembly_id
        a = self.sysr.assemblies.get_by_id(a_id)
        parent_id = parent_assembly_id
        if parent_id and parent_id != a_id and not a.parent:
            a.parent = self.sysr.assemblies.get_by_id(parent_id)
        if asym_id:
            obj = self.sysr.asym_units.get_by_id(asym_id)
        else:
            obj = self.sysr.entities.get_by_id(entity_id)
        # Postpone filling in range until finalize time, as we may not have
        # read segments yet
        self._read_args.append((a, obj, entity_poly_segment_id))

    def finalize(self):
        for (a, obj, entity_poly_segment_id) in self._read_args:
            a.append(self.sysr.ranges.get(obj, entity_poly_segment_id))

        self.system._make_complete_assembly()
        tup_complete = tuple(self.system.complete_assembly)

        for a in self.system.orphan_assemblies:
            # Any EntityRange or AsymUnitRange which covers an entire entity,
            # replace with Entity or AsymUnit object
            a[:] = [self._handle_component(x) for x in a]
            # If the input file defines the complete assembly, transfer
            # user-provided info to system.complete_assembly
            if tuple(a) == tup_complete:
                self.system.complete_assembly.name = a.name
                self.system.complete_assembly.description = a.description

    def _handle_component(self, comp):
        if isinstance(comp, ihm.EntityRange) \
           and comp.seq_id_range == comp.entity.seq_id_range:
            return comp.entity
        if isinstance(comp, ihm.AsymUnitRange) \
           and comp.seq_id_range == comp.asym.seq_id_range:
            return comp.asym
        else:
            return comp


class _LocalFiles(ihm.location.Repository):
    """Placeholder for files stored locally"""
    reference_provider = None
    reference_type = 'Supplementary Files'
    reference = None
    refers_to = 'Other'
    url = None


class _ExtRefHandler(Handler):
    category = '_ihm_external_reference_info'

    def __init__(self, *args):
        super(_ExtRefHandler, self).__init__(*args)
        self.type_map = {'doi': ihm.location.Repository,
                         'supplementary files': _LocalFiles}

    def __call__(self, reference_id, reference_type, reference, associated_url,
                 details):
        ref_id = reference_id
        typ = 'doi' if reference_type is None else reference_type.lower()
        repo = self.sysr.repos.get_by_id(
            ref_id, self.type_map.get(typ, ihm.location.Repository))
        self.copy_if_present(
            repo, locals(), keys=('details',),
            mapkeys={'reference': 'doi', 'associated_url': 'url'})

    def finalize(self):
        # Map use of placeholder _LocalFiles repository to repo=None
        for location in self.system.locations:
            if hasattr(location, 'repo') \
                    and isinstance(location.repo, _LocalFiles):
                location.repo = None


class _ExtFileHandler(Handler):
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

    def __call__(self, content_type, id, reference_id, details, file_path):
        typ = None if content_type is None else content_type.lower()
        f = self.sysr.external_files.get_by_id(
            id, self.type_map.get(typ, ihm.location.FileLocation))
        f.repo = self.sysr.repos.get_by_id(reference_id)
        self.copy_if_present(
            f, locals(), keys=['details'], mapkeys={'file_path': 'path'})
        # Handle DOI that is itself a file
        if file_path is None:
            f.path = '.'


class _DatasetListHandler(Handler):
    category = '_ihm_dataset_list'

    def __init__(self, *args):
        super(_DatasetListHandler, self).__init__(*args)
        # Map data_type to corresponding
        # subclass of ihm.dataset.Dataset
        self.type_map = dict(
            (x[1].data_type.lower(), x[1])
            for x in inspect.getmembers(ihm.dataset, inspect.isclass)
            if issubclass(x[1], ihm.dataset.Dataset))

    def __call__(self, data_type, id, details):
        typ = None if data_type is None else data_type.lower()
        f = self.sysr.datasets.get_by_id(
            id, self.type_map.get(typ, ihm.dataset.Dataset))
        f.details = details


class _DatasetGroupHandler(Handler):
    category = '_ihm_dataset_group'
    ignored_keywords = ['ordinal_id']

    def __call__(self, id, name, application, details):
        g = self.sysr.dataset_groups.get_by_id(id)
        self.copy_if_present(g, locals(),
                             keys=('name', 'application', 'details'))


class _DatasetGroupLinkHandler(Handler):
    category = '_ihm_dataset_group_link'

    def __call__(self, group_id, dataset_list_id):
        g = self.sysr.dataset_groups.get_by_id(group_id)
        ds = self.sysr.datasets.get_by_id(dataset_list_id)
        g.append(ds)


class _DatasetExtRefHandler(Handler):
    category = '_ihm_dataset_external_reference'

    def __call__(self, file_id, dataset_list_id):
        ds = self.sysr.datasets.get_by_id(dataset_list_id)
        f = self.sysr.external_files.get_by_id(file_id)
        ds.location = f


class _DatasetDBRefHandler(Handler):
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

    def __call__(self, dataset_list_id, db_name, id, version, details,
                 accession_code):
        ds = self.sysr.datasets.get_by_id(dataset_list_id)
        typ = None if db_name is None else db_name.lower()
        dbloc = self.sysr.db_locations.get_by_id(id,
                                                 self.type_map.get(typ, None))
        ds.location = dbloc
        self.copy_if_present(
            dbloc, locals(), keys=['version', 'details'],
            mapkeys={'accession_code': 'access_code'})


class _DataTransformationHandler(Handler):
    category = '_ihm_data_transformation'

    def __call__(self, id, tr_vector1, tr_vector2, tr_vector3, rot_matrix11,
                 rot_matrix21, rot_matrix31, rot_matrix12, rot_matrix22,
                 rot_matrix32, rot_matrix13, rot_matrix23, rot_matrix33):
        t = self.sysr.data_transformations.get_by_id(id)
        t.rot_matrix = _get_matrix33(locals(), 'rot_matrix')
        t.tr_vector = _get_vector3(locals(), 'tr_vector')


class _RelatedDatasetsHandler(Handler):
    category = '_ihm_related_datasets'
    ignored_keywords = ['ordinal_id']

    def __call__(self, dataset_list_id_derived, dataset_list_id_primary,
                 transformation_id):
        derived = self.sysr.datasets.get_by_id(dataset_list_id_derived)
        primary = self.sysr.datasets.get_by_id(dataset_list_id_primary)
        trans = self.sysr.data_transformations.get_by_id_or_none(
            transformation_id)
        if trans:
            primary = ihm.dataset.TransformedDataset(
                dataset=primary, transform=trans)
        derived.parents.append(primary)


class _ModelRepresentationHandler(Handler):
    category = '_ihm_model_representation'

    def __call__(self, id, name, details):
        rep = self.sysr.representations.get_by_id(id)
        self.copy_if_present(rep, locals(), keys=('name', 'details'))


def _make_atom_segment(asym, rigid, primitive, count, smodel, description):
    return ihm.representation.AtomicSegment(
        asym_unit=asym, rigid=rigid, starting_model=smodel,
        description=description)


def _make_residue_segment(asym, rigid, primitive, count, smodel, description):
    return ihm.representation.ResidueSegment(
        asym_unit=asym, rigid=rigid, primitive=primitive,
        starting_model=smodel, description=description)


def _make_multi_residue_segment(asym, rigid, primitive, count, smodel,
                                description):
    return ihm.representation.MultiResidueSegment(
        asym_unit=asym, rigid=rigid, primitive=primitive,
        starting_model=smodel, description=description)


def _make_feature_segment(asym, rigid, primitive, count, smodel, description):
    return ihm.representation.FeatureSegment(
        asym_unit=asym, rigid=rigid, primitive=primitive,
        count=count, starting_model=smodel, description=description)


class _ModelRepresentationDetailsHandler(Handler):
    category = '_ihm_model_representation_details'
    ignored_keywords = ['entity_description']

    _rigid_map = {'rigid': True, 'flexible': False, None: None}
    _segment_factory = {'by-atom': _make_atom_segment,
                        'by-residue': _make_residue_segment,
                        'multi-residue': _make_multi_residue_segment,
                        'by-feature': _make_feature_segment}

    def __init__(self, *args):
        super(_ModelRepresentationDetailsHandler, self).__init__(*args)
        self._read_args = []

    def __call__(self, entity_asym_id, entity_poly_segment_id,
                 representation_id, starting_model_id, model_object_primitive,
                 model_granularity, model_object_count, model_mode,
                 description):
        # Postpone until finalize time as we may not have segments yet
        self._read_args.append(
            (entity_asym_id, entity_poly_segment_id,
             representation_id, starting_model_id, model_object_primitive,
             model_granularity, model_object_count, model_mode, description))

    def finalize(self):
        for (entity_asym_id, entity_poly_segment_id,
             representation_id, starting_model_id, model_object_primitive,
             model_granularity, model_object_count, model_mode,
             description) in self._read_args:
            asym = self.sysr.ranges.get(
                self.sysr.asym_units.get_by_id(entity_asym_id),
                entity_poly_segment_id)
            rep = self.sysr.representations.get_by_id(representation_id)
            smodel = self.sysr.starting_models.get_by_id_or_none(
                starting_model_id)
            primitive = self.get_lower(model_object_primitive)
            gran = self.get_lower(model_granularity)
            primitive = self.get_lower(model_object_primitive)
            count = self.get_int(model_object_count)
            rigid = self._rigid_map[self.get_lower(model_mode)]
            segment = self._segment_factory[gran](
                asym, rigid, primitive, count, smodel, description)
            rep.append(segment)


# todo: support user subclass of StartingModel, pass it coordinates, seqdif
class _StartingModelDetailsHandler(Handler):
    category = '_ihm_starting_model_details'
    ignored_keywords = ['entity_description']

    def __call__(self, starting_model_id, asym_id, entity_poly_segment_id,
                 dataset_list_id, starting_model_auth_asym_id,
                 starting_model_sequence_offset, description):
        m = self.sysr.starting_models.get_by_id(starting_model_id)
        # We might not have a suitable range yet for this ID, so fill this
        # in at finalize time
        m.asym_unit = (asym_id, entity_poly_segment_id)
        m.dataset = self.sysr.datasets.get_by_id(dataset_list_id)
        self.copy_if_present(
            m, locals(), keys=('description',),
            mapkeys={'starting_model_auth_asym_id': 'asym_id'})
        if starting_model_sequence_offset is not None:
            m.offset = int(starting_model_sequence_offset)

    def finalize(self):
        for m in self.sysr.system.orphan_starting_models:
            # Skip any auto-generated models without range info
            if m.asym_unit is None:
                continue
            # Replace tuple with real Asym/Entity range object
            (asym_id, entity_poly_segment_id) = m.asym_unit
            m.asym_unit = self.sysr.ranges.get(
                self.sysr.asym_units.get_by_id(asym_id),
                entity_poly_segment_id)


class _StartingComputationalModelsHandler(Handler):
    category = '_ihm_starting_computational_models'

    def __call__(self, starting_model_id, script_file_id, software_id):
        m = self.sysr.starting_models.get_by_id(starting_model_id)
        if script_file_id is not None:
            m.script_file = self.sysr.external_files.get_by_id(script_file_id)
        if software_id is not None:
            m.software = self.sysr.software.get_by_id(software_id)


class _StartingComparativeModelsHandler(Handler):
    category = '_ihm_starting_comparative_models'
    ignored_keywords = ['ordinal_id']

    def __call__(self, starting_model_id, template_dataset_list_id,
                 alignment_file_id, template_auth_asym_id,
                 starting_model_seq_id_begin, starting_model_seq_id_end,
                 template_seq_id_begin, template_seq_id_end,
                 template_sequence_identity,
                 template_sequence_identity_denominator):
        m = self.sysr.starting_models.get_by_id(starting_model_id)
        dataset = self.sysr.datasets.get_by_id(template_dataset_list_id)
        aln = self.sysr.external_files.get_by_id_or_none(alignment_file_id)
        asym_id = template_auth_asym_id
        seq_id_range = (int(starting_model_seq_id_begin),
                        int(starting_model_seq_id_end))
        template_seq_id_range = (self.get_int(template_seq_id_begin),
                                 self.get_int(template_seq_id_end))
        identity = ihm.startmodel.SequenceIdentity(
            self.get_float(template_sequence_identity),
            self.get_int(template_sequence_identity_denominator))
        t = ihm.startmodel.Template(
            dataset, asym_id, seq_id_range, template_seq_id_range,
            identity, aln)
        m.templates.append(t)


class _ProtocolHandler(Handler):
    category = '_ihm_modeling_protocol'
    ignored_keywords = ['ordinal_id', 'struct_assembly_description']

    def __call__(self, id, protocol_name, num_steps):
        p = self.sysr.protocols.get_by_id(id)
        self.copy_if_present(p, locals(), mapkeys={'protocol_name': 'name'})


class _ProtocolDetailsHandler(Handler):
    category = '_ihm_modeling_protocol_details'

    def __call__(self, protocol_id, step_id, num_models_begin,
                 num_models_end, multi_scale_flag, multi_state_flag,
                 ordered_flag, ensemble_flag, struct_assembly_id,
                 dataset_group_id, software_id, script_file_id, step_name,
                 step_method, description):
        p = self.sysr.protocols.get_by_id(protocol_id)
        nbegin = self.get_int(num_models_begin)
        nend = self.get_int(num_models_end)
        mscale = self.get_bool(multi_scale_flag)
        mstate = self.get_bool(multi_state_flag)
        ensemble = self.get_bool(ensemble_flag)
        ordered = self.get_bool(ordered_flag)
        assembly = self.sysr.assemblies.get_by_id_or_none(
            struct_assembly_id)
        dg = self.sysr.dataset_groups.get_by_id_or_none(dataset_group_id)
        software = self.sysr.software.get_by_id_or_none(software_id)
        script = self.sysr.external_files.get_by_id_or_none(script_file_id)
        s = ihm.protocol.Step(assembly=assembly, dataset_group=dg,
                              method=None, num_models_begin=nbegin,
                              num_models_end=nend, multi_scale=mscale,
                              multi_state=mstate, ordered=ordered,
                              ensemble=ensemble, software=software,
                              script_file=script, description=description)
        s._id = step_id
        self.copy_if_present(
            s, locals(),
            mapkeys={'step_name': 'name', 'step_method': 'method'})
        p.steps.append(s)


class _PostProcessHandler(Handler):
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

    def __call__(self, protocol_id, analysis_id, type, id, num_models_begin,
                 num_models_end, struct_assembly_id, dataset_group_id,
                 software_id, script_file_id, feature, details):
        protocol = self.sysr.protocols.get_by_id(protocol_id)
        analysis = self.sysr.analyses.get_by_id(analysis_id)
        if analysis._id not in [a._id for a in protocol.analyses]:
            protocol.analyses.append(analysis)

        typ = type.lower() if type is not None else 'other'
        step = self.sysr.analysis_steps.get_by_id(
            id, self.type_map.get(typ, ihm.analysis.Step))
        analysis.steps.append(step)
        step.details = details

        if typ == 'none':
            # If this step was forward referenced, feature will have been set
            # to Python None - set it to explicit 'none' instead
            step.feature = 'none'
        else:
            step.num_models_begin = self.get_int(num_models_begin)
            step.num_models_end = self.get_int(num_models_end)
            step.assembly = self.sysr.assemblies.get_by_id_or_none(
                struct_assembly_id)
            step.dataset_group = self.sysr.dataset_groups.get_by_id_or_none(
                dataset_group_id)
            step.software = self.sysr.software.get_by_id_or_none(software_id)
            step.script_file = self.sysr.external_files.get_by_id_or_none(
                script_file_id)
            self.copy_if_present(step, locals(), keys=['feature'])


class _ModelListHandler(Handler):
    category = '_ihm_model_list'

    def __call__(self, model_id, model_name,
                 assembly_id, representation_id, protocol_id):
        model = self.sysr.models.get_by_id(model_id)

        self.copy_if_present(model, locals(), mapkeys={'model_name': 'name'})
        model.assembly = self.sysr.assemblies.get_by_id_or_none(assembly_id)
        model.representation = self.sysr.representations.get_by_id_or_none(
            representation_id)
        model.protocol = self.sysr.protocols.get_by_id_or_none(protocol_id)


class _ModelGroupHandler(Handler):
    category = '_ihm_model_group'

    def __call__(self, id, name, details):
        model_group = self.sysr.model_groups.get_by_id(id)
        self.copy_if_present(model_group, locals(), keys=('name', 'details'))

    def finalize(self):
        # Put all model groups not assigned to a state in their own state
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

        # Put all models not in a group in their own group in its own state
        # (e.g. this will catch models from a non-IHM file)
        models_in_groups = set()
        for mg in self.sysr.model_groups._obj_by_id.values():
            for m in mg:
                models_in_groups.add(m._id)
        ms = [m for mid, m in self.sysr.models._obj_by_id.items()
              if mid not in models_in_groups]
        if ms:
            mg = ihm.model.ModelGroup(ms)
            s = ihm.model.State([mg])
            self.system.state_groups.append(ihm.model.StateGroup([s]))


class _ModelGroupLinkHandler(Handler):
    category = '_ihm_model_group_link'

    def __call__(self, group_id, model_id):
        model_group = self.sysr.model_groups.get_by_id(group_id)
        model = self.sysr.models.get_by_id(model_id)
        model_group.append(model)


class _MultiStateHandler(Handler):
    category = '_ihm_multi_state_modeling'

    def __call__(self, state_group_id, state_id,
                 population_fraction, experiment_type, details, state_name,
                 state_type):
        state_group = self.sysr.state_groups.get_by_id(state_group_id)
        state = self.sysr.states.get_by_id(state_id)
        state_group.append(state)

        state.population_fraction = self.get_float(population_fraction)
        self.copy_if_present(
            state, locals(),
            keys=['experiment_type', 'details'],
            mapkeys={'state_name': 'name', 'state_type': 'type'})


class _MultiStateLinkHandler(Handler):
    category = '_ihm_multi_state_model_group_link'

    def __call__(self, state_id, model_group_id):
        state = self.sysr.states.get_by_id(state_id)
        model_group = self.sysr.model_groups.get_by_id(model_group_id)
        state.append(model_group)


class _EnsembleHandler(Handler):
    category = '_ihm_ensemble_info'

    # Map subsample type to corresponding subclass
    _type_map = dict((x[1].sub_sampling_type.lower(), x[1])
                     for x in inspect.getmembers(ihm.model, inspect.isclass)
                     if issubclass(x[1], ihm.model.Subsample))

    def __call__(self, ensemble_id, model_group_id, post_process_id,
                 ensemble_file_id, num_ensemble_models,
                 ensemble_precision_value, ensemble_name,
                 ensemble_clustering_method, ensemble_clustering_feature,
                 details, sub_sampling_type):
        ensemble = self.sysr.ensembles.get_by_id(ensemble_id)
        mg = self.sysr.model_groups.get_by_id_or_none(model_group_id)
        pp = self.sysr.analysis_steps.get_by_id_or_none(post_process_id)
        f = self.sysr.external_files.get_by_id_or_none(ensemble_file_id)

        ensemble.model_group = mg
        ensemble.num_models = self.get_int(num_ensemble_models)
        ensemble.precision = self.get_float(ensemble_precision_value)
        if sub_sampling_type:
            ensemble._sub_sampling_type = sub_sampling_type.lower()
        # note that num_ensemble_models_deposited is ignored (should be size of
        # model group anyway)
        ensemble.post_process = pp
        ensemble.file = f
        ensemble.details = details
        # Default to "other" if invalid method/feature read
        try:
            ensemble.clustering_method = ensemble_clustering_method
        except ValueError:
            ensemble.clustering_method = "Other"
        try:
            ensemble.clustering_feature = ensemble_clustering_feature
        except ValueError:
            ensemble.clustering_feature = "other"
        self.copy_if_present(
            ensemble, locals(),
            mapkeys={'ensemble_name': 'name'})

    def finalize(self):
        for e in self.sysr.system.ensembles:
            if hasattr(e, '_sub_sampling_type'):
                t = self._type_map.get(e._sub_sampling_type,
                                       ihm.model.Subsample)
                for s in e.subsamples:
                    s.__class__ = t
                del e._sub_sampling_type


class _SubsampleHandler(Handler):
    category = '_ihm_ensemble_sub_sample'

    def __call__(self, name, ensemble_id, num_models, model_group_id, file_id):
        ensemble = self.sysr.ensembles.get_by_id(ensemble_id)
        mg = self.sysr.model_groups.get_by_id_or_none(model_group_id)
        f = self.sysr.external_files.get_by_id_or_none(file_id)

        # We don't know the type yet (not until ensemble is read); this
        # will be corrected by EnsembleHandler.finalize()
        ss = ihm.model.Subsample(
            name=name, num_models=self.get_int(num_models), model_group=mg,
            file=f)
        ensemble.subsamples.append(ss)


class _DensityHandler(Handler):
    category = '_ihm_localization_density_files'

    def __init__(self, *args):
        super(_DensityHandler, self).__init__(*args)
        self._read_args = []

    def __call__(self, id, ensemble_id, file_id, asym_id,
                 entity_poly_segment_id):
        # Postpone handling until finalize time, since we might not have
        # ranges to resolve entity_poly_segment_id yet
        self._read_args.append((id, ensemble_id, file_id, asym_id,
                                entity_poly_segment_id))

    def finalize(self):
        for (id, ensemble_id, file_id, asym_id,
                entity_poly_segment_id) in self._read_args:
            density = self.sysr.densities.get_by_id(id)
            ensemble = self.sysr.ensembles.get_by_id(ensemble_id)
            f = self.sysr.external_files.get_by_id(file_id)

            asym = self.sysr.ranges.get(
                self.sysr.asym_units.get_by_id(asym_id),
                entity_poly_segment_id)
            density.asym_unit = asym
            density.file = f
            ensemble.densities.append(density)


class _EM3DRestraintHandler(Handler):
    category = '_ihm_3dem_restraint'

    def __call__(self, dataset_list_id, struct_assembly_id,
                 fitting_method_citation_id, fitting_method,
                 number_of_gaussians, model_id, cross_correlation_coefficient):
        # EM3D restraints don't have their own IDs - they use the dataset id
        r = self.sysr.em3d_restraints.get_by_dataset(dataset_list_id)
        r.assembly = self.sysr.assemblies.get_by_id_or_none(struct_assembly_id)
        r.fitting_method_citation = self.sysr.citations.get_by_id_or_none(
            fitting_method_citation_id)
        self.copy_if_present(r, locals(), keys=('fitting_method',))
        r.number_of_gaussians = self.get_int(number_of_gaussians)

        model = self.sysr.models.get_by_id(model_id)
        ccc = self.get_float(cross_correlation_coefficient)
        r.fits[model] = ihm.restraint.EM3DRestraintFit(ccc)


class _EM2DRestraintHandler(Handler):
    category = '_ihm_2dem_class_average_restraint'

    def __call__(self, id, dataset_list_id, number_raw_micrographs,
                 pixel_size_width, pixel_size_height, image_resolution,
                 image_segment_flag, number_of_projections,
                 struct_assembly_id, details):
        r = self.sysr.em2d_restraints.get_by_id(id)
        r.dataset = self.sysr.datasets.get_by_id(dataset_list_id)
        r.number_raw_micrographs = self.get_int(number_raw_micrographs)
        r.pixel_size_width = self.get_float(pixel_size_width)
        r.pixel_size_height = self.get_float(pixel_size_height)
        r.image_resolution = self.get_float(image_resolution)
        r.segment = self.get_bool(image_segment_flag)
        r.number_of_projections = self.get_int(number_of_projections)
        r.assembly = self.sysr.assemblies.get_by_id_or_none(
            struct_assembly_id)
        self.copy_if_present(r, locals(), keys=('details',))


class _EM2DFittingHandler(Handler):
    category = '_ihm_2dem_class_average_fitting'

    def __call__(self, restraint_id, model_id, cross_correlation_coefficient,
                 tr_vector1, tr_vector2, tr_vector3, rot_matrix11,
                 rot_matrix21, rot_matrix31, rot_matrix12, rot_matrix22,
                 rot_matrix32, rot_matrix13, rot_matrix23, rot_matrix33):
        r = self.sysr.em2d_restraints.get_by_id(restraint_id)
        model = self.sysr.models.get_by_id(model_id)
        ccc = self.get_float(cross_correlation_coefficient)
        tr_vector = _get_vector3(locals(), 'tr_vector')
        rot_matrix = _get_matrix33(locals(), 'rot_matrix')
        r.fits[model] = ihm.restraint.EM2DRestraintFit(
            cross_correlation_coefficient=ccc, rot_matrix=rot_matrix,
            tr_vector=tr_vector)


class _SASRestraintHandler(Handler):
    category = '_ihm_sas_restraint'

    def __call__(self, dataset_list_id, struct_assembly_id,
                 profile_segment_flag, fitting_atom_type, fitting_method,
                 details, fitting_state, radius_of_gyration,
                 number_of_gaussians, model_id, chi_value):
        # SAS restraints don't have their own IDs - they use the dataset id
        r = self.sysr.sas_restraints.get_by_dataset(dataset_list_id)
        r.assembly = self.sysr.assemblies.get_by_id_or_none(
            struct_assembly_id)
        r.segment = self.get_bool(profile_segment_flag)
        self.copy_if_present(
            r, locals(),
            keys=('fitting_atom_type', 'fitting_method', 'details'))
        fs = (fitting_state if fitting_state not in (None, ihm.unknown)
              else 'Single')
        r.multi_state = fs.lower() != 'single'
        r.radius_of_gyration = self.get_float(radius_of_gyration)
        r.number_of_gaussians = self.get_int(number_of_gaussians)

        model = self.sysr.models.get_by_id(model_id)
        r.fits[model] = ihm.restraint.SASRestraintFit(
            chi_value=self.get_float(chi_value))


class _SphereObjSiteHandler(Handler):
    category = '_ihm_sphere_obj_site'
    ignored_keywords = ['ordinal_id']

    def __call__(self, model_id, asym_id, rmsf, seq_id_begin, seq_id_end,
                 cartn_x, cartn_y, cartn_z, object_radius):
        model = self.sysr.models.get_by_id(model_id)
        asym = self.sysr.asym_units.get_by_id(asym_id)
        rmsf = self.get_float(rmsf)
        s = ihm.model.Sphere(
            asym_unit=asym, seq_id_range=(int(seq_id_begin), int(seq_id_end)),
            x=float(cartn_x), y=float(cartn_y), z=float(cartn_z),
            radius=float(object_radius), rmsf=rmsf)
        model.add_sphere(s)


class _AtomSiteHandler(Handler):
    category = '_atom_site'

    def __init__(self, *args):
        super(_AtomSiteHandler, self).__init__(*args)
        self._missing_sequence = collections.defaultdict(dict)

    def __call__(self, pdbx_pdb_model_num, label_asym_id, b_iso_or_equiv,
                 label_seq_id, label_atom_id, type_symbol, cartn_x, cartn_y,
                 cartn_z, occupancy, group_pdb, auth_seq_id,
                 pdbx_pdb_ins_code, auth_asym_id, label_comp_id):
        # seq_id can be None for non-polymers (HETATM)
        seq_id = self.get_int(label_seq_id)
        # todo: handle fields other than those output by us
        model = self.sysr.models.get_by_id(pdbx_pdb_model_num)
        if label_asym_id is None:
            # If no asym_id is provided (e.g. minimal PyMOL output) then
            # use the author-provided ID instead
            asym = self.sysr.asym_units.get_by_id(auth_asym_id)
            # Chances are the entity_poly table is missing too, so remember
            # the comp_id to help us construct missing sequence info
            self._missing_sequence[asym][seq_id] = label_comp_id
        else:
            asym = self.sysr.asym_units.get_by_id(label_asym_id)
        biso = self.get_float(b_iso_or_equiv)
        occupancy = self.get_float(occupancy)
        group = 'ATOM' if group_pdb is None else group_pdb
        a = ihm.model.Atom(
            asym_unit=asym, seq_id=seq_id, atom_id=label_atom_id,
            type_symbol=type_symbol, x=float(cartn_x), y=float(cartn_y),
            z=float(cartn_z), het=group != 'ATOM', biso=biso,
            occupancy=occupancy)
        model.add_atom(a)

        auth_seq_id = self.get_int_or_string(auth_seq_id)
        # Note any residues that have different seq_id and auth_seq_id
        if (auth_seq_id is not None and
                (seq_id != auth_seq_id
                 or pdbx_pdb_ins_code not in (None, ihm.unknown))):
            if asym.auth_seq_id_map == 0:
                asym.auth_seq_id_map = {}
            asym.auth_seq_id_map[seq_id] = auth_seq_id, pdbx_pdb_ins_code

    def finalize(self):
        # Fill in missing Entity information from comp_ids
        entity_from_seq = {}
        for asym, comp_from_seq_id in self._missing_sequence.items():
            if asym.entity is None:
                # Fill in gaps in seq_id with UNK residues
                seq_len = max(comp_from_seq_id.keys())
                unk = ihm.LPeptideAlphabet()['UNK']
                seq = [unk] * seq_len
                for seq_id, comp_id in comp_from_seq_id.items():
                    seq[seq_id - 1] = self.sysr.chem_comps.get_by_id(comp_id)
                seq = tuple(seq)  # Lists are not hashable
                if seq in entity_from_seq:
                    asym.entity = entity_from_seq[seq]
                else:
                    asym.entity = ihm.Entity(seq)
                    entity_from_seq[seq] = asym.entity
                    self.system.entities.append(asym.entity)


class _StartingModelCoordHandler(Handler):
    category = '_ihm_starting_model_coord'

    def __call__(self, starting_model_id, group_pdb, type_symbol, atom_id,
                 asym_id, seq_id, cartn_x, cartn_y, cartn_z, b_iso_or_equiv):
        model = self.sysr.starting_models.get_by_id(starting_model_id)
        asym = self.sysr.asym_units.get_by_id(asym_id)
        biso = self.get_float(b_iso_or_equiv)
        # seq_id can be None for non-polymers (HETATM)
        seq_id = self.get_int(seq_id)
        group = 'ATOM' if group_pdb is None else group_pdb
        a = ihm.model.Atom(
            asym_unit=asym, seq_id=seq_id, atom_id=atom_id,
            type_symbol=type_symbol, x=float(cartn_x), y=float(cartn_y),
            z=float(cartn_z), het=group != 'ATOM', biso=biso)
        model.add_atom(a)


class _StartingModelSeqDifHandler(Handler):
    category = '_ihm_starting_model_seq_dif'

    def __call__(self, starting_model_id, db_seq_id, seq_id, db_comp_id,
                 details):
        model = self.sysr.starting_models.get_by_id(starting_model_id)
        sd = ihm.startmodel.SeqDif(db_seq_id=self.get_int(db_seq_id),
                                   seq_id=self.get_int(seq_id),
                                   db_comp_id=db_comp_id,
                                   details=details)
        model.add_seq_dif(sd)


class _PolyResidueFeatureHandler(Handler):
    category = '_ihm_poly_residue_feature'

    def __call__(self, feature_id, entity_id, asym_id, seq_id_begin,
                 seq_id_end):
        f = self.sysr.features.get_by_id(
            feature_id, ihm.restraint.ResidueFeature)
        asym_or_entity = self._get_asym_or_entity(asym_id, entity_id)
        r1 = int(seq_id_begin)
        r2 = int(seq_id_end)
        f.ranges.append(asym_or_entity(r1, r2))


class _FeatureListHandler(Handler):
    category = '_ihm_feature_list'

    def __call__(self, feature_id, details):
        if details:
            f = self.sysr.features.get_by_id(feature_id)
            f.details = details


class _PolyAtomFeatureHandler(Handler):
    category = '_ihm_poly_atom_feature'

    def __call__(self, feature_id, entity_id, asym_id, seq_id, atom_id):
        f = self.sysr.features.get_by_id(
            feature_id, ihm.restraint.AtomFeature)
        asym_or_entity = self._get_asym_or_entity(asym_id, entity_id)
        seq_id = int(seq_id)
        atom = asym_or_entity.residue(seq_id).atom(atom_id)
        f.atoms.append(atom)


class _NonPolyFeatureHandler(Handler):
    category = '_ihm_non_poly_feature'

    def __call__(self, feature_id, entity_id, asym_id, atom_id):
        asym_or_entity = self._get_asym_or_entity(asym_id, entity_id)
        if atom_id is None:
            f = self.sysr.features.get_by_id(
                feature_id, ihm.restraint.NonPolyFeature)
            f.objs.append(asym_or_entity)
        else:
            f = self.sysr.features.get_by_id(
                feature_id, ihm.restraint.AtomFeature)
            # todo: handle multiple copies, e.g. waters?
            atom = asym_or_entity.residue(1).atom(atom_id)
            f.atoms.append(atom)


class _PseudoSiteFeatureHandler(Handler):
    category = '_ihm_pseudo_site_feature'

    def __call__(self, feature_id, pseudo_site_id):
        f = self.sysr.features.get_by_id(feature_id,
                                         ihm.restraint.PseudoSiteFeature)
        p = self.sysr.pseudo_sites.get_by_id(pseudo_site_id)
        f.site = p


class _PseudoSiteHandler(Handler):
    category = '_ihm_pseudo_site'

    def __call__(self, id, cartn_x, cartn_y, cartn_z, radius, description):
        p = self.sysr.pseudo_sites.get_by_id(id)
        p.x = self.get_float(cartn_x)
        p.y = self.get_float(cartn_y)
        p.z = self.get_float(cartn_z)
        p.radius = self.get_float(radius)
        p.description = description


def _make_harmonic(low, up, _get_float):
    low = _get_float(low)
    up = _get_float(up)
    return ihm.restraint.HarmonicDistanceRestraint(up if low is None else low)


def _make_upper_bound(low, up, _get_float):
    up = _get_float(up)
    return ihm.restraint.UpperBoundDistanceRestraint(up)


def _make_lower_bound(low, up, _get_float):
    low = _get_float(low)
    return ihm.restraint.LowerBoundDistanceRestraint(low)


def _make_lower_upper_bound(low, up, _get_float):
    low = _get_float(low)
    up = _get_float(up)
    return ihm.restraint.LowerUpperBoundDistanceRestraint(
        distance_lower_limit=low, distance_upper_limit=up)


def _make_unknown_distance(low, up, _get_float):
    return ihm.restraint.DistanceRestraint()


_handle_distance = {'harmonic': _make_harmonic,
                    'upper bound': _make_upper_bound,
                    'lower bound': _make_lower_bound,
                    'lower and upper bound': _make_lower_upper_bound,
                    None: _make_unknown_distance}


class _DerivedDistanceRestraintHandler(Handler):
    category = '_ihm_derived_distance_restraint'
    _cond_map = {'ALL': True, 'ANY': False, None: None}

    def __call__(self, id, group_id, dataset_list_id, feature_id_1,
                 feature_id_2, restraint_type, group_conditionality,
                 probability, mic_value, distance_lower_limit,
                 distance_upper_limit):
        r = self.sysr.dist_restraints.get_by_id(id)
        if group_id is not None:
            rg = self.sysr.dist_restraint_groups.get_by_id(group_id)
            rg.append(r)
        r.dataset = self.sysr.datasets.get_by_id_or_none(dataset_list_id)
        r.feature1 = self.sysr.features.get_by_id(feature_id_1)
        r.feature2 = self.sysr.features.get_by_id(feature_id_2)
        r.distance = _handle_distance[restraint_type](distance_lower_limit,
                                                      distance_upper_limit,
                                                      self.get_float)
        r.restrain_all = self._cond_map[group_conditionality]
        r.probability = self.get_float(probability)
        r.mic_value = self.get_float(mic_value)


class _PredictedContactRestraintHandler(Handler):
    category = '_ihm_predicted_contact_restraint'

    def _get_resatom(self, asym_id, seq_id, atom_id):
        asym = self.sysr.asym_units.get_by_id(asym_id)
        seq_id = self.get_int(seq_id)
        resatom = asym.residue(seq_id)
        if atom_id:
            resatom = resatom.atom(atom_id)
        return resatom

    def __call__(self, id, group_id, dataset_list_id, asym_id_1,
                 seq_id_1, rep_atom_1, asym_id_2, seq_id_2, rep_atom_2,
                 restraint_type, probability, distance_lower_limit,
                 distance_upper_limit, model_granularity, software_id):
        r = self.sysr.pred_cont_restraints.get_by_id(id)
        if group_id is not None:
            rg = self.sysr.pred_cont_restraint_groups.get_by_id(group_id)
            rg.append(r)
        r.dataset = self.sysr.datasets.get_by_id_or_none(dataset_list_id)
        r.resatom1 = self._get_resatom(asym_id_1, seq_id_1, rep_atom_1)
        r.resatom2 = self._get_resatom(asym_id_2, seq_id_2, rep_atom_2)
        r.distance = _handle_distance[restraint_type](distance_lower_limit,
                                                      distance_upper_limit,
                                                      self.get_float)
        r.by_residue = self.get_lower(model_granularity) == 'by-residue'
        r.probability = self.get_float(probability)
        r.software = self.sysr.software.get_by_id_or_none(software_id)


class _CenterHandler(Handler):
    category = '_ihm_geometric_object_center'

    def __call__(self, id, xcoord, ycoord, zcoord):
        c = self.sysr.centers.get_by_id(id)
        c.x = self.get_float(xcoord)
        c.y = self.get_float(ycoord)
        c.z = self.get_float(zcoord)


class _TransformationHandler(Handler):
    category = '_ihm_geometric_object_transformation'

    def __call__(self, id, tr_vector1, tr_vector2, tr_vector3, rot_matrix11,
                 rot_matrix21, rot_matrix31, rot_matrix12, rot_matrix22,
                 rot_matrix32, rot_matrix13, rot_matrix23, rot_matrix33):
        t = self.sysr.transformations.get_by_id(id)
        t.rot_matrix = _get_matrix33(locals(), 'rot_matrix')
        t.tr_vector = _get_vector3(locals(), 'tr_vector')


class _GeometricObjectHandler(Handler):
    category = '_ihm_geometric_object_list'

    # Map object_type to corresponding subclass (but not subsubclasses such
    # as XYPlane)
    _type_map = dict((x[1].type.lower(), x[1])
                     for x in inspect.getmembers(ihm.geometry, inspect.isclass)
                     if issubclass(x[1], ihm.geometry.GeometricObject)
                     and ihm.geometry.GeometricObject in x[1].__bases__)

    def __call__(self, object_type, object_id, object_name,
                 object_description):
        typ = object_type.lower() if object_type is not None else 'other'
        g = self.sysr.geometries.get_by_id(
            object_id, self._type_map.get(typ, ihm.geometry.GeometricObject))
        self.copy_if_present(g, locals(),
                             mapkeys={'object_name': 'name',
                                      'object_description': 'description'})


class _SphereHandler(Handler):
    category = '_ihm_geometric_object_sphere'

    def __call__(self, object_id, center_id, transformation_id, radius_r):
        s = self.sysr.geometries.get_by_id(object_id, ihm.geometry.Sphere)
        s.center = self.sysr.centers.get_by_id_or_none(center_id)
        s.transformation = self.sysr.transformations.get_by_id_or_none(
            transformation_id)
        s.radius = self.get_float(radius_r)


class _TorusHandler(Handler):
    category = '_ihm_geometric_object_torus'

    def __call__(self, object_id, center_id, transformation_id,
                 major_radius_r, minor_radius_r):
        t = self.sysr.geometries.get_by_id(object_id, ihm.geometry.Torus)
        t.center = self.sysr.centers.get_by_id_or_none(center_id)
        t.transformation = self.sysr.transformations.get_by_id_or_none(
            transformation_id)
        t.major_radius = self.get_float(major_radius_r)
        t.minor_radius = self.get_float(minor_radius_r)


class _HalfTorusHandler(Handler):
    category = '_ihm_geometric_object_half_torus'

    _inner_map = {'inner half': True, 'outer half': False}

    def __call__(self, object_id, thickness_th, section):
        t = self.sysr.geometries.get_by_id(object_id,
                                           ihm.geometry.HalfTorus)
        t.thickness = self.get_float(thickness_th)
        section = section.lower() if section is not None else ''
        t.inner = self._inner_map.get(section, None)


class _AxisHandler(Handler):
    category = '_ihm_geometric_object_axis'

    # Map axis_type to corresponding subclass
    _type_map = dict((x[1].axis_type.lower(), x[1])
                     for x in inspect.getmembers(ihm.geometry, inspect.isclass)
                     if issubclass(x[1], ihm.geometry.Axis)
                     and x[1] is not ihm.geometry.Axis)

    def __call__(self, axis_type, object_id, transformation_id):
        typ = axis_type.lower() if axis_type is not None else 'other'
        a = self.sysr.geometries.get_by_id(
            object_id, self._type_map.get(typ, ihm.geometry.Axis))
        a.transformation = self.sysr.transformations.get_by_id_or_none(
            transformation_id)


class _PlaneHandler(Handler):
    category = '_ihm_geometric_object_plane'

    # Map plane_type to corresponding subclass
    _type_map = dict((x[1].plane_type.lower(), x[1])
                     for x in inspect.getmembers(ihm.geometry, inspect.isclass)
                     if issubclass(x[1], ihm.geometry.Plane)
                     and x[1] is not ihm.geometry.Plane)

    def __call__(self, plane_type, object_id, transformation_id):
        typ = plane_type.lower() if plane_type is not None else 'other'
        a = self.sysr.geometries.get_by_id(
            object_id, self._type_map.get(typ, ihm.geometry.Plane))
        a.transformation = self.sysr.transformations.get_by_id_or_none(
            transformation_id)


class _GeometricRestraintHandler(Handler):
    category = '_ihm_geometric_object_distance_restraint'

    _cond_map = {'ALL': True, 'ANY': False, None: None}

    # Map object_characteristic to corresponding subclass
    _type_map = dict((x[1].object_characteristic.lower(), x[1])
                     for x in inspect.getmembers(ihm.restraint,
                                                 inspect.isclass)
                     if issubclass(x[1], ihm.restraint.GeometricRestraint))

    def __call__(self, object_characteristic, id, dataset_list_id, object_id,
                 feature_id, restraint_type, harmonic_force_constant,
                 group_conditionality, distance_lower_limit,
                 distance_upper_limit):
        typ = (object_characteristic or 'other').lower()
        r = self.sysr.geom_restraints.get_by_id(
            id, self._type_map.get(typ, ihm.restraint.GeometricRestraint))
        r.dataset = self.sysr.datasets.get_by_id_or_none(dataset_list_id)
        r.geometric_object = self.sysr.geometries.get_by_id(object_id)
        r.feature = self.sysr.features.get_by_id(feature_id)
        r.distance = _handle_distance[restraint_type](distance_lower_limit,
                                                      distance_upper_limit,
                                                      self.get_float)
        r.harmonic_force_constant = self.get_float(harmonic_force_constant)
        r.restrain_all = self._cond_map[group_conditionality]


class _PolySeqSchemeHandler(Handler):
    category = '_pdbx_poly_seq_scheme'

    if _format is not None:
        _add_c_handler = _format.add_poly_seq_scheme_handler

    # Note: do not change the ordering of the first 6 parameters to this
    # function; the C parser expects them in this order
    def __call__(self, asym_id, seq_id, auth_seq_num, pdb_ins_code,
                 pdb_strand_id):
        asym = self.sysr.asym_units.get_by_id(asym_id)
        seq_id = self.get_int(seq_id)
        if pdb_strand_id not in (None, ihm.unknown, asym_id):
            asym._strand_id = pdb_strand_id
        auth_seq_num = self.get_int_or_string(auth_seq_num)
        # Note any residues that have different seq_id and auth_seq_id
        if seq_id is not None and auth_seq_num is not None \
           and (seq_id != auth_seq_num
                or pdb_ins_code not in (None, ihm.unknown)):
            if asym.auth_seq_id_map == 0:
                asym.auth_seq_id_map = {}
            asym.auth_seq_id_map[seq_id] = auth_seq_num, pdb_ins_code

    def finalize(self):
        for asym in self.sysr.system.asym_units:
            # If every residue in auth_seq_id_map is offset by the same
            # amount, and no insertion codes, replace the map with a
            # simple offset
            offset = self._get_auth_seq_id_offset(asym)
            if offset is not None:
                asym.auth_seq_id_map = offset

    def _get_auth_seq_id_offset(self, asym):
        """Get the offset from seq_id to auth_seq_id. Return None if no
           consistent offset exists."""
        # Do nothing if the entity is not polymeric
        if asym.entity is None or not asym.entity.is_polymeric():
            return
        # Do nothing if no map exists
        if asym.auth_seq_id_map == 0:
            return
        rng = asym.seq_id_range
        offset = None
        for seq_id in range(rng[0], rng[1] + 1):
            # If a residue isn't in the map, it has an effective offset of 0,
            # which has to be inconsistent (since everything in the map has
            # a nonzero offset by construction)
            if seq_id not in asym.auth_seq_id_map:
                return
            auth_seq_id, ins_code = asym.auth_seq_id_map[seq_id]
            # If auth_seq_id is a string, we can't use any offset
            if not isinstance(auth_seq_id, int):
                return
            # If insertion codes are provided, we can't use any offset
            if ins_code not in (None, ihm.unknown):
                return
            this_offset = auth_seq_id - seq_id
            if offset is None:
                offset = this_offset
            elif offset != this_offset:
                # Offset is inconsistent
                return
        return offset


class _NonPolySchemeHandler(Handler):
    category = '_pdbx_nonpoly_scheme'

    def __call__(self, asym_id, entity_id, auth_seq_num, mon_id, pdb_ins_code,
                 pdb_strand_id):
        entity = self.sysr.entities.get_by_id(entity_id)
        # nonpolymer entities generally have information on their chemical
        # component in pdbx_entity_nonpoly, but if that's missing, at least
        # get the name from mon_id here, so that we don't end up with an
        # empty sequence
        if len(entity.sequence) == 0 and mon_id:
            if mon_id == 'HOH':
                s = ihm.WaterChemComp()
            else:
                s = ihm.NonPolymerChemComp(
                    mon_id, name=entity.description)
            entity.sequence.append(s)
        asym = self.sysr.asym_units.get_by_id(asym_id)
        if pdb_strand_id not in (None, ihm.unknown, asym_id):
            asym._strand_id = pdb_strand_id
        # todo: handle multiple instances (e.g. water)
        auth_seq_num = self.get_int_or_string(auth_seq_num)
        if auth_seq_num != 1 or pdb_ins_code not in (None, ihm.unknown):
            asym.auth_seq_id_map = {1: (auth_seq_num, pdb_ins_code)}


class _CrossLinkListHandler(Handler):
    category = '_ihm_cross_link_list'
    ignored_keywords = ['entity_description_1', 'entity_description_2',
                        'comp_id_1', 'comp_id_2']
    _linkers_by_name = None

    def __init__(self, *args):
        super(_CrossLinkListHandler, self).__init__(*args)
        self._seen_group_ids = set()
        self._linker_type = {}

    def _get_linker_by_name(self, name):
        """Look up old-style linker, by name rather than descriptor"""
        if self._linkers_by_name is None:
            self._linkers_by_name \
                = dict((x[1].auth_name, x[1])
                       for x in inspect.getmembers(ihm.cross_linkers)
                       if isinstance(x[1], ihm.ChemDescriptor))
        if name not in self._linkers_by_name:
            self._linkers_by_name[name] = ihm.ChemDescriptor(name)
        return self._linkers_by_name[name]

    def __call__(self, dataset_list_id, linker_chem_comp_descriptor_id,
                 group_id, id, entity_id_1, entity_id_2, seq_id_1, seq_id_2,
                 linker_type, details):
        dataset = self.sysr.datasets.get_by_id_or_none(dataset_list_id)
        if linker_chem_comp_descriptor_id is None and linker_type is not None:
            linker = self._get_linker_by_name(linker_type)
        else:
            linker = self.sysr.chem_descriptors.get_by_id(
                linker_chem_comp_descriptor_id)
            if linker_type:
                self._linker_type[linker] = linker_type
        # Group all crosslinks with same dataset and linker in one
        # CrossLinkRestraint object
        r = self.sysr.xl_restraints.get_by_attrs(dataset, linker)

        xl_group = self.sysr.experimental_xl_groups.get_by_id(group_id)
        xl = self.sysr.experimental_xls.get_by_id(id)

        if group_id not in self._seen_group_ids:
            self._seen_group_ids.add(group_id)
            r.experimental_cross_links.append(xl_group)
        xl_group.append(xl)
        xl.residue1 = self._get_entity_residue(entity_id_1, seq_id_1)
        xl.residue2 = self._get_entity_residue(entity_id_2, seq_id_2)
        xl.details = details

    def _get_entity_residue(self, entity_id, seq_id):
        entity = self.sysr.entities.get_by_id(entity_id)
        return entity.residue(int(seq_id))

    def finalize(self):
        # If any ChemDescriptor has an empty name, fill it in using linker_type
        for d in self.system.orphan_chem_descriptors:
            if d.auth_name is None:
                d.auth_name = self._linker_type.get(d)


class _CrossLinkRestraintHandler(Handler):
    category = '_ihm_cross_link_restraint'

    _cond_map = {'ALL': True, 'ANY': False, None: None}
    _distance_map = {'harmonic': ihm.restraint.HarmonicDistanceRestraint,
                     'lower bound': ihm.restraint.LowerBoundDistanceRestraint,
                     'upper bound': ihm.restraint.UpperBoundDistanceRestraint}

    # Map granularity to corresponding subclass
    _type_map = dict((x[1].granularity.lower(), x[1])
                     for x in inspect.getmembers(ihm.restraint,
                                                 inspect.isclass)
                     if issubclass(x[1], ihm.restraint.CrossLink)
                     and x[1] is not ihm.restraint.CrossLink)

    def __call__(self, model_granularity, id, group_id, asym_id_1, asym_id_2,
                 restraint_type, distance_threshold,
                 conditional_crosslink_flag, atom_id_1, atom_id_2, psi,
                 sigma_1, sigma_2):
        typ = (model_granularity or 'other').lower()
        xl = self.sysr.cross_links.get_by_id(
            id, self._type_map.get(typ, ihm.restraint.ResidueCrossLink))
        ex_xl = self.sysr.experimental_xls.get_by_id(group_id)

        xl.experimental_cross_link = ex_xl
        xl.asym1 = self.sysr.asym_units.get_by_id(asym_id_1)
        xl.asym2 = self.sysr.asym_units.get_by_id(asym_id_2)
        # todo: handle unknown restraint type
        _distcls = self._distance_map[restraint_type.lower()]
        xl.distance = _distcls(float(distance_threshold))
        xl.restrain_all = self._cond_map[conditional_crosslink_flag]
        if isinstance(xl, ihm.restraint.AtomCrossLink):
            xl.atom1 = atom_id_1
            xl.atom2 = atom_id_2
        xl.psi = self.get_float(psi)
        xl.sigma1 = self.get_float(sigma_1)
        xl.sigma2 = self.get_float(sigma_2)

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


class _CrossLinkPseudoSiteHandler(Handler):
    category = '_ihm_cross_link_pseudo_site'

    def __call__(self, id, restraint_id, cross_link_partner, pseudo_site_id,
                 model_id):
        xlps = self.sysr.cross_link_pseudo_sites.get_by_id(id)
        xlps.site = self.sysr.pseudo_sites.get_by_id(pseudo_site_id)
        xlps.model = self.sysr.models.get_by_id_or_none(model_id)

        xl = self.sysr.cross_links.get_by_id(restraint_id)
        partner = self.get_int(cross_link_partner)
        if partner == 2:
            if getattr(xl, 'pseudo2', None) is None:
                xl.pseudo2 = []
            xl.pseudo2.append(xlps)
        else:
            if getattr(xl, 'pseudo1', None) is None:
                xl.pseudo1 = []
            xl.pseudo1.append(xlps)


class _CrossLinkResultHandler(Handler):
    category = '_ihm_cross_link_result_parameters'
    ignored_keywords = ['ordinal_id']

    def __call__(self, restraint_id, model_id, psi, sigma_1, sigma_2):
        xl = self.sysr.cross_links.get_by_id(restraint_id)
        model = self.sysr.models.get_by_id(model_id)
        xl.fits[model] = ihm.restraint.CrossLinkFit(
            psi=self.get_float(psi), sigma1=self.get_float(sigma_1),
            sigma2=self.get_float(sigma_2))


class _OrderedEnsembleHandler(Handler):
    category = '_ihm_ordered_ensemble'

    def __call__(self, process_id, step_id, model_group_id_begin,
                 model_group_id_end, edge_description, ordered_by,
                 process_description, step_description):
        proc = self.sysr.ordered_procs.get_by_id(process_id)
        # todo: will this work with multiple processes?
        step = self.sysr.ordered_steps.get_by_id(step_id)
        edge = ihm.model.ProcessEdge(
            self.sysr.model_groups.get_by_id(model_group_id_begin),
            self.sysr.model_groups.get_by_id(model_group_id_end))
        self.copy_if_present(
            edge, locals(), mapkeys={'edge_description': 'description'})
        step.append(edge)

        if step_id not in [s._id for s in proc.steps]:
            proc.steps.append(step)

        self.copy_if_present(
            proc, locals(), keys=('ordered_by',),
            mapkeys={'process_description': 'description'})
        self.copy_if_present(
            step, locals(), mapkeys={'step_description': 'description'})


class UnknownCategoryWarning(Warning):
    """Warning for unknown categories encountered in the file
       by :func:`read`"""
    pass


class UnknownKeywordWarning(Warning):
    """Warning for unknown keywords encountered in the file by :func:`read`"""
    pass


class _UnknownCategoryHandler(object):
    def __init__(self):
        self.reset()

    def reset(self):
        self._seen_categories = set()

    def __call__(self, catname, line):
        # Only warn about a given category once
        if catname in self._seen_categories:
            return
        self._seen_categories.add(catname)
        warnings.warn("Unknown category %s encountered%s - will be ignored"
                      % (catname, " on line %d" % line if line else ""),
                      UnknownCategoryWarning, stacklevel=2)


class _UnknownKeywordHandler(object):
    def add_category_handlers(self, handlers):
        self._ignored_keywords = dict((h.category,
                                       frozenset(h.ignored_keywords))
                                      for h in handlers)

    def __call__(self, catname, keyname, line):
        if keyname in self._ignored_keywords[catname]:
            return
        warnings.warn("Unknown keyword %s.%s encountered%s - will be ignored"
                      % (catname, keyname,
                         " on line %d" % line if line else ""),
                      UnknownKeywordWarning, stacklevel=2)


# FLR part
# Note: This Handler is only here, because the category is officially
# still in the flr dictionary.
class _FLRChemDescriptorHandler(_ChemDescriptorHandler):
    category = '_flr_chemical_descriptor'


class _FLRExperimentHandler(Handler):
    category = '_flr_experiment'

    def __call__(self, ordinal_id, id, instrument_id,
                 inst_setting_id, exp_condition_id,
                 sample_id, details):
        # Get the object or create the object
        experiment = self.sysr.flr_experiments.get_by_id(id)
        # Fill the object
        instrument = self.sysr.flr_instruments.get_by_id(instrument_id)
        inst_setting = self.sysr.flr_inst_settings.get_by_id(inst_setting_id)
        exp_condition = self.sysr.flr_exp_conditions.get_by_id(
            exp_condition_id)
        sample = self.sysr.flr_samples.get_by_id(sample_id)
        experiment.add_entry(instrument=instrument, inst_setting=inst_setting,
                             exp_condition=exp_condition, sample=sample,
                             details=details)


class _FLRInstSettingHandler(Handler):
    category = '_flr_inst_setting'

    def __call__(self, id, details):
        # Get the object or create the object
        cur_inst_setting = self.sysr.flr_inst_settings.get_by_id(id)
        # Set the variables
        self.copy_if_present(cur_inst_setting, locals(), keys=('details',))


class _FLRExpConditionHandler(Handler):
    category = '_flr_exp_condition'

    def __call__(self, id, details):
        # Get the object or create the object
        cur_exp_condition = self.sysr.flr_exp_conditions.get_by_id(id)
        # Set the variables
        self.copy_if_present(cur_exp_condition, locals(), keys=('details',))


class _FLRInstrumentHandler(Handler):
    category = '_flr_instrument'

    def __call__(self, id, details):
        # Get the object or create the object
        cur_instrument = self.sysr.flr_instruments.get_by_id(id)
        # Set the variables
        self.copy_if_present(cur_instrument, locals(), keys=('details',))


class _FLREntityAssemblyHandler(Handler):
    category = '_flr_entity_assembly'

    def __call__(self, ordinal_id, assembly_id, entity_id, num_copies):
        # Get the object or create the object
        a = self.sysr.flr_entity_assemblies.get_by_id(assembly_id)
        # Get the entity
        entity = self.sysr.entities.get_by_id(entity_id)
        # Add the entity to the entity assembly
        a.add_entity(entity=entity, num_copies=self.get_int(num_copies))


class _FLRSampleConditionHandler(Handler):
    category = '_flr_sample_condition'

    def __call__(self, id, details):
        # Get the object or create the object
        cur_sample_condition = self.sysr.flr_sample_conditions.get_by_id(id)
        # Set the variables
        self.copy_if_present(cur_sample_condition, locals(), keys=('details',))


class _FLRSampleHandler(Handler):
    category = '_flr_sample'

    def __call__(self, id, entity_assembly_id, num_of_probes,
                 sample_condition_id, sample_description, sample_details,
                 solvent_phase):
        sample = self.sysr.flr_samples.get_by_id(id)
        sample.entity_assembly \
            = self.sysr.flr_entity_assemblies.get_by_id(entity_assembly_id)
        sample.num_of_probes = self.get_int(num_of_probes)
        sample.condition = cond \
            = self.sysr.flr_sample_conditions.get_by_id(sample_condition_id)
        self.copy_if_present(sample, locals(), keys=('solvent_phase',),
                             mapkeys={'sample_description': 'description',
                                      'sample_details': 'details'})


class _FLRProbeListHandler(Handler):
    category = '_flr_probe_list'

    def __call__(self, probe_id, chromophore_name, reactive_probe_flag,
                 reactive_probe_name, probe_origin, probe_link_type):
        cur_probe = self.sysr.flr_probes.get_by_id(probe_id)
        reactive_probe_flag = self.get_bool(reactive_probe_flag)
        cur_probe.probe_list_entry = ihm.flr.ProbeList(
            chromophore_name=chromophore_name,
            reactive_probe_flag=reactive_probe_flag,
            reactive_probe_name=reactive_probe_name,
            probe_origin=probe_origin, probe_link_type=probe_link_type)


class _FLRSampleProbeDetailsHandler(Handler):
    category = '_flr_sample_probe_details'

    def __call__(self, sample_probe_id, sample_id, probe_id, fluorophore_type,
                 description, poly_probe_position_id):
        spd = self.sysr.flr_sample_probe_details.get_by_id(sample_probe_id)
        spd.sample = self.sysr.flr_samples.get_by_id(sample_id)
        spd.probe = self.sysr.flr_probes.get_by_id(probe_id)
        spd.poly_probe_position = self.sysr.flr_poly_probe_positions.get_by_id(
            poly_probe_position_id)
        spd.fluorophore_type = fluorophore_type
        spd.description = description


class _FLRProbeDescriptorHandler(Handler):
    category = '_flr_probe_descriptor'

    def __call__(self, probe_id, reactive_probe_chem_descriptor_id,
                 chromophore_chem_descriptor_id, chromophore_center_atom):
        react_cd = self.sysr.chem_descriptors.get_by_id_or_none(
            reactive_probe_chem_descriptor_id)
        chrom_cd = self.sysr.chem_descriptors.get_by_id_or_none(
            chromophore_chem_descriptor_id)
        cur_probe = self.sysr.flr_probes.get_by_id(probe_id)
        cur_probe.probe_descriptor = ihm.flr.ProbeDescriptor(
            reactive_probe_chem_descriptor=react_cd,
            chromophore_chem_descriptor=chrom_cd,
            chromophore_center_atom=chromophore_center_atom)


class _FLRPolyProbePositionHandler(Handler):
    category = '_flr_poly_probe_position'

    def _get_resatom(self, entity_id, asym_id, seq_id, atom_id):
        entity = self.sysr.entities.get_by_id(entity_id)
        asym = self.sysr.asym_units.get_by_id_or_none(asym_id)
        if asym is not None:
            asym.entity = entity
            asym.id = asym_id
        seq_id = self.get_int(seq_id)
        resatom = entity.residue(seq_id)
        if asym is not None:
            resatom.asym = asym
        if atom_id:
            resatom = resatom.atom(atom_id)
        return resatom

    def __call__(self, id, entity_id, asym_id, seq_id, atom_id,
                 mutation_flag, modification_flag, auth_name):
        ppos = self.sysr.flr_poly_probe_positions.get_by_id(id)
        ppos.resatom = self._get_resatom(entity_id, asym_id, seq_id, atom_id)
        ppos.mutation_flag = self.get_bool(mutation_flag)
        ppos.modification_flag = self.get_bool(modification_flag)
        ppos.auth_name = auth_name


class _FLRPolyProbePositionModifiedHandler(Handler):
    category = '_flr_poly_probe_position_modified'

    def __call__(self, id, chem_descriptor_id):
        ppos = self.sysr.flr_poly_probe_positions.get_by_id(id)
        ppos.modified_chem_descriptor = \
            self.sysr.chem_descriptors.get_by_id_or_none(chem_descriptor_id)


class _FLRPolyProbePositionMutatedHandler(Handler):
    category = '_flr_poly_probe_position_mutated'

    def __call__(self, id, chem_comp_id, atom_id):
        ppos = self.sysr.flr_poly_probe_positions.get_by_id(id)
        ppos.mutated_chem_comp_id = \
            self.sysr.chem_comps.get_by_id(chem_comp_id)


class _FLRPolyProbeConjugateHandler(Handler):
    category = '_flr_poly_probe_conjugate'

    def __call__(self, id, sample_probe_id, chem_descriptor_id,
                 ambiguous_stoichiometry_flag, probe_stoichiometry):
        ppc = self.sysr.flr_poly_probe_conjugates.get_by_id(id)
        ppc.sample_probe = self.sysr.flr_sample_probe_details.get_by_id(
            sample_probe_id)
        ppc.chem_descriptor = self.sysr.chem_descriptors.get_by_id(
            chem_descriptor_id)
        ppc.ambiguous_stoichiometry = self.get_bool(
            ambiguous_stoichiometry_flag)
        ppc.probe_stoichiometry = self.get_float(probe_stoichiometry)


class _FLRFretForsterRadiusHandler(Handler):
    category = '_flr_fret_forster_radius'

    def __call__(self, id, donor_probe_id, acceptor_probe_id, forster_radius,
                 reduced_forster_radius):
        ffr = self.sysr.flr_fret_forster_radius.get_by_id(id)
        ffr.donor_probe = self.sysr.flr_probes.get_by_id(donor_probe_id)
        ffr.acceptor_probe = self.sysr.flr_probes.get_by_id(acceptor_probe_id)
        ffr.forster_radius = self.get_float(forster_radius)
        ffr.reduced_forster_radius = self.get_float(reduced_forster_radius)


class _FLRFretCalibrationParametersHandler(Handler):
    category = '_flr_fret_calibration_parameters'

    def __call__(self, id, phi_acceptor, alpha, alpha_sd, gg_gr_ratio, beta,
                 gamma, delta, a_b):
        p = self.sysr.flr_fret_calibration_parameters.get_by_id(id)
        p.phi_acceptor = self.get_float(phi_acceptor)
        p.alpha = self.get_float(alpha)
        p.alpha_sd = self.get_float(alpha_sd)
        p.gg_gr_ratio = self.get_float(gg_gr_ratio)
        p.beta = self.get_float(beta)
        p.gamma = self.get_float(gamma)
        p.delta = self.get_float(delta)
        p.a_b = self.get_float(a_b)


class _FLRFretAnalysisHandler(Handler):
    category = '_flr_fret_analysis'

    def __call__(self, id, experiment_id, type,
                 sample_probe_id_1, sample_probe_id_2,
                 forster_radius_id, dataset_list_id,
                 external_file_id, software_id):
        f = self.sysr.flr_fret_analyses.get_by_id(id)
        f.experiment = self.sysr.flr_experiments.get_by_id(experiment_id)
        f.type = type
        f.sample_probe_1 = self.sysr.flr_sample_probe_details.get_by_id(
            sample_probe_id_1)
        f.sample_probe_2 = self.sysr.flr_sample_probe_details.get_by_id(
            sample_probe_id_2)
        f.forster_radius = self.sysr.flr_fret_forster_radius.get_by_id(
            forster_radius_id)
        f.dataset = self.sysr.datasets.get_by_id(dataset_list_id)
        f.external_file = self.sysr.external_files.get_by_id_or_none(
            external_file_id)
        f.software = self.sysr.software.get_by_id_or_none(software_id)


class _FLRFretAnalysisIntensityHandler(Handler):
    category = '_flr_fret_analysis_intensity'

    def __call__(self, ordinal_id, analysis_id,
                 calibration_parameters_id, donor_only_fraction,
                 chi_square_reduced, method_name, details):
        f = self.sysr.flr_fret_analyses.get_by_id(analysis_id)
        f.type = 'intensity-based'
        f.calibration_parameters = \
            self.sysr.flr_fret_calibration_parameters.get_by_id(
                calibration_parameters_id)
        f.donor_only_fraction = self.get_float(donor_only_fraction)
        f.chi_square_reduced = self.get_float(chi_square_reduced)
        f.method_name = method_name
        f.details = details


class _FLRFretAnalysisLifetimeHandler(Handler):
    category = '_flr_fret_analysis_lifetime'

    def __call__(self, ordinal_id, analysis_id,
                 reference_measurement_group_id, lifetime_fit_model_id,
                 donor_only_fraction, chi_square_reduced, method_name,
                 details):
        f = self.sysr.flr_fret_analyses.get_by_id(analysis_id)
        f.type = 'lifetime-based'
        f.ref_measurement_group \
            = self.sysr.flr_ref_measurement_groups.get_by_id(
                reference_measurement_group_id)
        f.lifetime_fit_model = self.sysr.flr_lifetime_fit_models.get_by_id(
            lifetime_fit_model_id)
        f.donor_only_fraction = self.get_float(donor_only_fraction)
        f.chi_square_reduced = self.get_float(chi_square_reduced)
        f.method_name = method_name
        f.details = details


class _FLRLifetimeFitModelHandler(Handler):
    category = '_flr_lifetime_fit_model'

    def __call__(self, id, name, description,
                 external_file_id, citation_id):
        f = self.sysr.flr_lifetime_fit_models.get_by_id(id)
        f.name = name
        f.description = description
        f.external_file = \
            self.sysr.external_files.get_by_id_or_none(external_file_id)
        f.citation = \
            self.sysr.citations.get_by_id_or_none(citation_id)


class _FLRRefMeasurementHandler(Handler):
    category = '_flr_reference_measurement'

    def __call__(self, id, reference_sample_probe_id,
                 num_species, details):
        r = self.sysr.flr_ref_measurements.get_by_id(id)
        r.ref_sample_probe = self.sysr.flr_sample_probe_details.get_by_id(
            reference_sample_probe_id)
        r.details = details


class _FLRRefMeasurementGroupHandler(Handler):
    category = '_flr_reference_measurement_group'

    def __call__(self, id, num_measurements, details):
        g = self.sysr.flr_ref_measurement_groups.get_by_id(id)
        g.details = details


class _FLRRefMeasurementGroupLinkHandler(Handler):
    category = '_flr_reference_measurement_group_link'

    def __call__(self, group_id, reference_measurement_id):
        g = self.sysr.flr_ref_measurement_groups.get_by_id(group_id)
        r = self.sysr.flr_ref_measurements.get_by_id(reference_measurement_id)
        g.add_ref_measurement(r)


class _FLRRefMeasurementLifetimeHandler(Handler):
    category = '_flr_reference_measurement_lifetime'

    def __call__(self, ordinal_id, reference_measurement_id,
                 species_name, species_fraction, lifetime):
        lf = self.sysr.flr_ref_measurement_lifetimes.get_by_id(ordinal_id)
        lf.species_name = species_name
        lf.species_fraction = self.get_float(species_fraction)
        lf.lifetime = self.get_float(lifetime)

        # Add the lifetime to the reference measurement
        r = self.sysr.flr_ref_measurements.get_by_id(reference_measurement_id)
        r.add_lifetime(lf)


class _FLRPeakAssignmentHandler(Handler):
    category = '_flr_peak_assignment'

    def __call__(self, id, method_name, details):
        p = self.sysr.flr_peak_assignments.get_by_id(id)
        self.copy_if_present(p, locals(), keys=('method_name', 'details'))


class _FLRFretDistanceRestraintHandler(Handler):
    category = '_flr_fret_distance_restraint'

    def __call__(self, ordinal_id, id, group_id, sample_probe_id_1,
                 sample_probe_id_2, state_id, analysis_id, distance,
                 distance_error_plus, distance_error_minus, distance_type,
                 population_fraction, peak_assignment_id):
        r = self.sysr.flr_fret_distance_restraints.get_by_id(id)
        r.sample_probe_1 = self.sysr.flr_sample_probe_details.get_by_id(
            sample_probe_id_1)
        r.sample_probe_2 = self.sysr.flr_sample_probe_details.get_by_id(
            sample_probe_id_2)
        r.state = self.sysr.states.get_by_id_or_none(state_id)
        r.analysis = self.sysr.flr_fret_analyses.get_by_id(analysis_id)
        r.peak_assignment = self.sysr.flr_peak_assignments.get_by_id(
            peak_assignment_id)
        r.distance = self.get_float(distance)
        r.distance_error_plus = self.get_float(distance_error_plus)
        r.distance_error_minus = self.get_float(distance_error_minus)
        r.distance_type = distance_type
        r.population_fraction = self.get_float(population_fraction)

        # also create the fret_distance_restraint_group
        rg = self.sysr.flr_fret_distance_restraint_groups.get_by_id(group_id)
        rg.add_distance_restraint(r)


class _FLRFretModelQualityHandler(Handler):
    category = '_flr_fret_model_quality'

    def __call__(self, model_id, chi_square_reduced, dataset_group_id,
                 method, details):
        q = self.sysr.flr_fret_model_qualities.get_by_id(model_id)
        q.model = self.sysr.models.get_by_id(model_id)
        q.chi_square_reduced = self.get_float(chi_square_reduced)
        q.dataset_group = self.sysr.dataset_groups.get_by_id(dataset_group_id)
        self.copy_if_present(q, locals(), keys=('method', 'details'))


class _FLRFretModelDistanceHandler(Handler):
    category = '_flr_fret_model_distance'

    def __call__(self, id, restraint_id, model_id, distance,
                 distance_deviation):
        md = self.sysr.flr_fret_model_distances.get_by_id(id)
        md.restraint = self.sysr.flr_fret_distance_restraints.get_by_id(
            restraint_id)
        md.model = self.sysr.models.get_by_id(model_id)
        md.distance = self.get_float(distance)
        md.distance_deviation = self.get_float(distance_deviation)
        # todo: this will fail if we haven't read the restraint category
        # yet (should be in finalize instead)
        md.calculate_deviation()


class _FLRFPSGlobalParameterHandler(Handler):
    category = '_flr_fps_global_parameter'

    def __call__(self, id, forster_radius_value,
                 conversion_function_polynom_order, repetition,
                 av_grid_rel, av_min_grid_a, av_allowed_sphere,
                 av_search_nodes, av_e_samples_k, sim_viscosity_adjustment,
                 sim_dt_adjustment, sim_max_iter_k, sim_max_force,
                 sim_clash_tolerance_a, sim_reciprocal_kt, sim_clash_potential,
                 convergence_e, convergence_k, convergence_f, convergence_t):
        p = self.sysr.flr_fps_global_parameters.get_by_id(id)
        p.forster_radius = self.get_float(forster_radius_value)
        p.conversion_function_polynom_order = self.get_int(
            conversion_function_polynom_order)
        p.repetition = self.get_int(repetition)
        p.av_grid_rel = self.get_float(av_grid_rel)
        p.av_min_grid_a = self.get_float(av_min_grid_a)
        p.av_allowed_sphere = self.get_float(av_allowed_sphere)
        p.av_search_nodes = self.get_int(av_search_nodes)
        p.av_e_samples_k = self.get_float(av_e_samples_k)
        p.sim_viscosity_adjustment = self.get_float(sim_viscosity_adjustment)
        p.sim_dt_adjustment = self.get_float(sim_dt_adjustment)
        p.sim_max_iter_k = self.get_float(sim_max_iter_k)
        p.sim_max_force = self.get_float(sim_max_force)
        p.sim_clash_tolerance_a = self.get_float(sim_clash_tolerance_a)
        p.sim_reciprocal_kt = self.get_float(sim_reciprocal_kt)
        p.sim_clash_potential = sim_clash_potential
        p.convergence_e = self.get_float(convergence_e)
        p.convergence_k = self.get_float(convergence_k)
        p.convergence_f = self.get_float(convergence_f)
        p.convergence_t = self.get_float(convergence_t)


class _FLRFPSModelingHandler(Handler):
    category = '_flr_fps_modeling'

    def __call__(self, id, ihm_modeling_protocol_ordinal_id,
                 restraint_group_id, global_parameter_id,
                 probe_modeling_method, details):
        m = self.sysr.flr_fps_modeling.get_by_id(id)
        m.protocol = self.sysr.protocols.get_by_id(
            ihm_modeling_protocol_ordinal_id)
        m.restraint_group = \
            self.sysr.flr_fret_distance_restraint_groups.get_by_id(
                restraint_group_id)
        m.global_parameter = self.sysr.flr_fps_global_parameters.get_by_id(
            global_parameter_id)
        self.copy_if_present(m, locals(),
                             keys=('probe_modeling_method', 'details'))


class _FLRFPSAVParameterHandler(Handler):
    category = '_flr_fps_av_parameter'

    def __call__(self, id, num_linker_atoms, linker_length, linker_width,
                 probe_radius_1, probe_radius_2, probe_radius_3):
        p = self.sysr.flr_fps_av_parameters.get_by_id(id)
        p.num_linker_atoms = self.get_int(num_linker_atoms)
        p.linker_length = self.get_float(linker_length)
        p.linker_width = self.get_float(linker_width)
        p.probe_radius_1 = self.get_float(probe_radius_1)
        p.probe_radius_2 = self.get_float(probe_radius_2)
        p.probe_radius_3 = self.get_float(probe_radius_3)


class _FLRFPSAVModelingHandler(Handler):
    category = '_flr_fps_av_modeling'

    def __call__(self, id, sample_probe_id, fps_modeling_id, parameter_id):
        m = self.sysr.flr_fps_av_modeling.get_by_id(id)
        m.fps_modeling = self.sysr.flr_fps_modeling.get_by_id(fps_modeling_id)
        m.sample_probe = self.sysr.flr_sample_probe_details.get_by_id(
            sample_probe_id)
        m.parameter = self.sysr.flr_fps_av_parameters.get_by_id(parameter_id)


class _FLRFPSMPPHandler(Handler):
    category = '_flr_fps_mean_probe_position'

    def __call__(self, id, sample_probe_id, mpp_xcoord, mpp_ycoord,
                 mpp_zcoord):
        p = self.sysr.flr_fps_mean_probe_positions.get_by_id(id)
        p.sample_probe = self.sysr.flr_sample_probe_details.get_by_id(
            sample_probe_id)
        p.x = self.get_float(mpp_xcoord)
        p.y = self.get_float(mpp_ycoord)
        p.z = self.get_float(mpp_zcoord)


class _FLRFPSMPPAtomPositionHandler(Handler):
    category = '_flr_fps_mpp_atom_position'

    def __call__(self, id, group_id, seq_id, atom_id, asym_id, xcoord,
                 ycoord, zcoord):
        asym = self.sysr.asym_units.get_by_id(asym_id)
        seq_id = self.get_int(seq_id)

        p = self.sysr.flr_fps_mpp_atom_positions.get_by_id(id)
        p.atom = asym.residue(seq_id).atom(atom_id)
        p.x = self.get_float(xcoord)
        p.y = self.get_float(ycoord)
        p.z = self.get_float(zcoord)

        g = self.sysr.flr_fps_mpp_atom_position_groups.get_by_id(group_id)
        g.add_atom_position(p)


class _FLRFPSMPPModelingHandler(Handler):
    category = '_flr_fps_mpp_modeling'

    def __call__(self, ordinal_id, fps_modeling_id, mpp_id,
                 mpp_atom_position_group_id):
        m = self.sysr.flr_fps_mpp_modeling.get_by_id(ordinal_id)
        m.fps_modeling = self.sysr.flr_fps_modeling.get_by_id(fps_modeling_id)
        m.mpp = self.sysr.flr_fps_mean_probe_positions.get_by_id(mpp_id)
        m.mpp_atom_position_group = \
            self.sysr.flr_fps_mpp_atom_position_groups.get_by_id(
                mpp_atom_position_group_id)


_flr_handlers = [_FLRChemDescriptorHandler, _FLRInstSettingHandler,
                 _FLRExpConditionHandler, _FLRInstrumentHandler,
                 _FLRSampleConditionHandler, _FLREntityAssemblyHandler,
                 _FLRSampleHandler, _FLRExperimentHandler,
                 _FLRProbeListHandler, _FLRProbeDescriptorHandler,
                 _FLRPolyProbePositionHandler,
                 _FLRPolyProbePositionModifiedHandler,
                 _FLRPolyProbePositionMutatedHandler,
                 _FLRSampleProbeDetailsHandler, _FLRPolyProbeConjugateHandler,
                 _FLRFretForsterRadiusHandler,
                 _FLRFretCalibrationParametersHandler, _FLRFretAnalysisHandler,
                 _FLRFretAnalysisIntensityHandler,
                 _FLRFretAnalysisLifetimeHandler, _FLRLifetimeFitModelHandler,
                 _FLRRefMeasurementHandler, _FLRRefMeasurementGroupHandler,
                 _FLRRefMeasurementGroupLinkHandler,
                 _FLRRefMeasurementLifetimeHandler, _FLRPeakAssignmentHandler,
                 _FLRFretDistanceRestraintHandler, _FLRFretModelQualityHandler,
                 _FLRFretModelDistanceHandler, _FLRFPSGlobalParameterHandler,
                 _FLRFPSModelingHandler, _FLRFPSAVParameterHandler,
                 _FLRFPSAVModelingHandler, _FLRFPSMPPHandler,
                 _FLRFPSMPPAtomPositionHandler, _FLRFPSMPPModelingHandler]


class Variant(object):
    """Utility class to select the type of file to read with :func:`read`."""

    #: Class to track global file information, e.g. :class:`SystemReader`
    system_reader = None

    def get_handlers(self, sysr):
        """Get the :class:`Handler` objects to use to parse input.

           :param sysr: class to track global file information.
           :type sysr: :class:`SystemReader`
           :return: a list of :class:`Handler` objects.
        """
        pass

    def get_audit_conform_handler(self, sysr):
        """Get a :class:`Handler` to check the audit_conform table.
           If :func:`read` is called with ``reject_old_file=True``, this
           handler is used to check the audit_conform table and reject the
           file if it is deemed to be too old.

           :param sysr: class to track global file information.
           :type sysr: :class:`SystemReader`
           :return: a suitable handler.
           :rtype: :class:`Handler`
        """
        pass


class IHMVariant(Variant):
    """Used to select typical PDBx/IHM file input. See :func:`read`."""
    system_reader = SystemReader

    _handlers = [
        _StructHandler, _SoftwareHandler, _CitationHandler,
        _AuditAuthorHandler, _GrantHandler, _CitationAuthorHandler,
        _ChemCompHandler, _ChemDescriptorHandler, _EntityHandler,
        _EntitySrcNatHandler, _EntitySrcGenHandler, _EntitySrcSynHandler,
        _StructRefHandler, _StructRefSeqHandler, _StructRefSeqDifHandler,
        _EntityPolyHandler, _EntityPolySeqHandler, _EntityNonPolyHandler,
        _EntityPolySegmentHandler, _StructAsymHandler, _AssemblyDetailsHandler,
        _AssemblyHandler, _ExtRefHandler, _ExtFileHandler, _DatasetListHandler,
        _DatasetGroupHandler, _DatasetGroupLinkHandler, _DatasetExtRefHandler,
        _DatasetDBRefHandler, _DataTransformationHandler,
        _RelatedDatasetsHandler, _ModelRepresentationHandler,
        _ModelRepresentationDetailsHandler, _StartingModelDetailsHandler,
        _StartingComputationalModelsHandler, _StartingComparativeModelsHandler,
        _ProtocolHandler, _ProtocolDetailsHandler, _PostProcessHandler,
        _ModelListHandler, _ModelGroupHandler, _ModelGroupLinkHandler,
        _MultiStateHandler, _MultiStateLinkHandler, _EnsembleHandler,
        _DensityHandler, _SubsampleHandler, _EM3DRestraintHandler,
        _EM2DRestraintHandler, _EM2DFittingHandler, _SASRestraintHandler,
        _SphereObjSiteHandler, _AtomSiteHandler, _FeatureListHandler,
        _PolyResidueFeatureHandler, _PolyAtomFeatureHandler,
        _NonPolyFeatureHandler, _PseudoSiteFeatureHandler, _PseudoSiteHandler,
        _DerivedDistanceRestraintHandler, _PredictedContactRestraintHandler,
        _CenterHandler, _TransformationHandler, _GeometricObjectHandler,
        _SphereHandler, _TorusHandler, _HalfTorusHandler, _AxisHandler,
        _PlaneHandler, _GeometricRestraintHandler, _PolySeqSchemeHandler,
        _NonPolySchemeHandler, _CrossLinkListHandler,
        _CrossLinkRestraintHandler, _CrossLinkPseudoSiteHandler,
        _CrossLinkResultHandler, _StartingModelSeqDifHandler,
        _OrderedEnsembleHandler]

    def get_handlers(self, sysr):
        return [h(sysr) for h in self._handlers + _flr_handlers]

    def get_audit_conform_handler(self, sysr):
        return _AuditConformHandler(sysr)


def read(fh, model_class=ihm.model.Model, format='mmCIF', handlers=[],
         warn_unknown_category=False, warn_unknown_keyword=False,
         read_starting_model_coord=True,
         starting_model_class=ihm.startmodel.StartingModel,
         reject_old_file=False, variant=IHMVariant):
    """Read data from the file handle `fh`.

       Note that the reader currently expects to see a file compliant
       with the PDBx and/or IHM dictionaries. It is not particularly tolerant
       of noncompliant or incomplete files, and will probably throw an
       exception rather than warning about and trying to handle such files.
       Please `open an issue <https://github.com/ihmwg/python-ihm/issues>`_
       if you encounter such a problem.

       Files can be read in either the text-based mmCIF format or the BinaryCIF
       format. The mmCIF reader works by breaking the file into tokens, and
       using this stream of tokens to populate Python data structures.
       Two tokenizers are available: a pure Python implementation and a
       C-accelerated version. The C-accelerated version is much faster and
       so is used if built. The BinaryCIF reader needs the msgpack Python
       module to function.

       The file handle should be opened in text mode for mmCIF files.
       Traditionally, mmCIF files used ASCII encoding. More and more
       recent files are UTF-8 encoded instead, but some use other encodings
       such as latin-1. To handle most current files use something like::

           try:
               with open('input.cif', encoding='utf-8') as fh:
                   systems = ihm.reader.read(fh)
           except UnicodeDecodeError:
               with open('input.cif', encoding='latin-1') as fh:
                   systems = ihm.reader.read(fh)

       The file handle should be opened in binary mode for BinaryCIF files::

           with open('input.bcif', 'rb') as fh:
               systems = ihm.reader.read(fh, format='BCIF')

       :param file fh: The file handle to read from. (For BinaryCIF files,
              the file should be opened in binary mode. For mmCIF files,
              files opened in binary mode with Python 3 will be treated as
              if they are Latin-1-encoded.)
       :param model_class: The class to use to store model information (such
              as coordinates). For use with other software, it is recommended
              to subclass :class:`ihm.model.Model` and override
              :meth:`~ihm.model.Model.add_sphere` and/or
              :meth:`~ihm.model.Model.add_atom`, and provide that subclass
              here. See :meth:`ihm.model.Model.get_spheres` for more
              information.
       :param str format: The format of the file. This can be 'mmCIF' (the
              default) for the (text-based) mmCIF format or 'BCIF' for
              BinaryCIF.
       :param list handlers: A list of :class:`Handler` classes (not objects).
              These can be used to read extra categories from the file.
       :param bool warn_unknown_category: if set, emit an
              :exc:`UnknownCategoryWarning` for each unknown category
              encountered in the file.
       :param bool warn_unknown_keyword: if set, emit an
              :exc:`UnknownKeywordWarning` for each unknown keyword
              (within an otherwise-handled category) encountered in the file.
       :param bool read_starting_model_coord: if set, read coordinates for
              starting models, if provided in the file.
       :param starting_model_class: The class to use to store starting model
              information. If `read_starting_model_coord` is also set, it
              is recommended to subclass :class:`ihm.startmodel.StartingModel`
              and override :meth:`~ihm.startmodel.StartingModel.add_atom`
              and/or :meth:`~ihm.startmodel.StartingModel.add_seq_dif`.
       :param bool reject_old_file: If True, raise an
              :exc:`ihm.reader.OldFileError` if the file conforms to an
              older version of the dictionary than this library supports
              (by default the library will read what it can from the file).
       :param variant: A class or object that selects the type of file to
              read. This primarily controls the set of tables that are
              read from the file. In most cases the default
              :class:`IHMVariant` should be used.
       :type variant: :class:`Variant`
       :return: A list of :class:`ihm.System` objects.
    """
    if isinstance(variant, type):
        variant = variant()
    systems = []
    reader_map = {'mmCIF': ihm.format.CifReader,
                  'BCIF': ihm.format_bcif.BinaryCifReader}

    uchandler = _UnknownCategoryHandler() if warn_unknown_category else None
    ukhandler = _UnknownKeywordHandler() if warn_unknown_keyword else None

    r = reader_map[format](fh, {}, unknown_category_handler=uchandler,
                           unknown_keyword_handler=ukhandler)
    while True:
        s = variant.system_reader(model_class, starting_model_class)
        hs = variant.get_handlers(s) + [h(s) for h in handlers]
        if reject_old_file:
            hs.append(variant.get_audit_conform_handler(s))
        if read_starting_model_coord:
            hs.append(_StartingModelCoordHandler(s))
        if uchandler:
            uchandler.reset()
        if ukhandler:
            ukhandler.add_category_handlers(hs)
        r.category_handler = dict((h.category, h) for h in hs)
        more_data = r.read_file()
        for h in hs:
            h.finalize()
        s.finalize()
        systems.append(s.system)
        if not more_data:
            break

    return systems
