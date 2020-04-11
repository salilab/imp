"""Classes for providing extra information about an :class:`ihm.Entity`"""

# Handle different naming of urllib in Python 2/3
try:
    import urllib.request as urllib2
except ImportError:
    import urllib2
import sys


class Reference(object):
    """Base class for extra information about an :class:`ihm.Entity`.

       This class is not used directly; instead, use a subclass such as
       :class:`Sequence` or :class:`UniProtSequence`. These objects are
       then typically passed to the :class:`ihm.Entity` constructor."""
    pass


class Sequence(Reference):
    """Point to the sequence of an :class:`ihm.Entity` in a sequence database;
       convenience subclasses are provided for common sequence databases such
       as :class:`UniProtSequence`.

       :param str db_name: The name of the database.
       :param str db_code: The name of the sequence in the database.
       :param str accession: The database accession.
       :param str sequence: The complete sequence, as a string of
              one-letter codes
       :param int db_align_begin: The first residue in the database sequence
              that is used (defaults to the entire sequence).
       :param int db_align_end: The last residue in the database sequence
              that is used (defaults to the entire sequence).
       :param int align_end: The last residue in the :class:`~ihm.Entity`
              sequence that is taken from the reference (defaults to the entire
              entity sequence).
       :param int align_begin: The first residue in the :class:`~ihm.Entity`
              sequence that is taken from the reference (defaults to the entire
              entity sequence).
       :param str details: Longer text describing the sequence.
       :param seq_dif: Single-point mutations made to the sequence.
       :type seq_dif: Sequence of :class:`SeqDif` objects.
    """

    def __init__(self, db_name, db_code, accession, sequence,
                 db_align_begin=1, db_align_end=None, align_begin=1,
                 align_end=None, details=None, seq_dif=[]):
        self.db_name, self.db_code, self.accession = db_name, db_code, accession
        self.sequence, self.details = sequence, details
        self.db_align_begin, self.db_align_end = db_align_begin, db_align_end
        self.align_begin, self.align_end = align_begin, align_end
        self.seq_dif = []
        self.seq_dif.extend(seq_dif)


class UniProtSequence(Sequence):
    """Point to the sequence of an :class:`ihm.Entity` in UniProt.

       :param str db_code: The UniProt name (e.g. NUP84_YEAST)
       :param str accession: The UniProt accession (e.g. P52891)

       See :class:`Sequence` for a description of the remaining parameters.
    """

    _db_name = 'UNP'

    def __init__(self, db_code, accession, sequence, db_align_begin=1,
                 db_align_end=None, align_begin=1, align_end=None,
                 details=None, seq_dif=[]):
        super(UniProtSequence, self).__init__(
                self._db_name, db_code, accession, sequence, db_align_begin,
                db_align_end, align_begin, align_end, details, seq_dif)

    def __str__(self):
        return "<ihm.reference.UniProtSequence(%s)>" % self.accession

    @classmethod
    def from_accession(cls, accession, db_align_begin=1, db_align_end=None,
                       align_begin=1, align_end=None, details=None, seq_dif=[]):
        """Create :class:`UniProtSequence` from just an accession.
           This is done by querying the UniProt web API, so requires network
           access.

           :param str accession: The UniProt accession (e.g. P52891)

           See :class:`Sequence` for a description of the remaining parameters.
        """
        # urlopen returns bytes
        if sys.version_info[0] >= 3:
            def decode(t):
                return t.decode('ascii')
        else:
            decode = lambda t: t
        url = 'https://www.uniprot.org/uniprot/%s.fasta' % accession
        with urllib2.urlopen(url) as fh:
            header = decode(fh.readline())
            spl = header.split('|')
            if len(spl) < 3 or spl[0] != '>sp':
                raise ValueError("Cannot parse UniProt header %s" % header)
            cd = spl[2].split(None, 1)
            code = cd[0]
            details = cd[1].rstrip('\r\n') if len(cd) > 1 else None
            seq = decode(fh.read()).replace('\n', '')
            return cls(code, accession, seq, db_align_begin, db_align_end,
                       align_begin, align_end, details, seq_dif)


class SeqDif(object):
    """Annotate a sequence difference between a reference and entity sequence.
       See :class:`Sequence`.

       :param int seq_id: The residue index in the reference sequence.
       :param db_monomer: The monomer type (as a :class:`~ihm.ChemComp` object)
              in the reference sequence.
       :type db_monomer: :class:`ihm.ChemComp`
       :param monomer: The monomer type (as a :class:`~ihm.ChemComp` object)
              in the entity sequence.
       :type db_monomer: :class:`ihm.ChemComp`
       :param str details: Descriptive text for the sequence difference.
    """
    def __init__(self, seq_id, db_monomer, monomer, details=None):
        self.seq_id, self.db_monomer, self.monomer = seq_id, db_monomer, monomer
        self.details = details
