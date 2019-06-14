"""Classes for describing the source of an entity.
"""

class Source(object):
    """Base class to describe the source of an :class:`ihm.Entity`.
       See :class:`Manipulated`, :class:`Natural` and :class:`Synthetic`.
    """
    src_method = None


class Details(object):
    """Identifying information for an entity source.
       See :class:`Manipulated` or :class:`Natural`.

       :param ncbi_taxonomy_id: NCBI taxonomy identifier, e.g. "469008"
       :param scientific_name: Scientific name, e.g. "Escherichia coli"
       :param common_name: Common name
       :param strain: Strain, e.g. "BL21(DE3)PLYSS"
    """

    def __init__(self, ncbi_taxonomy_id=None, scientific_name=None,
                 common_name=None, strain=None):
        self.ncbi_taxonomy_id = ncbi_taxonomy_id
        self.scientific_name = scientific_name
        self.common_name = common_name
        self.strain = strain


class Manipulated(Source):
    """An entity isolated from a genetically manipulated source.
       See :class:`Entity`.

       :param gene: Details about the gene source.
       :type gene: :class:`Details`
       :param host: Details about the host organism.
       :type host: :class:`Details`
    """
    src_method = 'man'

    def __init__(self, gene=None, host=None):
        self.gene, self.host = gene, host


class Natural(Source, Details):
    """An entity isolated from a natural source. See :class:`Entity`.
       See :class:`Details` for a description of the parameters."""
    src_method = 'nat'


class Synthetic(Source, Details):
    """An entity obtained synthetically. See :class:`Entity`.
       See :class:`Details` for a description of the parameters."""
    src_method = 'syn'
