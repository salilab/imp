"""@namespace IMP.mmcif.dumper
   @brief Utility classes to dump out information in mmCIF format.
"""

from __future__ import print_function
import IMP.atom
import IMP.mmcif.data
import IMP.mmcif.dataset
from IMP.mmcif.format import _CifWriter
import operator
import os

class _Dumper(object):
    """Base class for helpers to dump output to mmCIF"""
    def __init__(self):
        pass
    def finalize_metadata(self, system):
        pass
    def finalize(self, system):
        pass

class _EntryDumper(_Dumper):
    def dump(self, system, writer):
        entry_id = 'imp_model'
        # Write CIF header (so this dumper should always be first)
        writer.fh.write("data_%s\n" % entry_id)
        with writer.category("_entry") as l:
            l.write(id=entry_id)

class _ChemCompDumper(_Dumper):
    def dump(self, system, writer):
        seen = {}
        std = dict.fromkeys(('ALA', 'CYS', 'ASP', 'GLU', 'PHE', 'GLY', 'HIS',
               'ILE', 'LYS', 'LEU', 'MET', 'ASN', 'PRO', 'GLN', 'ARG', 'SER',
               'THR', 'VAL', 'TRP', 'TYR'))
        with writer.loop("_chem_comp", ["id", "type"]) as l:
            for entity in system.entities.get_all():
                seq = entity.sequence
                for num, one_letter_code in enumerate(seq):
                    restyp = IMP.atom.get_residue_type(one_letter_code)
                    resid = restyp.get_string()
                    if resid not in seen:
                        seen[resid] = None
                        l.write(id=resid,
                                type='L-peptide linking' if resid in std \
                                                         else 'other')


class _EntityDumper(_Dumper):
    # todo: we currently only support amino acid sequences here (and
    # then only standard amino acids; need to add support for MSE etc.)
    def dump(self, system, writer):
        with writer.loop("_entity",
                         ["id", "type", "src_method", "pdbx_description",
                          "formula_weight", "pdbx_number_of_molecules",
                          "details"]) as l:
            for entity in system.entities.get_all():
                l.write(id=entity.id, type='polymer', src_method='man',
                        pdbx_description=entity.description,
                        formula_weight=writer.unknown,
                        pdbx_number_of_molecules=1, details=writer.unknown)


class _EntityPolyDumper(_Dumper):
    # todo: we currently only support amino acid sequences here
    def dump(self, system, writer):
        with writer.loop("_entity_poly",
                         ["entity_id", "type", "nstd_linkage",
                          "nstd_monomer", "pdbx_strand_id",
                          "pdbx_seq_one_letter_code",
                          "pdbx_seq_one_letter_code_can"]) as l:
            for entity in system.entities.get_all():
                seq = entity.sequence
                # Split into lines to get tidier CIF output
                seq = "\n".join(seq[i:i+70] for i in range(0, len(seq), 70))
                comp = entity.first_component
                l.write(entity_id=entity.id, type='polypeptide(L)',
                        nstd_linkage='no', nstd_monomer='no',
                        pdbx_strand_id=comp.asym_id,
                        pdbx_seq_one_letter_code=seq,
                        pdbx_seq_one_letter_code_can=seq)

class _EntityPolySeqDumper(_Dumper):
    def dump(self, system, writer):
        with writer.loop("_entity_poly_seq",
                         ["entity_id", "num", "mon_id", "hetero"]) as l:
            for entity in system.entities.get_all():
                seq = entity.sequence
                for num, one_letter_code in enumerate(seq):
                    restyp = IMP.atom.get_residue_type(one_letter_code)
                    l.write(entity_id=entity.id, num=num + 1,
                            mon_id=restyp.get_string(),
                            hetero=writer.omitted)


class _StructAsymDumper(_Dumper):
    def dump(self, system, writer):
        with writer.loop("_struct_asym",
                         ["id", "entity_id", "details"]) as l:
            for comp in system.components.get_all_modeled():
                l.write(id=comp.asym_id,
                        entity_id=comp.entity.id,
                        details=comp.name if comp.name else writer.omitted)


class _AssemblyDumper(_Dumper):
    def __init__(self):
        super(_AssemblyDumper, self).__init__()
        self.assemblies = []

    def add(self, a):
        """Add a new assembly. The first such assembly is assumed to contain
           all components. Duplicate assemblies will be pruned at the end."""
        self.assemblies.append(a)
        return a

    def get_subassembly(self, compdict):
        """Get an _Assembly consisting of the given components."""
        # Put components in creation order
        newa = IMP.mmcif.data._Assembly(c for c in self.assemblies[0]
                                        if c in compdict)
        return self.add(newa)

    def finalize(self, system):
        seen_assemblies = {}
        # Assign IDs to all assemblies
        self._assembly_by_id = []
        for a in self.assemblies:
            IMP.mmcif.data._assign_id(a, seen_assemblies, self._assembly_by_id)

    def dump(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_struct_assembly",
                         ["ordinal_id", "assembly_id", "parent_assembly_id",
                          "entity_description",
                          "entity_id", "asym_id", "seq_id_begin",
                          "seq_id_end"]) as l:
            for a in self._assembly_by_id:
                for comp in a:
                    entity = comp.entity
                    l.write(ordinal_id=ordinal, assembly_id=a.id,
                            # Currently all assemblies are not hierarchical,
                            # so each assembly is a self-parent
                            parent_assembly_id=a.id,
                            entity_description=entity.description,
                            entity_id=entity.id,
                            asym_id=comp.asym_id if comp.asym_id
                                                 else writer.omitted,
                            seq_id_begin=1,
                            seq_id_end=len(entity.sequence))
                    ordinal += 1


class _ModelRepresentationDumper(_Dumper):
    def _get_granularity(self, r):
        if isinstance(r.particles[0], IMP.atom.Residue):
            return 'by-residue'
        else:
            return 'by-feature'

    def dump(self, system, writer):
        ordinal_id = 1
        segment_id = 1
        with writer.loop("_ihm_model_representation",
                         ["ordinal_id", "representation_id",
                          "segment_id", "entity_id", "entity_description",
                          "entity_asym_id",
                          "seq_id_begin", "seq_id_end",
                          "model_object_primitive", "starting_model_id",
                          "model_mode", "model_granularity",
                          "model_object_count"]) as l:
            for comp in system.components.get_all_modeled():
                # For now, assume that representation of the same-named
                # component is the same in all states, so just take the first
                state = list(system._states.keys())[0]
                for r in state.representation[comp]:
                    # todo: handle multiple representations
                    l.write(ordinal_id=ordinal_id, representation_id=1,
                            segment_id=segment_id, entity_id=comp.entity.id,
                            entity_description=comp.entity.description,
                            entity_asym_id=comp.asym_id,
                            seq_id_begin=r.residue_range[0],
                            seq_id_end=r.residue_range[1],
                            model_object_primitive=r.primitive,
                            starting_model_id=r.starting_model.id
                                                  if r.starting_model
                                                  else writer.omitted,
                            model_mode='rigid' if r.rigid_body else 'flexible',
                            model_granularity=self._get_granularity(r),
                            model_object_count=len(r.particles))
                    ordinal_id += 1
                    segment_id += 1

class _MSESeqDif(object):
    """Track an MSE -> MET mutation in the starting model sequence"""
    comp_id = 'MET'
    db_comp_id = 'MSE'
    details = 'Conversion of modified residue MSE to MET'
    def __init__(self, res, comp, source, model):
        self.res, self.comp, self.source, self.model = res, comp, source, model
        self.model = model


class _StartingModelDumper(_Dumper):
    def finalize(self, system):
        for comp in system.components.get_all_modeled():
            for n, starting_model in enumerate(self._all_models(system, comp)):
                starting_model.id = '%s-m%d' % (comp.name, n + 1)

    def _all_models(self, system, comp):
        """Get the set of all starting models for a component"""
        for state in system._states.keys():
            seen_models = {}
            for rep in state.representation.get(comp, []):
                sm = rep.starting_model
                if sm and sm not in seen_models:
                    seen_models[sm] = None
                    yield sm

    def dump(self, system, writer):
        self.dump_details(system, writer)
        self.dump_comparative(system, writer)
        seq_dif = self.dump_coords(system, writer)
        self.dump_seq_dif(writer, seq_dif)

    def dump_details(self, system, writer):
        with writer.loop("_ihm_starting_model_details",
                     ["starting_model_id", "entity_id", "entity_description",
                      "asym_id", "seq_id_begin",
                      "seq_id_end", "starting_model_source",
                      "starting_model_auth_asym_id",
                      "starting_model_sequence_offset",
                      "dataset_list_id"]) as l:
            for comp in system.components.get_all_modeled():
                for sm in self._all_models(system, comp):
                    seq_id_begin, seq_id_end = sm.get_seq_id_range_all_sources()
                    l.write(starting_model_id=sm.id, entity_id=comp.entity.id,
                            entity_description=comp.entity.description,
                            asym_id=comp.asym_id,
                            seq_id_begin=seq_id_begin,
                            seq_id_end=seq_id_end,
                            starting_model_source=sm.sources[0].source,
                            starting_model_auth_asym_id=sm.chain_id,
                            dataset_list_id=sm.dataset.id,
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
            for comp in system.components.get_all_modeled():
                for sm in self._all_models(system, comp):
                    for template in [s for s in sm.sources
                          if isinstance(s, IMP.mmcif.metadata._TemplateSource)]:
                        seq_id_begin, seq_id_end = template.get_seq_id_range(sm)
                        l.write(ordinal_id=ordinal,
                          starting_model_id=sm.id,
                          starting_model_auth_asym_id=template.chain_id,
                          starting_model_seq_id_begin=seq_id_begin,
                          starting_model_seq_id_end=seq_id_end,
                          template_auth_asym_id=template.tm_chain_id,
                          template_seq_id_begin=template.tm_seq_id_begin,
                          template_seq_id_end=template.tm_seq_id_end,
                          template_sequence_identity=template.sequence_identity,
                          # Assume Modeller-style sequence identity for now
                          template_sequence_identity_denominator=1,
                          template_dataset_list_id=template.tm_dataset.id
                                                   if template.tm_dataset
                                                   else writer.unknown,
                          alignment_file_id=sm.alignment_file.id
                                            if hasattr(sm, 'alignment_file')
                                            else writer.unknown)
                        ordinal += 1

    def dump_coords(self, system, writer):
        seq_dif = []
        ordinal = 1
        with writer.loop("_ihm_starting_model_coord",
                     ["starting_model_id", "group_PDB", "id", "type_symbol",
                      "atom_id", "comp_id", "entity_id", "asym_id",
                      "seq_id", "Cartn_x",
                      "Cartn_y", "Cartn_z", "B_iso_or_equiv",
                      "ordinal_id"]) as l:
            for comp in system.components.get_all_modeled():
                for sm in self._all_models(system, comp):
                    m, sel = self._read_coords(sm)
                    last_res_index = None
                    for a in sel.get_selected_particles():
                        coord = IMP.core.XYZ(a).get_coordinates()
                        atom = IMP.atom.Atom(a)
                        element = atom.get_element()
                        element = IMP.atom.get_element_table().get_name(element)
                        atom_name = atom.get_atom_type().get_string().strip()
                        group_pdb = 'ATOM'
                        if atom_name.startswith('HET:'):
                            group_pdb = 'HETATM'
                            atom_name = atom_name[4:]
                        res = IMP.atom.get_residue(atom)
                        res_name = res.get_residue_type().get_string()
                        # MSE in the original PDB is automatically mutated
                        # by IMP to MET, so reflect that in the output,
                        # and pass back to populate the seq_dif category.
                        if res_name == 'MSE':
                            res_name = 'MET'
                            # Only add one seq_dif record per residue
                            ind = res.get_index()
                            if ind != last_res_index:
                                last_res_index = ind
                                # This should only happen when we're using
                                # a crystal structure as the source (a
                                # comparative model would use MET in
                                # the sequence)
                                assert(len(sm.sources) == 1)
                                seq_dif.append(_MSESeqDif(res, comp,
                                                          sm.sources[0], sm))
                        l.write(starting_model_id=sm.id,
                                group_PDB=group_pdb,
                                id=atom.get_input_index(), type_symbol=element,
                                atom_id=atom_name, comp_id=res_name,
                                entity_id=comp.entity.id,
                                asym_id=sm.chain_id,
                                seq_id=res.get_index() + sm.offset,
                                Cartn_x=coord[0],
                                Cartn_y=coord[1], Cartn_z=coord[2],
                                B_iso_or_equiv=atom.get_temperature_factor(),
                                ordinal_id=ordinal)
                        ordinal += 1
        return seq_dif

    def dump_seq_dif(self, writer, seq_dif):
        ordinal = 1
        with writer.loop("_ihm_starting_model_seq_dif",
                     ["ordinal_id", "entity_id", "asym_id",
                      "seq_id", "comp_id", "starting_model_id",
                      "db_asym_id", "db_seq_id", "db_comp_id",
                      "details"]) as l:
            for sd in seq_dif:
                l.write(ordinal_id=ordinal, entity_id=sd.comp.entity.id,
                        asym_id=sd.model.chain_id,
                        seq_id=sd.res.get_index(),
                        comp_id=sd.comp_id,
                        db_asym_id=sd.source.chain_id,
                        db_seq_id=sd.res.get_index() - sd.model.offset,
                        db_comp_id=sd.db_comp_id,
                        starting_model_id=sd.model.name,
                        details=sd.details)
                ordinal += 1

    def _read_coords(self, sm):
        """Read the coordinates for a starting model"""
        m = IMP.Model()
        # todo: support reading other subsets of the atoms (e.g. CA/CB)
        slt = IMP.atom.ChainPDBSelector(sm.chain_id) \
                   & IMP.atom.NonWaterNonHydrogenPDBSelector()
        hier = IMP.atom.read_pdb(sm.filename, m, slt)
        sel = IMP.atom.Selection(hier,
                     residue_indexes=list(range(sm.seq_id_begin - sm.offset,
                                                sm.seq_id_end + 1 - sm.offset)))
        return m, sel


class _DatasetDumper(_Dumper):
    def _dataset_by_id(self, system):
        return sorted(system.datasets.get_all(), key=operator.attrgetter('id'))

    def dump(self, system, writer):
        with writer.loop("_ihm_dataset_list",
                         ["id", "data_type", "database_hosted"]) as l:
            for d in self._dataset_by_id(system):
                l.write(id=d.id, data_type=d._data_type,
                        database_hosted=isinstance(d.location,
                                        IMP.mmcif.dataset.DatabaseLocation))
        self.dump_rel_dbs((d for d in self._dataset_by_id(system)
                           if isinstance(d.location,
                                         IMP.mmcif.dataset.DatabaseLocation)),
                          writer)
        self.dump_related(system, writer)

    def dump_rel_dbs(self, datasets, writer):
        ordinal = 1
        with writer.loop("_ihm_dataset_related_db_reference",
                         ["id", "dataset_list_id", "db_name",
                          "accession_code", "version", "details"]) as l:
            for d in datasets:
                l.write(id=ordinal, dataset_list_id=d.id,
                        db_name=d.location.db_name,
                        accession_code=d.location.access_code,
                        version=d.location.version if d.location.version
                                else writer.omitted,
                        details=d.location.details if d.location.details
                                else writer.omitted)
                ordinal += 1

    def dump_related(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_related_datasets",
                         ["ordinal_id", "dataset_list_id_derived",
                          "dataset_list_id_primary"]) as l:
            for derived in self._dataset_by_id(system):
                for parent in sorted(derived._parents.keys(),
                                     key=operator.attrgetter('id')):
                    l.write(ordinal_id=ordinal,
                            dataset_list_id_derived=derived.id,
                            dataset_list_id_primary=parent.id)
                    ordinal += 1


class _CitationDumper(_Dumper):
    def dump(self, system, writer):
        self.dump_citation(system, writer)
        self.dump_author(system, writer)

    def dump_citation(self, system, writer):
        with writer.loop("_citation",
                         ["id", "title", "journal_abbrev", "journal_volume",
                          "page_first", "page_last", "year",
                          "pdbx_database_id_PubMed",
                          "pdbx_database_id_DOI"]) as l:
            for n, c in enumerate(system._citations):
                if isinstance(c.page_range, (tuple, list)):
                    page_first, page_last = c.page_range
                else:
                    page_first = c.page_range
                    page_last = writer.omitted
                l.write(id=n+1, title=c.title, journal_abbrev=c.journal,
                        journal_volume=c.volume, page_first=page_first,
                        page_last=page_last, year=c.year,
                        pdbx_database_id_PubMed=c.pmid,
                        pdbx_database_id_DOI=c.doi)

    def dump_author(self, system, writer):
        with writer.loop("_citation_author",
                         ["citation_id", "name", "ordinal"]) as l:
            ordinal = 1
            for n, c in enumerate(system._citations):
                for a in c.authors:
                    l.write(citation_id=n+1, name=a, ordinal=ordinal)
                    ordinal += 1


class _SoftwareDumper(_Dumper):
    def dump(self, system, writer):
        ordinal = 1
        with writer.loop("_software",
                         ["pdbx_ordinal", "name", "classification", "version",
                          "type", "location"]) as l:
            for s in system._software:
                l.write(pdbx_ordinal=ordinal, name=s.name,
                        classification=s.classification, version=s.version,
                        type=s.type, location=s.url)
                ordinal += 1


class _ExternalReferenceDumper(_Dumper):
    """Output information on externally referenced files
       (i.e. anything that refers to a Location that isn't
       a DatabaseLocation)."""

    class _LocalFiles(object):
        reference_provider = _CifWriter.omitted
        reference_type = 'Supplementary Files'
        reference = _CifWriter.omitted
        refers_to = 'Other'
        associated_url = _CifWriter.omitted

        def __init__(self, top_directory):
            self.top_directory = top_directory

        def _get_full_path(self, path):
            return os.path.relpath(path, start=self.top_directory)

    class _Repository(object):
        reference_provider = _CifWriter.omitted
        reference_type = 'DOI'
        refers_to = 'Other'
        associated_url = _CifWriter.omitted

        def __init__(self, repo):
            self.id = repo.id
            self.reference = repo.doi
            if 'zenodo' in self.reference:
                self.reference_provider = 'Zenodo'
            if repo.url:
                self.associated_url = repo.url
                if repo.url.endswith(".zip"):
                    self.refers_to = 'Archive'
                else:
                    self.refers_to = 'File'

    def finalize(self, system):
        # Keep only locations that don't point into databases (these are
        # handled elsewhere)
        self._refs = list(system._external_files.get_all_nondb())
        # Assign IDs to all locations and repos (including the None repo, which
        # is for local files)
        seen_refs = {}
        seen_repos = {}
        self._ref_by_id = []
        self._repo_by_id = []
        # Special dummy repo for repo=None (local files)
        self._local_files = self._LocalFiles(os.getcwd())
        for r in self._refs:
            # Update location to point to parent repository, if any
            # todo: this could probably happen when the location is first made
            system._update_location(r.location)
            # Assign a unique ID to the reference
            IMP.mmcif.data._assign_id(r, seen_refs, self._ref_by_id)
            # Assign a unique ID to the repository
            IMP.mmcif.data._assign_id(r.location.repo or self._local_files,
                                      seen_repos, self._repo_by_id)

    def dump(self, system, writer):
        self.dump_repos(writer)
        self.dump_refs(writer)

    def dump_repos(self, writer):
        def map_repo(r):
            return r if isinstance(r, self._LocalFiles) else self._Repository(r)
        with writer.loop("_ihm_external_reference_info",
                         ["reference_id", "reference_provider",
                          "reference_type", "reference", "refers_to",
                          "associated_url"]) as l:
            for repo in [map_repo(r) for r in self._repo_by_id]:
                l.write(reference_id=repo.id,
                        reference_provider=repo.reference_provider,
                        reference_type=repo.reference_type,
                        reference=repo.reference, refers_to=repo.refers_to,
                        associated_url=repo.associated_url)

    def dump_refs(self, writer):
        with writer.loop("_ihm_external_files",
                         ["id", "reference_id", "file_path", "content_type",
                          "file_size_bytes", "details"]) as l:
            for r in self._ref_by_id:
                loc = r.location
                repo = loc.repo or self._local_files
                file_path=self._posix_path(repo._get_full_path(loc.path))
                if r.file_size is None:
                    file_size = writer.omitted
                else:
                    file_size = r.file_size
                l.write(id=loc.id, reference_id=repo.id,
                        file_path=file_path,
                        content_type=r.content_type,
                        file_size_bytes=file_size,
                        details=loc.details or writer.omitted)

    # On Windows systems, convert native paths to POSIX-like (/-separated) paths
    if os.sep == '/':
        def _posix_path(self, path):
            return path
    else:
        def _posix_path(self, path):
            return path.replace(os.sep, '/')
