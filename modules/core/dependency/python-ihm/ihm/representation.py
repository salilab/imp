"""Classes for handling representation of the system during modeling.
"""

class Segment(object):
    """Base class for part of a :class:`Representation`.
       See :class:`AtomicSegment`, :class:`ResidueSegment`,
       :class:`MultiResidueSegment`, and :class:`FeatureSegment`.
    """
    pass


class AtomicSegment(object):
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
    """

    primitive = 'atomistic'
    count = None
    granularity = 'by-atom'

    def __init__(self, asym_unit, rigid, starting_model=None):
        self.asym_unit = asym_unit
        self.starting_model, self.rigid = starting_model, rigid


class ResidueSegment(object):
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
    """

    count = None
    granularity = 'by-residue'

    def __init__(self, asym_unit, rigid, primitive, starting_model=None):
        self.asym_unit = asym_unit
        self.primitive = primitive
        self.starting_model, self.rigid = starting_model, rigid


class MultiResidueSegment(object):
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
    """

    count = None
    granularity = 'multi-residue'

    def __init__(self, asym_unit, rigid, primitive, starting_model=None):
        self.asym_unit = asym_unit
        self.primitive = primitive
        self.starting_model, self.rigid = starting_model, rigid


class FeatureSegment(object):
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
    """

    granularity = 'by-feature'

    def __init__(self, asym_unit, rigid, primitive, count, starting_model=None):
        self.asym_unit = asym_unit
        self.primitive, self.count = primitive, count
        self.starting_model, self.rigid = starting_model, rigid


class Representation(list):
    """Part of the system modeled as a set of geometric objects, such as
       spheres or atoms. This is implemented as a simple list of
       :class:`Segment` objects.

       Typically a Representation is assigned to a
       :class:`~ihm.model.Model`. See also
       :attr:`ihm.System.orphan_representations`.

       Multiple representations of the same system are possible (multi-scale).
    """

    # todo: use set rather than list?
    pass
