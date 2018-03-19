"""Classes to handle starting models."""

from .format import CifWriter

class Template(object):
    """A PDB file used as a comparative modeling template for part of a
       starting model.

       See :class:`StartingModel`.

       :param dataset: Pointer to where this template is stored.
       :type dataset: :class:`~ihm.dataset.Dataset`
       :param str asym_id: The asymmetric unit (chain) to use from the template
              dataset (not necessarily the same as the starting model's asym_id
              or the ID of the asym_unit in the final IHM model).
       :param tuple seq_id_range: The sequence range (in the IHM model) that
              is modeled by this template.
       :param tuple template_seq_id_range: The sequence range of the template
              that is used in comparative modeling.
       :param float sequence_identity: Sequence identity between template and
              the target sequence, as a percentage.
       :param int sequence_identity_denominator: Way in which sequence identity
              was calculated.
       :param alignment_file: Reference to the external file containing the
              template-target alignment.
       :type alignment_file: :class:`~ihm.location.Location`
       """
       # todo: handle sequence_identity_denominator as an enum, not int

    def __init__(self, dataset, asym_id, seq_id_range, template_seq_id_range,
                 sequence_identity, sequence_identity_denominator=1,
                 alignment_file=None):
        self.dataset, self.asym_id = dataset, asym_id
        self.seq_id_range = seq_id_range
        self.template_seq_id_range = template_seq_id_range
        self.sequence_identity = sequence_identity
        self.sequence_identity_denominator = sequence_identity_denominator
        self.alignment_file = alignment_file


class StartingModel(object):
    """A starting guess for modeling of an asymmetric unit

       See :attr:`~ihm.System.starting_models`.

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
    """
    def __init__(self, asym_unit, dataset, asym_id, templates=[], offset=0,
                 metadata=[]):
        self.asym_unit, self.templates = asym_unit, templates
        self.dataset, self.asym_id, self.offset = dataset, asym_id, offset
        self.metadata = metadata

    def get_seq_id_range_all_templates(self):
        """Get the seq_id range covered by all templates in this starting
           model. Where there are multiple templates, consolidate
           them; template info is given in starting_comparative_models."""
        def get_seq_id_range(template, full):
            # The template may cover more than the current starting model
            rng = template.seq_id_range
            return (max(rng[0], full[0]), min(rng[1], full[1]))

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
