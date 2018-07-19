"""Classes to handle starting models."""

from .format import CifWriter
try:
    from enum import IntEnum
except ImportError:
    IntEnum = object


class SequenceIdentityDenominator(IntEnum):
    """The denominator used while calculating the sequence identity.
       One of these constants can be passed to :class:`SequenceIdentity`."""

    #: Length of the shorter sequence
    SHORTER_LENGTH = 1

    #: Number of aligned positions (including gaps)
    NUM_ALIGNED_WITH_GAPS = 2

    #: Number of aligned residue pairs (not including the gaps)
    NUM_ALIGNED_WITHOUT_GAPS = 3

    #: Arithmetic mean sequence length
    MEAN_LENGTH = 4

    #: Another method not covered here
    OTHER = 5


class SequenceIdentity(object):
    """Describe the identity between template and target sequences.
       See :class:`Template`.

       :param value: Percentage sequence identity.
       :param denominator: Way in which sequence identity was calculated -
              see :class:`SequenceIdentityDenominator`.
    """
    def __init__(self, value,
                 denominator=SequenceIdentityDenominator.SHORTER_LENGTH):
        self.value = value
        self.denominator = denominator

    def __float__(self):
        return self.value


class Template(object):
    """A PDB file used as a comparative modeling template for part of a
       starting model.

       See :class:`StartingModel`.

       :param dataset: Pointer to where this template is stored.
       :type dataset: :class:`~ihm.dataset.Dataset`
       :param str asym_id: The asymmetric unit (chain) to use from the template
              dataset (not necessarily the same as the starting model's asym_id
              or the ID of the asym_unit in the final IHM model).
       :param tuple seq_id_range: The sequence range in the dataset that
              is modeled by this template. Note that this numbering may differ
              from the IHM numbering. See `offset` in :class:`StartingModel`.
       :param tuple template_seq_id_range: The sequence range of the template
              that is used in comparative modeling.
       :param sequence_identity: Sequence identity between template and
              the target sequence.
       :type sequence_identity: :class:`SequenceIdentity` or `float`
       :param alignment_file: Reference to the external file containing the
              template-target alignment.
       :type alignment_file: :class:`~ihm.location.Location`
       """
       # todo: handle sequence_identity_denominator as an enum, not int

    def __init__(self, dataset, asym_id, seq_id_range, template_seq_id_range,
                 sequence_identity, alignment_file=None):
        self.dataset, self.asym_id = dataset, asym_id
        self.seq_id_range = seq_id_range
        self.template_seq_id_range = template_seq_id_range
        if isinstance(sequence_identity, float):
            sequence_identity = SequenceIdentity(sequence_identity)
        self.sequence_identity = sequence_identity
        self.alignment_file = alignment_file


class StartingModel(object):
    """A starting guess for modeling of an asymmetric unit

       See :class:`ihm.representation.Segment` and
       :attr:`ihm.System.orphan_starting_models`.

       :param asym_unit: The asymmetric unit (or part of one) this starting
              model represents.
       :type asym_unit: :class:`~ihm.AsymUnit` or :class:`~ihm.AsymUnitRange`
       :param dataset: Pointer to where this model is stored.
       :type dataset: :class:`~ihm.dataset.Dataset`
       :param str asym_id: The asymmetric unit (chain) to use from the starting
              model's dataset (not necessarily the same as the ID of the
              asym_unit in the final model).
       :param list templates: A list of :class:`Template` objects, if this is
              a comparative model.
       :param int offset: Offset between the residue numbering in the dataset
              and the IHM model (the offset is added to the starting model
              numbering to give the IHM model numbering).
       :param list metadata: List of PDB metadata, such as HELIX records.
       :param software: The software used to generate the starting model.
       :type software: :class:`~ihm.Software`
       :param script_file: Reference to the external file containing the
              script used to generate the starting model (usually a
              :class:`~ihm.location.WorkflowFileLocation`).
       :type script_file: :class:`~ihm.location.Location`
    """
    def __init__(self, asym_unit, dataset, asym_id, templates=None, offset=0,
                 metadata=None, software=None, script_file=None):
        self.templates = templates if templates is not None else []
        self.metadata = metadata if metadata is not None else []
        self.asym_unit = asym_unit
        self.dataset, self.asym_id, self.offset = dataset, asym_id, offset
        self.software, self.script_file = software, script_file

    def get_atoms(self):
        """Yield :class:`~ihm.model.Atom` objects that represent this
           starting model. This allows the starting model coordinates to
           be embedded in the mmCIF file, which is useful if the starting
           model is not available elsewhere (or it has been modified).

           The default implementation returns no atoms; it is necessary
           to subclass and override this method.

           Note that the returned atoms should be those used in modeling,
           not those stored in the file. In particular, the numbering scheme
           should be that used in the IHM model (add `offset` to the dataset
           numbering). If any residues were changed (for example it is common
           to mutate MSE in the dataset to MET in the modeling) the final
           mutated name should be used (MET in this case) and
           :meth:`get_seq_dif` overridden to note the change.
        """
        return []

    def get_seq_dif(self):
        """Yield :class:`SeqDif` objects for any sequence changes between
           the dataset and the starting model. See :meth:`get_atoms`.

           Note that this is always called *after* :meth:`get_atoms`.
        """
        return []

    def get_seq_id_range_all_templates(self):
        """Get the seq_id range covered by all templates in this starting
           model. Where there are multiple templates, consolidate
           them; template info is given in starting_comparative_models."""
        def get_seq_id_range(template, full):
            # The template may cover more than the current starting model
            rng = template.seq_id_range
            return (max(rng[0]+self.offset, full[0]),
                    min(rng[1]+self.offset, full[1]))

        if self.templates:
            full = self.asym_unit.seq_id_range
            rng = get_seq_id_range(self.templates[0], full)
            for template in self.templates[1:]:
                this_rng = get_seq_id_range(template, full)
                rng = (min(rng[0], this_rng[0]), max(rng[1], this_rng[1]))
            return rng
        else:
            return self.asym_unit.seq_id_range


class PDBHelix(object):
    """Represent a HELIX record from a PDB file."""
    def __init__(self, line):
        self.helix_id = line[11:14].strip()
        self.start_resnam = line[14:18].strip()
        self.start_asym = line[19]
        self.start_resnum = int(line[21:25])
        self.end_resnam = line[27:30].strip()
        self.end_asym = line[31]
        self.end_resnum = int(line[33:37])
        self.helix_class = int(line[38:40])
        self.length = int(line[71:76])


class SeqDif(object):
    """Annotate a sequence difference between a dataset and starting model.
       See :meth:`StartingModel.get_seq_dif` and :class:`MSESeqDif`.

       :param int db_seq_id: The residue index in the dataset.
       :param int seq_id: The residue index in the starting model. This should
              normally be `db_seq_id + offset`.
       :param str db_comp_id: The name of the residue in the dataset.
       :param str details: Descriptive text for the sequence difference.
    """
    def __init__(self, db_seq_id, seq_id, db_comp_id, details=None):
        self.db_seq_id, self.seq_id = db_seq_id, seq_id
        self.db_comp_id, self.details = db_comp_id, details


class MSESeqDif(object):
    """Denote that a residue was mutated from MSE to MET.
       See :class:`SeqDif` for a description of the parameters.
    """
    def __init__(self, db_seq_id, seq_id,
                 details="Conversion of modified residue MSE to MET"):
        self.db_seq_id, self.seq_id = db_seq_id, seq_id
        self.db_comp_id, self.details = 'MSE', details
