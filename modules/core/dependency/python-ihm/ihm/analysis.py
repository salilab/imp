"""Classes for handling the analysis of a modeling run.
"""

class Step(object):
    """A single step in an :class:`Analysis`.
       See :class:`FilterStep`, :class:`ClusterStep`, :class:`RescoreStep`,
       :class:`OtherStep`.
    """
    pass


class FilterStep(Step):
    """A single filtering step in an :class:`Analysis`.

       :param str feature: feature energy/score;RMSD;dRMSD;other
       :param int num_models_begin: The number of models at the beginning
              of the step
       :param int num_models_end: The number of models at the end of the step
    """
    type = 'filter'

    def __init__(self, feature, num_models_begin, num_models_end):
        self.feature = feature
        self.num_models_begin = num_models_begin
        self.num_models_end = num_models_end


class ClusterStep(Step):
    """A single clustering step in an :class:`Analysis`.

       :param str feature: feature energy/score;RMSD;dRMSD;other
       :param int num_models_begin: The number of models at the beginning
              of the step
       :param int num_models_end: The number of models at the end of the step
    """
    type = 'cluster'

    def __init__(self, feature, num_models_begin, num_models_end):
        self.feature = feature
        self.num_models_begin = num_models_begin
        self.num_models_end = num_models_end


class RescoreStep(Step):
    """A single rescoring step in an :class:`Analysis`.

       :param str feature: feature energy/score;RMSD;dRMSD;other
       :param int num_models_begin: The number of models at the beginning
              of the step
       :param int num_models_end: The number of models at the end of the step
    """
    type = 'rescore'

    def __init__(self, feature, num_models_begin, num_models_end):
        self.feature = feature
        self.num_models_begin = num_models_begin
        self.num_models_end = num_models_end


class OtherStep(Step):
    """A single step in an :class:`Analysis` of some other type.

       :param str feature: feature energy/score;RMSD;dRMSD;other
       :param int num_models_begin: The number of models at the beginning
              of the step
       :param int num_models_end: The number of models at the end of the step
    """
    type = 'other'

    def __init__(self, feature, num_models_begin, num_models_end):
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
