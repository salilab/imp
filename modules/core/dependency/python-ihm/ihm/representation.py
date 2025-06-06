"""Classes for handling representation of the system during modeling.
"""


def _starting_model_report(seg):
    if seg.starting_model:
        if hasattr(seg.starting_model, '_id'):
            return " (from starting model %s)" % seg.starting_model._id
        else:
            return " (from starting model)"
    else:
        return ""


class Segment:
    """Base class for part of a :class:`Representation`.
       See :class:`AtomicSegment`, :class:`ResidueSegment`,
       :class:`MultiResidueSegment`, and :class:`FeatureSegment`.
    """
    def _get_report(self):
        """Return a textual description of the object, used by
           :meth:`ihm.System.report`"""
        return str(self)


class AtomicSegment(Segment):
    """Part of the system modeled atomistically, stored in
       a :class:`Representation`.

       :param asym_unit: The asymmetric unit (or part of one) that
              this segment represents.
       :type asym_unit: :class:`~ihm.AsymUnit` or :class:`~ihm.AsymUnitRange`
       :param bool rigid: Whether internal coordinates of the segment were
              fixed during modeling.
       :param starting_model: initial coordinates used for the segment
              (or None).
       :type starting_model: :class:`~ihm.startmodel.StartingModel`
       :param str description: Additional text describing this segment.
    """

    primitive = 'atomistic'
    count = None
    granularity = 'by-atom'

    def _get_report(self):
        asym = self.asym_unit
        return ("%s %d-%d as %s atoms%s"
                % (asym.details, asym.seq_id_range[0], asym.seq_id_range[1],
                   "rigid" if self.rigid else "flexible",
                   _starting_model_report(self)))

    def __init__(self, asym_unit, rigid, starting_model=None,
                 description=None):
        self.asym_unit = asym_unit
        self.starting_model, self.rigid = starting_model, rigid
        self.description = description


class ResidueSegment(Segment):
    """Part of the system modeled as a set of residues, stored in
       a :class:`Representation`.

       :param asym_unit: The asymmetric unit (or part of one) that
              this segment represents.
       :type asym_unit: :class:`~ihm.AsymUnit` or :class:`~ihm.AsymUnitRange`
       :param bool rigid: Whether internal coordinates of the segment were
              fixed during modeling.
       :param str primitive: The type of object used to represent this segment
              (sphere/gaussian/other).
       :param starting_model: initial coordinates used for the segment
              (or None).
       :type starting_model: :class:`~ihm.startmodel.StartingModel`
       :param str description: Additional text describing this segment.
    """

    count = None
    granularity = 'by-residue'

    def _get_report(self):
        asym = self.asym_unit
        return ("%s %d-%d as %s residues%s"
                % (asym.details, asym.seq_id_range[0], asym.seq_id_range[1],
                   "rigid" if self.rigid else "flexible",
                   _starting_model_report(self)))

    def __init__(self, asym_unit, rigid, primitive, starting_model=None,
                 description=None):
        self.asym_unit = asym_unit
        self.primitive = primitive
        self.starting_model, self.rigid = starting_model, rigid
        self.description = description


class MultiResidueSegment(Segment):
    """Part of the system modeled as a single object representing a
       range of residues, stored in a :class:`Representation`.

       :param asym_unit: The asymmetric unit (or part of one) that
              this segment represents.
       :type asym_unit: :class:`~ihm.AsymUnit` or :class:`~ihm.AsymUnitRange`
       :param bool rigid: Whether internal coordinates of the segment were
              fixed during modeling.
       :param str primitive: The type of object used to represent this segment
              (sphere/gaussian/other).
       :param starting_model: initial coordinates used for the segment
              (or None).
       :type starting_model: :class:`~ihm.startmodel.StartingModel`
       :param str description: Additional text describing this segment.
    """

    count = None
    granularity = 'multi-residue'

    def __init__(self, asym_unit, rigid, primitive, starting_model=None,
                 description=None):
        self.asym_unit = asym_unit
        self.primitive = primitive
        self.starting_model, self.rigid = starting_model, rigid
        self.description = description


class FeatureSegment(Segment):
    """Part of the system modeled as a number of geometric features,
       stored in a :class:`Representation`.

       :param asym_unit: The asymmetric unit (or part of one) that
              this segment represents.
       :type asym_unit: :class:`~ihm.AsymUnit` or :class:`~ihm.AsymUnitRange`
       :param bool rigid: Whether internal coordinates of the segment were
              fixed during modeling.
       :param str primitive: The type of object used to represent this segment
              (sphere/gaussian/other).
       :param int count: The number of objects used to represent this segment.
       :param starting_model: initial coordinates used for the segment
              (or None).
       :type starting_model: :class:`~ihm.startmodel.StartingModel`
       :param str description: Additional text describing this segment.
    """

    granularity = 'by-feature'

    def _get_report(self):
        asym = self.asym_unit
        return ("%s %d-%d as %d %s feature%s (%s)%s"
                % (asym.details, asym.seq_id_range[0], asym.seq_id_range[1],
                   self.count, "rigid" if self.rigid else "flexible",
                   "" if self.count == 1 else "s", self.primitive,
                   _starting_model_report(self)))

    def __init__(self, asym_unit, rigid, primitive, count, starting_model=None,
                 description=None):
        self.asym_unit = asym_unit
        self.primitive, self.count = primitive, count
        self.starting_model, self.rigid = starting_model, rigid
        self.description = description


class Representation(list):
    """Part of the system modeled as a set of geometric objects, such as
       spheres or atoms. This is implemented as a simple list of
       :class:`Segment` objects.

       :param sequence elements: Initial set of segments.
       :param str name: A short descriptive name.
       :param str details: A longer description of the representation.

       Typically a Representation is assigned to a
       :class:`~ihm.model.Model`. See also
       :attr:`ihm.System.orphan_representations`.

       Multiple representations of the same system are possible (multi-scale).
    """

    # For backwards compatibility with earlier versions of this class which
    # didn't specify name/details
    name = details = None

    # todo: use set rather than list?
    def __init__(self, elements=(), name=None, details=None):
        super().__init__(elements)
        self.name, self.details = name, details
