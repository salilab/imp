"""Utility classes to dump out information in mmCIF or BinaryCIF format"""

import re
import os
import collections
import operator
import itertools
import ihm.format
import ihm.format_bcif
import ihm.model
import ihm.representation
import ihm.source
import ihm.flr
import ihm.dataset
from . import util
from . import location
from . import restraint
from . import geometry


def _is_subrange(rng1, rng2):
    """Return True iff rng1 is wholly inside rng2"""
    # Nonpolymers should have an empty range
    if rng1 == (None, None) or rng2 == (None, None):
        return rng1 == rng2
    else:
        return rng1[0] >= rng2[0] and rng1[1] <= rng2[1]


class Dumper(object):
    """Base class for helpers to dump output to mmCIF or BinaryCIF.
       See :func:`write`."""
    def __init__(self):
        pass

    def finalize(self, system):
        """Called for all dumpers prior to :meth:`dump`.
           This can be used to assign numeric IDs to objects, check for
           sanity, etc."""
        pass

    def dump(self, system, writer):
        """Use `writer` to write information about `system` to
           mmCIF or BinaryCIF.

           :param system: The :class:`ihm.System` object containing all
                  information about the system.
           :type system: :class:`ihm.System`
           :param writer: Utility class to write data to the output file.
           :type writer: :class:`ihm.format.CifWriter` or
                 :class:`ihm.format_bcif.BinaryCifWriter`.
        """
        pass


def _get_transform(rot_matrix, tr_vector):
    """Return a dict encoding a transform, suitable for passing to
       loop.write()"""
    # mmCIF writer usually outputs floats to 3 decimal
    # places, but we need more precision for rotation
    # matrices
    rm = [["%.6f" % e for e in rot_matrix[i]] for i in range(3)]

    return {'rot_matrix11': rm[0][0], 'rot_matrix21': rm[1][0],
            'rot_matrix31': rm[2][0], 'rot_matrix12': rm[0][1],
            'rot_matrix22': rm[1][1], 'rot_matrix32': rm[2][1],
            'rot_matrix13': rm[0][2], 'rot_matrix23': rm[1][2],
            'rot_matrix33': rm[2][2], 'tr_vector1': tr_vector[0],
            'tr_vector2': tr_vector[1], 'tr_vector3': tr_vector[2]}


class _EntryDumper(Dumper):
    def dump(self, system, writer):
        # Write CIF header (so this dumper should always be first)
        writer.start_block(re.subn('[^0-9a-zA-z_-]', '', system.id)[0])
        with writer.category("_entry") as lp:
            lp.write(id=system.id)


class _AuditConformDumper(Dumper):
    URL = ("https://raw.githubusercontent.com/" +
           "ihmwg/IHM-dictionary/%s/ihm-extension.dic")

    def dump(self, system, writer):
        with writer.category("_audit_conform") as lp:
            # Update to match the version of the IHM dictionary we support:
            lp.write(dict_name="ihm-extension.dic", dict_version="1.17",
                     dict_location=self.URL % "f15a6bb")


class _StructDumper(Dumper):
    def dump(self, system, writer):
        with writer.category("_struct") as lp:
            mth = system.structure_determination_methodology
            lp.write(title=system.title, entry_id=system.id,
                     pdbx_structure_determination_methodology=mth,
                     pdbx_model_details=system.model_details)


class _CommentDumper(Dumper):
    def dump(self, system, writer):
        for comment in system.comments:
            writer.write_comment(comment)


class _SoftwareDumper(Dumper):
    def finalize(self, system):
        seen_software = {}
        self._software_by_id = []
        for s in system._all_software():
            util._remove_id(s)
        for s in system._all_software():
            util._assign_id(s, seen_software, self._software_by_id)

    def dump(self, system, writer):
        # todo: specify these attributes in only one place (e.g. in the
        # Software class)
        with writer.loop("_software",
                         ["pdbx_ordinal", "name", "classification",
                          "description", "version", "type", "location",
                          "citation_id"]) as lp:
            for s in self._software_by_id:
                lp.write(pdbx_ordinal=s._id, name=s.name,
                         classification=s.classification,
                         description=s.description, version=s.version,
                         type=s.type, location=s.location,
                         citation_id=s.citation._id if s.citation else None)


class _CitationDumper(Dumper):
    def finalize(self, system):
        primaries = []
        non_primaries = []
        for c in system._all_citations():
            (primaries if c.is_primary else non_primaries).append(c)
        # Put primary citations first in list
        self._all_citations = primaries + non_primaries
        for nc, c in enumerate(self._all_citations):
            c._id = nc + 1
        if primaries:
            if len(primaries) > 1:
                raise ValueError(
                    "Multiple Citations with is_primary=True; only one can "
                    "be primary: %s" % primaries)
            else:
                primaries[0]._id = 'primary'

    def dump(self, system, writer):
        self.dump_citations(self._all_citations, writer)
        self.dump_authors(self._all_citations, writer)

    def dump_citations(self, citations, writer):
        with writer.loop("_citation",
                         ["id", "title", "journal_abbrev", "journal_volume",
                          "page_first", "page_last", "year",
                          "pdbx_database_id_PubMed",
                          "pdbx_database_id_DOI"]) as lp:
            for c in citations:
                if isinstance(c.page_range, (tuple, list)):
                    page_first, page_last = c.page_range
                else:
                    page_first = c.page_range
                    page_last = None
                lp.write(id=c._id, title=c.title, journal_abbrev=c.journal,
                         journal_volume=c.volume, page_first=page_first,
                         page_last=page_last, year=c.year,
                         pdbx_database_id_PubMed=c.pmid,
                         pdbx_database_id_DOI=c.doi)

    def dump_authors(self, citations, writer):
        with writer.loop("_citation_author",
                         ["citation_id", "name", "ordinal"]) as lp:
            ordinal = itertools.count(1)
            for c in citations:
                for a in c.authors:
                    lp.write(citation_id=c._id, name=a, ordinal=next(ordinal))


class _AuditAuthorDumper(Dumper):
    def _get_citation_authors(self, system):
        # If system.authors is empty, get the set of all citation authors
        # instead
        seen_authors = set()
        # Only look at explictly-added citations (since these are likely to
        # describe the modeling) not that describe a method or a piece of
        # software we used (system._all_citations())
        for c in system.citations:
            for a in c.authors:
                if a not in seen_authors:
                    seen_authors.add(a)
                    yield a

    def dump(self, system, writer):
        authors = system.authors or self._get_citation_authors(system)
        with writer.loop("_audit_author",
                         ["name", "pdbx_ordinal"]) as lp:
            for n, author in enumerate(authors):
                lp.write(name=author, pdbx_ordinal=n + 1)


class _GrantDumper(Dumper):
    def dump(self, system, writer):
        with writer.loop("_pdbx_audit_support",
                         ["funding_organization", "country", "grant_number",
                          "ordinal"]) as lp:
            for n, grant in enumerate(system.grants):
                lp.write(funding_organization=grant.funding_organization,
                         country=grant.country,
                         grant_number=grant.grant_number, ordinal=n + 1)


class _ChemCompDumper(Dumper):
    def dump(self, system, writer):
        comps = frozenset(comp for e in system.entities for comp in e.sequence)

        with writer.loop("_chem_comp", ["id", "type", "name",
                                        "formula", "formula_weight"]) as lp:
            for comp in sorted(comps, key=operator.attrgetter('id')):
                if comp.ccd or comp.descriptors:
                    raise ValueError(
                        "Non-default values for 'ccd' or 'descriptors' are "
                        "not supported by the IHM dictionary for %s" % comp)
                lp.write(id=comp.id, type=comp.type, name=comp.name,
                         formula=comp.formula,
                         formula_weight=comp.formula_weight)


class _ChemDescriptorDumper(Dumper):
    def finalize(self, system):
        seen_desc = {}
        # Assign IDs to all descriptors
        self._descriptor_by_id = []
        for d in system._all_chem_descriptors():
            util._remove_id(d)
        for d in system._all_chem_descriptors():
            util._assign_id(d, seen_desc, self._descriptor_by_id)

    def dump(self, system, writer):
        with writer.loop(
                "_ihm_chemical_component_descriptor",
                ["id", "auth_name", "chemical_name",
                 "common_name", "smiles", "smiles_canonical", "inchi",
                 "inchi_key"]) as lp:
            # note that we don't write out chem_comp_id; this is no longer
            # present in the dictionary
            for d in self._descriptor_by_id:
                lp.write(id=d._id, auth_name=d.auth_name,
                         chemical_name=d.chemical_name,
                         common_name=d.common_name, smiles=d.smiles,
                         smiles_canonical=d.smiles_canonical, inchi=d.inchi,
                         inchi_key=d.inchi_key)


class _EntityDumper(Dumper):
    def finalize(self, system):
        # Assign IDs and check for duplicates
        seen = {}
        for num, entity in enumerate(system.entities):
            if entity in seen:
                raise ValueError("Duplicate entity %s found" % entity)
            entity._id = num + 1
            seen[entity] = None

    def dump(self, system, writer):
        with writer.loop("_entity",
                         ["id", "type", "src_method", "pdbx_description",
                          "formula_weight", "pdbx_number_of_molecules",
                          "details"]) as lp:
            for entity in system.entities:
                lp.write(id=entity._id, type=entity.type,
                         src_method=entity.src_method,
                         pdbx_description=entity.description,
                         formula_weight=entity.formula_weight,
                         pdbx_number_of_molecules=entity.number_of_molecules,
                         details=entity.details)


def _assign_src_ids(system, srccls):
    """Assign IDs to all entity sources of type `srccls`."""
    # Assign IDs
    seen_src = {}
    src_by_id = []
    for e in system.entities:
        if isinstance(e.source, srccls):
            util._remove_id(e.source)
    for e in system.entities:
        if isinstance(e.source, srccls):
            util._assign_id(e.source, seen_src, src_by_id)


class _EntitySrcGenDumper(Dumper):
    def finalize(self, system):
        _assign_src_ids(system, ihm.source.Manipulated)

    def dump(self, system, writer):
        with writer.loop(
                "_entity_src_gen",
                ["entity_id", "pdbx_src_id", "pdbx_gene_src_ncbi_taxonomy_id",
                 "pdbx_gene_src_scientific_name",
                 "gene_src_common_name", "gene_src_strain",
                 "pdbx_host_org_ncbi_taxonomy_id",
                 "pdbx_host_org_scientific_name",
                 "host_org_common_name", "pdbx_host_org_strain"]) as lp:
            for e in system.entities:
                if isinstance(e.source, ihm.source.Manipulated):
                    self._dump_source(lp, e)

    def _dump_source(self, lp, e):
        s = e.source
        # Skip output if all fields are blank
        if s.gene is None and s.host is None:
            return
        lp.write(entity_id=e._id, pdbx_src_id=s._id,
                 pdbx_gene_src_ncbi_taxonomy_id=s.gene.ncbi_taxonomy_id
                 if s.gene else None,
                 pdbx_gene_src_scientific_name=s.gene.scientific_name
                 if s.gene else None,
                 gene_src_strain=s.gene.strain if s.gene else None,
                 gene_src_common_name=s.gene.common_name if s.gene else None,
                 pdbx_host_org_ncbi_taxonomy_id=s.host.ncbi_taxonomy_id
                 if s.host else None,
                 pdbx_host_org_scientific_name=s.host.scientific_name
                 if s.host else None,
                 host_org_common_name=s.host.common_name if s.host else None,
                 pdbx_host_org_strain=s.host.strain if s.host else None)


class _EntitySrcNatDumper(Dumper):
    def finalize(self, system):
        _assign_src_ids(system, ihm.source.Natural)

    def dump(self, system, writer):
        with writer.loop(
                "_entity_src_nat",
                ["entity_id", "pdbx_src_id", "pdbx_ncbi_taxonomy_id",
                 "pdbx_organism_scientific", "common_name", "strain"]) as lp:
            for e in system.entities:
                s = e.source
                if isinstance(s, ihm.source.Natural):
                    lp.write(entity_id=e._id, pdbx_src_id=s._id,
                             pdbx_ncbi_taxonomy_id=s.ncbi_taxonomy_id,
                             pdbx_organism_scientific=s.scientific_name,
                             common_name=s.common_name, strain=s.strain)


class _EntitySrcSynDumper(Dumper):
    def finalize(self, system):
        _assign_src_ids(system, ihm.source.Synthetic)

    def dump(self, system, writer):
        # Note that _pdbx_entity_src_syn.strain is not used in current PDB
        # entries
        with writer.loop(
                "_pdbx_entity_src_syn",
                ["entity_id", "pdbx_src_id", "ncbi_taxonomy_id",
                 "organism_scientific", "organism_common_name"]) as lp:
            for e in system.entities:
                s = e.source
                if isinstance(s, ihm.source.Synthetic):
                    lp.write(entity_id=e._id, pdbx_src_id=s._id,
                             ncbi_taxonomy_id=s.ncbi_taxonomy_id,
                             organism_scientific=s.scientific_name,
                             organism_common_name=s.common_name)


def _prettyprint_seq(seq, width):
    """Join the sequence of strings together and generate a set of
       lines that don't exceed the provided width."""
    current_width = 0
    line = []
    for s in seq:
        if line and current_width + len(s) > width:
            yield ''.join(line)
            line = []
            current_width = 0
        line.append(s)
        current_width += len(s)
    if line:
        yield ''.join(line)


class _StructRefDumper(Dumper):
    def finalize(self, system):
        ref_id = itertools.count(1)
        align_id = itertools.count(1)
        for e in system.entities:
            for r in e.references:
                r._id = next(ref_id)
                for a in r._get_alignments():
                    a._id = next(align_id)

    def _get_sequence(self, reference):
        """Get the sequence string"""
        if reference.sequence in (None, ihm.unknown):
            return reference.sequence
        # We only want the subset of the sequence that overlaps with
        # our entities
        db_begin = min(a.db_begin for a in reference._get_alignments())
        db_end = max(a.db_end for a in reference._get_alignments())
        # Split into lines to get tidier CIF output
        return "\n".join(_prettyprint_seq(
            reference.sequence[db_begin - 1:db_end], 70))

    def _check_seq_dif(self, entity, ref, align):
        """Check all SeqDif objects for the Entity sequence. Return the mutated
           sequence (to match the reference)."""
        entseq = [comp.code_canonical for comp in entity.sequence]
        for sd in align.seq_dif:
            if sd.seq_id < 1 or sd.seq_id > len(entseq):
                raise IndexError("SeqDif.seq_id for %s is %d, out of "
                                 "range 1-%d"
                                 % (entity, sd.seq_id, len(entseq)))
            if sd.monomer.code_canonical != entseq[sd.seq_id - 1]:
                raise ValueError("SeqDif.monomer one-letter code (%s) does "
                                 "not match that in %s (%s at position %d)"
                                 % (sd.monomer.code_canonical, entity,
                                    entseq[sd.seq_id - 1], sd.seq_id))
            entseq[sd.seq_id - 1] = sd.db_monomer.code_canonical
        return ''.join(entseq)

    def _get_ranges(self, entity, ref, align):
        """Get the sequence ranges for an Entity and Reference"""
        return ((align.entity_begin,
                 len(entity.sequence) if align.entity_end is None
                 else align.entity_end),
                (align.db_begin,
                 len(ref.sequence) if align.db_end is None else align.db_end))

    def _check_reference_sequence(self, entity, ref):
        """Make sure that the Entity and Reference sequences match"""
        for align in ref._get_alignments():
            self._check_alignment(entity, ref, align)

    def _check_alignment(self, entity, ref, align):
        """Make sure that an alignment makes sense"""
        if ref.sequence in (None, ihm.unknown):
            # We just have to trust the range if the ref sequence is blank
            return
        entseq = self._check_seq_dif(entity, ref, align)

        def check_rng(rng, seq, rngstr, obj):
            if any(r < 1 or r > len(seq) for r in rng):
                raise IndexError("Alignment.%s for %s is (%d-%d), "
                                 "out of range 1-%d"
                                 % (rngstr, obj, rng[0], rng[1], len(seq)))
        entity_rng, db_rng = self._get_ranges(entity, ref, align)
        check_rng(entity_rng, entseq, "entity_begin,entity_end", entity)
        check_rng(db_rng, ref.sequence, "db_begin,db_end", ref)

        matchlen = min(entity_rng[1] - entity_rng[0], db_rng[1] - db_rng[0])
        entseq = entseq[entity_rng[0] - 1:entity_rng[0] + matchlen - 1]
        refseq = ref.sequence[db_rng[0] - 1:db_rng[0] + matchlen - 1]
        if refseq != entseq:
            raise ValueError(
                "Reference sequence from %s does not match entity canonical"
                " sequence (after mutations) for %s - you may need to "
                "adjust Alignment.db_begin,db_end (%d-%d), "
                "Alignment.entity_begin,entity_end (%d-%d), "
                "or add to Alignment.seq_dif:\n"
                "Reference: %s\nEntity:    %s\n"
                "Match:     %s"
                % (ref, entity, db_rng[0], db_rng[1],
                   entity_rng[0], entity_rng[1], refseq, entseq,
                   ''.join('*' if a == b else ' '
                           for (a, b) in zip(refseq, entseq))))

    def dump(self, system, writer):
        with writer.loop(
                "_struct_ref",
                ["id", "entity_id", "db_name", "db_code", "pdbx_db_accession",
                 "pdbx_align_begin", "pdbx_seq_one_letter_code",
                 "details"]) as lp:
            for e in system.entities:
                for r in e.references:
                    self._check_reference_sequence(e, r)
                    db_begin = min(a.db_begin for a in r._get_alignments())
                    lp.write(id=r._id, entity_id=e._id, db_name=r.db_name,
                             db_code=r.db_code, pdbx_db_accession=r.accession,
                             pdbx_align_begin=db_begin, details=r.details,
                             pdbx_seq_one_letter_code=self._get_sequence(r))
        self.dump_seq(system, writer)
        self.dump_seq_dif(system, writer)

    def dump_seq(self, system, writer):
        def _all_alignments():
            for e in system.entities:
                for r in e.references:
                    for a in r._get_alignments():
                        yield e, r, a
        with writer.loop(
                "_struct_ref_seq",
                ["align_id", "ref_id", "seq_align_beg", "seq_align_end",
                 "db_align_beg", "db_align_end"]) as lp:
            for e, r, a in _all_alignments():
                entity_rng, db_rng = self._get_ranges(e, r, a)
                matchlen = min(entity_rng[1] - entity_rng[0],
                               db_rng[1] - db_rng[0])
                lp.write(align_id=a._id, ref_id=r._id,
                         seq_align_beg=entity_rng[0],
                         seq_align_end=entity_rng[0] + matchlen,
                         db_align_beg=db_rng[0],
                         db_align_end=db_rng[0] + matchlen)

    def dump_seq_dif(self, system, writer):
        ordinal = itertools.count(1)
        with writer.loop(
                "_struct_ref_seq_dif",
                ["pdbx_ordinal", "align_id", "seq_num", "db_mon_id", "mon_id",
                 "details"]) as lp:
            for e in system.entities:
                for r in e.references:
                    for a in r._get_alignments():
                        for sd in a.seq_dif:
                            lp.write(pdbx_ordinal=next(ordinal),
                                     align_id=a._id, seq_num=sd.seq_id,
                                     db_mon_id=sd.db_monomer.id,
                                     mon_id=sd.monomer.id, details=sd.details)


class _EntityPolyDumper(Dumper):
    def __init__(self):
        super(_EntityPolyDumper, self).__init__()

        # Determine the type of the entire entity's sequence based on the
        # type(s) of all chemical components it contains
        self._seq_type_map = {
            frozenset(('D-peptide linking',)): 'polypeptide(D)',
            frozenset(('D-peptide linking',
                       'peptide linking')): 'polypeptide(D)',
            frozenset(('RNA linking',)): 'polyribonucleotide',
            frozenset(('DNA linking',)): 'polydeoxyribonucleotide',
            frozenset(('DNA linking', 'RNA linking')):
                'polydeoxyribonucleotide/polyribonucleotide hybrid'}

    def _get_sequence(self, entity):
        """Get the sequence for an entity as a string"""
        # Split into lines to get tidier CIF output
        return "\n".join(_prettyprint_seq((comp.code if len(comp.code) == 1
                                           else '(%s)' % comp.code
                                           for comp in entity.sequence), 70))

    def _get_canon(self, entity):
        """Get the canonical sequence for an entity as a string"""
        # Split into lines to get tidier CIF output
        seq = "\n".join(_prettyprint_seq(
            (comp.code_canonical for comp in entity.sequence), 70))
        return seq

    def _get_seq_type(self, entity):
        """Get the sequence type for an entity"""
        all_types = frozenset(comp.type for comp in entity.sequence)
        # For a mix of L-peptides and D-peptides, current PDB entries always
        # seem to use 'polypeptide(L)' so let's do that too:
        if 'L-peptide linking' in all_types:
            return 'polypeptide(L)'
        else:
            return self._seq_type_map.get(all_types, 'other')

    def dump(self, system, writer):
        # Get all asym units (if any) for each entity
        strands = collections.defaultdict(list)
        for asym in system.asym_units:
            strands[asym.entity._id].append(asym.strand_id)
        with writer.loop("_entity_poly",
                         ["entity_id", "type", "nstd_linkage",
                          "nstd_monomer", "pdbx_strand_id",
                          "pdbx_seq_one_letter_code",
                          "pdbx_seq_one_letter_code_can"]) as lp:
            for entity in system.entities:
                if not entity.is_polymeric():
                    continue
                nstd = any(isinstance(x, ihm.NonPolymerChemComp)
                           for x in entity.sequence)
                sids = strands[entity._id]
                lp.write(entity_id=entity._id, type=self._get_seq_type(entity),
                         nstd_linkage='no',
                         nstd_monomer='yes' if nstd else 'no',
                         pdbx_strand_id=",".join(sids) if sids else None,
                         pdbx_seq_one_letter_code=self._get_sequence(entity),
                         pdbx_seq_one_letter_code_can=self._get_canon(entity))


class _EntityNonPolyDumper(Dumper):
    def dump(self, system, writer):
        with writer.loop("_pdbx_entity_nonpoly",
                         ["entity_id", "name", "comp_id"]) as lp:
            for entity in system.entities:
                if entity.is_polymeric():
                    continue
                lp.write(entity_id=entity._id, name=entity.description,
                         comp_id=entity.sequence[0].id)


class _EntityPolySeqDumper(Dumper):
    def dump(self, system, writer):
        with writer.loop("_entity_poly_seq",
                         ["entity_id", "num", "mon_id", "hetero"]) as lp:
            for entity in system.entities:
                if not entity.is_polymeric():
                    continue
                for num, comp in enumerate(entity.sequence):
                    lp.write(entity_id=entity._id, num=num + 1, mon_id=comp.id)


class _EntityPolySegmentDumper(Dumper):
    def finalize(self, system):
        seen_ranges = {}
        self._ranges_by_id = []
        # Need to assign ranges for all starting models too
        for sm in system._all_starting_models():
            rng = sm.asym_unit
            util._remove_id(rng, attr='_range_id')

        for rng in system._all_entity_ranges():
            util._remove_id(rng, attr='_range_id')
        for rng in itertools.chain(system._all_entity_ranges(),
                                   (sm.asym_unit
                                    for sm in system._all_starting_models())):
            entity = rng.entity if hasattr(rng, 'entity') else rng
            if entity.is_polymeric():
                util._assign_id(rng, seen_ranges, self._ranges_by_id,
                                attr='_range_id',
                                # Two ranges are considered the same if they
                                # have the same entity ID and refer to
                                # the same residue range
                                seen_obj=(entity._id, rng.seq_id_range))
            else:
                rng._range_id = None

    def dump(self, system, writer):
        with writer.loop("_ihm_entity_poly_segment",
                         ["id", "entity_id", "seq_id_begin", "seq_id_end",
                          "comp_id_begin", "comp_id_end"]) as lp:
            for rng in self._ranges_by_id:
                entity = rng.entity if hasattr(rng, 'entity') else rng
                lp.write(
                    id=rng._range_id, entity_id=entity._id,
                    seq_id_begin=rng.seq_id_range[0],
                    seq_id_end=rng.seq_id_range[1],
                    comp_id_begin=entity.sequence[rng.seq_id_range[0] - 1].id,
                    comp_id_end=entity.sequence[rng.seq_id_range[1] - 1].id)


class _PolySeqSchemeDumper(Dumper):
    """Output the _pdbx_poly_seq_scheme table.
       This is needed because it is a parent category of atom_site.
       For now we assume we're using auth_seq_num==pdb_seq_num."""
    def dump(self, system, writer):
        with writer.loop("_pdbx_poly_seq_scheme",
                         ["asym_id", "entity_id", "seq_id", "mon_id",
                          "pdb_seq_num", "auth_seq_num", "pdb_mon_id",
                          "auth_mon_id", "pdb_strand_id",
                          "pdb_ins_code"]) as lp:
            for asym in system.asym_units:
                entity = asym.entity
                if not entity.is_polymeric():
                    continue
                for num, comp in enumerate(entity.sequence):
                    auth_seq_num, ins = asym._get_auth_seq_id_ins_code(num + 1)
                    lp.write(asym_id=asym._id, pdb_strand_id=asym.strand_id,
                             entity_id=entity._id,
                             seq_id=num + 1, pdb_seq_num=auth_seq_num,
                             auth_seq_num=auth_seq_num,
                             mon_id=comp.id, pdb_mon_id=comp.id,
                             auth_mon_id=comp.id,
                             pdb_ins_code=ins)


class _NonPolySchemeDumper(Dumper):
    """Output the _pdbx_nonpoly_scheme table.
       For now we assume we're using auth_seq_num==pdb_seq_num."""
    def dump(self, system, writer):
        with writer.loop("_pdbx_nonpoly_scheme",
                         ["asym_id", "entity_id", "mon_id", "ndb_seq_num",
                          "pdb_seq_num", "auth_seq_num",
                          "auth_mon_id", "pdb_strand_id",
                          "pdb_ins_code"]) as lp:
            for asym in system.asym_units:
                entity = asym.entity
                if entity.is_polymeric():
                    continue
                # todo: handle multiple waters
                for num, comp in enumerate(entity.sequence):
                    auth_seq_num, ins = asym._get_auth_seq_id_ins_code(num + 1)
                    # ndb_seq_num is described as the "NDB/RCSB residue
                    # number". We don't have one of those but real PDBs
                    # usually seem to just count sequentially from 1, so
                    # we'll do that too.
                    lp.write(asym_id=asym._id, pdb_strand_id=asym.strand_id,
                             entity_id=entity._id,
                             ndb_seq_num=num + 1,
                             pdb_seq_num=auth_seq_num,
                             auth_seq_num=auth_seq_num,
                             mon_id=comp.id,
                             auth_mon_id=comp.id, pdb_ins_code=ins)


class _AsymIDProvider(object):
    """Provide unique asym IDs"""
    def __init__(self, seen_ids):
        self.seen_ids = seen_ids
        self.ids = util._AsymIDs()
        self.index = -1

    def get_next_id(self):
        """Get the next unique ID"""
        self.index += 1
        while self.ids[self.index] in self.seen_ids:
            self.index += 1
        # Note tha we don't need to add our own IDs to seen_ids since
        # they are already guaranteed to be unique
        return self.ids[self.index]


class _StructAsymDumper(Dumper):
    def finalize(self, system):
        # Handle user-assigned IDs first
        seen_asym_ids = set()
        duplicates = set()
        for asym in system.asym_units:
            if asym.id is not None:
                if asym.id in seen_asym_ids:
                    duplicates.add(asym.id)
                asym._id = asym.id
                seen_asym_ids.add(asym.id)
        if duplicates:
            raise ValueError("One or more duplicate asym (chain) IDs "
                             "detected - %s" % ", ".join(sorted(duplicates)))
        ordinal = itertools.count(1)
        # Assign remaining asym IDs
        id_prov = _AsymIDProvider(seen_asym_ids)
        for asym in system.asym_units:
            if asym.id is None:
                asym._id = id_prov.get_next_id()
            asym._ordinal = next(ordinal)

    def dump(self, system, writer):
        with writer.loop("_struct_asym",
                         ["id", "entity_id", "details"]) as lp:
            for asym in system.asym_units:
                lp.write(id=asym._id, entity_id=asym.entity._id,
                         details=asym.details)


class _AssemblyDumperBase(Dumper):
    def finalize(self, system):
        # Sort each assembly by entity id/asym id/range
        def component_key(comp):
            return (comp.entity._id, comp._ordinal, comp.seq_id_range)
        for a in system._all_assemblies():
            a.sort(key=component_key)

        seen_assemblies = {}
        # Assign IDs to all assemblies; duplicate assemblies get same ID
        self._assembly_by_id = []
        description_by_id = {}
        all_assemblies = list(system._all_assemblies())
        seen_assembly_ids = {}
        for a in all_assemblies:
            # list isn't hashable but tuple is
            hasha = tuple(a)
            if hasha not in seen_assemblies:
                self._assembly_by_id.append(a)
                seen_assemblies[hasha] = a._id = len(self._assembly_by_id)
                description_by_id[a._id] = []
            else:
                a._id = seen_assemblies[hasha]
            if a.description and id(a) not in seen_assembly_ids:
                descs = description_by_id[a._id]
                # Don't duplicate descriptions
                if len(descs) == 0 or descs[-1] != a.description:
                    descs.append(a.description)
            seen_assembly_ids[id(a)] = None

        # If multiple assemblies map to the same ID, give them all the same
        # composite description
        for a_id, description in description_by_id.items():
            description_by_id[a_id] = ' & '.join(description) \
                                      if description else None
        for a in all_assemblies:
            a.description = description_by_id[a._id]


class _AssemblyDumper(_AssemblyDumperBase):
    def dump(self, system, writer):
        self.dump_summary(system, writer)
        self.dump_details(system, writer)

    def dump_summary(self, system, writer):
        with writer.loop("_ihm_struct_assembly",
                         ["id", "name", "description"]) as lp:
            for a in self._assembly_by_id:
                lp.write(id=a._id, name=a.name, description=a.description)

    def dump_details(self, system, writer):
        ordinal = itertools.count(1)
        with writer.loop("_ihm_struct_assembly_details",
                         ["id", "assembly_id", "parent_assembly_id",
                          "entity_description", "entity_id", "asym_id",
                          "entity_poly_segment_id"]) as lp:
            for a in self._assembly_by_id:
                for comp in a:
                    entity = comp.entity if hasattr(comp, 'entity') else comp
                    lp.write(
                        id=next(ordinal), assembly_id=a._id,
                        # if no hierarchy then assembly is self-parent
                        parent_assembly_id=a.parent._id if a.parent else a._id,
                        entity_description=entity.description,
                        entity_id=entity._id,
                        asym_id=comp._id if hasattr(comp, 'entity') else None,
                        entity_poly_segment_id=comp._range_id)


class _ExternalReferenceDumper(Dumper):
    """Output information on externally referenced files
       (i.e. anything that refers to a Location that isn't
       a DatabaseLocation)."""

    class _LocalFiles(object):
        reference_provider = None
        reference_type = 'Supplementary Files'
        reference = None
        refers_to = 'Other'
        url = None
        details = None

        def __init__(self, top_directory):
            self.top_directory = top_directory

        def _get_full_path(self, path):
            return os.path.relpath(path, start=self.top_directory)

    def finalize(self, system):
        # Keep only locations that don't point into databases (these are
        # handled elsewhere)
        self._refs = [x for x in system._all_locations()
                      if not isinstance(x, location.DatabaseLocation)]
        # Assign IDs to all locations and repos (including the None repo, which
        # is for local files)
        seen_refs = {}
        seen_repos = {}
        self._ref_by_id = []
        self._repo_by_id = []
        # Special dummy repo for repo=None (local files)
        self._local_files = self._LocalFiles(os.getcwd())
        for r in self._refs:
            util._remove_id(r)
            if r.repo:
                util._remove_id(r.repo)
        for r in self._refs:
            # Assign a unique ID to the reference
            util._assign_id(r, seen_refs, self._ref_by_id)
            # Assign a unique ID to the repository
            util._assign_id(r.repo or self._local_files,
                            seen_repos, self._repo_by_id)

    def dump(self, system, writer):
        self.dump_repos(writer)
        self.dump_refs(writer)

    def dump_repos(self, writer):
        with writer.loop("_ihm_external_reference_info",
                         ["reference_id", "reference_provider",
                          "reference_type", "reference", "refers_to",
                          "associated_url", "details"]) as lp:
            for repo in self._repo_by_id:
                lp.write(reference_id=repo._id,
                         reference_provider=repo.reference_provider,
                         reference_type=repo.reference_type,
                         reference=repo.reference, refers_to=repo.refers_to,
                         associated_url=repo.url, details=repo.details)

    def dump_refs(self, writer):
        with writer.loop("_ihm_external_files",
                         ["id", "reference_id", "file_path", "content_type",
                          "file_size_bytes", "details"]) as lp:
            for r in self._ref_by_id:
                repo = r.repo or self._local_files
                if r.path is None:
                    file_path = None
                else:
                    file_path = self._posix_path(repo._get_full_path(r.path))
                lp.write(id=r._id, reference_id=repo._id,
                         file_path=file_path, content_type=r.content_type,
                         file_size_bytes=r.file_size, details=r.details)

    # On Windows systems, convert native paths to POSIX-like (/-separated)
    # paths
    if os.sep == '/':
        def _posix_path(self, path):
            return path
    else:
        def _posix_path(self, path):
            return path.replace(os.sep, '/')


class _DatasetDumper(Dumper):
    def finalize(self, system):
        def _all_transforms(dataset):
            for p in dataset.parents:
                if isinstance(p, ihm.dataset.TransformedDataset):
                    yield p.transform
        seen_datasets = {}
        seen_transforms = {}
        # Assign IDs to all datasets and transforms
        self._dataset_by_id = []
        self._transform_by_id = []
        for d in system._all_datasets():
            for t in _all_transforms(d):
                # Can't use default _id attribute here since a given transform
                # may be used by both a dataset and a geometric object, and
                # since they live in different tables they need different IDs
                util._remove_id(t, attr='_dtid')
            util._remove_id(d)
        for d in system._all_datasets():
            util._assign_id(d, seen_datasets, self._dataset_by_id)
            for t in _all_transforms(d):
                util._assign_id(t, seen_transforms, self._transform_by_id,
                                attr='_dtid')

        # Assign IDs to all groups and remove duplicates
        seen_group_ids = {}
        self._dataset_group_by_id = []
        for g in system._all_dataset_groups():
            ids = tuple(sorted(d._id for d in g))
            if ids not in seen_group_ids:
                self._dataset_group_by_id.append(g)
                g._id = len(self._dataset_group_by_id)
                seen_group_ids[ids] = g
            else:
                g._id = seen_group_ids[ids]._id

    def dump(self, system, writer):
        with writer.loop("_ihm_dataset_list",
                         ["id", "data_type", "database_hosted",
                          "details"]) as lp:
            for d in self._dataset_by_id:
                lp.write(id=d._id, data_type=d.data_type, details=d.details,
                         database_hosted=isinstance(d.location,
                                                    location.DatabaseLocation))
        self.dump_groups(writer)
        self.dump_other((d for d in self._dataset_by_id
                         if d.location is not None
                         and not isinstance(d.location,
                                            location.DatabaseLocation)),
                        writer)
        self.dump_rel_dbs((d for d in self._dataset_by_id
                           if isinstance(d.location,
                                         location.DatabaseLocation)),
                          writer)
        self.dump_related(system, writer)
        self.dump_related_transform(system, writer)

    def dump_groups(self, writer):
        self.dump_group_summary(writer)
        self.dump_group_links(writer)

    def dump_group_summary(self, writer):
        with writer.loop("_ihm_dataset_group",
                         ["id", "name", "application", "details"]) as lp:
            for g in self._dataset_group_by_id:
                lp.write(id=g._id, name=g.name, application=g.application,
                         details=g.details)

    def dump_group_links(self, writer):
        with writer.loop("_ihm_dataset_group_link",
                         ["group_id", "dataset_list_id"]) as lp:
            for g in self._dataset_group_by_id:
                # Don't duplicate IDs, and output in sorted order
                for dataset_id in sorted(set(d._id for d in g)):
                    lp.write(group_id=g._id, dataset_list_id=dataset_id)

    def dump_other(self, datasets, writer):
        ordinal = itertools.count(1)
        with writer.loop("_ihm_dataset_external_reference",
                         ["id", "dataset_list_id", "file_id"]) as lp:
            for d in datasets:
                lp.write(id=next(ordinal), dataset_list_id=d._id,
                         file_id=d.location._id)

    def dump_rel_dbs(self, datasets, writer):
        ordinal = itertools.count(1)
        with writer.loop("_ihm_dataset_related_db_reference",
                         ["id", "dataset_list_id", "db_name",
                          "accession_code", "version", "details"]) as lp:
            for d in datasets:
                lp.write(id=next(ordinal), dataset_list_id=d._id,
                         db_name=d.location.db_name,
                         accession_code=d.location.access_code,
                         version=d.location.version,
                         details=d.location.details)

    def dump_related(self, system, writer):
        with writer.loop("_ihm_related_datasets",
                         ["dataset_list_id_derived",
                          "dataset_list_id_primary",
                          "transformation_id"]) as lp:
            for derived in self._dataset_by_id:
                ids = set()
                for p in derived.parents:
                    if isinstance(p, ihm.dataset.TransformedDataset):
                        ids.add((p.dataset._id, p.transform._dtid))
                    else:
                        ids.add((p._id, None))
                # Don't duplicate IDs, and sort by parent ID (cannot sort
                # by transform ID because it might be None and we can't
                # compare None with int)
                for pid, tid in sorted(ids, key=operator.itemgetter(0)):
                    lp.write(dataset_list_id_derived=derived._id,
                             dataset_list_id_primary=pid,
                             transformation_id=tid)

    def dump_related_transform(self, system, writer):
        with writer.loop(
                "_ihm_data_transformation",
                ["id",
                 "rot_matrix[1][1]", "rot_matrix[2][1]", "rot_matrix[3][1]",
                 "rot_matrix[1][2]", "rot_matrix[2][2]", "rot_matrix[3][2]",
                 "rot_matrix[1][3]", "rot_matrix[2][3]", "rot_matrix[3][3]",
                 "tr_vector[1]", "tr_vector[2]", "tr_vector[3]"]) as lp:
            for t in self._transform_by_id:
                lp.write(id=t._dtid,
                         **_get_transform(t.rot_matrix, t.tr_vector))


class _ModelRepresentationDumper(Dumper):
    def finalize(self, system):
        # Assign IDs to representations and segments
        for nr, r in enumerate(system._all_representations()):
            r._id = nr + 1
            for ns, s in enumerate(r):
                s._id = ns + 1

    def dump(self, system, writer):
        self.dump_summary(system, writer)
        self.dump_details(system, writer)

    def dump_summary(self, system, writer):
        with writer.loop("_ihm_model_representation",
                         ["id", "name", "details"]) as lp:
            for r in system._all_representations():
                lp.write(id=r._id, name=r.name, details=r.details)

    def dump_details(self, system, writer):
        ordinal = itertools.count(1)
        with writer.loop("_ihm_model_representation_details",
                         ["id", "representation_id",
                          "entity_id", "entity_description",
                          "entity_asym_id", "entity_poly_segment_id",
                          "model_object_primitive", "starting_model_id",
                          "model_mode", "model_granularity",
                          "model_object_count", "description"]) as lp:
            for r in system._all_representations():
                for segment in r:
                    entity = segment.asym_unit.entity
                    lp.write(
                        id=next(ordinal), representation_id=r._id,
                        entity_id=entity._id,
                        entity_description=entity.description,
                        entity_asym_id=segment.asym_unit._id,
                        entity_poly_segment_id=segment.asym_unit._range_id,
                        model_object_primitive=segment.primitive,
                        starting_model_id=segment.starting_model._id
                        if segment.starting_model else None,
                        model_mode='rigid' if segment.rigid else 'flexible',
                        model_granularity=segment.granularity,
                        model_object_count=segment.count,
                        description=segment.description)


class _StartingModelDumper(Dumper):
    def finalize(self, system):
        # Assign IDs to starting models
        for nm, m in enumerate(system._all_starting_models()):
            m._id = nm + 1

    def dump(self, system, writer):
        self.dump_details(system, writer)
        self.dump_computational(system, writer)
        self.dump_comparative(system, writer)
        self.dump_coords(system, writer)
        self.dump_seq_dif(system, writer)

    def dump_details(self, system, writer):
        # Map dataset types to starting model sources
        source_map = {'Comparative model': 'comparative model',
                      'Integrative model': 'integrative model',
                      'Experimental model': 'experimental model',
                      'De Novo model': 'ab initio model',
                      'Other': 'other'}
        with writer.loop(
                "_ihm_starting_model_details",
                ["starting_model_id", "entity_id", "entity_description",
                 "asym_id", "entity_poly_segment_id",
                 "starting_model_source",
                 "starting_model_auth_asym_id",
                 "starting_model_sequence_offset",
                 "dataset_list_id", "description"]) as lp:
            for sm in system._all_starting_models():
                lp.write(
                    starting_model_id=sm._id,
                    entity_id=sm.asym_unit.entity._id,
                    entity_description=sm.asym_unit.entity.description,
                    asym_id=sm.asym_unit._id,
                    entity_poly_segment_id=sm.asym_unit._range_id,
                    starting_model_source=source_map[sm.dataset.data_type],
                    starting_model_auth_asym_id=sm.asym_id,
                    dataset_list_id=sm.dataset._id,
                    starting_model_sequence_offset=sm.offset,
                    description=sm.description)

    def dump_computational(self, system, writer):
        """Dump details on computational models."""
        with writer.loop(
                "_ihm_starting_computational_models",
                ["starting_model_id", "software_id",
                 "script_file_id"]) as lp:
            for sm in system._all_starting_models():
                if sm.software or sm.script_file:
                    lp.write(starting_model_id=sm._id,
                             software_id=sm.software._id
                             if sm.software else None,
                             script_file_id=sm.script_file._id
                             if sm.script_file else None)

    def dump_comparative(self, system, writer):
        """Dump details on comparative models."""
        with writer.loop(
                "_ihm_starting_comparative_models",
                ["id", "starting_model_id",
                 "starting_model_auth_asym_id",
                 "starting_model_seq_id_begin",
                 "starting_model_seq_id_end",
                 "template_auth_asym_id", "template_seq_id_begin",
                 "template_seq_id_end", "template_sequence_identity",
                 "template_sequence_identity_denominator",
                 "template_dataset_list_id",
                 "alignment_file_id"]) as lp:
            ordinal = itertools.count(1)
            for sm in system._all_starting_models():
                for template in sm.templates:
                    self._dump_template(template, sm, lp, ordinal)

    def _dump_template(self, template, sm, lp, ordinal):
        off = sm.offset
        denom = template.sequence_identity.denominator
        if denom is not None and denom is not ihm.unknown:
            denom = int(denom)
        lp.write(id=next(ordinal),
                 starting_model_id=sm._id,
                 starting_model_auth_asym_id=sm.asym_id,
                 starting_model_seq_id_begin=template.seq_id_range[0] + off,
                 starting_model_seq_id_end=template.seq_id_range[1] + off,
                 template_auth_asym_id=template.asym_id,
                 template_seq_id_begin=template.template_seq_id_range[0],
                 template_seq_id_end=template.template_seq_id_range[1],
                 template_sequence_identity=template.sequence_identity.value,
                 template_sequence_identity_denominator=denom,
                 template_dataset_list_id=template.dataset._id
                 if template.dataset else None,
                 alignment_file_id=template.alignment_file._id
                 if template.alignment_file else None)

    def dump_coords(self, system, writer):
        """Write out coordinate information"""
        ordinal = itertools.count(1)
        with writer.loop(
                "_ihm_starting_model_coord",
                ["starting_model_id", "group_PDB", "id", "type_symbol",
                 "atom_id", "comp_id", "entity_id", "asym_id",
                 "seq_id", "Cartn_x", "Cartn_y", "Cartn_z", "B_iso_or_equiv",
                 "ordinal_id"]) as lp:
            for model in system._all_starting_models():
                for natom, atom in enumerate(model.get_atoms()):
                    comp = atom.asym_unit.entity.sequence[atom.seq_id - 1]
                    lp.write(starting_model_id=model._id,
                             group_PDB='HETATM' if atom.het else 'ATOM',
                             id=natom + 1,
                             type_symbol=atom.type_symbol,
                             atom_id=atom.atom_id,
                             comp_id=comp.id,
                             asym_id=atom.asym_unit._id,
                             entity_id=atom.asym_unit.entity._id,
                             seq_id=atom.seq_id,
                             Cartn_x=atom.x, Cartn_y=atom.y, Cartn_z=atom.z,
                             B_iso_or_equiv=atom.biso,
                             ordinal_id=next(ordinal))

    def dump_seq_dif(self, system, writer):
        """Write out sequence difference information"""
        ordinal = itertools.count(1)
        with writer.loop(
                "_ihm_starting_model_seq_dif",
                ["id", "entity_id", "asym_id",
                 "seq_id", "comp_id", "starting_model_id",
                 "db_asym_id", "db_seq_id", "db_comp_id",
                 "details"]) as lp:
            for model in system._all_starting_models():
                for sd in model.get_seq_dif():
                    comp = model.asym_unit.entity.sequence[sd.seq_id - 1]
                    lp.write(
                        id=next(ordinal),
                        entity_id=model.asym_unit.entity._id,
                        asym_id=model.asym_unit._id,
                        seq_id=sd.seq_id, comp_id=comp.id,
                        db_asym_id=model.asym_id, db_seq_id=sd.db_seq_id,
                        db_comp_id=sd.db_comp_id, starting_model_id=model._id,
                        details=sd.details)


class _ProtocolDumper(Dumper):
    def finalize(self, system):
        # Assign IDs to protocols and steps
        for np, p in enumerate(system._all_protocols()):
            p._id = np + 1
            for ns, s in enumerate(p.steps):
                s._id = ns + 1

    def dump(self, system, writer):
        self.dump_summary(system, writer)
        self.dump_details(system, writer)

    def dump_summary(self, system, writer):
        with writer.loop("_ihm_modeling_protocol",
                         ["id", "protocol_name", "num_steps"]) as lp:
            for p in system._all_protocols():
                lp.write(id=p._id,
                         protocol_name=p.name, num_steps=len(p.steps))

    def dump_details(self, system, writer):
        ordinal = itertools.count(1)
        with writer.loop("_ihm_modeling_protocol_details",
                         ["id", "protocol_id", "step_id",
                          "struct_assembly_id", "dataset_group_id",
                          "step_name", "step_method", "num_models_begin",
                          "num_models_end", "multi_scale_flag",
                          "multi_state_flag", "ordered_flag",
                          "ensemble_flag", "software_id", "script_file_id",
                          "description"]) as lp:
            for p in system._all_protocols():
                for s in p.steps:
                    if s.ensemble == 'default':
                        ensemble = len(system.ensembles) > 0
                    else:
                        ensemble = s.ensemble
                    lp.write(
                        id=next(ordinal), protocol_id=p._id,
                        step_id=s._id,
                        struct_assembly_id=s.assembly._id,
                        dataset_group_id=s.dataset_group._id
                        if s.dataset_group else None,
                        step_name=s.name, step_method=s.method,
                        num_models_begin=s.num_models_begin,
                        num_models_end=s.num_models_end,
                        multi_state_flag=s.multi_state,
                        ordered_flag=s.ordered,
                        multi_scale_flag=s.multi_scale,
                        ensemble_flag=ensemble,
                        software_id=s.software._id if s.software else None,
                        script_file_id=s.script_file._id
                        if s.script_file else None,
                        description=s.description)


class _PostProcessDumper(Dumper):
    def finalize(self, system):
        pp_id = itertools.count(1)
        # Assign IDs to analyses and steps
        # todo: handle case where one analysis is referred to from multiple
        # protocols
        for p in system._all_protocols():
            for na, a in enumerate(p.analyses):
                a._id = na + 1
                for ns, s in enumerate(a.steps):
                    s._id = ns + 1
                    # Assign globally unique postproc id
                    s._post_proc_id = next(pp_id)

    def dump(self, system, writer):
        with writer.loop("_ihm_modeling_post_process",
                         ["id", "protocol_id", "analysis_id", "step_id",
                          "type", "feature", "num_models_begin",
                          "num_models_end", "struct_assembly_id",
                          "dataset_group_id", "software_id",
                          "script_file_id", "details"]) as lp:
            for p in system._all_protocols():
                for a in p.analyses:
                    for s in a.steps:
                        lp.write(
                            id=s._post_proc_id, protocol_id=p._id,
                            analysis_id=a._id, step_id=s._id, type=s.type,
                            feature=s.feature,
                            num_models_begin=s.num_models_begin,
                            num_models_end=s.num_models_end,
                            struct_assembly_id=s.assembly._id if s.assembly
                            else None,
                            dataset_group_id=s.dataset_group._id
                            if s.dataset_group else None,
                            software_id=s.software._id if s.software
                            else None,
                            script_file_id=s.script_file._id
                            if s.script_file else None,
                            details=s.details)


class _RangeChecker(object):
    """Check Atom or Sphere objects to make sure they match the
       Representation and Assembly"""
    def __init__(self, model):
        self._setup_representation(model)
        self._setup_assembly(model)
        self._seen_atoms = set()

    def _setup_representation(self, model):
        """Make map from asym_id to representation segments for that ID"""
        r = model.representation if model.representation else []
        self.repr_asym_ids = {}
        for segment in r:
            asym_id = segment.asym_unit._id
            if asym_id not in self.repr_asym_ids:
                self.repr_asym_ids[asym_id] = []
            self.repr_asym_ids[asym_id].append(segment)
        self._last_repr_segment_matched = None

    def _setup_assembly(self, model):
        """Make map from asym_id to assembly seq_id ranges for that ID"""
        a = model.assembly if model.assembly else []
        self.asmb_asym_ids = {}
        for obj in a:
            if hasattr(obj, 'entity'):
                asym_id = obj._id
                if asym_id not in self.asmb_asym_ids:
                    self.asmb_asym_ids[asym_id] = []
                self.asmb_asym_ids[asym_id].append(obj.seq_id_range)
        self._last_asmb_range_matched = None
        self._last_asmb_asym_matched = None

    def _type_check_atom(self, obj, segment):
        """Check an Atom object against a representation segment."""
        # Atom objects can only match an AtomicSegment
        return isinstance(segment, ihm.representation.AtomicSegment)

    def _type_check_sphere(self, obj, segment):
        """Check a Sphere object against a representation segment."""
        if isinstance(segment, ihm.representation.ResidueSegment):
            # Only 1-residue Spheres are OK for by-residue segments
            return obj.seq_id_range[0] == obj.seq_id_range[1]
        elif isinstance(segment, ihm.representation.MultiResidueSegment):
            # Sphere must cover the entire range for multi-residue segments
            return (
                obj.seq_id_range[0] == segment.asym_unit.seq_id_range[0]
                and obj.seq_id_range[1] == segment.asym_unit.seq_id_range[1])
        elif isinstance(segment, ihm.representation.FeatureSegment):
            # Sphere can cover any set of residues but must fall within the
            # segment range for by-feature (already checked)
            return True
        else:
            # Spheres can never be used to represent a by-atom segment
            return False

    def __call__(self, obj):
        """Check the given Atom or Sphere object"""
        asym = obj.asym_unit
        if isinstance(obj, ihm.model.Sphere):
            type_check = self._type_check_sphere
            seq_id_range = obj.seq_id_range
        else:
            type_check = self._type_check_atom
            seq_id_range = (obj.seq_id, obj.seq_id)
            self._check_duplicate_atom(obj)
        self._check_assembly(obj, asym, seq_id_range)
        self._check_representation(obj, asym, type_check, seq_id_range)

    def _check_duplicate_atom(self, atom):
        # e.g. multiple bulk water oxygen atoms can have "same" seq_id (None)
        if atom.seq_id is None:
            return
        k = (atom.asym_unit._id, atom.atom_id, atom.seq_id)
        if k in self._seen_atoms:
            raise ValueError(
                "Multiple atoms with same atom_id (%s) and seq_id (%d) "
                "found in asym ID %s"
                % (atom.atom_id, atom.seq_id, atom.asym_unit._id))
        self._seen_atoms.add(k)

    def _check_assembly(self, obj, asym, seq_id_range):
        # Check last match first
        last_rng = self._last_asmb_range_matched
        if last_rng and asym._id == self._last_asmb_asym_matched \
           and _is_subrange(seq_id_range, last_rng):
            return
        # Check asym_id
        if asym._id not in self.asmb_asym_ids:
            raise ValueError(
                "%s refers to an asym ID (%s) that is not in this "
                "model's assembly (which includes the following asym IDs: %s)"
                % (obj, asym._id,
                   ", ".join(sorted(a for a in self.asmb_asym_ids))))
        # Check range
        for rng in self.asmb_asym_ids[asym._id]:
            if _is_subrange(seq_id_range, rng):
                self._last_asmb_asym_matched = asym._id
                self._last_asmb_range_matched = rng
                return

        def print_range(rng):
            if rng == (None, None):
                return "None"
            else:
                return "%d-%d" % rng
        raise ValueError(
            "%s seq_id range (%s) does not match any range "
            "in the assembly for asym ID %s (ranges are %s)"
            % (obj, print_range(seq_id_range), asym._id,
               ", ".join(print_range(x)
                         for x in self.asmb_asym_ids[asym._id])))

    def _check_representation(self, obj, asym, type_check, seq_id_range):
        # Check last match first
        last_seg = self._last_repr_segment_matched
        if last_seg and asym._id == last_seg.asym_unit._id \
           and _is_subrange(seq_id_range, last_seg.asym_unit.seq_id_range) \
           and type_check(obj, last_seg):
            return
        # Check asym_id
        if asym._id not in self.repr_asym_ids:
            raise ValueError(
                "%s refers to an asym ID (%s) that is not in this "
                "model's representation (which includes the following asym "
                "IDs: %s)"
                % (obj, asym._id,
                   ", ".join(sorted(a for a in self.repr_asym_ids))))
        # Check range
        bad_type_segments = []
        for segment in self.repr_asym_ids[asym._id]:
            rng = segment.asym_unit.seq_id_range
            if _is_subrange(seq_id_range, rng):
                if type_check(obj, segment):
                    self._last_repr_segment_matched = segment
                    return
                else:
                    bad_type_segments.append(segment)
        if bad_type_segments:
            raise ValueError(
                "%s does not match the type of any representation "
                "segment in the seq_id_range (%d-%d) for asym ID %s. "
                "Representation segments are: %s"
                % (obj, seq_id_range[0], seq_id_range[1], asym._id,
                   ", ".join(str(s) for s in bad_type_segments)))
        else:
            raise ValueError(
                "%s seq_id range (%d-%d) does not match any range "
                "in the representation for asym ID %s (representation "
                "ranges are %s)"
                % (obj, seq_id_range[0], seq_id_range[1], asym._id,
                   ", ".join("%d-%d" % x.asym_unit.seq_id_range
                             for x in self.repr_asym_ids[asym._id])))


class _ModelDumperBase(Dumper):

    def finalize(self, system):
        # Remove any existing ID
        for g in system._all_model_groups(only_in_states=False):
            if hasattr(g, '_id'):
                del g._id
            for m in g:
                if hasattr(m, '_id'):
                    del m._id
        model_id = itertools.count(1)
        # Assign IDs to models and groups in states
        for ng, g in enumerate(system._all_model_groups()):
            g._id = ng + 1
            for m in g:
                if not hasattr(m, '_id'):
                    m._id = next(model_id)
        # Check for any groups not referenced by states
        for g in system._all_model_groups(only_in_states=False):
            if not hasattr(g, '_id'):
                raise ValueError("%s is referenced only by an Ensemble or "
                                 "OrderedProcess. ModelGroups should be "
                                 "stored in State objects." % g)

    def dump_atom_type(self, seen_types, system, writer):
        """Output the atom_type table with a list of elements used
           in atom_site. This table is needed by atom_site. Note that we
           output it *after* atom_site (otherwise we would need to iterate
           through all atoms in the system twice)."""
        elements = [x for x in sorted(seen_types.keys()) if x is not None]
        with writer.loop("_atom_type", ["symbol"]) as lp:
            for element in elements:
                lp.write(symbol=element)

    def dump_atoms(self, system, writer, add_ihm=True):
        seen_types = {}
        ordinal = itertools.count(1)
        it = ["group_PDB", "id", "type_symbol", "label_atom_id",
              "label_alt_id", "label_comp_id", "label_seq_id", "auth_seq_id",
              "pdbx_PDB_ins_code", "label_asym_id", "Cartn_x", "Cartn_y",
              "Cartn_z", "occupancy", "label_entity_id", "auth_asym_id",
              "B_iso_or_equiv", "pdbx_PDB_model_num"]
        if add_ihm:
            it.append("ihm_model_id")
        with writer.loop("_atom_site", it) as lp:
            for group, model in system._all_models():
                rngcheck = _RangeChecker(model)
                for atom in model.get_atoms():
                    rngcheck(atom)
                    seq_id = 1 if atom.seq_id is None else atom.seq_id
                    comp = atom.asym_unit.entity.sequence[seq_id - 1]
                    seen_types[atom.type_symbol] = None
                    auth_seq_id, ins = \
                        atom.asym_unit._get_auth_seq_id_ins_code(seq_id)
                    lp.write(id=next(ordinal),
                             type_symbol=atom.type_symbol,
                             group_PDB='HETATM' if atom.het else 'ATOM',
                             label_atom_id=atom.atom_id,
                             label_comp_id=comp.id,
                             label_asym_id=atom.asym_unit._id,
                             label_entity_id=atom.asym_unit.entity._id,
                             label_seq_id=atom.seq_id,
                             auth_seq_id=auth_seq_id,
                             pdbx_PDB_ins_code=ins or ihm.unknown,
                             auth_asym_id=atom.asym_unit.strand_id,
                             Cartn_x=atom.x, Cartn_y=atom.y, Cartn_z=atom.z,
                             B_iso_or_equiv=atom.biso,
                             occupancy=atom.occupancy,
                             pdbx_PDB_model_num=model._id,
                             ihm_model_id=model._id)
        return seen_types


class _ModelDumper(_ModelDumperBase):
    def dump(self, system, writer):
        self.dump_model_list(system, writer)
        self.dump_model_groups(system, writer)
        seen_types = self.dump_atoms(system, writer)
        self.dump_spheres(system, writer)
        self.dump_atom_type(seen_types, system, writer)

    def dump_model_groups(self, system, writer):
        self.dump_model_group_summary(system, writer)
        self.dump_model_group_link(system, writer)

    def dump_model_list(self, system, writer):
        with writer.loop("_ihm_model_list",
                         ["model_id", "model_name", "assembly_id",
                          "protocol_id", "representation_id"]) as lp:
            for group, model in system._all_models():
                lp.write(model_id=model._id,
                         model_name=model.name,
                         assembly_id=model.assembly._id,
                         protocol_id=model.protocol._id
                         if model.protocol else None,
                         representation_id=model.representation._id)

    def dump_model_group_summary(self, system, writer):
        with writer.loop("_ihm_model_group", ["id", "name", "details"]) as lp:
            for group in system._all_model_groups():
                lp.write(id=group._id, name=group.name)

    def dump_model_group_link(self, system, writer):
        with writer.loop("_ihm_model_group_link",
                         ["group_id", "model_id"]) as lp:
            for group in system._all_model_groups():
                for model_id in sorted(set(model._id for model in group)):
                    lp.write(model_id=model_id, group_id=group._id)

    def dump_spheres(self, system, writer):
        ordinal = itertools.count(1)
        with writer.loop("_ihm_sphere_obj_site",
                         ["id", "entity_id", "seq_id_begin",
                          "seq_id_end", "asym_id", "Cartn_x",
                          "Cartn_y", "Cartn_z", "object_radius", "rmsf",
                          "model_id"]) as lp:
            for group, model in system._all_models():
                rngcheck = _RangeChecker(model)
                for sphere in model.get_spheres():
                    rngcheck(sphere)
                    lp.write(id=next(ordinal),
                             entity_id=sphere.asym_unit.entity._id,
                             seq_id_begin=sphere.seq_id_range[0],
                             seq_id_end=sphere.seq_id_range[1],
                             asym_id=sphere.asym_unit._id,
                             Cartn_x=sphere.x, Cartn_y=sphere.y,
                             Cartn_z=sphere.z, object_radius=sphere.radius,
                             rmsf=sphere.rmsf, model_id=model._id)


class _EnsembleDumper(Dumper):
    def finalize(self, system):
        # Assign IDs
        for ne, e in enumerate(system.ensembles):
            e._id = ne + 1

    def dump(self, system, writer):
        self.dump_info(system, writer)
        self.dump_subsamples(system, writer)

    def dump_info(self, system, writer):
        with writer.loop("_ihm_ensemble_info",
                         ["ensemble_id", "ensemble_name", "post_process_id",
                          "model_group_id", "ensemble_clustering_method",
                          "ensemble_clustering_feature",
                          "num_ensemble_models",
                          "num_ensemble_models_deposited",
                          "ensemble_precision_value",
                          "ensemble_file_id", "details",
                          "sub_sample_flag", "sub_sampling_type"]) as lp:
            for e in system.ensembles:
                if e.subsamples:
                    sstype = e.subsamples[0].sub_sampling_type
                else:
                    sstype = None
                lp.write(ensemble_id=e._id, ensemble_name=e.name,
                         post_process_id=e.post_process._id if e.post_process
                         else None,
                         model_group_id=e.model_group._id,
                         ensemble_clustering_method=e.clustering_method,
                         ensemble_clustering_feature=e.clustering_feature,
                         num_ensemble_models=e.num_models,
                         num_ensemble_models_deposited=e.num_models_deposited,
                         ensemble_precision_value=e.precision,
                         ensemble_file_id=e.file._id if e.file else None,
                         details=e.details,
                         sub_sample_flag=len(e.subsamples) > 0,
                         sub_sampling_type=sstype)

    def dump_subsamples(self, system, writer):
        ordinal = itertools.count(1)
        with writer.loop("_ihm_ensemble_sub_sample",
                         ["id", "name", "ensemble_id", "num_models",
                          "num_models_deposited", "model_group_id",
                          "file_id"]) as lp:
            for e in system.ensembles:
                for s in e.subsamples:
                    lp.write(id=next(ordinal), name=s.name, ensemble_id=e._id,
                             num_models=s.num_models,
                             num_models_deposited=s.num_models_deposited,
                             model_group_id=s.model_group._id
                             if s.model_group else None,
                             file_id=s.file._id if s.file else None)
                    if type(s) != type(e.subsamples[0]):    # noqa: E721
                        raise TypeError(
                            "Subsamples are not all of the same type "
                            "(%s vs %s) for ensemble %s"
                            % (s, e.subsamples[0], e))


class _DensityDumper(Dumper):
    def finalize(self, system):
        # Assign globally unique IDs
        did = itertools.count(1)
        for e in system.ensembles:
            for d in e.densities:
                d._id = next(did)

    def dump(self, system, writer):
        with writer.loop("_ihm_localization_density_files",
                         ["id", "file_id", "ensemble_id", "entity_id",
                          "asym_id", "entity_poly_segment_id"]) as lp:
            for ensemble in system.ensembles:
                for density in ensemble.densities:
                    lp.write(
                        id=density._id, file_id=density.file._id,
                        ensemble_id=ensemble._id,
                        entity_id=density.asym_unit.entity._id,
                        asym_id=density.asym_unit._id,
                        entity_poly_segment_id=density.asym_unit._range_id)


class _MultiStateDumper(Dumper):
    def finalize(self, system):
        state_id = itertools.count(1)
        # Assign IDs
        for ng, g in enumerate(system.state_groups):
            g._id = ng + 1
            for state in g:
                state._id = next(state_id)

    def dump(self, system, writer):
        # Nothing to do for single state modeling
        if len(system.state_groups) == 1 and len(system.state_groups[0]) <= 1:
            return
        self.dump_summary(system, writer)
        self.dump_model_groups(system, writer)

    def dump_summary(self, system, writer):
        with writer.loop("_ihm_multi_state_modeling",
                         ["state_id", "state_group_id",
                          "population_fraction", "state_type", "state_name",
                          "experiment_type", "details"]) as lp:
            for state_group in system.state_groups:
                for state in state_group:
                    lp.write(state_id=state._id,
                             state_group_id=state_group._id,
                             population_fraction=state.population_fraction,
                             state_type=state.type,
                             state_name=state.name,
                             experiment_type=state.experiment_type,
                             details=state.details)

    def dump_model_groups(self, system, writer):
        with writer.loop("_ihm_multi_state_model_group_link",
                         ["state_id", "model_group_id"]) as lp:
            for state_group in system.state_groups:
                for state in state_group:
                    for model_group in state:
                        lp.write(state_id=state._id,
                                 model_group_id=model_group._id)


class _OrderedDumper(Dumper):
    def finalize(self, system):
        for nproc, proc in enumerate(system.ordered_processes):
            proc._id = nproc + 1
            edge_id = itertools.count(1)
            for nstep, step in enumerate(proc.steps):
                step._id = nstep + 1
                for edge in step:
                    edge._id = next(edge_id)

    def dump(self, system, writer):
        with writer.loop("_ihm_ordered_ensemble",
                         ["process_id", "process_description", "ordered_by",
                          "step_id", "step_description",
                          "edge_id", "edge_description",
                          "model_group_id_begin", "model_group_id_end"]) as lp:
            for proc in system.ordered_processes:
                for step in proc.steps:
                    for edge in step:
                        lp.write(process_id=proc._id,
                                 process_description=proc.description,
                                 ordered_by=proc.ordered_by, step_id=step._id,
                                 step_description=step.description,
                                 edge_id=edge._id,
                                 edge_description=edge.description,
                                 model_group_id_begin=edge.group_begin._id,
                                 model_group_id_end=edge.group_end._id)


class _GeometricObjectDumper(Dumper):
    def finalize(self, system):
        seen_objects = {}
        seen_centers = {}
        seen_transformations = {}
        self._centers_by_id = []
        self._transformations_by_id = []
        self._objects_by_id = []

        for o in system._all_geometric_objects():
            util._remove_id(o)
            if hasattr(o, 'center'):
                util._remove_id(o.center)
            if hasattr(o, 'transformation') and o.transformation:
                util._remove_id(o.transformation)

        for o in system._all_geometric_objects():
            util._assign_id(o, seen_objects, self._objects_by_id)
            if hasattr(o, 'center'):
                util._assign_id(o.center, seen_centers, self._centers_by_id)
            if hasattr(o, 'transformation') and o.transformation:
                util._assign_id(o.transformation, seen_transformations,
                                self._transformations_by_id)

    def dump(self, system, writer):
        self.dump_centers(writer)
        self.dump_transformations(writer)
        self.dump_generic(writer)
        self.dump_sphere(writer)
        self.dump_torus(writer)
        self.dump_half_torus(writer)
        self.dump_axis(writer)
        self.dump_plane(writer)

    def dump_centers(self, writer):
        with writer.loop("_ihm_geometric_object_center",
                         ["id", "xcoord", "ycoord", "zcoord"]) as lp:
            for c in self._centers_by_id:
                lp.write(id=c._id, xcoord=c.x, ycoord=c.y, zcoord=c.z)

    def dump_transformations(self, writer):
        with writer.loop(
                "_ihm_geometric_object_transformation",
                ["id",
                 "rot_matrix[1][1]", "rot_matrix[2][1]", "rot_matrix[3][1]",
                 "rot_matrix[1][2]", "rot_matrix[2][2]", "rot_matrix[3][2]",
                 "rot_matrix[1][3]", "rot_matrix[2][3]", "rot_matrix[3][3]",
                 "tr_vector[1]", "tr_vector[2]", "tr_vector[3]"]) as lp:
            for t in self._transformations_by_id:
                lp.write(id=t._id, **_get_transform(t.rot_matrix, t.tr_vector))

    def dump_generic(self, writer):
        with writer.loop("_ihm_geometric_object_list",
                         ["object_id", "object_type", "object_name",
                          "object_description"]) as lp:
            for o in self._objects_by_id:
                lp.write(object_id=o._id, object_type=o.type,
                         object_name=o.name, object_description=o.description)

    def dump_sphere(self, writer):
        with writer.loop("_ihm_geometric_object_sphere",
                         ["object_id", "center_id", "transformation_id",
                          "radius_r"]) as lp:
            for o in self._objects_by_id:
                if not isinstance(o, geometry.Sphere):
                    continue
                lp.write(object_id=o._id, center_id=o.center._id,
                         transformation_id=o.transformation._id
                         if o.transformation else None,
                         radius_r=o.radius)

    def dump_torus(self, writer):
        with writer.loop("_ihm_geometric_object_torus",
                         ["object_id", "center_id", "transformation_id",
                          "major_radius_R", "minor_radius_r"]) as lp:
            for o in self._objects_by_id:
                if not isinstance(o, (geometry.Torus, geometry.HalfTorus)):
                    continue
                lp.write(object_id=o._id, center_id=o.center._id,
                         transformation_id=o.transformation._id
                         if o.transformation else None,
                         major_radius_R=o.major_radius,
                         minor_radius_r=o.minor_radius)

    def dump_half_torus(self, writer):
        section_map = {True: 'inner half', False: 'outer half'}
        with writer.loop("_ihm_geometric_object_half_torus",
                         ["object_id", "thickness_th", "section"]) as lp:
            for o in self._objects_by_id:
                if not isinstance(o, geometry.HalfTorus):
                    continue
                lp.write(object_id=o._id, thickness_th=o.thickness,
                         section=section_map.get(o.inner, 'other'))

    def dump_axis(self, writer):
        with writer.loop("_ihm_geometric_object_axis",
                         ["object_id", "axis_type",
                          "transformation_id"]) as lp:
            for o in self._objects_by_id:
                if not isinstance(o, geometry.Axis):
                    continue
                lp.write(object_id=o._id, axis_type=o.axis_type,
                         transformation_id=o.transformation._id
                         if o.transformation else None)

    def dump_plane(self, writer):
        with writer.loop("_ihm_geometric_object_plane",
                         ["object_id", "plane_type",
                          "transformation_id"]) as lp:
            for o in self._objects_by_id:
                if not isinstance(o, geometry.Plane):
                    continue
                lp.write(object_id=o._id, plane_type=o.plane_type,
                         transformation_id=o.transformation._id
                         if o.transformation else None)


class _FeatureDumper(Dumper):
    def finalize(self, system):
        seen_features = {}
        self._features_by_id = []
        for f in system._all_features():
            util._remove_id(f)
        for f in system._all_features():
            util._assign_id(f, seen_features, self._features_by_id)

    def dump(self, system, writer):
        self.dump_list(writer)
        self.dump_poly_residue(writer)
        self.dump_poly_atom(writer)
        self.dump_non_poly(writer)
        self.dump_pseudo_site(writer)

    def dump_list(self, writer):
        with writer.loop("_ihm_feature_list",
                         ["feature_id", "feature_type", "entity_type",
                          "details"]) as lp:
            for f in self._features_by_id:
                lp.write(feature_id=f._id, feature_type=f.type,
                         entity_type=f._get_entity_type(),
                         details=f.details)

    def dump_poly_residue(self, writer):
        def _get_entity(x):
            return x if isinstance(x, ihm.Entity) else x.entity

        def _get_asym_id(x):
            return (x._id if isinstance(x, (ihm.AsymUnit, ihm.AsymUnitRange))
                    else None)
        ordinal = itertools.count(1)
        with writer.loop("_ihm_poly_residue_feature",
                         ["ordinal_id", "feature_id", "entity_id", "asym_id",
                          "seq_id_begin", "comp_id_begin", "seq_id_end",
                          "comp_id_end"]) as lp:
            for f in self._features_by_id:
                if not isinstance(f, restraint.ResidueFeature):
                    continue
                for r in f.ranges:
                    entity = _get_entity(r)
                    seq = entity.sequence
                    lp.write(ordinal_id=next(ordinal), feature_id=f._id,
                             entity_id=entity._id, asym_id=_get_asym_id(r),
                             seq_id_begin=r.seq_id_range[0],
                             comp_id_begin=seq[r.seq_id_range[0] - 1].id,
                             seq_id_end=r.seq_id_range[1],
                             comp_id_end=seq[r.seq_id_range[1] - 1].id)

    def dump_poly_atom(self, writer):
        ordinal = itertools.count(1)
        with writer.loop("_ihm_poly_atom_feature",
                         ["ordinal_id", "feature_id", "entity_id", "asym_id",
                          "seq_id", "comp_id", "atom_id"]) as lp:
            for f in self._features_by_id:
                if not isinstance(f, restraint.AtomFeature):
                    continue
                for a in f.atoms:
                    r = a.residue
                    entity = r.entity if r.entity else r.asym.entity
                    if entity.is_polymeric():
                        seq = entity.sequence
                        lp.write(ordinal_id=next(ordinal), feature_id=f._id,
                                 entity_id=entity._id,
                                 asym_id=r.asym._id if r.asym else None,
                                 seq_id=r.seq_id, comp_id=seq[r.seq_id - 1].id,
                                 atom_id=a.id)

    def dump_non_poly(self, writer):
        ordinal = itertools.count(1)
        with writer.loop("_ihm_non_poly_feature",
                         ["ordinal_id", "feature_id", "entity_id", "asym_id",
                          "comp_id", "atom_id"]) as lp:
            for f in self._features_by_id:
                if isinstance(f, restraint.AtomFeature):
                    for a in f.atoms:
                        r = a.residue
                        entity = r.entity if r.entity else r.asym.entity
                        if not entity.is_polymeric():
                            seq = entity.sequence
                            lp.write(ordinal_id=next(ordinal),
                                     feature_id=f._id,
                                     entity_id=entity._id,
                                     asym_id=r.asym._id if r.asym else None,
                                     comp_id=seq[r.seq_id - 1].id,
                                     atom_id=a.id)
                elif isinstance(f, restraint.NonPolyFeature):
                    _ = f._get_entity_type()  # trigger check for poly/nonpoly
                    for a in f.objs:
                        entity = a if isinstance(a, ihm.Entity) else a.entity
                        asym_id = (a._id if isinstance(a, ihm.AsymUnit)
                                   else None)
                        seq = entity.sequence
                        lp.write(ordinal_id=next(ordinal), feature_id=f._id,
                                 entity_id=entity._id,
                                 asym_id=asym_id, comp_id=seq[0].id,
                                 atom_id=None)

    def dump_pseudo_site(self, writer):
        with writer.loop("_ihm_pseudo_site_feature",
                         ["feature_id", "pseudo_site_id"]) as lp:
            for f in self._features_by_id:
                if not isinstance(f, restraint.PseudoSiteFeature):
                    continue
                lp.write(feature_id=f._id, pseudo_site_id=f.site._id)


class _PseudoSiteDumper(Dumper):
    def finalize(self, system):
        seen_sites = {}
        self._sites_by_id = []
        for f in system._all_pseudo_sites():
            util._remove_id(f)
        for f in system._all_pseudo_sites():
            util._assign_id(f, seen_sites, self._sites_by_id)

    def dump(self, system, writer):
        with writer.loop("_ihm_pseudo_site",
                         ["id", "Cartn_x", "Cartn_y",
                          "Cartn_z", "radius", "description"]) as lp:
            for s in self._sites_by_id:
                lp.write(id=s._id, Cartn_x=s.x, Cartn_y=s.y,
                         Cartn_z=s.z, radius=s.radius,
                         description=s.description)


class _CrossLinkDumper(Dumper):
    def _all_restraints(self, system):
        return [r for r in system._all_restraints()
                if isinstance(r, restraint.CrossLinkRestraint)]

    def finalize(self, system):
        self.finalize_experimental(system)
        self.finalize_modeling(system)

    def finalize_experimental(self, system):
        seen_cross_links = {}
        seen_group_ids = {}
        xl_id = itertools.count(1)
        self._ex_xls_by_id = []
        for r in self._all_restraints(system):
            for g in r.experimental_cross_links:
                for xl in g:
                    # Assign identical cross-links the same ID and group ID
                    sig = (xl.residue1.entity, xl.residue1.seq_id,
                           xl.residue2.entity, xl.residue2.seq_id,
                           r.linker)
                    if sig in seen_cross_links:
                        xl._id, xl._group_id = seen_cross_links[sig]
                    else:
                        if id(g) not in seen_group_ids:
                            seen_group_ids[id(g)] = len(seen_group_ids) + 1
                        xl._group_id = seen_group_ids[id(g)]
                        xl._id = next(xl_id)
                        self._ex_xls_by_id.append((r, xl))
                        seen_cross_links[sig] = xl._id, xl._group_id

    def finalize_modeling(self, system):
        seen_cross_links = {}
        xl_id = itertools.count(1)
        self._xls_by_id = []
        for r in self._all_restraints(system):
            for xl in r.cross_links:
                # Assign identical cross-links the same ID
                ex_xl = xl.experimental_cross_link
                sig = (xl.asym1._id, ex_xl.residue1.seq_id, xl.atom1,
                       xl.asym2._id, ex_xl.residue2.seq_id, xl.atom2,
                       r.linker)
                if sig in seen_cross_links:
                    xl._id = seen_cross_links[sig]
                else:
                    xl._id = next(xl_id)
                    self._xls_by_id.append((r, xl))
                    seen_cross_links[sig] = xl._id

    def dump(self, system, writer):
        self.dump_list(system, writer)
        pseudo_xls = self.dump_restraint(system, writer)
        self.dump_pseudo_sites(system, writer, pseudo_xls)
        self.dump_results(system, writer)

    def dump_list(self, system, writer):
        with writer.loop("_ihm_cross_link_list",
                         ["id", "group_id", "entity_description_1",
                          "entity_id_1", "seq_id_1", "comp_id_1",
                          "entity_description_2",
                          "entity_id_2", "seq_id_2", "comp_id_2",
                          "linker_chem_comp_descriptor_id", "linker_type",
                          "dataset_list_id", "details"]) as lp:
            for r, xl in self._ex_xls_by_id:
                entity1 = xl.residue1.entity
                entity2 = xl.residue2.entity
                seq1 = entity1.sequence
                seq2 = entity2.sequence
                lp.write(id=xl._id, group_id=xl._group_id,
                         entity_description_1=entity1.description,
                         entity_id_1=entity1._id,
                         seq_id_1=xl.residue1.seq_id,
                         comp_id_1=seq1[xl.residue1.seq_id - 1].id,
                         entity_description_2=entity2.description,
                         entity_id_2=entity2._id,
                         seq_id_2=xl.residue2.seq_id,
                         comp_id_2=seq2[xl.residue2.seq_id - 1].id,
                         linker_chem_comp_descriptor_id=r.linker._id,
                         linker_type=r.linker.auth_name,
                         dataset_list_id=r.dataset._id,
                         details=xl.details)

    def dump_restraint(self, system, writer):
        pseudo_xls = []
        with writer.loop("_ihm_cross_link_restraint",
                         ["id", "group_id", "entity_id_1", "asym_id_1",
                          "seq_id_1", "comp_id_1",
                          "entity_id_2", "asym_id_2", "seq_id_2", "comp_id_2",
                          "atom_id_1", "atom_id_2",
                          "restraint_type", "conditional_crosslink_flag",
                          "model_granularity", "distance_threshold",
                          "psi", "sigma_1", "sigma_2",
                          "pseudo_site_flag"]) as lp:
            condmap = {True: 'ALL', False: 'ANY', None: None}
            for r, xl in self._xls_by_id:
                ex_xl = xl.experimental_cross_link
                entity1 = ex_xl.residue1.entity
                entity2 = ex_xl.residue2.entity
                seq1 = entity1.sequence
                seq2 = entity2.sequence
                pseudo = False
                for np, ps in enumerate((xl.pseudo1, xl.pseudo2)):
                    if ps:
                        pseudo = True
                        for p in ps:
                            pseudo_xls.append((p, np, xl))
                lp.write(id=xl._id, group_id=ex_xl._id,
                         entity_id_1=entity1._id, asym_id_1=xl.asym1._id,
                         seq_id_1=ex_xl.residue1.seq_id,
                         comp_id_1=seq1[ex_xl.residue1.seq_id - 1].id,
                         entity_id_2=entity2._id, asym_id_2=xl.asym2._id,
                         seq_id_2=ex_xl.residue2.seq_id,
                         comp_id_2=seq2[ex_xl.residue2.seq_id - 1].id,
                         atom_id_1=xl.atom1, atom_id_2=xl.atom2,
                         restraint_type=xl.distance.restraint_type,
                         conditional_crosslink_flag=condmap[xl.restrain_all],
                         model_granularity=xl.granularity,
                         distance_threshold=xl.distance.distance,
                         psi=xl.psi, sigma_1=xl.sigma1, sigma_2=xl.sigma2,
                         pseudo_site_flag=pseudo)
        return pseudo_xls

    def dump_pseudo_sites(self, system, writer, pseudo_xls):
        with writer.loop("_ihm_cross_link_pseudo_site",
                         ["id", "restraint_id", "cross_link_partner",
                          "pseudo_site_id", "model_id"]) as lp:
            ordinal = itertools.count(1)
            for p, partner, rsr in pseudo_xls:
                lp.write(id=next(ordinal), restraint_id=rsr._id,
                         cross_link_partner=partner + 1,
                         pseudo_site_id=p.site._id,
                         model_id=p.model._id if p.model else None)

    def dump_results(self, system, writer):
        with writer.loop("_ihm_cross_link_result_parameters",
                         ["id", "restraint_id", "model_id",
                          "psi", "sigma_1", "sigma_2"]) as lp:
            ordinal = itertools.count(1)
            for r in self._all_restraints(system):
                for xl in r.cross_links:
                    # all fits ordered by model ID
                    for model, fit in sorted(xl.fits.items(),
                                             key=lambda i: i[0]._id):
                        lp.write(id=next(ordinal), restraint_id=xl._id,
                                 model_id=model._id, psi=fit.psi,
                                 sigma_1=fit.sigma1, sigma_2=fit.sigma2)


class _GeometricRestraintDumper(Dumper):
    def _all_restraints(self, system):
        return [r for r in system._all_restraints()
                if isinstance(r, restraint.GeometricRestraint)]

    def finalize(self, system):
        for nr, r in enumerate(self._all_restraints(system)):
            r._id = nr + 1

    def dump(self, system, writer):
        condmap = {True: 'ALL', False: 'ANY', None: None}
        with writer.loop("_ihm_geometric_object_distance_restraint",
                         ["id", "object_id", "feature_id",
                          "object_characteristic", "restraint_type",
                          "harmonic_force_constant",
                          "distance_lower_limit", "distance_upper_limit",
                          "group_conditionality", "dataset_list_id"]) as lp:
            for r in self._all_restraints(system):
                lp.write(id=r._id, object_id=r.geometric_object._id,
                         feature_id=r.feature._id,
                         object_characteristic=r.object_characteristic,
                         restraint_type=r.distance.restraint_type,
                         distance_lower_limit=r.distance.distance_lower_limit,
                         distance_upper_limit=r.distance.distance_upper_limit,
                         harmonic_force_constant=r.harmonic_force_constant,
                         group_conditionality=condmap[r.restrain_all],
                         dataset_list_id=r.dataset._id if r.dataset else None)


def _finalize_restraints_and_groups(system, restraint_class):
    """Assign IDs to all restraints of the given class, and also assign IDs
       to groups of these restraints."""
    def _all_restraints():
        return [r for r in system._all_restraints()
                if isinstance(r, restraint_class)]

    def _all_restraint_groups():
        return [rg for rg in system.restraint_groups
                if all(isinstance(r, restraint_class)
                       for r in rg) and len(rg) > 0]

    restraints_by_id = []
    seen_restraints = {}
    for r in _all_restraints():
        util._remove_id(r)
    for r in _all_restraints():
        util._assign_id(r, seen_restraints, restraints_by_id)

    group_for_id = {}
    for nrg, rg in enumerate(_all_restraint_groups()):
        rg._id = nrg + 1
        for r in rg:
            if r._id in group_for_id:
                raise ValueError("%s cannot be in more than one group" % r)
            group_for_id[r._id] = rg._id
    return restraints_by_id, group_for_id


class _DerivedDistanceRestraintDumper(Dumper):
    def finalize(self, system):
        (self._restraints_by_id,
         self._group_for_id) = _finalize_restraints_and_groups(
            system, restraint.DerivedDistanceRestraint)

    def dump(self, system, writer):
        condmap = {True: 'ALL', False: 'ANY', None: None}
        with writer.loop("_ihm_derived_distance_restraint",
                         ["id", "group_id", "feature_id_1", "feature_id_2",
                          "restraint_type", "distance_lower_limit",
                          "distance_upper_limit", "probability", "mic_value",
                          "group_conditionality", "dataset_list_id"]) as lp:
            for r in self._restraints_by_id:
                lp.write(id=r._id, feature_id_1=r.feature1._id,
                         group_id=self._group_for_id.get(r._id, None),
                         feature_id_2=r.feature2._id,
                         restraint_type=r.distance.restraint_type,
                         distance_lower_limit=r.distance.distance_lower_limit,
                         distance_upper_limit=r.distance.distance_upper_limit,
                         probability=r.probability, mic_value=r.mic_value,
                         group_conditionality=condmap[r.restrain_all],
                         dataset_list_id=r.dataset._id if r.dataset else None)


class _PredictedContactRestraintDumper(Dumper):
    def finalize(self, system):
        (self._restraints_by_id,
         self._group_for_id) = _finalize_restraints_and_groups(
            system, restraint.PredictedContactRestraint)

    def dump(self, system, writer):
        with writer.loop("_ihm_predicted_contact_restraint",
                         ["id", "group_id", "entity_id_1", "asym_id_1",
                          "comp_id_1", "seq_id_1", "rep_atom_1", "entity_id_2",
                          "asym_id_2", "comp_id_2", "seq_id_2", "rep_atom_2",
                          "restraint_type", "distance_lower_limit",
                          "distance_upper_limit", "probability",
                          "model_granularity", "dataset_list_id",
                          "software_id"]) as lp:
            for r in self._restraints_by_id:
                e = r.resatom1.asym.entity
                comp1 = e.sequence[r.resatom1.seq_id - 1].id
                e = r.resatom2.asym.entity
                comp2 = e.sequence[r.resatom2.seq_id - 1].id
                atom1 = atom2 = None
                if isinstance(r.resatom1, ihm.Atom):
                    atom1 = r.resatom1.id
                if isinstance(r.resatom2, ihm.Atom):
                    atom2 = r.resatom2.id
                lp.write(id=r._id,
                         group_id=self._group_for_id.get(r._id, None),
                         entity_id_1=r.resatom1.asym.entity._id,
                         asym_id_1=r.resatom1.asym._id,
                         comp_id_1=comp1, seq_id_1=r.resatom1.seq_id,
                         rep_atom_1=atom1,
                         entity_id_2=r.resatom2.asym.entity._id,
                         asym_id_2=r.resatom2.asym._id,
                         comp_id_2=comp2, seq_id_2=r.resatom2.seq_id,
                         rep_atom_2=atom2,
                         restraint_type=r.distance.restraint_type,
                         distance_lower_limit=r.distance.distance_lower_limit,
                         distance_upper_limit=r.distance.distance_upper_limit,
                         probability=r.probability,
                         model_granularity="by-residue" if r.by_residue
                         else 'by-feature',
                         dataset_list_id=r.dataset._id if r.dataset else None,
                         software_id=r.software._id if r.software else None)


class _EM3DDumper(Dumper):
    def _all_restraints(self, system):
        return [r for r in system._all_restraints()
                if isinstance(r, restraint.EM3DRestraint)]

    def finalize(self, system):
        for nr, r in enumerate(self._all_restraints(system)):
            r._id = nr + 1

    def dump(self, system, writer):
        ordinal = itertools.count(1)
        with writer.loop("_ihm_3dem_restraint",
                         ["id", "dataset_list_id", "fitting_method",
                          "fitting_method_citation_id",
                          "struct_assembly_id",
                          "number_of_gaussians", "model_id",
                          "cross_correlation_coefficient"]) as lp:
            for r in self._all_restraints(system):
                if r.fitting_method_citation:
                    citation_id = r.fitting_method_citation._id
                else:
                    citation_id = None
                # all fits ordered by model ID
                for model, fit in sorted(r.fits.items(),
                                         key=lambda i: i[0]._id):
                    ccc = fit.cross_correlation_coefficient
                    lp.write(id=next(ordinal),
                             dataset_list_id=r.dataset._id,
                             fitting_method=r.fitting_method,
                             fitting_method_citation_id=citation_id,
                             struct_assembly_id=r.assembly._id,
                             number_of_gaussians=r.number_of_gaussians,
                             model_id=model._id,
                             cross_correlation_coefficient=ccc)


class _EM2DDumper(Dumper):
    def _all_restraints(self, system):
        return [r for r in system._all_restraints()
                if isinstance(r, restraint.EM2DRestraint)]

    def finalize(self, system):
        for nr, r in enumerate(self._all_restraints(system)):
            r._id = nr + 1

    def dump(self, system, writer):
        self.dump_restraint(system, writer)
        self.dump_fitting(system, writer)

    def dump_restraint(self, system, writer):
        with writer.loop("_ihm_2dem_class_average_restraint",
                         ["id", "dataset_list_id", "number_raw_micrographs",
                          "pixel_size_width", "pixel_size_height",
                          "image_resolution", "image_segment_flag",
                          "number_of_projections", "struct_assembly_id",
                          "details"]) as lp:
            for r in self._all_restraints(system):
                lp.write(id=r._id, dataset_list_id=r.dataset._id,
                         number_raw_micrographs=r.number_raw_micrographs,
                         pixel_size_width=r.pixel_size_width,
                         pixel_size_height=r.pixel_size_height,
                         image_resolution=r.image_resolution,
                         number_of_projections=r.number_of_projections,
                         struct_assembly_id=r.assembly._id,
                         image_segment_flag=r.segment,
                         details=r.details)

    def dump_fitting(self, system, writer):
        ordinal = itertools.count(1)
        with writer.loop(
                "_ihm_2dem_class_average_fitting",
                ["id", "restraint_id", "model_id",
                 "cross_correlation_coefficient", "rot_matrix[1][1]",
                 "rot_matrix[2][1]", "rot_matrix[3][1]", "rot_matrix[1][2]",
                 "rot_matrix[2][2]", "rot_matrix[3][2]", "rot_matrix[1][3]",
                 "rot_matrix[2][3]", "rot_matrix[3][3]", "tr_vector[1]",
                 "tr_vector[2]", "tr_vector[3]"]) as lp:
            for r in self._all_restraints(system):
                # all fits ordered by model ID
                for model, fit in sorted(r.fits.items(),
                                         key=lambda i: i[0]._id):
                    ccc = fit.cross_correlation_coefficient
                    if fit.tr_vector is None:
                        t = [None] * 3
                    else:
                        t = fit.tr_vector
                    if fit.rot_matrix is None:
                        rm = [[None] * 3] * 3
                    else:
                        # mmCIF writer usually outputs floats to 3 decimal
                        # places, but we need more precision for rotation
                        # matrices
                        rm = [["%.6f" % e for e in fit.rot_matrix[i]]
                              for i in range(3)]
                    lp.write(id=next(ordinal), restraint_id=r._id,
                             model_id=model._id,
                             cross_correlation_coefficient=ccc,
                             rot_matrix11=rm[0][0], rot_matrix21=rm[1][0],
                             rot_matrix31=rm[2][0], rot_matrix12=rm[0][1],
                             rot_matrix22=rm[1][1], rot_matrix32=rm[2][1],
                             rot_matrix13=rm[0][2], rot_matrix23=rm[1][2],
                             rot_matrix33=rm[2][2], tr_vector1=t[0],
                             tr_vector2=t[1], tr_vector3=t[2])


class _SASDumper(Dumper):
    def _all_restraints(self, system):
        return [r for r in system._all_restraints()
                if isinstance(r, restraint.SASRestraint)]

    def finalize(self, system):
        for nr, r in enumerate(self._all_restraints(system)):
            r._id = nr + 1

    def dump(self, system, writer):
        ordinal = itertools.count(1)
        with writer.loop("_ihm_sas_restraint",
                         ["id", "dataset_list_id", "model_id",
                          "struct_assembly_id", "profile_segment_flag",
                          "fitting_atom_type", "fitting_method",
                          "fitting_state", "radius_of_gyration",
                          "chi_value", "details"]) as lp:
            for r in self._all_restraints(system):
                # all fits ordered by model ID
                for model, fit in sorted(r.fits.items(),
                                         key=lambda i: i[0]._id):
                    lp.write(id=next(ordinal),
                             dataset_list_id=r.dataset._id,
                             fitting_method=r.fitting_method,
                             fitting_atom_type=r.fitting_atom_type,
                             fitting_state='Multiple' if r.multi_state
                             else 'Single',
                             profile_segment_flag=r.segment,
                             radius_of_gyration=r.radius_of_gyration,
                             struct_assembly_id=r.assembly._id,
                             model_id=model._id,
                             chi_value=fit.chi_value,
                             details=r.details)


def _assign_all_ids(all_objs_func):
    """Given a function that returns a list of all objects, assign IDs and
       return a list of objects sorted by ID"""
    objs_by_id = []
    obj_id = itertools.count(1)
    for f in all_objs_func():
        util._remove_id(f)
    for f in all_objs_func():
        if not hasattr(f, '_id'):
            f._id = next(obj_id)
            objs_by_id.append(f)
    return objs_by_id


class _FLRExperimentDumper(Dumper):
    def finalize(self, system):
        def all_experiments():
            return itertools.chain.from_iterable(f._all_experiments()
                                                 for f in system.flr_data)
        self._experiments_by_id = _assign_all_ids(all_experiments)

    def dump(self, system, writer):
        with writer.loop(
                '_flr_experiment',
                ['ordinal_id', 'id', 'instrument_id', 'inst_setting_id',
                 'exp_condition_id', 'sample_id', 'details']) as lp:
            ordinal = 1
            for x in self._experiments_by_id:
                for i in range(len(x.sample_list)):
                    lp.write(ordinal_id=ordinal, id=x._id,
                             instrument_id=x.instrument_list[i]._id,
                             inst_setting_id=x.inst_setting_list[i]._id,
                             exp_condition_id=x.exp_condition_list[i]._id,
                             sample_id=x.sample_list[i]._id,
                             details=x.details_list[i])
                    ordinal += 1


class _FLRInstSettingDumper(Dumper):
    def finalize(self, system):
        def all_inst_settings():
            return itertools.chain.from_iterable(f._all_inst_settings()
                                                 for f in system.flr_data)
        self._inst_settings_by_id = _assign_all_ids(all_inst_settings)

    def dump(self, system, writer):
        with writer.loop('_flr_inst_setting', ['id', 'details']) as lp:
            for x in self._inst_settings_by_id:
                lp.write(id=x._id, details=x.details)


class _FLR_ExpConditionDumper(Dumper):
    def finalize(self, system):
        def all_exp_conditions():
            return itertools.chain.from_iterable(f._all_exp_conditions()
                                                 for f in system.flr_data)
        self._exp_conditions_by_id = _assign_all_ids(all_exp_conditions)

    def dump(self, system, writer):
        with writer.loop('_flr_exp_condition', ['id', 'details']) as lp:
            for x in self._exp_conditions_by_id:
                lp.write(id=x._id, details=x.details)


class _FLRInstrumentDumper(Dumper):
    def finalize(self, system):
        def all_instruments():
            return itertools.chain.from_iterable(f._all_instruments()
                                                 for f in system.flr_data)
        self._instruments_by_id = _assign_all_ids(all_instruments)

    def dump(self, system, writer):
        with writer.loop('_flr_instrument', ['id', 'details']) as lp:
            for x in self._instruments_by_id:
                lp.write(id=x._id, details=x.details)


class _FLREntityAssemblyDumper(Dumper):
    def finalize(self, system):
        def all_entity_assemblies():
            return itertools.chain.from_iterable(
                (s.entity_assembly for s in f._all_samples())
                for f in system.flr_data)
        self._entity_assemblies_by_id = _assign_all_ids(all_entity_assemblies)

    def dump(self, system, writer):
        with writer.loop(
                '_flr_entity_assembly',
                ['ordinal_id', 'assembly_id', 'entity_id', 'num_copies',
                 'entity_description']) as lp:
            ordinal = itertools.count(1)
            for x in self._entity_assemblies_by_id:
                for i in range(len(x.entity_list)):
                    lp.write(ordinal_id=next(ordinal),
                             assembly_id=x._id,
                             entity_id=x.entity_list[i]._id,
                             num_copies=x.num_copies_list[i],
                             entity_description=x.entity_list[i].description)


class _FLRSampleConditionDumper(Dumper):
    def finalize(self, system):
        def all_sample_conditions():
            return itertools.chain.from_iterable(
                (s.condition for s in f._all_samples())
                for f in system.flr_data)
        self._sample_conditions_by_id = _assign_all_ids(all_sample_conditions)

    def dump(self, system, writer):
        with writer.loop('_flr_sample_condition', ['id', 'details']) as lp:
            for x in self._sample_conditions_by_id:
                lp.write(id=x._id, details=x.details)


class _FLRSampleDumper(Dumper):
    def finalize(self, system):
        def all_samples():
            return itertools.chain.from_iterable(f._all_samples()
                                                 for f in system.flr_data)
        self._samples_by_id = _assign_all_ids(all_samples)

    def dump(self, system, writer):
        with writer.loop('_flr_sample',
                         ['id', 'entity_assembly_id', 'num_of_probes',
                          'sample_condition_id', 'sample_description',
                          'sample_details', 'solvent_phase']) as lp:
            for x in self._samples_by_id:
                lp.write(id=x._id, entity_assembly_id=x.entity_assembly._id,
                         num_of_probes=x.num_of_probes,
                         sample_condition_id=x.condition._id,
                         sample_description=x.description,
                         sample_details=x.details,
                         solvent_phase=x.solvent_phase)


class _FLRProbeDumper(Dumper):
    def finalize(self, system):
        def all_probes():
            return itertools.chain.from_iterable(f._all_probes()
                                                 for f in system.flr_data)
        self._probes_by_id = _assign_all_ids(all_probes)

    def dump(self, system, writer):
        self.dump_probe_list(system, writer)
        self.dump_probe_descriptor(system, writer)

    def dump_probe_list(self, system, writer):
        with writer.loop('_flr_probe_list',
                         ['probe_id', 'chromophore_name',
                          'reactive_probe_flag',
                          'reactive_probe_name', 'probe_origin',
                          'probe_link_type']) as lp:
            for x in self._probes_by_id:
                entry = x.probe_list_entry
                lp.write(probe_id=x._id,
                         chromophore_name=entry.chromophore_name,
                         reactive_probe_flag=entry.reactive_probe_flag,
                         reactive_probe_name=entry.reactive_probe_name,
                         probe_origin=entry.probe_origin,
                         probe_link_type=entry.probe_link_type)

    def dump_probe_descriptor(self, system, writer):
        with writer.loop('_flr_probe_descriptor',
                         ['probe_id', 'reactive_probe_chem_descriptor_id',
                          'chromophore_chem_descriptor_id',
                          'chromophore_center_atom']) as lp:
            for x in self._probes_by_id:
                reactive = x.probe_descriptor.reactive_probe_chem_descriptor
                chrom = x.probe_descriptor.chromophore_chem_descriptor
                reactive_id = None if reactive is None else reactive._id
                chrom_id = None if chrom is None else chrom._id
                center = x.probe_descriptor.chromophore_center_atom
                lp.write(probe_id=x._id,
                         reactive_probe_chem_descriptor_id=reactive_id,
                         chromophore_chem_descriptor_id=chrom_id,
                         chromophore_center_atom=center)


class _FLRSampleProbeDetailsDumper(Dumper):
    def finalize(self, system):
        def all_sample_probe_details():
            return itertools.chain.from_iterable(f._all_sample_probe_details()
                                                 for f in system.flr_data)
        self._sample_probe_details_by_id = _assign_all_ids(
            all_sample_probe_details)

    def dump(self, system, writer):
        with writer.loop('_flr_sample_probe_details',
                         ['sample_probe_id', 'sample_id', 'probe_id',
                          'fluorophore_type', 'description',
                          'poly_probe_position_id']) as lp:
            for x in self._sample_probe_details_by_id:
                lp.write(sample_probe_id=x._id,
                         sample_id=x.sample._id,
                         probe_id=x.probe._id,
                         fluorophore_type=x.fluorophore_type,
                         description=x.description,
                         poly_probe_position_id=x.poly_probe_position._id)


class _FLRPolyProbePositionDumper(Dumper):
    def finalize(self, system):
        def all_poly_probe_positions():
            return itertools.chain.from_iterable(f._all_poly_probe_positions()
                                                 for f in system.flr_data)
        self._positions_by_id = _assign_all_ids(all_poly_probe_positions)

    def dump(self, system, writer):
        self.dump_position(system, writer)
        self.dump_position_mutated(system, writer)
        self.dump_position_modified(system, writer)

    def dump_position(self, system, writer):
        with writer.loop('_flr_poly_probe_position',
                         ['id', 'entity_id', 'entity_description',
                          'asym_id',
                          'seq_id', 'comp_id', 'atom_id',
                          'mutation_flag', 'modification_flag',
                          'auth_name']) as lp:
            for x in self._positions_by_id:
                atom = None
                if isinstance(x.resatom, ihm.Atom):
                    atom = x.resatom.id
                if x.resatom.asym is None:
                    comp = x.resatom.entity.sequence[x.resatom.seq_id - 1].id
                    a_id = None
                    e_id = x.resatom.entity._id
                    e_desc = x.resatom.entity.description
                else:
                    e = x.resatom.asym.entity
                    comp = e.sequence[x.resatom.seq_id - 1].id
                    a_id = x.resatom.asym._id
                    e_id = x.resatom.asym.entity._id
                    e_desc = x.resatom.asym.entity.description
                lp.write(id=x._id, entity_id=e_id,
                         entity_description=e_desc,
                         asym_id=a_id,
                         seq_id=x.resatom.seq_id,
                         comp_id=comp, atom_id=atom,
                         mutation_flag=x.mutation_flag,
                         modification_flag=x.modification_flag,
                         auth_name=x.auth_name)

    def dump_position_mutated(self, system, writer):
        with writer.loop('_flr_poly_probe_position_mutated',
                         ['id', 'chem_comp_id', 'atom_id']) as lp:
            for x in self._positions_by_id:
                if x.mutation_flag is True:
                    atom = None
                    if isinstance(x.resatom, ihm.Atom):
                        atom = x.resatom.id
                    lp.write(id=x._id,
                             chem_comp_id=x.mutated_chem_comp_id.id,
                             atom_id=atom)

    def dump_position_modified(self, system, writer):
        with writer.loop('_flr_poly_probe_position_modified',
                         ['id', 'chem_descriptor_id', 'atom_id']) as lp:
            for x in self._positions_by_id:
                if x.modification_flag is True:
                    atom = None
                    if isinstance(x.resatom, ihm.Atom):
                        atom = x.resatom.id
                    lp.write(id=x._id,
                             chem_descriptor_id=x.modified_chem_descriptor._id,
                             atom_id=atom)


class _FLRConjugateDumper(Dumper):
    def finalize(self, system):
        def all_conjugates():
            return itertools.chain.from_iterable(f.poly_probe_conjugates
                                                 for f in system.flr_data)
        self._conjugates_by_id = _assign_all_ids(all_conjugates)

    def dump(self, system, writer):
        with writer.loop('_flr_poly_probe_conjugate',
                         ['id', 'sample_probe_id', 'chem_descriptor_id',
                          'ambiguous_stoichiometry_flag',
                          'probe_stoichiometry']) as lp:
            for x in self._conjugates_by_id:
                lp.write(
                    id=x._id,
                    sample_probe_id=x.sample_probe._id,
                    chem_descriptor_id=x.chem_descriptor._id,
                    ambiguous_stoichiometry_flag=x.ambiguous_stoichiometry,
                    probe_stoichiometry=x.probe_stoichiometry)


class _FLRForsterRadiusDumper(Dumper):
    def finalize(self, system):
        def all_forster_radii():
            return itertools.chain.from_iterable(f._all_forster_radii()
                                                 for f in system.flr_data)
        self._radii_by_id = _assign_all_ids(all_forster_radii)

    def dump(self, system, writer):
        with writer.loop('_flr_fret_forster_radius',
                         ['id', 'donor_probe_id', 'acceptor_probe_id',
                          'forster_radius', 'reduced_forster_radius']) as lp:
            for x in self._radii_by_id:
                lp.write(id=x._id, donor_probe_id=x.donor_probe._id,
                         acceptor_probe_id=x.acceptor_probe._id,
                         forster_radius=x.forster_radius,
                         reduced_forster_radius=x.reduced_forster_radius)


class _FLRCalibrationParametersDumper(Dumper):
    def finalize(self, system):
        def all_calibration_parameters():
            return itertools.chain.from_iterable(
                f._all_calibration_parameters() for f in system.flr_data)
        self._parameters_by_id = _assign_all_ids(all_calibration_parameters)

    def dump(self, system, writer):
        with writer.loop('_flr_fret_calibration_parameters',
                         ['id', 'phi_acceptor', 'alpha', 'alpha_sd',
                          'gG_gR_ratio', 'beta', 'gamma', 'delta',
                          'a_b']) as lp:
            for x in self._parameters_by_id:
                lp.write(id=x._id, phi_acceptor=x.phi_acceptor,
                         alpha=x.alpha, alpha_sd=x.alpha_sd,
                         gG_gR_ratio=x.gg_gr_ratio, beta=x.beta,
                         gamma=x.gamma, delta=x.delta, a_b=x.a_b)


class _FLRLifetimeFitModelDumper(Dumper):
    def finalize(self, system):
        def all_lifetime_fit_models():
            return itertools.chain.from_iterable(f._all_lifetime_fit_models()
                                                 for f in system.flr_data)
        self._lifetime_fit_models_by_id = \
            _assign_all_ids(all_lifetime_fit_models)

    def dump(self, system, writer):
        with writer.loop('_flr_lifetime_fit_model',
                         ['id', 'name', 'description',
                          'external_file_id', 'citation_id']) as lp:
            for x in self._lifetime_fit_models_by_id:
                lp.write(id=x._id, name=x.name,
                         description=x.description,
                         external_file_id=None if x.external_file is None
                         else x.external_file._id,
                         citation_id=None if x.citation is None
                         else x.citation._id)


class _FLRRefMeasurementDumper(Dumper):
    def finalize(self, system):
        def all_ref_measurement_groups():
            return itertools.chain.from_iterable(
                f._all_ref_measurement_groups() for f in system.flr_data)
        self._ref_measurement_groups_by_id = _assign_all_ids(
            all_ref_measurement_groups)

        def _all_ref_measurements():
            return itertools.chain.from_iterable(f._all_ref_measurements()
                                                 for f in system.flr_data)
        self._ref_measurements_by_id = _assign_all_ids(_all_ref_measurements)

        def _all_ref_measurement_lifetimes():
            return itertools.chain.from_iterable(
                f._all_ref_measurement_lifetimes() for f in system.flr_data)
        self._ref_measurement_lifetimes_by_id = _assign_all_ids(
            _all_ref_measurement_lifetimes)

    def dump(self, system, writer):
        self.dump_ref_measurement_group(system, writer)
        self.dump_ref_measurement_group_link(system, writer)
        self.dump_ref_measurement(system, writer)
        self.dump_ref_measurement_lifetimes(system, writer)

    def dump_ref_measurement_group(self, system, writer):
        with writer.loop('_flr_reference_measurement_group',
                         ['id', 'num_measurements', 'details']) as lp:
            for x in self._ref_measurement_groups_by_id:
                lp.write(id=x._id,
                         num_measurements=len(x.ref_measurement_list),
                         details=x.details)

    def dump_ref_measurement_group_link(self, system, writer):
        with writer.loop('_flr_reference_measurement_group_link',
                         ['group_id', 'reference_measurement_id']) as lp:
            for x in self._ref_measurement_groups_by_id:
                for m in x.ref_measurement_list:
                    lp.write(group_id=x._id,
                             reference_measurement_id=m._id)

    def dump_ref_measurement(self, system, writer):
        with writer.loop('_flr_reference_measurement',
                         ['id', 'reference_sample_probe_id',
                          'num_species', 'details']) as lp:
            for x in self._ref_measurements_by_id:
                lp.write(id=x._id,
                         reference_sample_probe_id=x.ref_sample_probe._id,
                         num_species=len(x.list_of_lifetimes),
                         details=x.details)

    def dump_ref_measurement_lifetimes(self, system, writer):
        with writer.loop('_flr_reference_measurement_lifetime',
                         ['ordinal_id', 'reference_measurement_id',
                          'species_name', 'species_fraction',
                          'lifetime']) as lp:
            ordinal = itertools.count(1)
            for x in self._ref_measurements_by_id:
                for m in x.list_of_lifetimes:
                    lp.write(ordinal_id=next(ordinal),
                             reference_measurement_id=x._id,
                             species_name=m.species_name,
                             species_fraction=m.species_fraction,
                             lifetime=m.lifetime)


class _FLRAnalysisDumper(Dumper):
    def finalize(self, system):
        def all_analyses():
            return itertools.chain.from_iterable(f._all_analyses()
                                                 for f in system.flr_data)
        self._analyses_by_id = _assign_all_ids(all_analyses)

    def dump(self, system, writer):
        self.dump_fret_analysis_general(system, writer)
        self.dump_fret_analysis_intensity(system, writer)
        self.dump_fret_analysis_lifetime(system, writer)

    def dump_fret_analysis_general(self, system, writer):
        with writer.loop('_flr_fret_analysis',
                         ['id', 'experiment_id', 'type',
                          'sample_probe_id_1', 'sample_probe_id_2',
                          'forster_radius_id', 'dataset_list_id',
                          'external_file_id', 'software_id']) as lp:
            for x in self._analyses_by_id:
                lp.write(id=x._id,
                         experiment_id=x.experiment._id,
                         type=x.type,
                         sample_probe_id_1=x.sample_probe_1._id,
                         sample_probe_id_2=x.sample_probe_2._id,
                         forster_radius_id=x.forster_radius._id,
                         dataset_list_id=x.dataset._id,
                         external_file_id=None if x.external_file is None
                         else x.external_file._id,
                         software_id=None if x.software is None
                         else x.software._id)

    def dump_fret_analysis_intensity(self, system, writer):
        with writer.loop('_flr_fret_analysis_intensity',
                         ['ordinal_id', 'analysis_id',
                          'calibration_parameters_id', 'donor_only_fraction',
                          'chi_square_reduced', 'method_name',
                          'details']) as lp:
            ordinal = itertools.count(1)
            for x in self._analyses_by_id:
                # if it is an intensity-based analysis.
                if 'intensity' in x.type:
                    p = x.calibration_parameters
                    lp.write(
                        ordinal_id=next(ordinal),
                        analysis_id=x._id,
                        calibration_parameters_id=None if p is None else p._id,
                        donor_only_fraction=x.donor_only_fraction,
                        chi_square_reduced=x.chi_square_reduced,
                        method_name=x.method_name,
                        details=x.details)

    def dump_fret_analysis_lifetime(self, system, writer):
        with writer.loop('_flr_fret_analysis_lifetime',
                         ['ordinal_id', 'analysis_id',
                          'reference_measurement_group_id',
                          'lifetime_fit_model_id',
                          'donor_only_fraction', 'chi_square_reduced',
                          'method_name', 'details']) as lp:
            ordinal = itertools.count(1)
            for x in self._analyses_by_id:
                # if it is a lifetime-based analysis
                if 'lifetime' in x.type:
                    mgid = x.ref_measurement_group._id
                    lp.write(
                        ordinal_id=next(ordinal),
                        analysis_id=x._id,
                        reference_measurement_group_id=mgid,
                        lifetime_fit_model_id=x.lifetime_fit_model._id,
                        donor_only_fraction=x.donor_only_fraction,
                        chi_square_reduced=x.chi_square_reduced,
                        method_name=x.method_name,
                        details=x.details)


class _FLRPeakAssignmentDumper(Dumper):
    def finalize(self, system):
        def all_peak_assignments():
            return itertools.chain.from_iterable(f._all_peak_assignments()
                                                 for f in system.flr_data)
        self._peak_assignments_by_id = _assign_all_ids(all_peak_assignments)

    def dump(self, system, writer):
        with writer.loop('_flr_peak_assignment',
                         ['id', 'method_name', 'details']) as lp:
            for x in self._peak_assignments_by_id:
                lp.write(id=x._id, method_name=x.method_name,
                         details=x.details)


class _FLRDistanceRestraintDumper(Dumper):
    def finalize(self, system):
        def all_restraint_groups():
            return itertools.chain.from_iterable(f.distance_restraint_groups
                                                 for f in system.flr_data)
        self._restraint_groups_by_id = _assign_all_ids(all_restraint_groups)

        def _all_restraints():
            return itertools.chain.from_iterable(
                rg.distance_restraint_list
                for rg in self._restraint_groups_by_id)
        for i, r in enumerate(_all_restraints()):
            r._id = i + 1

    def dump(self, system, writer):
        with writer.loop('_flr_fret_distance_restraint',
                         ['ordinal_id', 'id', 'group_id', 'sample_probe_id_1',
                          'sample_probe_id_2', 'state_id', 'analysis_id',
                          'distance', 'distance_error_plus',
                          'distance_error_minus', 'distance_type',
                          'population_fraction', 'peak_assignment_id']) as lp:
            ordinal = itertools.count(1)
            for rg in self._restraint_groups_by_id:
                for r in rg.distance_restraint_list:
                    lp.write(ordinal_id=next(ordinal), id=r._id,
                             group_id=rg._id,
                             sample_probe_id_1=r.sample_probe_1._id,
                             sample_probe_id_2=r.sample_probe_2._id,
                             state_id=None if r.state is None else r.state._id,
                             analysis_id=r.analysis._id, distance=r.distance,
                             distance_error_plus=r.distance_error_plus,
                             distance_error_minus=r.distance_error_minus,
                             distance_type=r.distance_type,
                             population_fraction=r.population_fraction,
                             peak_assignment_id=r.peak_assignment._id)


class _FLRModelQualityDumper(Dumper):
    def finalize(self, system):
        def all_model_qualities():
            return itertools.chain.from_iterable(f.fret_model_qualities
                                                 for f in system.flr_data)
        self._model_qualities_by_id = _assign_all_ids(all_model_qualities)

    def dump(self, system, writer):
        with writer.loop('_flr_fret_model_quality',
                         ['model_id', 'chi_square_reduced', 'dataset_group_id',
                          'method', 'details']) as lp:
            for x in self._model_qualities_by_id:
                lp.write(model_id=x.model._id,
                         chi_square_reduced=x.chi_square_reduced,
                         dataset_group_id=x.dataset_group._id,
                         method=x.method, details=x.details)


class _FLRModelDistanceDumper(Dumper):
    def finalize(self, system):
        def all_model_distances():
            return itertools.chain.from_iterable(f.fret_model_distances
                                                 for f in system.flr_data)
        self._model_distances_by_id = _assign_all_ids(all_model_distances)

    def dump(self, system, writer):
        with writer.loop('_flr_fret_model_distance',
                         ['id', 'restraint_id', 'model_id', 'distance',
                          'distance_deviation']) as lp:
            for x in self._model_distances_by_id:
                lp.write(id=x._id, restraint_id=x.restraint._id,
                         model_id=x.model._id, distance=x.distance,
                         distance_deviation=x.distance_deviation)


class _FLRFPSModelingDumper(Dumper):
    def finalize(self, system):
        def all_fps_modeling():
            return itertools.chain.from_iterable(f._all_fps_modeling()
                                                 for f in system.flr_data)
        self._fps_modeling_by_id = _assign_all_ids(all_fps_modeling)

        def all_fps_global_parameters():
            return itertools.chain.from_iterable(f._all_fps_global_parameters()
                                                 for f in system.flr_data)
        self._fps_modeling_by_id = _assign_all_ids(all_fps_modeling)
        self._fps_parameters_by_id = _assign_all_ids(all_fps_global_parameters)

    def dump(self, system, writer):
        self.dump_fps_modeling(system, writer)
        self.dump_fps_global_parameters(system, writer)

    def dump_fps_modeling(self, system, writer):
        with writer.loop('_flr_FPS_modeling',
                         ['id', 'ihm_modeling_protocol_ordinal_id',
                          'restraint_group_id', 'global_parameter_id',
                          'probe_modeling_method', 'details']) as lp:
            for x in self._fps_modeling_by_id:
                lp.write(id=x._id,
                         ihm_modeling_protocol_ordinal_id=x.protocol._id,
                         restraint_group_id=x.restraint_group._id,
                         global_parameter_id=x.global_parameter._id,
                         probe_modeling_method=x.probe_modeling_method,
                         details=x.details)

    def dump_fps_global_parameters(self, system, writer):
        with writer.loop('_flr_FPS_global_parameter',
                         ['id', 'forster_radius_value',
                          'conversion_function_polynom_order', 'repetition',
                          'AV_grid_rel', 'AV_min_grid_A', 'AV_allowed_sphere',
                          'AV_search_nodes', 'AV_E_samples_k',
                          'sim_viscosity_adjustment', 'sim_dt_adjustment',
                          'sim_max_iter_k', 'sim_max_force',
                          'sim_clash_tolerance_A', 'sim_reciprocal_kT',
                          'sim_clash_potential', 'convergence_E',
                          'convergence_K', 'convergence_F',
                          'convergence_T']) as lp:
            for x in self._fps_parameters_by_id:
                polynom_order = x.conversion_function_polynom_order
                lp.write(id=x._id,
                         forster_radius_value=x.forster_radius,
                         conversion_function_polynom_order=polynom_order,
                         repetition=x.repetition,
                         AV_grid_rel=x.av_grid_rel,
                         AV_min_grid_A=x.av_min_grid_a,
                         AV_allowed_sphere=x.av_allowed_sphere,
                         AV_search_nodes=x.av_search_nodes,
                         AV_E_samples_k=x.av_e_samples_k,
                         sim_viscosity_adjustment=x.sim_viscosity_adjustment,
                         sim_dt_adjustment=x.sim_dt_adjustment,
                         sim_max_iter_k=x.sim_max_iter_k,
                         sim_max_force=x.sim_max_force,
                         sim_clash_tolerance_A=x.sim_clash_tolerance_a,
                         sim_reciprocal_kT=x.sim_reciprocal_kt,
                         sim_clash_potential=x.sim_clash_potential,
                         convergence_E=x.convergence_e,
                         convergence_K=x.convergence_k,
                         convergence_F=x.convergence_f,
                         convergence_T=x.convergence_t)


class _FLRFPSAVModelingDumper(Dumper):
    def finalize(self, system):
        def all_fps_av_modeling():
            return itertools.chain.from_iterable(f._all_fps_av_modeling()
                                                 for f in system.flr_data)
        self._fps_av_modeling_by_id = _assign_all_ids(all_fps_av_modeling)

        def all_fps_av_parameter():
            return itertools.chain.from_iterable(f._all_fps_av_parameter()
                                                 for f in system.flr_data)
        self._fps_av_modeling_by_id = _assign_all_ids(all_fps_av_modeling)
        self._fps_av_parameter_by_id = _assign_all_ids(all_fps_av_parameter)

    def dump(self, system, writer):
        self.dump_parameter(system, writer)
        self.dump_modeling(system, writer)

    def dump_parameter(self, system, writer):
        with writer.loop('_flr_FPS_AV_parameter',
                         ['id', 'num_linker_atoms', 'linker_length',
                          'linker_width', 'probe_radius_1', 'probe_radius_2',
                          'probe_radius_3']) as lp:
            for x in self._fps_av_parameter_by_id:
                lp.write(id=x._id,
                         num_linker_atoms=x.num_linker_atoms,
                         linker_length=x.linker_length,
                         linker_width=x.linker_width,
                         probe_radius_1=x.probe_radius_1,
                         probe_radius_2=x.probe_radius_2,
                         probe_radius_3=x.probe_radius_3)

    def dump_modeling(self, system, writer):
        with writer.loop('_flr_FPS_AV_modeling',
                         ['id', 'sample_probe_id', 'FPS_modeling_id',
                          'parameter_id']) as lp:
            for x in self._fps_av_modeling_by_id:
                lp.write(id=x._id,
                         sample_probe_id=x.sample_probe._id,
                         FPS_modeling_id=x.fps_modeling._id,
                         parameter_id=x.parameter._id)


class _FLRFPSMPPModelingDumper(Dumper):
    def finalize(self, system):
        def all_fps_mpp_modeling():
            return itertools.chain.from_iterable(f._all_fps_mpp_modeling()
                                                 for f in system.flr_data)
        self._fps_mpp_modeling_by_id = _assign_all_ids(all_fps_mpp_modeling)

        def all_fps_mean_probe_position():
            return itertools.chain.from_iterable(
                f._all_fps_mean_probe_position() for f in system.flr_data)
        self._fps_mpp_modeling_by_id = _assign_all_ids(all_fps_mpp_modeling)
        self._fps_mpp_by_id = _assign_all_ids(all_fps_mean_probe_position)

        def all_atom_position_group():
            return itertools.chain.from_iterable(
                f._all_fps_atom_position_group() for f in system.flr_data)
        self._atom_group_by_id = _assign_all_ids(all_atom_position_group)

        def _all_atom_positions():
            return itertools.chain.from_iterable(
                ag.mpp_atom_position_list for ag in self._atom_group_by_id)
        for i, a in enumerate(_all_atom_positions()):
            a._id = i + 1

    def dump(self, system, writer):
        self.dump_mean_probe_position(system, writer)
        self.dump_mpp_atom_position(system, writer)
        self.dump_mpp_modeling(system, writer)

    def dump_mean_probe_position(self, system, writer):
        with writer.loop('_flr_FPS_mean_probe_position',
                         ['id', 'sample_probe_id', 'mpp_xcoord', 'mpp_ycoord',
                          'mpp_zcoord']) as lp:
            for x in self._fps_mpp_by_id:
                lp.write(id=x._id, sample_probe_id=x.sample_probe._id,
                         mpp_xcoord=x.x, mpp_ycoord=x.y, mpp_zcoord=x.z)

    def dump_mpp_atom_position(self, system, writer):
        with writer.loop('_flr_FPS_MPP_atom_position',
                         ['id', 'entity_id', 'seq_id', 'comp_id', 'atom_id',
                          'asym_id', 'xcoord', 'ycoord', 'zcoord',
                          'group_id']) as lp:
            for group in self._atom_group_by_id:
                for x in group.mpp_atom_position_list:
                    comp = x.atom.asym.entity.sequence[x.atom.seq_id - 1].id
                    lp.write(id=x._id, entity_id=x.atom.asym.entity._id,
                             seq_id=x.atom.seq_id, comp_id=comp,
                             atom_id=x.atom.id, asym_id=x.atom.asym._id,
                             xcoord=x.x, ycoord=x.y, zcoord=x.z,
                             group_id=group._id)

    def dump_mpp_modeling(self, system, writer):
        ordinal = itertools.count(1)
        with writer.loop('_flr_FPS_MPP_modeling',
                         ['ordinal_id', 'FPS_modeling_id', 'mpp_id',
                          'mpp_atom_position_group_id']) as lp:
            for x in self._fps_mpp_modeling_by_id:
                lp.write(
                    ordinal_id=next(ordinal),
                    FPS_modeling_id=x.fps_modeling._id, mpp_id=x.mpp._id,
                    mpp_atom_position_group_id=x.mpp_atom_position_group._id)


_flr_dumpers = [_FLRExperimentDumper, _FLRInstSettingDumper,
                _FLR_ExpConditionDumper, _FLRInstrumentDumper,
                _FLREntityAssemblyDumper, _FLRSampleConditionDumper,
                _FLRSampleDumper, _FLRProbeDumper,
                _FLRSampleProbeDetailsDumper, _FLRPolyProbePositionDumper,
                _FLRConjugateDumper, _FLRForsterRadiusDumper,
                _FLRCalibrationParametersDumper, _FLRLifetimeFitModelDumper,
                _FLRRefMeasurementDumper, _FLRAnalysisDumper,
                _FLRPeakAssignmentDumper, _FLRDistanceRestraintDumper,
                _FLRModelQualityDumper, _FLRModelDistanceDumper,
                _FLRFPSModelingDumper, _FLRFPSAVModelingDumper,
                _FLRFPSMPPModelingDumper]


class _NullLoopCategoryWriter(object):
    """A do-nothing replacement for format._CifLoopWriter
       or format._CifCategoryWriter"""
    def write(self, *args, **keys):
        pass

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        pass


class _IgnoreWriter(object):
    """Utility class which normally just passes through to the default
       ``base_writer``, but ignores selected categories."""
    def __init__(self, base_writer, ignores):
        self._base_writer = base_writer
        # Allow for categories with or without leading underscore
        self._ignore_category = frozenset('_' + c.lstrip('_').lower()
                                          for c in ignores)

    def category(self, category):
        if category in self._ignore_category:
            return _NullLoopCategoryWriter()
        else:
            return self._base_writer.category(category)

    def loop(self, category, keys):
        if category in self._ignore_category:
            return _NullLoopCategoryWriter()
        else:
            return self._base_writer.loop(category, keys)

    # Pass through other methods to base_writer
    def flush(self):
        return self._base_writer.flush()

    def end_block(self):
        return self._base_writer.end_block()

    def start_block(self, name):
        return self._base_writer.start_block(name)

    def write_comment(self, comment):
        return self._base_writer.write_comment(comment)


class Variant(object):
    """Utility class to select the type of file to output by :func:`write`."""

    def get_dumpers(self):
        """Get the :class:`Dumper` objects to use to write output.

           :return: a list of :class:`Dumper` objects.
        """
        pass

    def get_system_writer(self, system, writer_class, writer):
        """Get a writer tailored to the given system.
           By default, this just returns the ``writer`` unchanged."""
        return writer


class IHMVariant(Variant):
    """Used to select typical PDBx/IHM file output. See :func:`write`."""
    _dumpers = [
        _EntryDumper,  # must be first
        _StructDumper, _CommentDumper, _AuditConformDumper, _CitationDumper,
        _SoftwareDumper, _AuditAuthorDumper, _GrantDumper, _ChemCompDumper,
        _ChemDescriptorDumper, _EntityDumper, _EntitySrcGenDumper,
        _EntitySrcNatDumper, _EntitySrcSynDumper, _StructRefDumper,
        _EntityPolyDumper, _EntityNonPolyDumper, _EntityPolySeqDumper,
        _EntityPolySegmentDumper, _StructAsymDumper, _PolySeqSchemeDumper,
        _NonPolySchemeDumper, _AssemblyDumper, _ExternalReferenceDumper,
        _DatasetDumper, _ModelRepresentationDumper, _StartingModelDumper,
        _ProtocolDumper, _PostProcessDumper, _PseudoSiteDumper,
        _GeometricObjectDumper, _FeatureDumper, _CrossLinkDumper,
        _GeometricRestraintDumper, _DerivedDistanceRestraintDumper,
        _PredictedContactRestraintDumper, _EM3DDumper, _EM2DDumper, _SASDumper,
        _ModelDumper, _EnsembleDumper, _DensityDumper, _MultiStateDumper,
        _OrderedDumper]

    def get_dumpers(self):
        return [d() for d in self._dumpers + _flr_dumpers]


class IgnoreVariant(IHMVariant):
    """Exclude selected CIF categories from output.

       This generates the same PDBx/IHM output as :class:`IHMVariant`,
       but explicitly listed CIF categories are discarded, for example::

           ihm.dumper.write(fh, systems,
                            variant=IgnoreVariant(['_audit_conform']))

       This is intended for advanced users that have a working knowledge
       of the PDBx and IHM CIF dictionaries.

       :param sequence ignores: A list or tuple of CIF categories to exclude.
    """
    def __init__(self, ignores):
        self._ignores = ignores

    def get_system_writer(self, system, writer_class, writer):
        return _IgnoreWriter(writer, self._ignores)


def write(fh, systems, format='mmCIF', dumpers=[], variant=IHMVariant):
    """Write out all `systems` to the file handle `fh`.
       Files can be written in either the text-based mmCIF format or the
       BinaryCIF format. The BinaryCIF writer needs the msgpack Python
       module to function.

       The file handle should be opened in binary mode for BinaryCIF files.
       For mmCIF, text mode should be used, usually with UTF-8 encoding, e.g.::

           with open('output.cif', 'w', encoding='utf-8') as fh:
               ihm.dumper.write(fh, systems)
           with open('output.bcif', 'wb') as fh:
               ihm.dumper.write(fh, systems, format='BCIF')

       :param file fh: The file handle to write to.
       :param list systems: The list of :class:`ihm.System` objects to write.
       :param str format: The format of the file. This can be 'mmCIF' (the
              default) for the (text-based) mmCIF format or 'BCIF' for
              BinaryCIF.
       :param list dumpers: A list of :class:`Dumper` classes (not objects).
              These can be used to add extra categories to the file.
       :param variant: A class or object that selects the type of file to
              output. This primarily controls the set of tables that are
              written to the file. In most cases the default
              :class:`IHMVariant` should be used.
       :type variant: :class:`Variant`
    """
    if isinstance(variant, type):
        variant = variant()
    dumpers = variant.get_dumpers() + [d() for d in dumpers]
    writer_map = {'mmCIF': ihm.format.CifWriter,
                  'BCIF': ihm.format_bcif.BinaryCifWriter}

    writer = writer_map[format](fh)
    for system in systems:
        w = variant.get_system_writer(system, writer_map[format], writer)
        system._before_write()

        for d in dumpers:
            d.finalize(system)
        system._check_after_write()
        for d in dumpers:
            d.dump(system, w)
        w.end_block()  # start_block is called by EntryDumper
    writer.flush()
