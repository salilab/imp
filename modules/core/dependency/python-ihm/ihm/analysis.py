"""Classes for handling the analysis of a modeling run.
"""


import inspect
import sys


class Feature:
    """Property of a system that was used in an analysis step, such as
       score or energy. See :class:`Step`. Generic features can use this
       class directly, but usually a subclass such as :class:`ScoreFeature`,
       :class:`RMSDFeature`, :class:`DRMSDFeature` or :class:`NoFeature`
       is used.

       :param str name: Optional descriptive name of the feature.
    """
    type = 'other'

    def __init__(self, name=None):
        self.name = name


class ScoreFeature(Feature):
    """Score or energy feature used in an analysis step.
       See :class:`Feature` for more information."""
    type = 'energy/score'


class RMSDFeature(Feature):
    """Root-mean-square (RMS) distance feature used in an analysis step.
       See :class:`Feature` for more information."""
    type = 'RMSD'


class DRMSDFeature(Feature):
    """Distance root-mean-square (dRMS) distance feature used in an
       analysis step. See :class:`Feature` for more information."""
    type = 'dRMSD'


class NoFeature(Feature):
    """Specify that an analysis step did not use any feature of the system.
       See :class:`Feature` for more information."""
    type = 'none'


class Step:
    """A single step in an :class:`Analysis`.

       Normally one of the more specific derived classes is used;
       see :class:`FilterStep`, :class:`ClusterStep`, :class:`RescoreStep`,
       :class:`ValidationStep`, and :class:`EmptyStep`, although this base
       class can be used for a generic 'other' step.

       :param str feature: Property of the system used in the analysis, such
              as energy or distance RMSD
       :type feature: :class:`Feature`
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
       :param str details: Additional text describing this step
    """
    type = 'other'

    def _get_report(self):
        return ("%s (%s->%s models)"
                % (self.type, self.num_models_begin, self.num_models_end))

    def __init__(self, feature, num_models_begin, num_models_end,
                 assembly=None, dataset_group=None, software=None,
                 script_file=None, details=None):
        self.assembly, self.dataset_group = assembly, dataset_group
        self.software = software
        self.num_models_begin = num_models_begin
        self.num_models_end = num_models_end
        self.script_file = script_file
        self.details = details
        self.feature = self._get_feature_object(feature)

    def _get_feature_object(self, f):
        """Allow for feature to be given as a string for backwards
           compatibility. Match it against Feature.type"""
        if not isinstance(f, str):
            return f
        module = sys.modules[__name__]
        features = [x[1] for x in inspect.getmembers(module, inspect.isclass)
                    if issubclass(x[1], Feature)]
        fm = {feat_cls.type.lower(): feat_cls for feat_cls in features}
        feat_cls = fm.get(f.lower())
        if feat_cls is None:
            raise ValueError(
                "Invalid feature type %s; should be one of %s"
                % (f, ", ".join(sorted(feat.type for feat in features))))
        return feat_cls()


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
        f = NoFeature()
        super().__init__(
            feature=f, num_models_begin=None, num_models_end=None)


class Analysis:
    """Analysis of a modeling run.
       Each analysis consists of a number of steps (e.g. filtering,
       rescoring, clustering). A modeling run may be followed by any number
       of separate analyses.

       See :attr:`ihm.protocol.Protocol.analyses`.
    """
    def __init__(self):
        #: All analysis steps (:class:`Step` objects)
        self.steps = []
