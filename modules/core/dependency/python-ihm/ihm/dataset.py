# coding=utf-8

"""Classes for handling experimental datasets used by mmCIF models.
"""


class Dataset(object):
    """A set of input data, for example, a crystal structure or EM map.

       :param location: a pointer to where the
              dataset is stored. This is usually a subclass of
              :class:`~ihm.location.DatabaseLocation` if the dataset is
              deposited in a database such as PDB or EMDB, or
              :class:`~ihm.location.InputFileLocation` if the dataset is stored
              in an external file.
       :type location: :class:`ihm.location.Location`
       :param str details: Text giving more information about the dataset.
    """

    _eq_keys = ['location']

    # Datasets compare equal iff they are the same class and have the
    # same attributes
    def _eq_vals(self):
        return tuple([self.__class__]
                     + [getattr(self, x) for x in self._eq_keys])

    def __eq__(self, other):
        return self._eq_vals() == other._eq_vals()

    def __hash__(self):
        return hash(self._eq_vals())

    data_type = 'Other'

    def __init__(self, location, details=None):
        self.location, self.details = location, details

        #: A list of :class:`Dataset` and/or :class:`TransformedDataset`
        #: objects from which this one was derived.
        #: For example, a 3D EM map may be derived from a set of 2D images.
        self.parents = []

    def add_primary(self, dataset):
        """Add another Dataset from which this one was ultimately derived,
           i.e. it is added as a parent, unless a parent already exists,
           in which case it is added as a grandparent, and so on."""
        root = self
        while root.parents:
            if len(root.parents) > 1:
                raise ValueError("This dataset has multiple parents - don't "
                                 "know which one to add to")
            root = root.parents[0]
        root.parents.append(dataset)


class TransformedDataset(object):
    """A :class:`Dataset` that should be rotated or translated before using.
       This is typically used for derived datasets
       (see :attr:`Dataset.parents`) where the derived dataset lies in a
       different dataset from the parent (for example, it was moved to better
       align with the model's reference frame or other experimental data).
       The transformation that places the derived dataset on the parent
       is recorded here.

       :param dataset: The (parent) dataset.
       :type dataset: :class:`Dataset`
       :param transform: The rotation and translation that places a
              derived dataset on this dataset.
       :type transform: :class:`ihm.geometry.Transformation`
    """
    def __init__(self, dataset, transform):
        self.dataset, self.transform = dataset, transform


class DatasetGroup(list):
    """A set of :class:`Dataset` objects that are handled together.
       This is implemented as a simple list.

       :param sequence elements: Initial set of datasets.
       :param str name: Short text name of this group.
       :param str application: Text that shows how this group is used.
       :param str details: Longer text that describes this group.

       Normally a group is passed to one or more
       :class:`~ihm.protocol.Protocol` or :class:`~ihm.analysis.Analysis`
       objects, although unused groups can still be included in the file
       if desired by adding them to :attr:`ihm.System.orphan_dataset_groups`.
    """

    # For backwards compatibility with earlier versions of this class which
    # didn't specify name/application/details
    name = application = details = None

    def __init__(self, elements=(), name=None, application=None, details=None):
        super(DatasetGroup, self).__init__(elements)
        self.name, self.application = name, application
        self.details = details


class CXMSDataset(Dataset):
    """Processed cross-links from a CX-MS experiment"""
    data_type = 'CX-MS data'


class MassSpecDataset(Dataset):
    """Raw mass spectrometry files such as peaklists"""
    data_type = 'Mass Spectrometry data'


class HDXDataset(Dataset):
    """Data from a hydrogen/deuterium exchange experiment"""
    data_type = 'H/D exchange data'


class PDBDataset(Dataset):
    """An experimentally-determined 3D structure as a set of a coordinates,
       usually in a PDB file"""
    data_type = 'Experimental model'


class ComparativeModelDataset(Dataset):
    """A 3D structure determined by comparative modeling"""
    data_type = 'Comparative model'


class IntegrativeModelDataset(Dataset):
    """A 3D structure determined by integrative modeling"""
    data_type = 'Integrative model'


class DeNovoModelDataset(Dataset):
    """A 3D structure determined by de novo modeling"""
    data_type = 'De Novo model'


class NMRDataset(Dataset):
    """A nuclear magnetic resonance (NMR) dataset"""
    data_type = 'NMR data'


class MutagenesisDataset(Dataset):
    """Mutagenesis data"""
    data_type = 'Mutagenesis data'


class EMDensityDataset(Dataset):
    """A 3D electron microscopy dataset"""
    data_type = '3DEM volume'


class EMMicrographsDataset(Dataset):
    """Raw 2D electron micrographs"""
    data_type = 'EM raw micrographs'


class EM2DClassDataset(Dataset):
    """2DEM class average"""
    data_type = '2DEM class average'


class SASDataset(Dataset):
    """SAS data"""
    data_type = 'SAS data'


class FRETDataset(Dataset):
    """Data from a Förster resonance energy transfer (FRET) experiment"""
    data_type = 'Single molecule FRET data'


class YeastTwoHybridDataset(Dataset):
    """Yeast two-hybrid data"""
    data_type = 'Yeast two-hybrid screening data'


class GeneticInteractionsDataset(Dataset):
    """Quantitative measurements of genetic interactions"""
    data_type = 'Quantitative measurements of genetic interactions'
