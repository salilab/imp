"""Utility classes to dump out information in mmCIF format"""

import re
import os
import operator
import ihm.format
from . import util
from . import location
from . import restraint

# Standard amino acids, mapping from 1 to 3 letter codes
_amino_acids = {'A':'ALA', 'C':'CYS', 'D':'ASP', 'E':'GLU', 'F':'PHE',
                'G':'GLY', 'H':'HIS', 'I':'ILE', 'K':'LYS', 'L':'LEU',
                'M':'MET', 'N':'ASN', 'P':'PRO', 'Q':'GLN', 'R':'ARG',
                'S':'SER', 'T':'THR', 'V':'VAL', 'W':'TRP', 'Y':'TYR'}

class _Dumper(object):
    """Base class for helpers to dump output to mmCIF"""
    def __init__(self):
        pass
    def finalize(self, system):
        """Called for all dumpers prior to `dump` - can assign IDs, etc"""
        pass
    def dump(self, system, writer):
        """Use `writer` to write information about `system` to mmCIF"""
        pass


class _EntryDumper(_Dumper):
    def dump(self, system, writer):
        # Write CIF header (so this dumper should always be first)
        writer.fh.write("data_%s\n" % re.subn('[^0-9a-zA-z_]', '',
                                              system.name)[0])
        with writer.category("_entry") as l:
            l.write(id=system.name)


class _SoftwareDumper(_Dumper):
    def dump(self, system, writer):
        ordinal = 1
        seen_software = {}
        # todo: specify these attributes in only one place (e.g. in the Software
        # class)
        with writer.loop("_software",
                         ["pdbx_ordinal", "name", "classification",
                          "description", "version", "type", "location"]) as l:
            for s in system.software:
                # Remove duplicates
                if s in seen_software:
                    continue
                seen_software[s] = None
                l.write(pdbx_ordinal=ordinal, name=s.name,
                        classification=s.classification,
                        description=s.description, version=s.version,
                        type=s.type, location=s.location)
                ordinal += 1


class _CitationDumper(_Dumper):
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


class _AuditAuthorDumper(_Dumper):
    """Populate the mmCIF audit_author category (a list of the people that
       authored this mmCIF file; here we assume that's just the authors of
       any associated publications)"""

    def dump(self, system, writer):
        seen_authors = {}
        with writer.loop("_audit_author",
                         ["name", "pdbx_ordinal"]) as l:
            ordinal = 1
            for c in system._all_citations():
                for a in c.authors:
                    if a not in seen_authors:
                        seen_authors[a] = None
                        l.write(name=a, pdbx_ordinal=ordinal)
                        ordinal += 1


class _ChemCompDumper(_Dumper):
    def dump(self, system, writer):
        seen = {}

        with writer.loop("_chem_comp", ["id", "type"]) as l:
            for entity in system.entities:
                seq = entity.sequence
                for num, one_letter_code in enumerate(seq):
                    resid = _amino_acids[one_letter_code]
                    if resid not in seen:
                        seen[resid] = None
                        l.write(id=resid, type='L-peptide linking')


class _EntityDumper(_Dumper):
    # todo: we currently only support amino acid sequences here (and
    # then only standard amino acids; need to add support for MSE etc.)

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


class _EntityPolyDumper(_Dumper):
    # todo: we currently only support amino acid sequences here
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
                seq = entity.sequence
                # Split into lines to get tidier CIF output
                seq = "\n".join(seq[i:i+70] for i in range(0, len(seq), 70))
                l.write(entity_id=entity._id, type='polypeptide(L)',
                        nstd_linkage='no', nstd_monomer='no',
                        pdbx_strand_id=strand.get(entity._id, None),
                        pdbx_seq_one_letter_code=seq,
                        pdbx_seq_one_letter_code_can=seq)


class _EntityPolySeqDumper(_Dumper):
    def dump(self, system, writer):
        with writer.loop("_entity_poly_seq",
                         ["entity_id", "num", "mon_id", "hetero"]) as l:
            for entity in system.entities:
                seq = entity.sequence
                for num, one_letter_code in enumerate(seq):
                    resid = _amino_acids[one_letter_code]
                    l.write(entity_id=entity._id, num=num + 1, mon_id=resid)


class _StructAsymDumper(_Dumper):
    def finalize(self, system):
        ordinal = 1
        # Assign asym IDs
        for asym, asym_id in zip(system.asym_units, util._AsymIDs()):
            asym.ordinal = ordinal
            asym._id = asym_id
            ordinal += 1

    def dump(self, system, writer):
        with writer.loop("_struct_asym",
                         ["id", "entity_id", "details"]) as l:
            for asym in system.asym_units:
                l.write(id=asym._id, entity_id=asym.entity._id,
                        details=asym.details)


class _AssemblyDumper(_Dumper):
    def finalize(self, system):
        # Fill in complete assembly
        system._make_complete_assembly()

        # Sort each assembly by entity/asym id
        def component_key(comp):
            return (comp.entity._id, comp.asym.ordinal if comp.asym else 0)
        for a in system._all_assemblies():
            a.sort(key=component_key)

        seen_assemblies = {}
        # Assign IDs to all assemblies; duplicate assemblies get same ID
        self._assembly_by_id = []
        for a in system._all_assemblies():
            # list isn't hashable but tuple is
            hasha = tuple(a)
            if hasha not in seen_assemblies:
                self._assembly_by_id.append(a)
                seen_assemblies[hasha] = a._id = len(self._assembly_by_id)
            else:
                a._id = seen_assemblies[hasha]

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
                    entity = comp.entity
                    seqrange = comp.seq_id_range
                    l.write(ordinal_id=ordinal, assembly_id=a._id,
                            # if no hierarchy then assembly is self-parent
                            parent_assembly_id=a.parent._id if a.parent
                                               else a._id,
                            entity_description=entity.description,
                            entity_id=entity._id,
                            asym_id=comp.asym._id if comp.asym
                                                  else writer.omitted,
                            seq_id_begin=seqrange[0],
                            seq_id_end=seqrange[1])
                    ordinal += 1

class _ExternalReferenceDumper(_Dumper):
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


class _DatasetDumper(_Dumper):
    def finalize(self, system):
        seen_datasets = {}
        # Assign IDs to all datasets
        self._dataset_by_id = []
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

class _ModelRepresentationDumper(_Dumper):
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


class _StartingModelDumper(_Dumper):
    def finalize(self, system):
        # Assign IDs to starting models
        for nm, m in enumerate(system.starting_models):
            m._id = nm + 1

    def dump(self, system, writer):
        self.dump_details(system, writer)
        self.dump_comparative(system, writer)
        # todo: handle seq_id, coords

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
             for sm in system.starting_models:
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
            for sm in system.starting_models:
                for template in sm.templates:
                    denom = template.sequence_identity_denominator
                    l.write(ordinal_id=ordinal,
                      starting_model_id=sm._id,
                      starting_model_auth_asym_id=sm.asym_id,
                      starting_model_seq_id_begin=template.seq_id_range[0],
                      starting_model_seq_id_end=template.seq_id_range[1],
                      template_auth_asym_id=template.asym_id,
                      template_seq_id_begin=template.template_seq_id_range[0],
                      template_seq_id_end=template.template_seq_id_range[1],
                      template_sequence_identity=template.sequence_identity,
                      template_sequence_identity_denominator=denom,
                      template_dataset_list_id=template.dataset._id
                                               if template.dataset else None,
                      alignment_file_id=template.alignment_file._id
                                        if template.alignment_file else None)
                    ordinal += 1


class _ProtocolDumper(_Dumper):
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
                          "multi_state_flag", "time_ordered_flag"]) as l:
            for p in system._all_protocols():
                for s in p.steps:
                    l.write(ordinal_id=ordinal, protocol_id=p._id,
                            step_id=s._id,
                            struct_assembly_id=s.assembly._id,
                            dataset_group_id=s.dataset_group._id,
                            struct_assembly_description=s.assembly.description,
                            protocol_name=p.name,
                            step_name=s.name, step_method=s.method,
                            num_models_begin=s.num_models_begin,
                            num_models_end=s.num_models_end,
                            multi_state_flag=s.multi_state,
                            time_ordered_flag=s.ordered,
                            multi_scale_flag=s.multi_scale)
                    ordinal += 1


class _PostProcessDumper(_Dumper):
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
                          "num_models_end"]) as l:
            for p in system._all_protocols():
                for a in p.analyses:
                    for s in a.steps:
                        l.write(id=s._post_proc_id, protocol_id=p._id,
                                analysis_id=a._id, step_id=s._id, type=s.type,
                                feature=s.feature,
                                num_models_begin=s.num_models_begin,
                                num_models_end=s.num_models_end)

class _ModelDumper(object):

    def finalize(self, system):
        # Remove any existing ID
        for g in system._all_model_groups():
            for m in g:
                if hasattr(m, '_id'):
                    del m._id
        model_id = 1
        # Assign IDs to models and groups
        for ng, g in enumerate(system._all_model_groups()):
            g._id = ng + 1
            for m in g:
                if not hasattr(m, '_id'):
                    m._id = model_id
                    model_id += 1

    def dump(self, system, writer):
        self.dump_model_list(system, writer)
        self.dump_atoms(system, writer)
        self.dump_spheres(system, writer)

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
                        protocol_id=model.protocol._id,
                        representation_id=model.representation._id)
                ordinal += 1

    def dump_atoms(self, system, writer):
        ordinal = 1
        with writer.loop("_atom_site",
                         ["id", "label_atom_id", "label_comp_id",
                          "label_seq_id",
                          "label_asym_id", "Cartn_x",
                          "Cartn_y", "Cartn_z", "label_entity_id",
                          "model_id"]) as l:
            for group, model in system._all_models():
                for atom in model.get_atoms():
                    oneletter = atom.asym_unit.entity.sequence[atom.seq_id-1]
                    l.write(id=ordinal,
                            label_atom_id=atom.atom_id,
                            label_comp_id=_amino_acids[oneletter],
                            label_asym_id=atom.asym_unit._id,
                            label_entity_id=atom.asym_unit.entity._id,
                            label_seq_id=atom.seq_id,
                            Cartn_x=atom.x, Cartn_y=atom.y, Cartn_z=atom.z,
                            model_id=model._id)
                    ordinal += 1

    def dump_spheres(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_sphere_obj_site",
                         ["ordinal_id", "entity_id", "seq_id_begin",
                          "seq_id_end", "asym_id", "Cartn_x",
                          "Cartn_y", "Cartn_z", "object_radius", "rmsf",
                          "model_id"]) as l:
            for group, model in system._all_models():
                for sphere in model.get_spheres():
                    l.write(ordinal_id=ordinal,
                            entity_id=sphere.asym_unit.entity._id,
                            seq_id_begin=sphere.seq_id_range[0],
                            seq_id_end=sphere.seq_id_range[1],
                            asym_id=sphere.asym_unit._id,
                            Cartn_x=sphere.x, Cartn_y=sphere.y,
                            Cartn_z=sphere.z, object_radius=sphere.radius,
                            rmsf=sphere.rmsf, model_id=model._id)
                    ordinal += 1


class _EnsembleDumper(object):
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


class _DensityDumper(object):
    def finalize(self, system):
        # Assign IDs
        for e in system.ensembles:
            for nd, d in enumerate(e.densities):
                d._id = nd + 1

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


class _MultiStateDumper(object):
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

class _EM3DDumper(_Dumper):
    def _all_restraints(self, system):
        return [r for r in system.restraints
                if isinstance(r, restraint.EM3DRestraint)]

    def finalize(self, system):
        for nr, r in enumerate(self._all_restraints(system)):
            r._id = nr + 1

    def dump(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_3dem_restraint",
                         ["ordinal_id", "dataset_list_id", "fitting_method",
                          "fitting_method_citation",
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
                            fitting_method_citation=citation_id,
                            struct_assembly_id=r.assembly._id,
                            number_of_gaussians=r.number_of_gaussians,
                            model_id=model._id,
                            cross_correlation_coefficient=ccc)
                    ordinal += 1


class _EM2DDumper(_Dumper):
    def _all_restraints(self, system):
        return [r for r in system.restraints
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


class _SASDumper(_Dumper):
    def _all_restraints(self, system):
        return [r for r in system.restraints
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


def write(fh, systems):
    """Write out all `systems` to the mmCIF file handle `fh`"""
    dumpers = [_EntryDumper(), # must be first
               _SoftwareDumper(),
               _CitationDumper(),
               _AuditAuthorDumper(),
               _ChemCompDumper(),
               _EntityDumper(),
               _EntityPolyDumper(),
               _EntityPolySeqDumper(),
               _StructAsymDumper(),
               _AssemblyDumper(),
               _ExternalReferenceDumper(),
               _DatasetDumper(),
               _ModelRepresentationDumper(),
               _StartingModelDumper(),
               _ProtocolDumper(),
               _PostProcessDumper(),
               _EM3DDumper(),
               _EM2DDumper(),
               _SASDumper(),
               _ModelDumper(),
               _EnsembleDumper(),
               _DensityDumper(),
               _MultiStateDumper()]
    writer = ihm.format.CifWriter(fh)
    for system in systems:
        for d in dumpers:
            d.finalize(system)
        for d in dumpers:
            d.dump(system, writer)
