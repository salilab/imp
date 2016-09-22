"""@namespace IMP.pmi.metadata
Classes for attaching metadata to PMI objects.
"""

from __future__ import print_function, division
import os

class Metadata(object):
    """Base class for all metadata"""
    pass


class RootMetadata(Metadata):
    """Metadata that only makes sense for the top-level PMI object."""
    pass


class Software(RootMetadata):
    """Software (other than IMP) used as part of the modeling protocol."""
    def __init__(self, name, classification, description, url, type='program',
                 version=None):
        self.name = name
        self.classification = classification
        self.description = description
        self.url = url
        self.type = type
        self.version = version


class Citation(RootMetadata):
    """A publication that describes the modeling."""
    def __init__(self, pmid, title, journal, volume, page_range, year, authors,
                 doi):
        self.title, self.journal, self.volume = title, journal, volume
        self.page_range, self.year = page_range, year
        self.pmid, self.authors, self.doi = pmid, authors, doi


class Dataset(Metadata):
    """A set of input data, for example, a crystal structure or EM map."""

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

    _data_type = 'unspecified'
    def __init__(self, location):
        self.location = location
        self._primaries = {}

    def add_primary(self, dataset):
        """Add another Dataset from which this one was derived.
           For example, a 3D EM map may be derived from a set of 2D images."""
        self._primaries[dataset] = None

class CXMSDataset(Dataset):
    """Processed crosslinks from a CX-MS experiment"""
    _data_type = 'CX-MS data'

class MassSpecDataset(Dataset):
    """Raw mass spectrometry files such as peaklists"""
    _data_type = 'Mass Spectrometry data'

class EMDensityDataset(Dataset):
    """A 3D electron microscopy dataset"""
    _data_type = '3DEM volume'

class PDBDataset(Dataset):
    """An experimentally-determined 3D structure as a set of a coordinates,
       usually in a PDB file"""
    _data_type = 'Experimental model'

class ComparativeModelDataset(Dataset):
    """A 3D structure determined by comparative modeling"""
    _data_type = 'Comparative model'

class EMMicrographsDataset(Dataset):
    """Raw 2D electron micrographs"""
    _eq_keys = Dataset._eq_keys + ['number']

    _data_type = 'EM raw micrographs'
    def __init__(self, location, number):
        super(EMMicrographsDataset, self).__init__(location)
        self.number = number

class EM2DClassDataset(Dataset):
    """2DEM class average"""
    _data_type = '2DEM class average'

class Location(Metadata):
    """Identifies the location where a resource can be found."""
    _eq_keys = []
    _allow_duplicates = False

    def __init__(self):
        pass

    # Locations compare equal iff they are the same class, have the
    # same attributes, and allow_duplicates=False
    def _eq_vals(self):
        if self._allow_duplicates:
            return id(self)
        else:
            return tuple([self.__class__]
                         + [getattr(self, x) for x in self._eq_keys])
    def __eq__(self, other):
        return self._eq_vals() == other._eq_vals()
    def __hash__(self):
        return hash(self._eq_vals())

class DatabaseLocation(Location):
    """A dataset stored in an official database (PDB, EMDB, PRIDE, etc.)"""

    # details can differ without affecting dataset equality
    _eq_keys = Location._eq_keys + ['db_name', 'access_code', 'version']

    def __init__(self, db_name, db_code, version=None, details=None):
        super(DatabaseLocation, self).__init__()
        self.db_name = db_name
        self.access_code = db_code
        self.version, self.details = version, details

class EMDBLocation(DatabaseLocation):
    """Something stored in the EMDB database."""
    def __init__(self, db_code, version=None, details=None):
        DatabaseLocation.__init__(self, 'EMDB', db_code, version, details)

class PDBLocation(DatabaseLocation):
    """Something stored in the PDB database."""
    def __init__(self, db_code, version=None, details=None):
        DatabaseLocation.__init__(self, 'PDB', db_code, version, details)

class MassIVELocation(DatabaseLocation):
    """Something stored in the MassIVE database."""
    def __init__(self, db_code, version=None, details=None):
        DatabaseLocation.__init__(self, 'MassIVE', db_code, version, details)

class RepositoryFileLocation(Location):
    """An individual file or directory in a repository.
       A repository in this context is simply a collection of files -
       it does not have to be under version control (git, svn, etc.)

       @see Repository"""

    _eq_keys = Location._eq_keys + ['doi', 'path']

    def __init__(self, doi, path):
        """Constructor.
           @param doi the Digital Object Identifer for the repository.
           @param path the location of the file or directory in the repository.
        """
        super(RepositoryFileLocation, self).__init__()
        self.doi, self.path = doi, path

def get_default_file_location(fname):
    """Get a Location for a local file.
       We don't know the DOI yet - that will be filled in later."""
    return RepositoryFileLocation(doi=None, path=fname)

class Repository(Metadata):
    """A repository containing modeling files.
       This can be used if the PMI script plus inputs files are part of a
       repository, which has been archived somewhere with a DOI.
       This will be used to construct permanent references to files
       used in this modeling, even if they haven't been uploaded to
       a database such as PDB or EMDB (by creating a RepositoryFile object).

       @see RepositoryFile."""

    def __init__(self, doi, root):
        """Constructor.
           @param doi the Digital Object Identifer for the repository.
           @param root the relative path to the top-level directory
                  of the repository from the working directory of the script.
        """
        self.doi, self._root = doi, root

    def get_path(self, fname):
        """Return a path relative to the top of the repository"""
        return RepositoryFileLocation(self.doi,
                                      os.path.relpath(fname, self._root))
