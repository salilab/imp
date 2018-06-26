"""Classes for handling the analysis of a modeling run.
"""

class Step(object):
    """A single step in an :class:`Analysis`.

       Normally one of the base classes is used; see :class:`FilterStep`,
       :class:`ClusterStep`, :class:`RescoreStep`, :class:`ValidationStep`,
       :class:`OtherStep`, and :class:`EmptyStep`.

       :param str feature: feature energy/score;RMSD;dRMSD;other
       :param int num_models_begin: The number of models at the beginning
              of the step
       :param int num_models_end: The number of models at the end of the step
       :param assembly: The part of the system analyzed in this step
       :type assembly: :class:`~ihm.Assembly`
       :param dataset_group: The collection of datasets used in this analysis,
              if applicable
       :type dataset_group: :class:`~ihm.dataset.DatasetGroup`
       :param software: The software used in this step
       :type software: :class:`~ihm.Software`
       :param script_file: Reference to the external file containing the
              script used in this step (usually a
              :class:`~ihm.location.WorkflowFileLocation`).
       :type script_file: :class:`~ihm.location.Location`
    """
    type = 'other'

    def __init__(self, feature, num_models_begin, num_models_end,
                 assembly=None, dataset_group=None, software=None,
                 script_file=None):
        self.assembly, self.dataset_group = assembly, dataset_group
        self.feature, self.software = feature, software
        self.num_models_begin = num_models_begin
        self.num_models_end = num_models_end
        self.script_file = script_file


class FilterStep(Step):
    """A single filtering step in an :class:`Analysis`.
       See :class:`Step` for a description of the parameters.
    """
    type = 'filter'


class ClusterStep(Step):
    """A single clustering step in an :class:`Analysis`.
       See :class:`Step` for a description of the parameters.
    """
    type = 'cluster'


class RescoreStep(Step):
    """A single rescoring step in an :class:`Analysis`.
       See :class:`Step` for a description of the parameters.
    """
    type = 'rescore'


class ValidationStep(Step):
    """A single validation step in an :class:`Analysis`.
       See :class:`Step` for a description of the parameters.
    """
    type = 'validation'


class EmptyStep(Step):
    """A 'do nothing' step in an :class:`Analysis`.
       This can be used if modeling outputs were used directly without
       any kind of analysis.
    """
    type = 'none'
    def __init__(self):
        super(EmptyStep, self).__init__(
                feature='none', num_models_begin=None, num_models_end=None)


class Analysis(object):
    """Analysis of a modeling run.
       Each analysis consists of a number of steps (e.g. filtering,
       rescoring, clustering). A modeling run may be followed by any number
       of separate analyses.

       See :attr:`ihm.protocol.Protocol.analyses`.
    """
    def __init__(self):
        #: All analysis steps (:class:`Step` objects)
        self.steps = []
