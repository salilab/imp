"""Classes for providing extra information about an :class:`ihm.Entity`"""

# Handle different naming of urllib in Python 2/3
try:
    import urllib.request as urlreq
except ImportError:
    import urllib2
    import contextlib

    class CompatRequest(object):
        pass

    # Python 2's urlopen is not a context manager, so wrap it
    @contextlib.contextmanager
    def urlopen(*args, **keys):
        try:
            fh = urllib2.urlopen(*args, **keys)
            yield fh
        finally:
            fh.close()
    # Provide Python-3-like urllib.request.urlopen
    urlreq = CompatRequest()
    urlreq.urlopen = urlopen
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

       These objects are typically passed to the :class:`ihm.Entity`
       constructor.

       See also :attr:`alignments` to describe the correspondence between
       the database and entity sequences.

       :param str db_name: The name of the database.
       :param str db_code: The name of the sequence in the database.
       :param str accession: The database accession.
       :param str sequence: The complete sequence, as a string of
              one-letter codes.
       :param str details: Longer text describing the sequence.
    """

    def __init__(self, db_name, db_code, accession, sequence, details=None):
        self.db_name, self.db_code = db_name, db_code
        self.accession = accession
        self.sequence, self.details = sequence, details

        #: All alignments between the reference and entity sequences, as
        #: :class:`Alignment` objects. If none are provided, a simple 1:1
        #: alignment is assumed.
        self.alignments = []

    def _get_alignments(self):
        if self.alignments:
            return self.alignments
        elif not hasattr(self, '_default_alignment'):
            self._default_alignment = Alignment()
        return [self._default_alignment]


class UniProtSequence(Sequence):
    """Point to the sequence of an :class:`ihm.Entity` in UniProt.

       These objects are typically passed to the :class:`ihm.Entity`
       constructor.

       :param str db_code: The UniProt name (e.g. NUP84_YEAST)
       :param str accession: The UniProt accession (e.g. P52891)

       See :class:`Sequence` for a description of the remaining parameters.
    """

    _db_name = 'UNP'

    def __init__(self, db_code, accession, sequence, details=None):
        super(UniProtSequence, self).__init__(
            self._db_name, db_code, accession, sequence, details)

    def __str__(self):
        return "<ihm.reference.UniProtSequence(%s)>" % self.accession

    @classmethod
    def from_accession(cls, accession):
        """Create :class:`UniProtSequence` from just an accession.
           This is done by querying the UniProt web API, so requires network
           access.

           :param str accession: The UniProt accession (e.g. P52891)
        """
        # urlopen returns bytes
        if sys.version_info[0] >= 3:
            def decode(t):
                return t.decode('ascii')
        else:
            def decode(t):
                return t
        url = 'https://www.uniprot.org/uniprot/%s.fasta' % accession
        with urlreq.urlopen(url) as fh:
            header = decode(fh.readline())
            spl = header.split('|')
            if len(spl) < 3 or spl[0] not in ('>sp', '>tr'):
                raise ValueError("Cannot parse UniProt header %s" % header)
            cd = spl[2].split(None, 1)
            code = cd[0]
            details = cd[1].rstrip('\r\n') if len(cd) > 1 else None
            seq = decode(fh.read()).replace('\n', '')
            return cls(code, accession, seq, details)


class Alignment(object):
    """A sequence range that aligns between the database and the entity.
       This describes part of the sequence in the sequence database
       (:class:`Sequence`) and in the :class:`ihm.Entity`. The two ranges
       must be the same length and have the same primary sequence (any
       differences must be described with :class:`SeqDif` objects).

       :param int db_begin: The first residue in the database sequence
              that is used (defaults to the entire sequence).
       :param int db_end: The last residue in the database sequence
              that is used (or None, the default, to use the entire sequence).
       :param int entity_begin: The first residue in the :class:`~ihm.Entity`
              sequence that is taken from the reference (defaults to the entire
              entity sequence).
       :param int entity_end: The last residue in the :class:`~ihm.Entity`
              sequence that is taken from the reference (or None, the default,
              to use the entire sequence).
       :param seq_dif: Single-point mutations made to the sequence.
       :type seq_dif: Sequence of :class:`SeqDif` objects.
    """
    def __init__(self, db_begin=1, db_end=None, entity_begin=1,
                 entity_end=None, seq_dif=[]):
        self.db_begin, self.db_end = db_begin, db_end
        self.entity_begin, self.entity_end = entity_begin, entity_end
        self.seq_dif = []
        self.seq_dif.extend(seq_dif)


class SeqDif(object):
    """Annotate a sequence difference between a reference and entity sequence.
       See :class:`Alignment`.

       :param int seq_id: The residue index in the entity sequence.
       :param db_monomer: The monomer type (as a :class:`~ihm.ChemComp` object)
              in the reference sequence.
       :type db_monomer: :class:`ihm.ChemComp`
       :param monomer: The monomer type (as a :class:`~ihm.ChemComp` object)
              in the entity sequence.
       :type monomer: :class:`ihm.ChemComp`
       :param str details: Descriptive text for the sequence difference.
    """
    def __init__(self, seq_id, db_monomer, monomer, details=None):
        self.seq_id, self.db_monomer = seq_id, db_monomer
        self.monomer, self.details = monomer, details
