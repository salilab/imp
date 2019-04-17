"""Classes for tracking external data used by mmCIF models.
"""

import os

class Location(object):
    """Identifies the location where a resource can be found.

       Typically the resource may be found in a file (either on the local
       disk or at a DOI) - for this use one of the subclasses of
       :class:`FileLocation`. Alternatively the resource may be found in
       an experiment-specific database such as PDB or EMDB - for this use
       :class:`DatabaseLocation` or one of its subclasses. A Location may
       be passed to

         - a :class:`~ihm.dataset.Dataset` to point to where an
           experimental dataset may be found;
         - an :class:`~ihm.model.Ensemble` to point to coordinates for an
           entire ensemble, for example as a DCD file;
         - a :class:`LocalizationDensity` to point to an external localization
           density, for example in MRC format;
         - :data:`ihm.System.locations` to point to other files relating
           to the modeling in general, such as a modeling control script
           (:class:`WorkflowFileLocation`) or a command script for a
           visualization package such as ChimeraX
           (:class:`VisualizationFileLocation`);
         - a :class:`ihm.protocol.Step` or :class:`ihm.analysis.Step` to
           describe an individual modeling step;
         - or a :class:`~ihm.startmodel.StartingModel` to describe how a
           starting model was constructed.

       :param str details: Additional details about the dataset, if known.

    """

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
        # We can never be equal to None
        return other is not None and self._eq_vals() == other._eq_vals()
    def __hash__(self):
        return hash(self._eq_vals())


class DatabaseLocation(Location):
    """A dataset stored in an official database (PDB, EMDB, PRIDE, etc.).

       :param str db_name: The name of the database.
       :param str db_code: The accession code inside the database.
       :param str version: The version of the dataset in the database.
       :param str details: Additional details about the dataset, if known.
    """

    _eq_keys = Location._eq_keys + ['db_name', 'access_code', 'version']

    def __init__(self, db_name, db_code, version=None, details=None):
        super(DatabaseLocation, self).__init__(details)
        self.db_name = db_name
        self.access_code = db_code
        self.version = version


class EMDBLocation(DatabaseLocation):
    """Something stored in the EMDB database.
       See :class:`DatabaseLocation` for a description of the parameters
       and :class:`Location` for discussion of the usage of these objects."""
    _db_name = 'EMDB'
    def __init__(self, db_code, version=None, details=None):
        super(EMDBLocation, self).__init__(self._db_name, db_code,
                                           version, details)


class PDBLocation(DatabaseLocation):
    """Something stored in the PDB database.
       See :class:`DatabaseLocation` for a description of the parameters
       and :class:`Location` for discussion of the usage of these objects."""
    _db_name = 'PDB'
    def __init__(self, db_code, version=None, details=None):
        super(PDBLocation, self).__init__(self._db_name, db_code,
                                          version, details)


class MassIVELocation(DatabaseLocation):
    """Something stored in the MassIVE database.
       See :class:`DatabaseLocation` for a description of the parameters
       and :class:`Location` for discussion of the usage of these objects."""
    _db_name = 'MASSIVE'
    def __init__(self, db_code, version=None, details=None):
        super(MassIVELocation, self).__init__(self._db_name, db_code, version,
                                              details)


class EMPIARLocation(DatabaseLocation):
    """Something stored in the EMPIAR database.
       See :class:`DatabaseLocation` for a description of the parameters
       and :class:`Location` for discussion of the usage of these objects."""
    _db_name = 'EMPIAR'
    def __init__(self, db_code, version=None, details=None):
        super(EMPIARLocation, self).__init__(self._db_name, db_code, version,
                                             details)


class SASBDBLocation(DatabaseLocation):
    """Something stored in the SASBDB database.
       See :class:`DatabaseLocation` for a description of the parameters
       and :class:`Location` for discussion of the usage of these objects."""
    _db_name = 'SASBDB'
    def __init__(self, db_code, version=None, details=None):
        super(SASBDBLocation, self).__init__(self._db_name, db_code, version,
                                             details)


class PRIDELocation(DatabaseLocation):
    """Something stored in the PRIDE database.
       See :class:`DatabaseLocation` for a description of the parameters
       and :class:`Location` for discussion of the usage of these objects."""
    _db_name = 'PRIDE'
    def __init__(self, db_code, version=None, details=None):
        super(PRIDELocation, self).__init__(self._db_name, db_code, version,
                                            details)


class BioGRIDLocation(DatabaseLocation):
    """Something stored in the BioGRID database.
       See :class:`DatabaseLocation` for a description of the parameters
       and :class:`Location` for discussion of the usage of these objects."""
    _db_name = 'BioGRID'
    def __init__(self, db_code, version=None, details=None):
        super(BioGRIDLocation, self).__init__(self._db_name, db_code, version,
                                              details)


class FileLocation(Location):
    """Base class for an individual file or directory stored externally.

       :param str path: the location of the file or directory (this can
              be `None` if `repo` is set, to refer to the entire repository)
       :param repo: object that describes the repository
              containing the file, or `None` if it is stored on the local disk
       :type repo: :class:`Repository`
       :param str details: optional description of the file
    """

    _eq_keys = Location._eq_keys + ['repo', 'path', 'content_type']

    content_type = None

    def __init__(self, path, repo=None, details=None):
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


class InputFileLocation(FileLocation):
    """An externally stored file used as input.
       See :class:`FileLocation` for a description of the parameters
       and :class:`Location` for discussion of the usage of these objects.

       For example, any :class:`~ihm.dataset.Dataset` that isn't stored in
       a domain-specific database would use this class."""
    content_type = 'Input data or restraints'


class OutputFileLocation(FileLocation):
    """An externally stored file used for output.
       See :class:`FileLocation` for a description of the parameters
       and :class:`Location` for discussion of the usage of these objects.

       For example, this can be used to point to an externally-stored
       :class:`model ensemble <ihm.model.Ensemble>` or a
       :class:`localization density <ihm.model.LocalizationDensity>`.
    """
    content_type = "Modeling or post-processing output"


class WorkflowFileLocation(FileLocation):
    """An externally stored file that controls the workflow (e.g. a script).
       See :class:`FileLocation` for a description of the parameters
       and :class:`Location` for discussion of the usage of these objects.

       Typically these objects are used to provide more information on how
       a :class:`~ihm.startmodel.StartingModel` was generated, how an
       individual :class:`ihm.protocol.Step` or :class:`ihm.analysis.Step`
       was performed, or to describe the overall modeling (by addition
       to :data:`ihm.System.locations`). This can be useful to capture fine
       details of the modeling that aren't covered by the mmCIF dictionary,
       and to allow models to be precisely reproduced.
    """
    content_type = "Modeling workflow or script"


class VisualizationFileLocation(FileLocation):
    """An externally stored file that is used for visualization.
       See :class:`FileLocation` for a description of the parameters
       and :class:`Location` for discussion of the usage of these objects.
    """
    content_type = "Visualization script"


class Repository(object):
    """A repository containing modeling files, i.e. a collection of related
       files at a remote, public location. This can include code repositories
       such as GitHub, file archival services such as Zenodo, or any other
       service that provides a DOI, such as the supplementary information for
       a publication.

       This can also be used if the script plus related files are part of a
       repository, which has been archived somewhere with a DOI.
       This will be used to construct permanent references to files
       used in this modeling, even if they haven't been uploaded to
       a database such as PDB or EMDB.
       See :meth:`ihm.System.update_locations_in_repositories`.

       See also :class:`FileLocation`.

       :param str doi: the Digital Object Identifier for the repository
       :param str root: the path on the local disk to the top-level
              directory of the repository, or `None` if files in this
              repository aren't checked out.
       :param str url: If given, a location that this repository can be
              downloaded from.
       :param str top_directory: If given, prefix all paths for files in
              this repository with this value. This is useful when the
              archived version of the repository is found in a subdirectory
              at the URL or DOI (for example, GitHub repositories
              archived at Zenodo get placed in a subdirectory named
              for the repository and git hash).
    """

    reference_type = 'DOI'

    # Two repositories compare equal if their DOIs and URLs are the same
    def __eq__(self, other):
        return self.doi == other.doi and self.url == other.url
    def __hash__(self):
        return hash((self.doi, self.url))

    def __init__(self, doi, root=None, url=None, top_directory=None):
        # todo: DOI should be optional (could also use URL, local path)
        self.doi = doi
        self.url, self.top_directory = url, top_directory
        if root is not None:
            # Store absolute path in case the working directory changes later
            self._root = os.path.abspath(root)

    reference = property(lambda self: self.doi)

    def __get_reference_provider(self):
        if 'zenodo' in self.reference:
            return 'Zenodo'
    reference_provider = property(__get_reference_provider)
    def __get_refers_to(self):
        if self.url:
            return 'Archive' if self.url.endswith(".zip") else 'File'
        return 'Other'
    refers_to = property(__get_refers_to)

    @staticmethod
    def _update_in_repos(fileloc, repos):
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
