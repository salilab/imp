"""@namespace IMP.mmcif.dataset

   Classes for tracking data used by mmCIF models.
"""

import os
try:
    from collections import OrderedDict
except ImportError:
    from IMP.mmcif._compat_collections import OrderedDict


class Dataset(object):
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


class PDBDataset(Dataset):
    """An experimentally-determined 3D structure as a set of a coordinates,
       usually in a PDB file"""
    _data_type = 'Experimental model'


class Location(object):
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
