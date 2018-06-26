"""Classes for handling modeling protocols.
"""

class Step(object):
    """A single step in a :class:`Protocol`.

       :param assembly: The part of the system modeled in this step
       :type assembly: :class:`~ihm.Assembly`
       :param dataset_group: The collection of datasets used in this modeling
       :type dataset_group: :class:`~ihm.dataset.DatasetGroup`
       :param str method: Description of the method used (e.g. "Monte Carlo")
       :param str name: A descriptive name for the step
       :param int num_models_begin: The number of models at the beginning
              of the step
       :param int num_models_end: The number of models at the end of the step
       :param software: The software used in this step
       :type software: :class:`~ihm.Software`
       :param script_file: Reference to the external file containing the
              script used in this step (usually a
              :class:`~ihm.location.WorkflowFileLocation`).
       :type script_file: :class:`~ihm.location.Location`
       :param bool multi_scale: Indicates if the modeling is multi-scale
       :param bool multi_state: Indicates if the modeling is multi-state
       :param bool ordered: Indicates if the modeling is ordered
    """
    def __init__(self, assembly, dataset_group, method, num_models_begin=None,
                 num_models_end=None, software=None, script_file=None,
                 multi_scale=False, multi_state=False, ordered=False,
                 name=None):
        self.assembly = assembly
        self.dataset_group = dataset_group
        self.method = method
        self.num_models_begin = num_models_begin
        self.num_models_end = num_models_end
        self.multi_scale, self.multi_state = multi_scale, multi_state
        self.software, self.ordered, self.name = software, ordered, name
        self.script_file = script_file


class Protocol(object):
    """A modeling protocol.
       Each protocol consists of a number of protocol steps (e.g. sampling,
       refinement) followed by a number of analyses.

       Normally a protocol is passed to one or more :class:`~ihm.model.Model`
       objects, although unused protocols can still be included in the file
       if desired by adding them to :attr:`~ihm.System.orphan_protocols`.
    """
    def __init__(self, name=None):
        self.name = name

        #: All modeling steps (:class:`Step` objects)
        self.steps = []

        #: All analyses (:class:`~ihm.analysis.Analysis` objects)
        self.analyses = []
