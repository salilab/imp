"""Classes for providing extra information about an :class:`ihm.Entity`"""

import urllib.request
import ihm


class Reference:
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

    def _signature(self):
        # Ignore "details"
        return ((self.db_name, self.db_code, self.accession, self.sequence)
                + tuple(a._signature() for a in self.alignments))

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
        super().__init__(self._db_name, db_code, accession, sequence, details)

    def __str__(self):
        return "<ihm.reference.UniProtSequence(%r)>" % self.accession

    @classmethod
    def from_accession(cls, accession):
        """Create :class:`UniProtSequence` from just an accession.
           This is done by querying the UniProt web API, so requires network
           access.

           :param str accession: The UniProt accession (e.g. P52891)
        """
        # urlopen returns bytes
        def decode(t):
            return t.decode('ascii')
        url = 'https://www.uniprot.org/uniprot/%s.fasta' % accession
        with urllib.request.urlopen(url) as fh:
            header = decode(fh.readline())
            spl = header.split('|')
            if len(spl) < 3 or spl[0] not in ('>sp', '>tr'):
                raise ValueError("Cannot parse UniProt header %s" % header)
            cd = spl[2].split(None, 1)
            code = cd[0]
            details = cd[1].rstrip('\r\n') if len(cd) > 1 else None
            seq = decode(fh.read()).replace('\n', '')
            return cls(code, accession, seq, details)

    def add_missing_sequence(self):
        """Fill in any missing sequence information.
           This is done by querying the UniProt web API, so requires network
           access.
        """
        if not self.sequence:
            acc = self.from_accession(self.accession)
            self.sequence = acc.sequence
            # If we are missing details too, use that from UniProt
            if not self.details:
                self.details = acc.details


class Alignment:
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

    def _signature(self):
        return ((self.db_begin, self.db_end, self.entity_begin,
                 self.entity_end)
                + tuple(s._signature() for s in self.seq_dif))


class SeqDif:
    """Annotate a sequence difference between a reference and entity sequence.
       This is generally used for simple mutations; for insertions or
       deletions, use the :class:`InsertionSeqDif` or :class:`DeletionSeqDif`
       subclasses instead.
       See also :class:`Alignment`.

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
        # Only used for deletions; not currently exposed in the base class
        self.db_seq_id = ihm.unknown

    def _signature(self):
        # Don't ignore "details", as these distinguish insertions from
        # deletions
        return (self.seq_id, self.db_seq_id, self.db_monomer, self.monomer,
                self.details)


class InsertionSeqDif(SeqDif):
    """Annotate an insertion of a residue relative to the reference sequence.
       This is used to describe a residue that is present in the entity
       sequence but not in the reference, such as an expression tag.
       See also :class:`SeqDif` and :class:`Alignment`.

       :param int seq_id: The residue index in the entity sequence.
       :param monomer: The monomer type (as a :class:`~ihm.ChemComp` object)
              in the entity sequence.
       :type monomer: :class:`ihm.ChemComp`
       :param str details: Descriptive text for the sequence difference.
    """
    def __init__(self, seq_id, monomer, details='insertion'):
        super().__init__(seq_id=seq_id, db_monomer=ihm.unknown,
                         monomer=monomer, details=details)


class DeletionSeqDif(SeqDif):
    """Annotate a deletion of a residue from the reference sequence.
       This is used to describe a residue that is present in the reference
       sequence but not in the entity.
       See also :class:`SeqDif` and :class:`Alignment`.

       :param int db_seq_id: The residue index in the reference sequence.
       :param db_monomer: The monomer type (as a :class:`~ihm.ChemComp` object)
              in the reference sequence.
       :type db_monomer: :class:`ihm.ChemComp`
       :param str details: Descriptive text for the sequence difference.
    """
    def __init__(self, db_seq_id, db_monomer, details='deletion'):
        super().__init__(seq_id=ihm.unknown, db_monomer=db_monomer,
                         monomer=ihm.unknown, details=details)
        self.db_seq_id = db_seq_id
