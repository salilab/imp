"""@namespace IMP.pmi.metadata
Classes for attaching metadata to PMI objects.

@note All of these classes are deprecated; use classes from the ihm package
      instead.
"""

from __future__ import print_function, division
import IMP
import ihm.location
import ihm.dataset

@IMP.deprecated_object("2.9", "Use ihm.Software instead")
class Software(ihm.Software):
    def __init__(self, name, classification, description, url, type='program',
                 version=None):
        super(Software, self).__init__(name=name, classification=classification,
                        description=description, location=url,
                        type=type, version=version)


@IMP.deprecated_object("2.9", "Use ihm.Citation instead")
class Citation(ihm.Citation):
    pass


@IMP.deprecated_object("2.9", "Use ihm.location.Repository instead")
class Repository(ihm.location.Repository):
    pass


@IMP.deprecated_object("2.9",
                       "Use a subclass of ihm.location.FileLocation instead")
class FileLocation(ihm.location.FileLocation):
    pass


@IMP.deprecated_object("2.9", "Use ihm.dataset.EMMicrographsDataset instead")
class EMMicrographsDataset(ihm.dataset.EMMicrographsDataset):
    def __init__(self, location, number):
        super(EMMicrographsDataset, self).__init__(location)
        self.number = number # todo: add to restraint


@IMP.deprecated_object("2.9", "Use ihm.location.MassIVELocation instead")
class MassIVELocation(ihm.location.MassIVELocation):
    pass


@IMP.deprecated_object("2.9", "Use ihm.dataset.MassSpecDataset instead")
class MassSpecDataset(ihm.dataset.MassSpecDataset):
    pass


@IMP.deprecated_object("2.9", "Use ihm.location.EMDBLocation instead")
class EMDBLocation(ihm.location.EMDBLocation):
    pass


@IMP.deprecated_object("2.9", "Use ihm.dataset.EMDensityDataset instead")
class EMDensityDataset(ihm.dataset.EMDensityDataset):
    pass


@IMP.deprecated_function("2.9", "Use ihm.location.WorkflowFileLocation instead")
def PythonScript(location):
    location.content_type = ihm.location.WorkflowFileLocation.content_type
    return location
