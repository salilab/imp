"""Classes to handle starting models."""

from .format import CifWriter

class Source(object):
    """Base class for all sources for starting models."""
    def get_seq_id_range(self, starting_model):
        """Get the range of sequence in the starting model covered by
           this source. By default, the source covers the entire model."""
        return (starting_model.seq_id_begin, starting_model.seq_id_end)


class PDBSource(Source):
    """An experimental PDB file used as part of a starting model"""
    source = 'experimental model'
    db_name = 'PDB'
    sequence_identity = 100.0

    def __init__(self, db_code, chain_id, metadata):
        self.db_code = db_code
        self.chain_id = chain_id
        self.metadata = metadata


class TemplateSource(Source):
    """A PDB file used as a template for a comparative starting model"""
    source = 'comparative model'
    db_name = db_code = None
    tm_dataset = None

    def __init__(self, tm_code, tm_seq_id_begin, tm_seq_id_end, seq_id_begin,
                 chain_id, seq_id_end, seq_id):
        # Remove any unique suffix
        stripped_tm_code = tm_code.split('_')[0]
        # Assume a code of 1abcX or 1abcX_N refers to a real PDB structure
        if len(stripped_tm_code) == 5:
            self._orig_tm_code = None
            self.tm_db_code = stripped_tm_code[:4].upper()
            self.tm_chain_id = stripped_tm_code[4]
        else:
            # Otherwise, will need to look up in TEMPLATE PATH remarks
            self._orig_tm_code = tm_code
            self.tm_db_code = None
            self.tm_chain_id = tm_code[-1]
        self.sequence_identity = seq_id
        self.tm_seq_id_begin = tm_seq_id_begin
        self.tm_seq_id_end = tm_seq_id_end
        self.chain_id = chain_id
        self._seq_id_begin, self._seq_id_end = seq_id_begin, seq_id_end

    def get_seq_id_range(self, starting_model):
        # The template may cover more than the current starting model
        seq_id_begin = max(starting_model.seq_id_begin, self._seq_id_begin)
        seq_id_end = min(starting_model.seq_id_end, self._seq_id_end)
        return (seq_id_begin, seq_id_end)


class UnknownSource(Source):
    """Part of a starting model from an unknown source"""
    db_code = None
    db_name = CifWriter.unknown
    chain_id = CifWriter.unknown
    sequence_identity = CifWriter.unknown
    # Map dataset types to starting model sources
    _source_map = {'Comparative model': 'comparative model',
                   'Integrative model': 'integrative model',
                   'Experimental model': 'experimental model'}

    def __init__(self, dataset, chain):
        self.source = self._source_map[dataset.data_type]
        self.chain_id = chain


class StartingModel(object):
    """A starting guess for modeling of an asymmetric unit

       See :attr:`~ihm.System.starting_models`.

       :param asym_unit: The asymmetric unit this model represents.
       :type asym_unit: :class:`~ihm.AsymUnit`
       :param dataset: Pointer to where this model is stored.
       :type dataset: :class:`~ihm.dataset.Dataset`
       :param str asym_id: The asymmetric unit (chain) to use from the starting
              model.
       :param list sources: A list of :class:`Source` objects.
       :param int offset: Offset between the residue numbering in the dataset
              and the IHM model (the offset is added to the starting model
              numbering to give the IHM model numbering).
    """
    def __init__(self, asym_unit, dataset, asym_id, sources, offset=0):
        self.asym_unit, self.sources = asym_unit, sources
        self.dataset, self.asym_id, self.offset = dataset, asym_id, offset

    def get_seq_id_range_all_sources(self):
        """Get the seq_id range covered by all sources in this starting
           model. Where there are multiple sources (to date, this can only
           mean multiple templates for a comparative model) consolidate
           them; template info is given in starting_comparative_models."""
        class FullSeqIdRange(object):
            pass
        s = FullSeqIdRange()
        s.seq_id_begin = 1
        s.seq_id_end = len(self.asym_unit.entity.sequence)
        source0 = self.sources[0]
        seq_id_begin, seq_id_end = source0.get_seq_id_range(s)
        for source in self.sources[1:]:
            this_begin, this_end = source.get_seq_id_range(s)
            seq_id_begin = min(seq_id_begin, this_begin)
            seq_id_end = max(seq_id_end, this_end)
        return seq_id_begin, seq_id_end


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
