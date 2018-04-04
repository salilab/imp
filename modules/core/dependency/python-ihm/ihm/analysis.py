"""Classes for handling the analysis of a modeling run.
"""

class Step(object):
    """A single step in an :class:`Analysis`.
       See :class:`FilterStep`, :class:`ClusterStep`, :class:`RescoreStep`,
       :class:`ValidationStep`, :class:`OtherStep`.
    """
    pass


class FilterStep(Step):
    """A single filtering step in an :class:`Analysis`.

       :param str feature: feature energy/score;RMSD;dRMSD;other
       :param int num_models_begin: The number of models at the beginning
              of the step
       :param int num_models_end: The number of models at the end of the step
       :param assembly: The part of the system analyzed in this step
       :type assembly: :class:`~ihm.Assembly`
       :param dataset_group: The collection of datasets used in this analysis,
              if applicable
       :type dataset_group: :class:`~ihm.dataset.DatasetGroup`
    """
    type = 'filter'

    def __init__(self, feature, num_models_begin, num_models_end,
                 assembly=None, dataset_group=None):
        self.assembly, self.dataset_group = assembly, dataset_group
        self.feature = feature
        self.num_models_begin = num_models_begin
        self.num_models_end = num_models_end


class ClusterStep(Step):
    """A single clustering step in an :class:`Analysis`.

       :param str feature: feature energy/score;RMSD;dRMSD;other
       :param int num_models_begin: The number of models at the beginning
              of the step
       :param int num_models_end: The number of models at the end of the step
       :param assembly: The part of the system analyzed in this step
       :type assembly: :class:`~ihm.Assembly`
       :param dataset_group: The collection of datasets used in this analysis,
              if applicable
       :type dataset_group: :class:`~ihm.dataset.DatasetGroup`
    """
    type = 'cluster'

    def __init__(self, feature, num_models_begin, num_models_end,
                 assembly=None, dataset_group=None):
        self.assembly, self.dataset_group = assembly, dataset_group
        self.feature = feature
        self.num_models_begin = num_models_begin
        self.num_models_end = num_models_end


class RescoreStep(Step):
    """A single rescoring step in an :class:`Analysis`.

       :param str feature: feature energy/score;RMSD;dRMSD;other
       :param int num_models_begin: The number of models at the beginning
              of the step
       :param int num_models_end: The number of models at the end of the step
       :param assembly: The part of the system analyzed in this step
       :type assembly: :class:`~ihm.Assembly`
       :param dataset_group: The collection of datasets used in this analysis,
              if applicable
       :type dataset_group: :class:`~ihm.dataset.DatasetGroup`
    """
    type = 'rescore'

    def __init__(self, feature, num_models_begin, num_models_end,
                 assembly=None, dataset_group=None):
        self.assembly, self.dataset_group = assembly, dataset_group
        self.feature = feature
        self.num_models_begin = num_models_begin
        self.num_models_end = num_models_end


class ValidationStep(Step):
    """A single validation step in an :class:`Analysis`.

       :param str feature: feature energy/score;RMSD;dRMSD;other
       :param int num_models_begin: The number of models at the beginning
              of the step
       :param int num_models_end: The number of models at the end of the step
       :param assembly: The part of the system analyzed in this step
       :type assembly: :class:`~ihm.Assembly`
       :param dataset_group: The collection of datasets used in this analysis,
              if applicable
       :type dataset_group: :class:`~ihm.dataset.DatasetGroup`
    """
    type = 'validation'

    def __init__(self, feature, num_models_begin, num_models_end,
                 assembly=None, dataset_group=None):
        self.assembly, self.dataset_group = assembly, dataset_group
        self.feature = feature
        self.num_models_begin = num_models_begin
        self.num_models_end = num_models_end


class OtherStep(Step):
    """A single step in an :class:`Analysis` of some other type.

       :param str feature: feature energy/score;RMSD;dRMSD;other
       :param int num_models_begin: The number of models at the beginning
              of the step
       :param int num_models_end: The number of models at the end of the step
       :param assembly: The part of the system analyzed in this step
       :type assembly: :class:`~ihm.Assembly`
       :param dataset_group: The collection of datasets used in this analysis,
              if applicable
       :type dataset_group: :class:`~ihm.dataset.DatasetGroup`
    """
    type = 'other'

    def __init__(self, feature, num_models_begin, num_models_end,
                 assembly=None, dataset_group=None):
        self.assembly, self.dataset_group = assembly, dataset_group
        self.feature = feature
        self.num_models_begin = num_models_begin
        self.num_models_end = num_models_end


class EmptyStep(Step):
    """A 'do nothing' step in an :class:`Analysis`.
       This can be used if modeling outputs were used directly without
       any kind of analysis.
    """
    type = 'none'
    feature = 'none'
    num_models_begin = num_models_end = None
    assembly = dataset_group = None


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
