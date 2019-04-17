"""Utility classes to dump out information in mmCIF format"""

import re
import os
import operator
import ihm.format
import ihm.format_bcif
import ihm.model
import ihm.representation
import ihm.source
from . import util
from . import location
from . import restraint
from . import geometry

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


class _EntryDumper(Dumper):
    def dump(self, system, writer):
        # Write CIF header (so this dumper should always be first)
        writer.start_block(re.subn('[^0-9a-zA-z_]', '', system.id)[0])
        with writer.category("_entry") as l:
            l.write(id=system.id)


class _AuditConformDumper(Dumper):
    URL = ("https://raw.githubusercontent.com/" +
           "ihmwg/IHM-dictionary/%s/ihm-extension.dic")

    def dump(self, system, writer):
        with writer.category("_audit_conform") as l:
            # Update to match the version of the IHM dictionary we support:
            l.write(dict_name="ihm-extension.dic", dict_version="0.137",
                    dict_location=self.URL % "7ea672a")


class _StructDumper(Dumper):
    def dump(self, system, writer):
        with writer.category("_struct") as l:
            l.write(title=system.title, entry_id=system.id)


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
        # todo: specify these attributes in only one place (e.g. in the Software
        # class)
        with writer.loop("_software",
                         ["pdbx_ordinal", "name", "classification",
                          "description", "version", "type", "location"]) as l:
            for s in self._software_by_id:
                l.write(pdbx_ordinal=s._id, name=s.name,
                        classification=s.classification,
                        description=s.description, version=s.version,
                        type=s.type, location=s.location)


class _CitationDumper(Dumper):
    def finalize(self, system):
        for nc, c in enumerate(system._all_citations()):
            c._id = nc + 1

    def dump(self, system, writer):
        citations = list(system._all_citations())
        self.dump_citations(citations, writer)
        self.dump_authors(citations, writer)

    def dump_citations(self, citations, writer):
        with writer.loop("_citation",
                         ["id", "title", "journal_abbrev", "journal_volume",
                          "page_first", "page_last", "year",
                          "pdbx_database_id_PubMed",
                          "pdbx_database_id_DOI"]) as l:
            for c in citations:
                if isinstance(c.page_range, (tuple, list)):
                    page_first, page_last = c.page_range
                else:
                    page_first = c.page_range
                    page_last = None
                l.write(id=c._id, title=c.title, journal_abbrev=c.journal,
                        journal_volume=c.volume, page_first=page_first,
                        page_last=page_last, year=c.year,
                        pdbx_database_id_PubMed=c.pmid,
                        pdbx_database_id_DOI=c.doi)

    def dump_authors(self, citations, writer):
        with writer.loop("_citation_author",
                         ["citation_id", "name", "ordinal"]) as l:
            ordinal = 1
            for c in citations:
                for a in c.authors:
                    l.write(citation_id=c._id, name=a, ordinal=ordinal)
                    ordinal += 1


class _AuditAuthorDumper(Dumper):
    def _get_citation_authors(self, system):
        # If system.authors is empty, get the set of all citation authors
        # instead
        seen_authors = set()
        for c in system._all_citations():
            for a in c.authors:
                if a not in seen_authors:
                    seen_authors.add(a)
                    yield a

    def dump(self, system, writer):
        authors = system.authors or self._get_citation_authors(system)
        with writer.loop("_audit_author",
                         ["name", "pdbx_ordinal"]) as l:
            for n, author in enumerate(authors):
                l.write(name=author, pdbx_ordinal=n+1)


class _GrantDumper(Dumper):
    def dump(self, system, writer):
        with writer.loop("_pdbx_audit_support",
                         ["funding_organization", "country", "grant_number",
                          "ordinal"]) as l:
            for n, grant in enumerate(system.grants):
                l.write(funding_organization=grant.funding_organization,
                        country=grant.country,
                        grant_number=grant.grant_number, ordinal=n+1)


class _ChemCompDumper(Dumper):
    def dump(self, system, writer):
        comps = frozenset(comp for e in system.entities for comp in e.sequence)

        with writer.loop("_chem_comp", ["id", "type", "name",
                                        "formula", "formula_weight"]) as l:
            for comp in sorted(comps, key=operator.attrgetter('id')):
                l.write(id=comp.id, type=comp.type, name=comp.name,
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
        with writer.loop("_ihm_chemical_descriptor",
                ["id", "auth_name", "chem_comp_id", "chemical_name",
                 "common_name", "smiles", "smiles_canonical", "inchi",
                 "inchi_key"]) as l:
            for d in self._descriptor_by_id:
                l.write(id=d._id, auth_name=d.auth_name,
                        chem_comp_id=d.chem_comp_id,
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
                          "details"]) as l:
            for entity in system.entities:
                l.write(id=entity._id, type=entity.type,
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
        with writer.loop("_entity_src_gen",
                ["entity_id", "pdbx_src_id", "pdbx_gene_src_ncbi_taxonomy_id",
                 "pdbx_gene_src_scientific_name",
                 "gene_src_common_name", "gene_src_strain",
                 "pdbx_host_org_ncbi_taxonomy_id",
                 "pdbx_host_org_scientific_name",
                 "host_org_common_name", "pdbx_host_org_strain"]) as l:
            for e in system.entities:
                if isinstance(e.source, ihm.source.Manipulated):
                    self._dump_source(l, e)

    def _dump_source(self, l, e):
        s = e.source
        l.write(entity_id=e._id, pdbx_src_id=s._id,
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
        with writer.loop("_entity_src_nat",
                ["entity_id", "pdbx_src_id", "pdbx_ncbi_taxonomy_id",
                 "pdbx_organism_scientific", "common_name", "strain"]) as l:
            for e in system.entities:
                s = e.source
                if isinstance(s, ihm.source.Natural):
                    l.write(entity_id=e._id, pdbx_src_id=s._id,
                            pdbx_ncbi_taxonomy_id=s.ncbi_taxonomy_id,
                            pdbx_organism_scientific=s.scientific_name,
                            common_name=s.common_name, strain=s.strain)


class _EntitySrcSynDumper(Dumper):
    def finalize(self, system):
        _assign_src_ids(system, ihm.source.Synthetic)

    def dump(self, system, writer):
        # Note that _pdbx_entity_src_syn.strain is not used in current PDB
        # entries
        with writer.loop("_pdbx_entity_src_syn",
                ["entity_id", "pdbx_src_id", "ncbi_taxonomy_id",
                 "organism_scientific", "organism_common_name"]) as l:
            for e in system.entities:
                s = e.source
                if isinstance(s, ihm.source.Synthetic):
                    l.write(entity_id=e._id, pdbx_src_id=s._id,
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
        # Get the first asym unit (if any) for each entity
        strand = {}
        for asym in system.asym_units:
            if asym.entity._id not in strand:
                strand[asym.entity._id] = asym._id
        with writer.loop("_entity_poly",
                         ["entity_id", "type", "nstd_linkage",
                          "nstd_monomer", "pdbx_strand_id",
                          "pdbx_seq_one_letter_code",
                          "pdbx_seq_one_letter_code_can"]) as l:
            for entity in system.entities:
                if not entity.is_polymeric():
                    continue
                l.write(entity_id=entity._id, type=self._get_seq_type(entity),
                        nstd_linkage='no', nstd_monomer='no',
                        pdbx_strand_id=strand.get(entity._id, None),
                        pdbx_seq_one_letter_code=self._get_sequence(entity),
                        pdbx_seq_one_letter_code_can=self._get_canon(entity))


class _EntityNonPolyDumper(Dumper):
    def dump(self, system, writer):
        with writer.loop("_pdbx_entity_nonpoly",
                         ["entity_id", "name", "comp_id"]) as l:
            for entity in system.entities:
                if entity.is_polymeric():
                    continue
                l.write(entity_id=entity._id, name=entity.description,
                        comp_id=entity.sequence[0].id)


class _EntityPolySeqDumper(Dumper):
    def dump(self, system, writer):
        with writer.loop("_entity_poly_seq",
                         ["entity_id", "num", "mon_id", "hetero"]) as l:
            for entity in system.entities:
                if not entity.is_polymeric():
                    continue
                for num, comp in enumerate(entity.sequence):
                    l.write(entity_id=entity._id, num=num + 1, mon_id=comp.id)


class _PolySeqSchemeDumper(Dumper):
    """Output the _pdbx_poly_seq_scheme table.
       This is needed because it is a parent category of atom_site.
       For now we assume we're using auth_seq_num==pdb_seq_num."""
    def dump(self, system, writer):
        with writer.loop("_pdbx_poly_seq_scheme",
                         ["asym_id", "entity_id", "seq_id", "mon_id",
                          "pdb_seq_num", "auth_seq_num", "pdb_mon_id",
                          "auth_mon_id", "pdb_strand_id"]) as l:
            for asym in system.asym_units:
                entity = asym.entity
                if not entity.is_polymeric():
                    continue
                for num, comp in enumerate(entity.sequence):
                    auth_seq_num = asym._get_auth_seq_id(num+1)
                    l.write(asym_id=asym._id, pdb_strand_id=asym._id,
                            entity_id=entity._id,
                            seq_id=num+1, pdb_seq_num=auth_seq_num,
                            auth_seq_num=auth_seq_num,
                            mon_id=comp.id, pdb_mon_id=comp.id,
                            auth_mon_id=comp.id)


class _NonPolySchemeDumper(Dumper):
    """Output the _pdbx_nonpoly_scheme table.
       For now we assume we're using auth_seq_num==pdb_seq_num."""
    def dump(self, system, writer):
        with writer.loop("_pdbx_nonpoly_scheme",
                         ["asym_id", "entity_id", "mon_id",
                          "pdb_seq_num", "auth_seq_num", "pdb_mon_id",
                          "auth_mon_id", "pdb_strand_id"]) as l:
            for asym in system.asym_units:
                entity = asym.entity
                if entity.is_polymeric():
                    continue
                # todo: handle multiple waters
                for num, comp in enumerate(entity.sequence):
                    auth_seq_num = asym._get_auth_seq_id(num+1)
                    l.write(asym_id=asym._id, pdb_strand_id=asym._id,
                            entity_id=entity._id,
                            pdb_seq_num=auth_seq_num,
                            auth_seq_num=auth_seq_num,
                            mon_id=comp.id, pdb_mon_id=comp.id,
                            auth_mon_id=comp.id)


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
        ordinal = 1
        # Assign remaining asym IDs
        id_prov = _AsymIDProvider(seen_asym_ids)
        for asym in system.asym_units:
            if asym.id is None:
                asym._id = id_prov.get_next_id()
            asym._ordinal = ordinal
            ordinal += 1

    def dump(self, system, writer):
        with writer.loop("_struct_asym",
                         ["id", "entity_id", "details"]) as l:
            for asym in system.asym_units:
                l.write(id=asym._id, entity_id=asym.entity._id,
                        details=asym.details)


class _AssemblyDumper(Dumper):
    def finalize(self, system):
        # Fill in complete assembly
        system._make_complete_assembly()

        # Sort each assembly by entity/asym id/range
        def component_key(comp):
            if hasattr(comp, 'entity'): # asymmetric unit or range
                return (comp.entity._id, comp._ordinal, comp.seq_id_range)
            else: # entity or range
                return (comp._id, 0, comp.seq_id_range)
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

    def dump_details(self, system, writer):
        with writer.loop("_ihm_struct_assembly_details",
                         ["assembly_id", "assembly_name",
                          "assembly_description"]) as l:
            for a in self._assembly_by_id:
                l.write(assembly_id=a._id, assembly_name=a.name,
                        assembly_description=a.description)

    def dump(self, system, writer):
        self.dump_details(system, writer)
        ordinal = 1
        with writer.loop("_ihm_struct_assembly",
                         ["ordinal_id", "assembly_id", "parent_assembly_id",
                          "entity_description",
                          "entity_id", "asym_id", "seq_id_begin",
                          "seq_id_end"]) as l:
            for a in self._assembly_by_id:
                for comp in a:
                    entity = comp.entity if hasattr(comp, 'entity') else comp
                    seqrange = comp.seq_id_range
                    l.write(ordinal_id=ordinal, assembly_id=a._id,
                            # if no hierarchy then assembly is self-parent
                            parent_assembly_id=a.parent._id if a.parent
                                               else a._id,
                            entity_description=entity.description,
                            entity_id=entity._id,
                            asym_id=comp._id if hasattr(comp, 'entity')
                                             else None,
                            seq_id_begin=seqrange[0],
                            seq_id_end=seqrange[1])
                    ordinal += 1

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
                          "associated_url"]) as l:
            for repo in self._repo_by_id:
                l.write(reference_id=repo._id,
                        reference_provider=repo.reference_provider,
                        reference_type=repo.reference_type,
                        reference=repo.reference, refers_to=repo.refers_to,
                        associated_url=repo.url)

    def dump_refs(self, writer):
        with writer.loop("_ihm_external_files",
                         ["id", "reference_id", "file_path", "content_type",
                          "file_size_bytes", "details"]) as l:
            for r in self._ref_by_id:
                repo = r.repo or self._local_files
                file_path = self._posix_path(repo._get_full_path(r.path))
                l.write(id=r._id, reference_id=repo._id,
                        file_path=file_path, content_type=r.content_type,
                        file_size_bytes=r.file_size, details=r.details)

    # On Windows systems, convert native paths to POSIX-like (/-separated) paths
    if os.sep == '/':
        def _posix_path(self, path):
            return path
    else:
        def _posix_path(self, path):
            return path.replace(os.sep, '/')


class _DatasetDumper(Dumper):
    def finalize(self, system):
        seen_datasets = {}
        # Assign IDs to all datasets
        self._dataset_by_id = []
        for d in system._all_datasets():
            util._remove_id(d)
        for d in system._all_datasets():
            util._assign_id(d, seen_datasets, self._dataset_by_id)

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
                         ["id", "data_type", "database_hosted"]) as l:
            for d in self._dataset_by_id:
                l.write(id=d._id, data_type=d.data_type,
                        database_hosted=isinstance(d.location,
                                                   location.DatabaseLocation))
        self.dump_groups(writer)
        self.dump_other((d for d in self._dataset_by_id
                         if not isinstance(d.location,
                                           location.DatabaseLocation)),
                        writer)
        self.dump_rel_dbs((d for d in self._dataset_by_id
                           if isinstance(d.location,
                                         location.DatabaseLocation)),
                          writer)
        self.dump_related(system, writer)

    def dump_groups(self, writer):
        ordinal = 1
        with writer.loop("_ihm_dataset_group",
                         ["ordinal_id", "group_id", "dataset_list_id"]) as l:
            for g in self._dataset_group_by_id:
                # Don't duplicate IDs, and output in sorted order
                for dataset_id in sorted(set(d._id for d in g)):
                    l.write(ordinal_id=ordinal, group_id=g._id,
                            dataset_list_id=dataset_id)
                    ordinal += 1

    def dump_other(self, datasets, writer):
        ordinal = 1
        with writer.loop("_ihm_dataset_external_reference",
                         ["id", "dataset_list_id", "file_id"]) as l:
            for d in datasets:
                l.write(id=ordinal, dataset_list_id=d._id,
                        file_id=d.location._id)
                ordinal += 1

    def dump_rel_dbs(self, datasets, writer):
        ordinal = 1
        with writer.loop("_ihm_dataset_related_db_reference",
                         ["id", "dataset_list_id", "db_name",
                          "accession_code", "version", "details"]) as l:
            for d in datasets:
                l.write(id=ordinal, dataset_list_id=d._id,
                        db_name=d.location.db_name,
                        accession_code=d.location.access_code,
                        version=d.location.version,
                        details=d.location.details)
                ordinal += 1

    def dump_related(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_related_datasets",
                         ["ordinal_id", "dataset_list_id_derived",
                          "dataset_list_id_primary"]) as l:
            for derived in self._dataset_by_id:
                # Don't duplicate IDs, and output in sorted order
                for parent_id in sorted(set(d._id for d in derived.parents)):
                    l.write(ordinal_id=ordinal,
                            dataset_list_id_derived=derived._id,
                            dataset_list_id_primary=parent_id)
                    ordinal += 1

class _ModelRepresentationDumper(Dumper):
    def finalize(self, system):
        # Assign IDs to representations and segments
        for nr, r in enumerate(system._all_representations()):
            r._id = nr + 1
            for ns, s in enumerate(r):
                s._id = ns + 1

    def dump(self, system, writer):
        ordinal_id = 1
        with writer.loop("_ihm_model_representation",
                         ["ordinal_id", "representation_id",
                          "segment_id", "entity_id", "entity_description",
                          "entity_asym_id",
                          "seq_id_begin", "seq_id_end",
                          "model_object_primitive", "starting_model_id",
                          "model_mode", "model_granularity",
                          "model_object_count"]) as l:
            for r in system._all_representations():
                for segment in r:
                    entity = segment.asym_unit.entity
                    l.write(ordinal_id=ordinal_id, representation_id=r._id,
                            segment_id=segment._id, entity_id=entity._id,
                            entity_description=entity.description,
                            entity_asym_id=segment.asym_unit._id,
                            seq_id_begin=segment.asym_unit.seq_id_range[0],
                            seq_id_end=segment.asym_unit.seq_id_range[1],
                            model_object_primitive=segment.primitive,
                            starting_model_id=segment.starting_model._id
                                                  if segment.starting_model
                                                  else None,
                            model_mode='rigid' if segment.rigid else 'flexible',
                            model_granularity=segment.granularity,
                            model_object_count=segment.count)
                    ordinal_id += 1


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
                      'Experimental model': 'experimental model'}
        with writer.loop("_ihm_starting_model_details",
                     ["starting_model_id", "entity_id", "entity_description",
                      "asym_id", "seq_id_begin",
                      "seq_id_end", "starting_model_source",
                      "starting_model_auth_asym_id",
                      "starting_model_sequence_offset",
                      "dataset_list_id"]) as l:
             for sm in system._all_starting_models():
                seq_id_range = sm.get_seq_id_range_all_templates()
                l.write(starting_model_id=sm._id,
                        entity_id=sm.asym_unit.entity._id,
                        entity_description=sm.asym_unit.entity.description,
                        asym_id=sm.asym_unit._id,
                        seq_id_begin=seq_id_range[0],
                        seq_id_end=seq_id_range[1],
                        starting_model_source=source_map[sm.dataset.data_type],
                        starting_model_auth_asym_id=sm.asym_id,
                        dataset_list_id=sm.dataset._id,
                        starting_model_sequence_offset=sm.offset)

    def dump_computational(self, system, writer):
        """Dump details on computational models."""
        with writer.loop("_ihm_starting_computational_models",
                     ["starting_model_id", "software_id",
                      "script_file_id"]) as l:
            for sm in system._all_starting_models():
                if sm.software or sm.script_file:
                    l.write(starting_model_id=sm._id,
                            software_id=sm.software._id
                                        if sm.software else None,
                            script_file_id=sm.script_file._id
                                            if sm.script_file else None)

    def dump_comparative(self, system, writer):
        """Dump details on comparative models."""
        with writer.loop("_ihm_starting_comparative_models",
                     ["ordinal_id", "starting_model_id",
                      "starting_model_auth_asym_id",
                      "starting_model_seq_id_begin",
                      "starting_model_seq_id_end",
                      "template_auth_asym_id", "template_seq_id_begin",
                      "template_seq_id_end", "template_sequence_identity",
                      "template_sequence_identity_denominator",
                      "template_dataset_list_id",
                      "alignment_file_id"]) as l:
            ordinal = 1
            for sm in system._all_starting_models():
                off = sm.offset
                for template in sm.templates:
                    l.write(ordinal_id=ordinal,
                      starting_model_id=sm._id,
                      starting_model_auth_asym_id=sm.asym_id,
                      starting_model_seq_id_begin=template.seq_id_range[0]+off,
                      starting_model_seq_id_end=template.seq_id_range[1]+off,
                      template_auth_asym_id=template.asym_id,
                      template_seq_id_begin=template.template_seq_id_range[0],
                      template_seq_id_end=template.template_seq_id_range[1],
                      template_sequence_identity=
                                  float(template.sequence_identity),
                      template_sequence_identity_denominator=
                                  int(template.sequence_identity.denominator),
                      template_dataset_list_id=template.dataset._id
                                               if template.dataset else None,
                      alignment_file_id=template.alignment_file._id
                                        if template.alignment_file else None)
                    ordinal += 1

    def dump_coords(self, system, writer):
        """Write out coordinate information"""
        ordinal = 1
        with writer.loop("_ihm_starting_model_coord",
                     ["starting_model_id", "group_PDB", "id", "type_symbol",
                      "atom_id", "comp_id", "entity_id", "asym_id",
                      "seq_id", "Cartn_x",
                      "Cartn_y", "Cartn_z", "B_iso_or_equiv",
                      "ordinal_id"]) as l:
            for model in system._all_starting_models():
                for natom, atom in enumerate(model.get_atoms()):
                    comp = atom.asym_unit.entity.sequence[atom.seq_id-1]
                    l.write(starting_model_id=model._id,
                            group_PDB='HETATM' if atom.het else 'ATOM',
                            id=natom+1,
                            type_symbol=atom.type_symbol,
                            atom_id=atom.atom_id,
                            comp_id=comp.id,
                            asym_id=atom.asym_unit._id,
                            entity_id=atom.asym_unit.entity._id,
                            seq_id=atom.seq_id,
                            Cartn_x=atom.x, Cartn_y=atom.y, Cartn_z=atom.z,
                            B_iso_or_equiv=atom.biso,
                            ordinal_id=ordinal)
                    ordinal += 1

    def dump_seq_dif(self, system, writer):
        """Write out sequence difference information"""
        ordinal = 1
        with writer.loop("_ihm_starting_model_seq_dif",
                     ["ordinal_id", "entity_id", "asym_id",
                      "seq_id", "comp_id", "starting_model_id",
                      "db_asym_id", "db_seq_id", "db_comp_id",
                      "details"]) as l:
            for model in system._all_starting_models():
                for sd in model.get_seq_dif():
                    comp = model.asym_unit.entity.sequence[sd.seq_id-1]
                    l.write(ordinal_id=ordinal,
                        entity_id=model.asym_unit.entity._id,
                        asym_id=model.asym_unit._id,
                        seq_id=sd.seq_id, comp_id=comp.id,
                        db_asym_id=model.asym_id, db_seq_id=sd.db_seq_id,
                        db_comp_id=sd.db_comp_id, starting_model_id=model._id,
                        details=sd.details)
                    ordinal += 1


class _ProtocolDumper(Dumper):
    def finalize(self, system):
        # Assign IDs to protocols and steps
        for np, p in enumerate(system._all_protocols()):
            p._id = np + 1
            for ns, s in enumerate(p.steps):
                s._id = ns + 1

    def dump(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_modeling_protocol",
                         ["ordinal_id", "protocol_id", "step_id",
                          "struct_assembly_id", "dataset_group_id",
                          "struct_assembly_description", "protocol_name",
                          "step_name", "step_method", "num_models_begin",
                          "num_models_end", "multi_scale_flag",
                          "multi_state_flag", "ordered_flag",
                          "software_id", "script_file_id"]) as l:
            for p in system._all_protocols():
                for s in p.steps:
                    l.write(ordinal_id=ordinal, protocol_id=p._id,
                            step_id=s._id,
                            struct_assembly_id=s.assembly._id,
                            dataset_group_id=s.dataset_group._id
                                             if s.dataset_group else None,
                            struct_assembly_description=s.assembly.description,
                            protocol_name=p.name,
                            step_name=s.name, step_method=s.method,
                            num_models_begin=s.num_models_begin,
                            num_models_end=s.num_models_end,
                            multi_state_flag=s.multi_state,
                            ordered_flag=s.ordered,
                            multi_scale_flag=s.multi_scale,
                            software_id=s.software._id if s.software else None,
                            script_file_id=s.script_file._id
                                           if s.script_file else None)
                    ordinal += 1


class _PostProcessDumper(Dumper):
    def finalize(self, system):
        pp_id = 1
        # Assign IDs to analyses and steps
        # todo: handle case where one analysis is referred to from multiple
        # protocols
        for p in system._all_protocols():
            for na, a in enumerate(p.analyses):
                a._id = na + 1
                for ns, s in enumerate(a.steps):
                    s._id = ns + 1
                    # Assign globally unique postproc id
                    s._post_proc_id = pp_id
                    pp_id += 1

    def dump(self, system, writer):
        with writer.loop("_ihm_modeling_post_process",
                         ["id", "protocol_id", "analysis_id", "step_id",
                          "type", "feature", "num_models_begin",
                          "num_models_end", "struct_assembly_id",
                          "dataset_group_id", "software_id",
                          "script_file_id"]) as l:
            for p in system._all_protocols():
                for a in p.analyses:
                    for s in a.steps:
                        l.write(id=s._post_proc_id, protocol_id=p._id,
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
                                               if s.script_file else None)


class _RangeChecker(object):
    """Check Atom or Sphere objects to make sure they match the
       Representation and Assembly"""
    def __init__(self, model):
        self._setup_representation(model)
        self._setup_assembly(model)

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
            return (obj.seq_id_range[0] == segment.asym_unit.seq_id_range[0]
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
        self._check_assembly(obj, asym, seq_id_range)
        self._check_representation(obj, asym, type_check, seq_id_range)

    def _check_assembly(self, obj, asym, seq_id_range):
        # Check last match first
        last_rng = self._last_asmb_range_matched
        if last_rng and asym._id == self._last_asmb_asym_matched \
           and seq_id_range[0] >= last_rng[0] \
           and seq_id_range[1] <= last_rng[1]:
            return
        # Check asym_id
        if asym._id not in self.asmb_asym_ids:
            raise ValueError("%s refers to an asym ID (%s) that is not in this "
                 "model's assembly (which includes the following asym IDs: %s)"
                 % (obj, asym._id,
                    ", ".join(sorted(a for a in self.asmb_asym_ids))))
        # Check range
        for rng in self.asmb_asym_ids[asym._id]:
            if seq_id_range[0] >= rng[0] and seq_id_range[1] <= rng[1]:
                self._last_asmb_asym_matched = asym._id
                self._last_asmb_range_matched = rng
                return
        raise ValueError("%s seq_id range (%d-%d) does not match any range "
                    "in the assembly for asym ID %s (ranges are %s)"
                    % (obj, seq_id_range[0], seq_id_range[1], asym._id,
                       ", ".join("%d-%d" % x
                                 for x in self.asmb_asym_ids[asym._id])))

    def _check_representation(self, obj, asym, type_check, seq_id_range):
        # Check last match first
        last_seg = self._last_repr_segment_matched
        if last_seg and asym._id == last_seg.asym_unit._id \
           and seq_id_range[0] >= last_seg.asym_unit.seq_id_range[0] \
           and seq_id_range[1] <= last_seg.asym_unit.seq_id_range[1] \
           and type_check(obj, last_seg):
            return
        # Check asym_id
        if asym._id not in self.repr_asym_ids:
            raise ValueError("%s refers to an asym ID (%s) that is not in this "
                 "model's representation (which includes the following asym "
                 "IDs: %s)"
                 % (obj, asym._id,
                    ", ".join(sorted(a for a in self.repr_asym_ids))))
        # Check range
        bad_type_segments = []
        for segment in self.repr_asym_ids[asym._id]:
            rng = segment.asym_unit.seq_id_range
            if seq_id_range[0] >= rng[0] and seq_id_range[1] <= rng[1]:
                if type_check(obj, segment):
                    self._last_repr_segment_matched = segment
                    return
                else:
                    bad_type_segments.append(segment)
        if bad_type_segments:
            raise ValueError("%s does not match the type of any representation "
                    "segment in the seq_id_range (%d-%d) for asym ID %s. "
                    "Representation segments are: %s"
                    % (obj, seq_id_range[0], seq_id_range[1], asym._id,
                       ", ".join(str(s) for s in bad_type_segments)))
        else:
            raise ValueError("%s seq_id range (%d-%d) does not match any range "
                    "in the representation for asym ID %s (representation "
                    "ranges are %s)"
                    % (obj, seq_id_range[0], seq_id_range[1], asym._id,
                       ", ".join("%d-%d" % x.asym_unit.seq_id_range
                                 for x in self.repr_asym_ids[asym._id])))


class _ModelDumper(Dumper):

    def finalize(self, system):
        # Remove any existing ID
        for g in system._all_model_groups(only_in_states=False):
            if hasattr(g, '_id'):
                del g._id
            for m in g:
                if hasattr(m, '_id'):
                    del m._id
        model_id = 1
        # Assign IDs to models and groups in states
        for ng, g in enumerate(system._all_model_groups()):
            g._id = ng + 1
            for m in g:
                if not hasattr(m, '_id'):
                    m._id = model_id
                    model_id += 1
        # Check for any groups not referenced by states
        for g in system._all_model_groups(only_in_states=False):
            if not hasattr(g, '_id'):
                raise ValueError("%s is referenced only by an Ensemble or "
                                 "OrderedProcess. ModelGroups should be "
                                 "stored in State objects." % g)

    def dump(self, system, writer):
        self.dump_model_list(system, writer)
        seen_types = self.dump_atoms(system, writer)
        self.dump_spheres(system, writer)
        self.dump_atom_type(seen_types, system, writer)

    def dump_model_list(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_model_list",
                         ["ordinal_id", "model_id", "model_group_id",
                          "model_name", "model_group_name", "assembly_id",
                          "protocol_id", "representation_id"]) as l:
            for group, model in system._all_models():
                l.write(ordinal_id=ordinal, model_id=model._id,
                        model_group_id=group._id,
                        model_name=model.name,
                        model_group_name=group.name,
                        assembly_id=model.assembly._id,
                        protocol_id=model.protocol._id
                                    if model.protocol else None,
                        representation_id=model.representation._id)
                ordinal += 1

    def dump_atom_type(self, seen_types, system, writer):
        """Output the atom_type table with a list of elements used in atom_site.
           This table is needed by atom_site. Note that we output it *after*
           atom_site (otherwise we would need to iterate through all atoms in
           the system twice)."""
        elements = [x for x in sorted(seen_types.keys()) if x is not None]
        with writer.loop("_atom_type", ["symbol"]) as l:
            for element in elements:
                l.write(symbol=element)

    def dump_atoms(self, system, writer):
        seen_types = {}
        ordinal = 1
        with writer.loop("_atom_site",
                         ["group_PDB", "id", "type_symbol",
                          "label_atom_id", "label_alt_id", "label_comp_id",
                          "label_seq_id",
                          "label_asym_id", "Cartn_x",
                          "Cartn_y", "Cartn_z", "label_entity_id",
                          "auth_asym_id",
                          "B_iso_or_equiv", "pdbx_PDB_model_num",
                          "ihm_model_id"]) as l:
            for group, model in system._all_models():
                rngcheck = _RangeChecker(model)
                for atom in model.get_atoms():
                    rngcheck(atom)
                    comp = atom.asym_unit.entity.sequence[atom.seq_id-1]
                    seen_types[atom.type_symbol] = None
                    l.write(id=ordinal,
                            type_symbol=atom.type_symbol,
                            group_PDB='HETATM' if atom.het else 'ATOM',
                            label_atom_id=atom.atom_id,
                            label_comp_id=comp.id,
                            label_asym_id=atom.asym_unit._id,
                            label_entity_id=atom.asym_unit.entity._id,
                            label_seq_id=atom.seq_id,
                            auth_asym_id=atom.asym_unit._id,
                            Cartn_x=atom.x, Cartn_y=atom.y, Cartn_z=atom.z,
                            B_iso_or_equiv=atom.biso,
                            pdbx_PDB_model_num=model._id,
                            ihm_model_id=model._id)
                    ordinal += 1
        return seen_types

    def dump_spheres(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_sphere_obj_site",
                         ["ordinal_id", "entity_id", "seq_id_begin",
                          "seq_id_end", "asym_id", "Cartn_x",
                          "Cartn_y", "Cartn_z", "object_radius", "rmsf",
                          "model_id"]) as l:
            for group, model in system._all_models():
                rngcheck = _RangeChecker(model)
                for sphere in model.get_spheres():
                    rngcheck(sphere)
                    l.write(ordinal_id=ordinal,
                            entity_id=sphere.asym_unit.entity._id,
                            seq_id_begin=sphere.seq_id_range[0],
                            seq_id_end=sphere.seq_id_range[1],
                            asym_id=sphere.asym_unit._id,
                            Cartn_x=sphere.x, Cartn_y=sphere.y,
                            Cartn_z=sphere.z, object_radius=sphere.radius,
                            rmsf=sphere.rmsf, model_id=model._id)
                    ordinal += 1


class _EnsembleDumper(Dumper):
    def finalize(self, system):
        # Assign IDs
        for ne, e in enumerate(system.ensembles):
            e._id = ne + 1

    def dump(self, system, writer):
        with writer.loop("_ihm_ensemble_info",
                         ["ensemble_id", "ensemble_name", "post_process_id",
                          "model_group_id", "ensemble_clustering_method",
                          "ensemble_clustering_feature",
                          "num_ensemble_models",
                          "num_ensemble_models_deposited",
                          "ensemble_precision_value",
                          "ensemble_file_id"]) as l:
            for e in system.ensembles:
                l.write(ensemble_id=e._id, ensemble_name=e.name,
                        post_process_id=e.post_process._id if e.post_process
                                        else None,
                        model_group_id=e.model_group._id,
                        ensemble_clustering_method=e.clustering_method,
                        ensemble_clustering_feature=e.clustering_feature,
                        num_ensemble_models=e.num_models,
                        num_ensemble_models_deposited=e.num_models_deposited,
                        ensemble_precision_value=e.precision,
                        ensemble_file_id=e.file._id if e.file else None)


class _DensityDumper(Dumper):
    def finalize(self, system):
        # Assign globally unique IDs
        did = 1
        for e in system.ensembles:
            for d in e.densities:
                d._id = did
                did += 1

    def dump(self, system, writer):
        with writer.loop("_ihm_localization_density_files",
                         ["id", "file_id", "ensemble_id", "entity_id",
                          "asym_id", "seq_id_begin", "seq_id_end"]) as l:
            for ensemble in system.ensembles:
                for density in ensemble.densities:
                    l.write(id=density._id, file_id=density.file._id,
                            ensemble_id=ensemble._id,
                            entity_id=density.asym_unit.entity._id,
                            asym_id=density.asym_unit._id,
                            seq_id_begin=density.asym_unit.seq_id_range[0],
                            seq_id_end=density.asym_unit.seq_id_range[1])


class _MultiStateDumper(Dumper):
    def finalize(self, system):
        state_id = 1
        # Assign IDs
        for ng, g in enumerate(system.state_groups):
            g._id = ng + 1
            for state in g:
                state._id = state_id
                state_id += 1

    def dump(self, system, writer):
        # Nothing to do for single state modeling
        if len(system.state_groups) == 1 and len(system.state_groups[0]) <= 1:
            return
        with writer.loop("_ihm_multi_state_modeling",
                         ["ordinal_id", "state_id", "state_group_id",
                          "population_fraction", "state_type", "state_name",
                          "model_group_id", "experiment_type", "details"]) as l:
            ordinal = 1
            for state_group in system.state_groups:
                for state in state_group:
                    for model_group in state:
                        l.write(ordinal_id=ordinal, state_id=state._id,
                                state_group_id=state_group._id,
                                population_fraction=state.population_fraction,
                                model_group_id=model_group._id,
                                state_type=state.type,
                                state_name=state.name,
                                experiment_type=state.experiment_type,
                                details=state.details)
                        ordinal += 1


class _OrderedDumper(Dumper):
    def finalize(self, system):
        for nproc, proc in enumerate(system.ordered_processes):
            proc._id = nproc + 1
            edge_id = 1
            for nstep, step in enumerate(proc.steps):
                step._id = nstep + 1
                for edge in step:
                    edge._id = edge_id
                    edge_id += 1

    def dump(self, system, writer):
        with writer.loop("_ihm_ordered_ensemble",
                         ["process_id", "process_description", "ordered_by",
                          "step_id", "step_description",
                          "edge_id", "edge_description",
                          "model_group_id_begin", "model_group_id_end"]) as l:
            for proc in system.ordered_processes:
                for step in proc.steps:
                    for edge in step:
                        l.write(process_id=proc._id,
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
                         ["id", "xcoord", "ycoord", "zcoord"]) as l:
            for c in self._centers_by_id:
                l.write(id=c._id, xcoord=c.x, ycoord=c.y, zcoord=c.z)

    def dump_transformations(self, writer):
        with writer.loop("_ihm_geometric_object_transformation",
                ["id",
                 "rot_matrix[1][1]", "rot_matrix[2][1]", "rot_matrix[3][1]",
                 "rot_matrix[1][2]", "rot_matrix[2][2]", "rot_matrix[3][2]",
                 "rot_matrix[1][3]", "rot_matrix[2][3]", "rot_matrix[3][3]",
                 "tr_vector[1]", "tr_vector[2]", "tr_vector[3]"]) as l:
            for t in self._transformations_by_id:
                # mmCIF writer usually outputs floats to 3 decimal
                # places, but we need more precision for rotation
                # matrices
                rm = [["%.6f" % e for e in t.rot_matrix[i]]
                      for i in range(3)]
                l.write(id=t._id, rot_matrix11=rm[0][0], rot_matrix21=rm[1][0],
                        rot_matrix31=rm[2][0], rot_matrix12=rm[0][1],
                        rot_matrix22=rm[1][1], rot_matrix32=rm[2][1],
                        rot_matrix13=rm[0][2], rot_matrix23=rm[1][2],
                        rot_matrix33=rm[2][2], tr_vector1=t.tr_vector[0],
                        tr_vector2=t.tr_vector[1], tr_vector3=t.tr_vector[2])

    def dump_generic(self, writer):
        with writer.loop("_ihm_geometric_object_list",
                         ["object_id", "object_type", "object_name",
                          "object_description", "other_details"]) as l:
            for o in self._objects_by_id:
                l.write(object_id=o._id, object_type=o.type, object_name=o.name,
                        object_description=o.description,
                        other_details=o.details)

    def dump_sphere(self, writer):
        with writer.loop("_ihm_geometric_object_sphere",
                         ["object_id", "center_id", "transformation_id",
                          "radius_r"]) as l:
            for o in self._objects_by_id:
                if not isinstance(o, geometry.Sphere):
                    continue
                l.write(object_id=o._id, center_id=o.center._id,
                        transformation_id=o.transformation._id
                                          if o.transformation else None,
                        radius_r=o.radius)

    def dump_torus(self, writer):
        with writer.loop("_ihm_geometric_object_torus",
                         ["object_id", "center_id", "transformation_id",
                          "major_radius_R", "minor_radius_r"]) as l:
            for o in self._objects_by_id:
                if not isinstance(o, (geometry.Torus, geometry.HalfTorus)):
                    continue
                l.write(object_id=o._id, center_id=o.center._id,
                        transformation_id=o.transformation._id
                                          if o.transformation else None,
                        major_radius_R=o.major_radius,
                        minor_radius_r=o.minor_radius)

    def dump_half_torus(self, writer):
        section_map = {True: 'inner half', False: 'outer half'}
        with writer.loop("_ihm_geometric_object_half_torus",
                         ["object_id", "thickness_th", "section"]) as l:
            for o in self._objects_by_id:
                if not isinstance(o, geometry.HalfTorus):
                    continue
                l.write(object_id=o._id, thickness_th=o.thickness,
                        section=section_map.get(o.inner, 'other'))

    def dump_axis(self, writer):
        with writer.loop("_ihm_geometric_object_axis",
                         ["object_id", "axis_type", "transformation_id"]) as l:
            for o in self._objects_by_id:
                if not isinstance(o, geometry.Axis):
                    continue
                l.write(object_id=o._id, axis_type=o.axis_type,
                        transformation_id=o.transformation._id
                                          if o.transformation else None)

    def dump_plane(self, writer):
        with writer.loop("_ihm_geometric_object_plane",
                         ["object_id", "plane_type", "transformation_id"]) as l:
            for o in self._objects_by_id:
                if not isinstance(o, geometry.Plane):
                    continue
                l.write(object_id=o._id, plane_type=o.plane_type,
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
                         ["feature_id", "feature_type", "entity_type"]) as l:
            for f in self._features_by_id:
                l.write(feature_id=f._id, feature_type=f.type,
                        entity_type=f._get_entity_type())

    def dump_poly_residue(self, writer):
        ordinal = 1
        with writer.loop("_ihm_poly_residue_feature",
                         ["ordinal_id", "feature_id", "entity_id", "asym_id",
                          "seq_id_begin", "comp_id_begin", "seq_id_end",
                          "comp_id_end"]) as l:
            for f in self._features_by_id:
                if not isinstance(f, restraint.ResidueFeature):
                    continue
                for r in f.ranges:
                    seq = r.entity.sequence
                    l.write(ordinal_id=ordinal, feature_id=f._id,
                            entity_id=r.entity._id, asym_id=r._id,
                            seq_id_begin=r.seq_id_range[0],
                            comp_id_begin=seq[r.seq_id_range[0]-1].id,
                            seq_id_end=r.seq_id_range[1],
                            comp_id_end=seq[r.seq_id_range[1]-1].id)
                    ordinal += 1

    def dump_poly_atom(self, writer):
        ordinal = 1
        with writer.loop("_ihm_poly_atom_feature",
                         ["ordinal_id", "feature_id", "entity_id", "asym_id",
                          "seq_id", "comp_id", "atom_id"]) as l:
            for f in self._features_by_id:
                if not isinstance(f, restraint.AtomFeature):
                    continue
                for a in f.atoms:
                    r = a.residue
                    if r.asym.entity.is_polymeric():
                        seq = r.asym.entity.sequence
                        l.write(ordinal_id=ordinal, feature_id=f._id,
                                entity_id=r.asym.entity._id, asym_id=r.asym._id,
                                seq_id=r.seq_id, comp_id=seq[r.seq_id-1].id,
                                atom_id=a.id)
                        ordinal += 1

    def dump_non_poly(self, writer):
        ordinal = 1
        with writer.loop("_ihm_non_poly_feature",
                         ["ordinal_id", "feature_id", "entity_id", "asym_id",
                          "comp_id", "atom_id"]) as l:
            for f in self._features_by_id:
                if isinstance(f, restraint.AtomFeature):
                    for a in f.atoms:
                        r = a.residue
                        if not r.asym.entity.is_polymeric():
                            seq = r.asym.entity.sequence
                            l.write(ordinal_id=ordinal, feature_id=f._id,
                                    entity_id=r.asym.entity._id,
                                    asym_id=r.asym._id,
                                    comp_id=seq[r.seq_id-1].id, atom_id=a.id)
                            ordinal += 1
                elif isinstance(f, restraint.NonPolyFeature):
                    _ = f._get_entity_type() # trigger check for poly/nonpoly
                    for a in f.asyms:
                        seq = a.entity.sequence
                        l.write(ordinal_id=ordinal, feature_id=f._id,
                                entity_id=a.entity._id,
                                asym_id=a._id, comp_id=seq[0].id,
                                atom_id=None)
                        ordinal += 1

    def dump_pseudo_site(self, writer):
        with writer.loop("_ihm_pseudo_site_feature",
                         ["feature_id", "Cartn_x", "Cartn_y",
                          "Cartn_z", "radius", "description"]) as l:
            for f in self._features_by_id:
                if not isinstance(f, restraint.PseudoSiteFeature):
                    continue
                l.write(feature_id=f._id, Cartn_x=f.x, Cartn_y=f.y,
                        Cartn_z=f.z, radius=f.radius, description=f.description)


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
        xl_id = 1
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
                        xl._id = xl_id
                        xl_id += 1
                        self._ex_xls_by_id.append((r, xl))
                        seen_cross_links[sig] = xl._id, xl._group_id

    def finalize_modeling(self, system):
        seen_cross_links = {}
        seen_group_ids = {}
        xl_id = 1
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
                    xl._id = xl_id
                    xl_id += 1
                    self._xls_by_id.append((r, xl))
                    seen_cross_links[sig] = xl._id

    def dump(self, system, writer):
        self.dump_list(system, writer)
        self.dump_restraint(system, writer)
        self.dump_results(system, writer)

    def dump_list(self, system, writer):
        with writer.loop("_ihm_cross_link_list",
                         ["id", "group_id", "entity_description_1",
                          "entity_id_1", "seq_id_1", "comp_id_1",
                          "entity_description_2",
                          "entity_id_2", "seq_id_2", "comp_id_2",
                          "linker_descriptor_id", "linker_type",
                          "dataset_list_id"]) as l:
            for r, xl in self._ex_xls_by_id:
                entity1 = xl.residue1.entity
                entity2 = xl.residue2.entity
                seq1 = entity1.sequence
                seq2 = entity2.sequence
                l.write(id=xl._id, group_id=xl._group_id,
                        entity_description_1=entity1.description,
                        entity_id_1=entity1._id,
                        seq_id_1=xl.residue1.seq_id,
                        comp_id_1=seq1[xl.residue1.seq_id-1].id,
                        entity_description_2=entity2.description,
                        entity_id_2=entity2._id,
                        seq_id_2=xl.residue2.seq_id,
                        comp_id_2=seq2[xl.residue2.seq_id-1].id,
                        linker_descriptor_id=r.linker._id,
                        linker_type=r.linker.auth_name,
                        dataset_list_id=r.dataset._id)

    def dump_restraint(self, system, writer):
        with writer.loop("_ihm_cross_link_restraint",
                         ["id", "group_id", "entity_id_1", "asym_id_1",
                          "seq_id_1", "comp_id_1",
                          "entity_id_2", "asym_id_2", "seq_id_2", "comp_id_2",
                          "atom_id_1", "atom_id_2",
                          "restraint_type", "conditional_crosslink_flag",
                          "model_granularity", "distance_threshold",
                          "psi", "sigma_1", "sigma_2"]) as l:
            condmap = {True: 'ALL', False: 'ANY', None: None}
            for r, xl in self._xls_by_id:
                ex_xl = xl.experimental_cross_link
                entity1 = ex_xl.residue1.entity
                entity2 = ex_xl.residue2.entity
                seq1 = entity1.sequence
                seq2 = entity2.sequence
                l.write(id=xl._id, group_id=ex_xl._id,
                        entity_id_1=entity1._id, asym_id_1=xl.asym1._id,
                        seq_id_1=ex_xl.residue1.seq_id,
                        comp_id_1=seq1[ex_xl.residue1.seq_id-1].id,
                        entity_id_2=entity2._id, asym_id_2=xl.asym2._id,
                        seq_id_2=ex_xl.residue2.seq_id,
                        comp_id_2=seq2[ex_xl.residue2.seq_id-1].id,
                        atom_id_1=xl.atom1, atom_id_2=xl.atom2,
                        restraint_type=xl.distance.restraint_type,
                        conditional_crosslink_flag=condmap[xl.restrain_all],
                        model_granularity=xl.granularity,
                        distance_threshold=xl.distance.distance,
                        psi=xl.psi, sigma_1=xl.sigma1, sigma_2=xl.sigma2)

    def dump_results(self, system, writer):
        with writer.loop("_ihm_cross_link_result_parameters",
                         ["ordinal_id", "restraint_id", "model_id",
                          "psi", "sigma_1", "sigma_2"]) as l:
            ordinal = 1
            for r in self._all_restraints(system):
                for xl in r.cross_links:
                    # all fits ordered by model ID
                    for model, fit in sorted(xl.fits.items(),
                                             key=lambda i: i[0]._id):
                        l.write(ordinal_id=ordinal, restraint_id=xl._id,
                                model_id=model._id, psi=fit.psi,
                                sigma_1=fit.sigma1, sigma_2=fit.sigma2)
                        ordinal += 1


class _GeometricRestraintDumper(Dumper):
    def _all_restraints(self, system):
        return [r for r in system._all_restraints()
                if isinstance(r, restraint.GeometricRestraint)]

    def finalize(self, system):
        for nr, r in enumerate(self._all_restraints(system)):
            r._id = nr + 1

    def dump(self, system, writer):
        condmap = {True: 'ALL', False: 'ANY', None: None}
        ordinal = 1
        with writer.loop("_ihm_geometric_object_distance_restraint",
                         ["id", "object_id", "feature_id",
                          "object_characteristic", "restraint_type",
                          "harmonic_force_constant",
                          "distance_lower_limit", "distance_upper_limit",
                          "group_conditionality", "dataset_list_id"]) as l:
            for r in self._all_restraints(system):
                l.write(id=r._id, object_id=r.geometric_object._id,
                        feature_id=r.feature._id,
                        object_characteristic=r.object_characteristic,
                        restraint_type=r.distance.restraint_type,
                        distance_lower_limit=r.distance.distance_lower_limit,
                        distance_upper_limit=r.distance.distance_upper_limit,
                        harmonic_force_constant=r.harmonic_force_constant,
                        group_conditionality=condmap[r.restrain_all],
                        dataset_list_id=r.dataset._id if r.dataset else None)


class _DerivedDistanceRestraintDumper(Dumper):
    def _all_restraints(self, system):
        return [r for r in system._all_restraints()
                if isinstance(r, restraint.DerivedDistanceRestraint)]

    def _all_restraint_groups(self, system):
        return [rg for rg in system.restraint_groups
                if all(isinstance(r, restraint.DerivedDistanceRestraint)
                       for r in rg) and len(rg) > 0]

    def finalize(self, system):
        self._restraints_by_id = []
        seen_restraints = {}
        for r in self._all_restraints(system):
            util._remove_id(r)
        for r in self._all_restraints(system):
            util._assign_id(r, seen_restraints, self._restraints_by_id)

        self._group_for_id = {}
        for nrg, rg in enumerate(self._all_restraint_groups(system)):
            rg._id = nrg + 1
            for r in rg:
                if r._id in self._group_for_id:
                    raise ValueError("%s cannot be in more than one group" % r)
                self._group_for_id[r._id] = rg._id

    def dump(self, system, writer):
        condmap = {True: 'ALL', False: 'ANY', None: None}
        ordinal = 1
        with writer.loop("_ihm_derived_distance_restraint",
                         ["id", "group_id", "feature_id_1", "feature_id_2",
                          "restraint_type", "distance_lower_limit",
                          "distance_upper_limit", "probability",
                          "group_conditionality", "dataset_list_id"]) as l:
            for r in self._restraints_by_id:
                l.write(id=r._id, feature_id_1=r.feature1._id,
                        group_id=self._group_for_id.get(r._id, None),
                        feature_id_2=r.feature2._id,
                        restraint_type=r.distance.restraint_type,
                        distance_lower_limit=r.distance.distance_lower_limit,
                        distance_upper_limit=r.distance.distance_upper_limit,
                        probability=r.probability,
                        group_conditionality=condmap[r.restrain_all],
                        dataset_list_id=r.dataset._id if r.dataset else None)


class _EM3DDumper(Dumper):
    def _all_restraints(self, system):
        return [r for r in system._all_restraints()
                if isinstance(r, restraint.EM3DRestraint)]

    def finalize(self, system):
        for nr, r in enumerate(self._all_restraints(system)):
            r._id = nr + 1

    def dump(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_3dem_restraint",
                         ["ordinal_id", "dataset_list_id", "fitting_method",
                          "fitting_method_citation_id",
                          "struct_assembly_id",
                          "number_of_gaussians", "model_id",
                          "cross_correlation_coefficient"]) as l:
            for r in self._all_restraints(system):
                if r.fitting_method_citation:
                    citation_id = r.fitting_method_citation._id
                else:
                    citation_id = None
                # all fits ordered by model ID
                for model, fit in sorted(r.fits.items(),
                                         key=lambda i: i[0]._id):
                    ccc = fit.cross_correlation_coefficient
                    l.write(ordinal_id=ordinal,
                            dataset_list_id=r.dataset._id,
                            fitting_method=r.fitting_method,
                            fitting_method_citation_id=citation_id,
                            struct_assembly_id=r.assembly._id,
                            number_of_gaussians=r.number_of_gaussians,
                            model_id=model._id,
                            cross_correlation_coefficient=ccc)
                    ordinal += 1


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
                          "details"]) as l:
            for r in self._all_restraints(system):
                l.write(id=r._id, dataset_list_id=r.dataset._id,
                        number_raw_micrographs=r.number_raw_micrographs,
                        pixel_size_width=r.pixel_size_width,
                        pixel_size_height=r.pixel_size_height,
                        image_resolution=r.image_resolution,
                        number_of_projections=r.number_of_projections,
                        struct_assembly_id=r.assembly._id,
                        image_segment_flag=r.segment,
                        details=r.details)

    def dump_fitting(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_2dem_class_average_fitting",
                ["ordinal_id", "restraint_id", "model_id",
                 "cross_correlation_coefficient", "rot_matrix[1][1]",
                 "rot_matrix[2][1]", "rot_matrix[3][1]", "rot_matrix[1][2]",
                 "rot_matrix[2][2]", "rot_matrix[3][2]", "rot_matrix[1][3]",
                 "rot_matrix[2][3]", "rot_matrix[3][3]", "tr_vector[1]",
                 "tr_vector[2]", "tr_vector[3]"]) as l:
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
                    l.write(ordinal_id=ordinal, restraint_id=r._id,
                            model_id=model._id,
                            cross_correlation_coefficient=ccc,
                            rot_matrix11=rm[0][0], rot_matrix21=rm[1][0],
                            rot_matrix31=rm[2][0], rot_matrix12=rm[0][1],
                            rot_matrix22=rm[1][1], rot_matrix32=rm[2][1],
                            rot_matrix13=rm[0][2], rot_matrix23=rm[1][2],
                            rot_matrix33=rm[2][2], tr_vector1=t[0],
                            tr_vector2=t[1], tr_vector3=t[2])
                    ordinal += 1


class _SASDumper(Dumper):
    def _all_restraints(self, system):
        return [r for r in system._all_restraints()
                if isinstance(r, restraint.SASRestraint)]

    def finalize(self, system):
        for nr, r in enumerate(self._all_restraints(system)):
            r._id = nr + 1

    def dump(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_sas_restraint",
                         ["ordinal_id", "dataset_list_id", "model_id",
                          "struct_assembly_id", "profile_segment_flag",
                          "fitting_atom_type", "fitting_method",
                          "fitting_state", "radius_of_gyration",
                          "chi_value", "details"]) as l:
            for r in self._all_restraints(system):
                # all fits ordered by model ID
                for model, fit in sorted(r.fits.items(),
                                         key=lambda i: i[0]._id):
                    l.write(ordinal_id=ordinal,
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
                    ordinal += 1

def _init_restraint_groups(system):
    """Initialize all RestraintGroups by removing any assigned ID"""
    for g in system.restraint_groups:
        util._remove_id(g)

def _check_restraint_groups(system):
    """Check that all RestraintGroups were successfully dumped"""
    for g in system.restraint_groups:
        if len(g) > 0 and not hasattr(g, '_id'):
            raise TypeError(
                "RestraintGroup(%s) contains an unsupported combination of "
                "Restraints. Due to limitations of the underlying dictionary, "
                "all objects in a RestraintGroup must be of the same type, "
                "and only certain types (currently only "
                "DerivedDistanceRestraint) can be grouped." % g)

def write(fh, systems, format='mmCIF', dumpers=[]):
    """Write out all `systems` to the file handle `fh`.
       Files can be written in either the text-based mmCIF format or the
       BinaryCIF format. The BinaryCIF writer needs the msgpack Python
       module to function.

       :param file fh: The file handle to write to.
       :param list systems: The list of :class:`ihm.System` objects to write.
       :param str format: The format of the file. This can be 'mmCIF' (the
              default) for the (text-based) mmCIF format or 'BCIF' for
              BinaryCIF.
       :param list dumpers: A list of :class:`Dumper` classes (not objects).
              These can be used to add extra categories to the file."""
    dumpers = [_EntryDumper(), # must be first
               _StructDumper(), _CommentDumper(),
               _AuditConformDumper(), _SoftwareDumper(),
               _CitationDumper(),
               _AuditAuthorDumper(), _GrantDumper(),
               _ChemCompDumper(), _ChemDescriptorDumper(),
               _EntityDumper(), _EntitySrcGenDumper(), _EntitySrcNatDumper(),
               _EntitySrcSynDumper(), _EntityPolyDumper(),
               _EntityNonPolyDumper(),
               _EntityPolySeqDumper(),
               _StructAsymDumper(),
               _PolySeqSchemeDumper(),
               _NonPolySchemeDumper(),
               _AssemblyDumper(),
               _ExternalReferenceDumper(),
               _DatasetDumper(),
               _ModelRepresentationDumper(),
               _StartingModelDumper(),
               _ProtocolDumper(),
               _PostProcessDumper(),
               _GeometricObjectDumper(), _FeatureDumper(),
               _CrossLinkDumper(), _GeometricRestraintDumper(),
               _DerivedDistanceRestraintDumper(),
               _EM3DDumper(),
               _EM2DDumper(),
               _SASDumper(),
               _ModelDumper(),
               _EnsembleDumper(),
               _DensityDumper(),
               _MultiStateDumper(), _OrderedDumper()] + [d() for d in dumpers]
    writer_map = {'mmCIF': ihm.format.CifWriter,
                  'BCIF': ihm.format_bcif.BinaryCifWriter}

    writer = writer_map[format](fh)
    for system in systems:
        _init_restraint_groups(system)
        for d in dumpers:
            d.finalize(system)
        _check_restraint_groups(system)
        for d in dumpers:
            d.dump(system, writer)
    writer.flush()
