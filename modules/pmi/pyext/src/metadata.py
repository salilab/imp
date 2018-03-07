"""@namespace IMP.pmi.metadata
Classes for attaching metadata to PMI objects.
"""

from __future__ import print_function, division
from IMP.pmi.tools import OrderedDict
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


class PythonScript(RootMetadata):
    """A Python script used as part of the modeling."""
    def __init__(self, location):
        self.location = location


class ChimeraXCommandScript(RootMetadata):
    """A ChimeraX command script used to visualize the model."""
    def __init__(self, location):
        self.location = location


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
        self._parents = OrderedDict()

    def add_parent(self, dataset):
        """Add another Dataset from which this one was derived.
           For example, a 3D EM map may be derived from a set of 2D images."""
        self._parents[dataset] = None

    def add_primary(self, dataset):
        """Add another Dataset from which the ultimate parent of this one
           was derived."""
        if len(self._parents) == 0:
            self.add_parent(dataset)
        elif len(self._parents) == 1:
            list(self._parents.keys())[0].add_parent(dataset)
        else:
            raise ValueError("This dataset has multiple parents - don't "
                             "know which one to add to")

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

class IntegrativeModelDataset(Dataset):
    """A 3D structure determined by integrative modeling"""
    _data_type = 'Integrative model'

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

class SASDataset(Dataset):
    """SAS data"""
    _data_type = 'SAS data'

class Location(Metadata):
    """Identifies the location where a resource can be found."""

    # 'details' can differ without affecting dataset equality
    _eq_keys = []
    _allow_duplicates = False

    def __init__(self, details=None):
        self.details = details

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

    _eq_keys = Location._eq_keys + ['db_name', 'access_code', 'version']

    def __init__(self, db_name, db_code, version=None, details=None):
        super(DatabaseLocation, self).__init__(details)
        self.db_name = db_name
        self.access_code = db_code
        self.version = version

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
        DatabaseLocation.__init__(self, 'MASSIVE', db_code, version, details)

class EMPIARLocation(DatabaseLocation):
    """Something stored in the EMPIAR database."""
    def __init__(self, db_code, version=None, details=None):
        DatabaseLocation.__init__(self, 'EMPIAR', db_code, version, details)

class SASBDBLocation(DatabaseLocation):
    """Something stored in the SASBDB database."""
    def __init__(self, db_code, version=None, details=None):
        DatabaseLocation.__init__(self, 'SASBDB', db_code, version, details)

class FileLocation(Location):
    """An individual file or directory.
       This may be in a repository (if `repo` is not None) or only on the
       local disk (if `repo` is None)."""

    _eq_keys = Location._eq_keys + ['repo', 'path']

    def __init__(self, path, repo=None, details=None):
        """Constructor.
           @param path the location of the file or directory.
           @param repo a Repository object that describes the repository
                  containing the file (if any).
        """
        super(FileLocation, self).__init__(details)
        self.repo = repo
        if repo:
            self.path = path
            # Cannot determine file size if non-local
            self.file_size = None
        else:
            if not os.path.exists(path):
                raise ValueError("%s does not exist" % path)
            self.file_size = os.stat(path).st_size
            # Store absolute path in case the working directory changes later
            self.path = os.path.abspath(path)

class Repository(Metadata):
    """A repository containing modeling files.
       This can be used if the PMI script plus input files are part of a
       repository, which has been archived somewhere with a DOI.
       This will be used to construct permanent references to files
       used in this modeling, even if they haven't been uploaded to
       a database such as PDB or EMDB.

       @see FileLocation."""

    # Two repositories compare equal if their DOIs and URLs are the same
    def __eq__(self, other):
        return self.doi == other.doi and self.url == other.url
    def __hash__(self):
        return hash((self.doi, self.url))

    def __init__(self, doi, root=None, url=None,
                 top_directory=None):
        """Constructor.
           @param doi the Digital Object Identifier for the repository.
           @param root the relative path to the top-level directory
                  of the repository from the working directory of the script,
                  or None if files in this repository aren't checked out.
           @param url If given, a location that this repository can be
                  downloaded from.
           @param top_directory If given, prefix all paths for files in this
                  repository with this value. This is useful when the archived
                  version of the repository is found in a subdirectory at the
                  URL or DOI (for example, GitHub repositories archived at
                  Zenodo get placed in a subdirectory named for the repository
                  and git hash).
        """
        # todo: DOI should be optional (could also use URL, local path)
        self.doi = doi
        self.url, self.top_directory = url, top_directory
        if root:
            # Store absolute path in case the working directory changes later
            self._root = os.path.abspath(root)

    @staticmethod
    def update_in_repos(fileloc, repos):
        """If the given FileLocation maps to somewhere within one of the
           passed repositories, update it to reflect that."""
        if fileloc.repo:
            return
        orig_path = fileloc.path
        for repo in repos:
            relpath = os.path.relpath(orig_path, repo._root)
            if not relpath.startswith('..'):
                # Prefer the shortest paths if multiple repositories can match
                if fileloc.repo is None or len(fileloc.path) > len(relpath):
                    fileloc.repo = repo
                    fileloc.path = relpath

    def _get_full_path(self, path):
        """Prefix the given path with our top-level directory"""
        return os.path.join(self.top_directory or "", path)
