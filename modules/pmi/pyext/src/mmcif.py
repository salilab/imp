"""@namespace IMP.pmi.mmcif
   @brief Support for the mmCIF file format.
"""

from __future__ import print_function
import copy
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.em
import IMP.isd
import IMP.pmi.representation
import IMP.pmi.tools
from IMP.pmi.tools import OrderedDict
import IMP.pmi.output
import IMP.pmi.metadata
import re
import sys
import os
import textwrap
import weakref

class _LineWriter(object):
    def __init__(self, writer, line_len=80, multi_line_len=70):
        self.writer = writer
        self.line_len = line_len
        self.multi_line_len = multi_line_len
        self.column = 0
    def write(self, val):
        if isinstance(val, str) and len(val) > self.multi_line_len:
            self.writer.fh.write("\n;")
            for i in range(0, len(val), self.multi_line_len):
                self.writer.fh.write(val[i:i+self.multi_line_len])
                self.writer.fh.write("\n")
            self.writer.fh.write(";\n")
            self.column = 0
            return
        val = self.writer._repr(val)
        if self.column > 0:
            if self.column + len(val) + 1 > self.line_len:
                self.writer.fh.write("\n")
                self.column = 0
            else:
                self.writer.fh.write(" ")
                self.column += 1
        self.writer.fh.write(val)
        self.column += len(val)


class CifCategoryWriter(object):
    def __init__(self, writer, category):
        self.writer = writer
        self.category = category
    def write(self, **kwargs):
        self.writer._write(self.category, kwargs)
    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        pass


class CifLoopWriter(object):
    def __init__(self, writer, category, keys):
        self.writer = writer
        self.category = category
        self.keys = keys
        # Remove characters that we can't use in Python identifiers
        self.python_keys = [k.replace('[', '').replace(']', '') for k in keys]
        self._empty_loop = True
    def write(self, **kwargs):
        if self._empty_loop:
            f = self.writer.fh
            f.write("#\nloop_\n")
            for k in self.keys:
                f.write("%s.%s\n" % (self.category, k))
            self._empty_loop = False
        l = _LineWriter(self.writer)
        for k in self.python_keys:
            l.write(kwargs.get(k, self.writer.omitted))
        self.writer.fh.write("\n")
    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        if not self._empty_loop:
            self.writer.fh.write("#\n")


class CifWriter(object):
    omitted = '.'
    unknown = '?'
    _boolmap = {False: 'NO', True: 'YES'}

    def __init__(self, fh):
        self.fh = fh
    def category(self, category):
        return CifCategoryWriter(self, category)
    def loop(self, category, keys):
        return CifLoopWriter(self, category, keys)
    def write_comment(self, comment):
        for line in textwrap.wrap(comment, 78):
            self.fh.write('# ' + line + '\n')
    def _write(self, category, kwargs):
        for key in kwargs:
            self.fh.write("%s.%s %s\n" % (category, key,
                                          self._repr(kwargs[key])))
    def _repr(self, obj):
        if isinstance(obj, str) and '"' not in obj \
           and "'" not in obj and " " not in obj:
            return obj
        elif isinstance(obj, float):
            return "%.3f" % obj
        elif isinstance(obj, bool):
            return self._boolmap[obj]
        else:
            return repr(obj)

def _get_by_residue(p):
    """Determine whether the given particle represents a specific residue
       or a more coarse-grained object."""
    return IMP.atom.Residue.get_is_setup(p) or IMP.atom.Atom.get_is_setup(p)

class AsymIDMapper(object):
    """Map a Particle to an asym_id (chain ID)"""
    def __init__(self, prot):
        self.o = IMP.pmi.output.Output()
        self.prot = prot
        self.name = 'cif-output'
        self.o.dictionary_pdbs[self.name] = self.prot
        self.o._init_dictchain(self.name, self.prot)

    def __getitem__(self, p):
        protname, is_a_bead = self.o.get_prot_name_from_particle(self.name, p)
        return self.o.dictchain[self.name][protname]

class ComponentMapper(object):
    """Map a Particle to a component name"""
    def __init__(self, prot):
        self.o = IMP.pmi.output.Output()
        self.prot = prot
        self.name = 'cif-output'
        self.o.dictionary_pdbs[self.name] = self.prot
        self.o._init_dictchain(self.name, self.prot)

    def __getitem__(self, p):
        protname, is_a_bead = self.o.get_prot_name_from_particle(self.name, p)
        return protname

class Dumper(object):
    """Base class for helpers to dump output to mmCIF"""
    def __init__(self, simo):
        self.simo = weakref.proxy(simo)

    def finalize(self):
        pass


class EntryDumper(Dumper):
    def dump(self, writer):
        entry_id = 'imp_model'
        # Write CIF header (so this dumper should always be first)
        writer.fh.write("data_%s\n" % entry_id)
        with writer.category("_entry") as l:
            l.write(id=entry_id)


class SoftwareDumper(Dumper):
    software = [
       IMP.pmi.metadata.Software(
             name="Integrative Modeling Platform (IMP)",
             version=IMP.__version__,
             classification="integrative model building",
             description="integrative model building",
             url='https://integrativemodeling.org'),
       IMP.pmi.metadata.Software(
            name="IMP PMI module",
            version=IMP.pmi.__version__,
            classification="integrative model building",
            description="integrative model building",
            url='https://integrativemodeling.org')
       ]

    def __init__(self, simo):
        super(SoftwareDumper, self).__init__(simo)
        self.modeller_used = False

    def set_modeller_used(self, version, date):
        if self.modeller_used:
            return
        self.modeller_used = True
        self.software.append(IMP.pmi.metadata.Software(
                    name='MODELLER', classification='comparative modeling',
                    description='Comparative modeling by satisfaction '
                                'of spatial restraints, build ' + date,
                    url='https://salilab.org/modeller/',
                    version=version))

    def dump(self, writer):
        ordinal = 1
        with writer.loop("_software",
                         ["pdbx_ordinal", "name", "classification", "version",
                          "type", "location"]) as l:
            for m in self.software + self.simo._metadata:
                if isinstance(m, IMP.pmi.metadata.Software):
                    l.write(pdbx_ordinal=ordinal, name=m.name,
                            classification=m.classification, version=m.version,
                            type=m.type, location=m.url)
                    ordinal += 1

class AuditAuthorDumper(Dumper):
    """Populate the mmCIF audit_author category (a list of the people that
       authored this mmCIF file; here we assume that's just the authors of
       any associated publications)"""
    def dump(self, writer):
        citations = [m for m in self.simo._metadata
                     if isinstance(m, IMP.pmi.metadata.Citation)]
        seen_authors = {}
        with writer.loop("_audit_author",
                         ["name", "pdbx_ordinal"]) as l:
            ordinal = 1
            for n, c in enumerate(citations):
                for a in c.authors:
                    if a not in seen_authors:
                        seen_authors[a] = None
                        l.write(name=a, pdbx_ordinal=ordinal)
                        ordinal += 1


class CitationDumper(Dumper):
    def dump(self, writer):
        citations = [m for m in self.simo._metadata
                     if isinstance(m, IMP.pmi.metadata.Citation)]
        with writer.loop("_citation",
                         ["id", "title", "journal_abbrev", "journal_volume",
                          "page_first", "page_last", "year",
                          "pdbx_database_id_PubMed",
                          "pdbx_database_id_DOI"]) as l:
            for n, c in enumerate(citations):
                if isinstance(c.page_range, (tuple, list)):
                    page_first, page_last = c.page_range
                else:
                    page_first = c.page_range
                    page_last = CifWriter.omitted
                l.write(id=n+1, title=c.title, journal_abbrev=c.journal,
                        journal_volume=c.volume, page_first=page_first,
                        page_last=page_last, year=c.year,
                        pdbx_database_id_PubMed=c.pmid,
                        pdbx_database_id_DOI=c.doi)

        with writer.loop("_citation_author",
                         ["citation_id", "name", "ordinal"]) as l:
            ordinal = 1
            for n, c in enumerate(citations):
                for a in c.authors:
                    l.write(citation_id=n+1, name=a, ordinal=ordinal)
                    ordinal += 1


class EntityDumper(Dumper):
    # todo: we currently only support amino acid sequences here (and
    # then only standard amino acids; need to add support for MSE etc.)
    def dump(self, writer):
        with writer.loop("_entity",
                         ["id", "type", "src_method", "pdbx_description",
                          "formula_weight", "pdbx_number_of_molecules",
                          "details"]) as l:
            for entity in self.simo.entities.get_all():
                l.write(id=entity.id, type='polymer', src_method='man',
                        pdbx_description=entity.first_component,
                        formula_weight=writer.unknown,
                        pdbx_number_of_molecules=1, details=writer.unknown)


class EntityPolyDumper(Dumper):
    # todo: we currently only support amino acid sequences here
    def __init__(self, simo):
        super(EntityPolyDumper, self).__init__(simo)
        self.output = IMP.pmi.output.Output()

    def dump(self, writer):
        with writer.loop("_entity_poly",
                         ["entity_id", "type", "nstd_linkage",
                          "nstd_monomer", "pdbx_strand_id",
                          "pdbx_seq_one_letter_code",
                          "pdbx_seq_one_letter_code_can"]) as l:
            for entity in self.simo.entities.get_all():
                seq = entity.sequence
                name = entity.first_component
                chain_id = self.simo.get_chain_for_component(name, self.output)
                l.write(entity_id=entity.id, type='polypeptide(L)',
                        nstd_linkage='no', nstd_monomer='no',
                        pdbx_strand_id=chain_id,
                        pdbx_seq_one_letter_code=seq,
                        pdbx_seq_one_letter_code_can=seq)


class ChemCompDumper(Dumper):
    def dump(self, writer):
        seen = {}
        std = dict.fromkeys(('ALA', 'CYS', 'ASP', 'GLU', 'PHE', 'GLY', 'HIS',
               'ILE', 'LYS', 'LEU', 'MET', 'ASN', 'PRO', 'GLN', 'ARG', 'SER',
               'THR', 'VAL', 'TRP', 'TYR'))
        with writer.loop("_chem_comp", ["id", "type"]) as l:
            for entity in self.simo.entities.get_all():
                seq = entity.sequence
                for num, one_letter_code in enumerate(seq):
                    restyp = IMP.atom.get_residue_type(one_letter_code)
                    resid = restyp.get_string()
                    if resid not in seen:
                        seen[resid] = None
                        l.write(id=resid,
                                type='L-peptide linking' if resid in std \
                                                         else 'other')

class EntityPolySeqDumper(Dumper):
    def dump(self, writer):
        with writer.loop("_entity_poly_seq",
                         ["entity_id", "num", "mon_id", "hetero"]) as l:
            for entity in self.simo.entities.get_all():
                seq = entity.sequence
                for num, one_letter_code in enumerate(seq):
                    restyp = IMP.atom.get_residue_type(one_letter_code)
                    l.write(entity_id=entity.id, num=num + 1,
                            mon_id=restyp.get_string(),
                            hetero=CifWriter.omitted)

class StructAsymDumper(Dumper):
    def __init__(self, simo):
        super(StructAsymDumper, self).__init__(simo)
        self.output = IMP.pmi.output.Output()

    def dump(self, writer):
        with writer.loop("_struct_asym",
                         ["id", "entity_id", "details"]) as l:
            for comp in self.simo.all_modeled_components:
                entity = self.simo.entities[comp]
                chain_id = self.simo.get_chain_for_component(comp, self.output)
                l.write(id=chain_id,
                        entity_id=entity.id,
                        details=comp)

class _PDBFragment(object):
    """Record details about part of a PDB file used as input
       for a component."""
    primitive = 'sphere'
    granularity = 'by-residue'
    num = CifWriter.omitted
    def __init__(self, m, component, start, end, offset, pdbname, chain, hier):
        self.component, self.start, self.end, self.offset, self.pdbname \
              = component, start, end, offset, pdbname
        self.chain, self.hier = chain, hier
        sel = IMP.atom.NonWaterNonHydrogenPDBSelector() \
              & IMP.atom.ChainPDBSelector(chain)
        self.starting_hier = IMP.atom.read_pdb(pdbname, m, sel)

    def combine(self, other):
        pass

class _BeadsFragment(object):
    """Record details about beads used to represent part of a component."""
    primitive = 'sphere'
    granularity = 'by-feature'
    chain = None
    def __init__(self, m, component, start, end, num, hier):
        self.component, self.start, self.end, self.num, self.hier \
              = component, start, end, num, hier

    def combine(self, other):
        # todo: don't combine if one fragment is rigid and the other flexible
        if type(other) == type(self) and other.start == self.end + 1:
            self.end = other.end
            self.num += other.num
            return True

class _StartingModel(object):
    """Record details about an input model (e.g. comparative modeling
       template) used for a component."""

    source = CifWriter.unknown
    db_name = CifWriter.unknown
    db_code = CifWriter.unknown
    sequence_identity = CifWriter.unknown

    def __init__(self, fragment):
        self.fragments = [fragment]

class ModelRepresentationDumper(Dumper):
    def __init__(self, simo):
        super(ModelRepresentationDumper, self).__init__(simo)
        # dict of fragments, ordered by component name
        self.fragments = OrderedDict()
        self.output = IMP.pmi.output.Output()

    def add_fragment(self, fragment):
        """Add a model fragment."""
        comp = fragment.component
        if comp not in self.fragments:
            self.fragments[comp] = []
        fragments = self.fragments[comp]
        if len(fragments) == 0 or not fragments[-1].combine(fragment):
            fragments.append(fragment)

    def get_model_mode(self, fragment):
        """Determine the model_mode for a given fragment ('rigid' or
           'flexible')"""
        leaves = IMP.atom.get_leaves(fragment.hier)
        # Assume all leaves are set up as rigid/flexible in the same way
        if IMP.core.RigidMember.get_is_setup(leaves[0]):
            return 'rigid'
        else:
            return 'flexible'

    def dump(self, writer):
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
            for comp, fragments in self.fragments.items():
                chain_id = self.simo.get_chain_for_component(comp, self.output)
                for f in fragments:
                    entity = self.simo.entities[f.component]
                    starting_model_id = CifWriter.omitted
                    if hasattr(f, 'pdbname'):
                        starting_model_id \
                               = self.starting_model[comp, f.pdbname].name
                    # todo: handle multiple representations
                    l.write(ordinal_id=ordinal_id,
                            representation_id=1,
                            segment_id=segment_id,
                            entity_id=entity.id,
                            entity_description=entity.description,
                            entity_asym_id=chain_id,
                            seq_id_begin=f.start,
                            seq_id_end=f.end,
                            model_object_primitive=f.primitive,
                            starting_model_id=starting_model_id,
                            model_mode=self.get_model_mode(f),
                            model_granularity=f.granularity,
                            model_object_count=f.num)
                    ordinal_id += 1
                    segment_id += 1

class PDBSource(object):
    """An experimental PDB file used as part of a starting model"""
    source = 'experimental model'
    db_name = 'PDB'
    sequence_identity = 100.0

    def __init__(self, model, db_code, chain_id, metadata):
        self.db_code = db_code
        self.chain_id = chain_id
        self.metadata = metadata

    def get_seq_id_range(self, model):
        # Assume the structure covers the entire sequence
        return (model.seq_id_begin, model.seq_id_end)

class TemplateSource(object):
    """A PDB file used as a template for a comparative starting model"""
    source = 'comparative model'
    db_name = 'PDB'

    def __init__(self, code, seq_id_begin, seq_id_end, seq_id,
                 model):
        # Assume a code of 1abcX refers to a real PDB structure
        if len(code) == 5:
            self.db_code = code[:4].upper()
            self.chain_id = code[4]
        else:
            self.db_code = self.chain_id = CifWriter.unknown
        self.sequence_identity = seq_id
        self._seq_id_begin, self._seq_id_end = seq_id_begin, seq_id_end

    def get_seq_id_range(self, model):
        # The template may cover more than the current starting model
        seq_id_begin = max(model.seq_id_begin, self._seq_id_begin)
        seq_id_end = min(model.seq_id_end, self._seq_id_end)
        return (seq_id_begin, seq_id_end)

class UnknownSource(object):
    """Part of a starting model from an unknown source"""
    source = CifWriter.unknown
    db_code = CifWriter.unknown
    db_name = CifWriter.unknown
    chain_id = CifWriter.unknown
    sequence_identity = CifWriter.unknown

    def __init__(self, model):
        pass

    def get_seq_id_range(self, model):
        return (model.seq_id_begin, model.seq_id_end)

class DatasetGroup(object):
    """A group of datasets"""
    def __init__(self, datasets, include_primaries):
        self._datasets = list(datasets)
        self.include_primaries = include_primaries

    def finalize(self):
        """Get final datasets for each restraint and remove duplicates"""
        final_datasets = OrderedDict()
        for ds in self._datasets:
            if isinstance(ds, RestraintDataset):
                d = ds.dataset
            else:
                d = ds
            final_datasets[d] = None
            if self.include_primaries:
                for p in d._primaries.keys():
                    final_datasets[p] = None
        self._datasets = final_datasets.keys()


class DatasetDumper(Dumper):
    def __init__(self, simo):
        super(DatasetDumper, self).__init__(simo)
        self._datasets = []
        self._dataset_groups = []

    def get_all_group(self, include_primaries=False):
        """Get a DatasetGroup encompassing all datasets so far"""
        g = DatasetGroup(self._datasets, include_primaries)
        self._dataset_groups.append(g)
        return g

    def add(self, dataset):
        """Add a new dataset.
           Note that ids are assigned later."""
        self._datasets.append(dataset)
        return dataset

    def finalize(self):
        seen_datasets = {}
        # Assign IDs to all datasets
        self._dataset_by_id = []
        for d in self._flatten_dataset(self._datasets):
            if d not in seen_datasets:
                if not hasattr(d, 'id'):
                    self._dataset_by_id.append(d)
                    d.id = len(self._dataset_by_id)
                    if isinstance(d.location,
                                  IMP.pmi.metadata.RepositoryFileLocation) \
                       and not d.location.doi:
                        newloc = self.simo._get_location(d.location.path)
                        d.location.doi = newloc.doi
                        d.location.path = newloc.path
                seen_datasets[d] = d.id
            else:
                d.id = seen_datasets[d]

        # Make sure that all datasets are listed, even if they weren't used
        all_group = self.get_all_group(True)
        # Assign IDs to all groups and remove duplicates
        seen_group_ids = {}
        self._dataset_group_by_id = []
        for g in self._dataset_groups:
            g.finalize()
            ids = tuple(sorted(d.id for d in g._datasets))
            if ids not in seen_group_ids:
                self._dataset_group_by_id.append(g)
                g.id = len(self._dataset_group_by_id)
                seen_group_ids[ids] = g
            else:
                g.id = seen_group_ids[ids].id

    def _flatten_dataset(self, d):
        if isinstance(d, list):
            for p in d:
                for x in self._flatten_dataset(p):
                    yield x
        elif isinstance(d, RestraintDataset):
            for x in self._flatten_dataset(d.dataset):
                yield x
        else:
            for p in d._primaries.keys():
                for x in self._flatten_dataset(p):
                    yield x
            yield d

    def dump(self, writer):
        ordinal = 1
        with writer.loop("_ihm_dataset_list",
                         ["ordinal_id", "id", "group_id", "data_type",
                          "database_hosted"]) as l:
            for g in self._dataset_group_by_id:
                for d in g._datasets:
                    l.write(ordinal_id=ordinal, id=d.id, group_id=g.id,
                            data_type=d._data_type,
                            database_hosted=isinstance(d.location,
                                            IMP.pmi.metadata.DatabaseLocation))
                    ordinal += 1
        self.dump_other((d for d in self._dataset_by_id
                         if not isinstance(d.location,
                                            IMP.pmi.metadata.DatabaseLocation)),
                        writer)
        self.dump_rel_dbs((d for d in self._dataset_by_id
                           if isinstance(d.location,
                                            IMP.pmi.metadata.DatabaseLocation)),
                          writer)
        self.dump_related(writer)

    def dump_related(self, writer):
        ordinal = 1
        with writer.loop("_ihm_related_datasets",
                         ["ordinal_id", "dataset_list_id_derived",
                          "data_type_derived", "dataset_list_id_primary",
                          "data_type_primary"]) as l:
            for derived in self._dataset_by_id:
                for primary in sorted(derived._primaries.keys(),
                                      lambda d: d.id):
                    l.write(ordinal_id=ordinal,
                            dataset_list_id_derived=derived.id,
                            data_type_derived=derived._data_type,
                            dataset_list_id_primary=primary.id,
                            data_type_primary=primary._data_type)
                    ordinal += 1

    def dump_rel_dbs(self, datasets, writer):
        ordinal = 1
        with writer.loop("_ihm_dataset_related_db_reference",
                         ["id", "dataset_list_id", "db_name",
                          "access_code", "version", "data_type",
                          "details"]) as l:
            for d in datasets:
                l.write(id=ordinal, dataset_list_id=d.id,
                        db_name=d.location.db_name,
                        access_code=d.location.access_code,
                        version=d.location.version if d.location.version
                                else CifWriter.omitted,
                        data_type=d._data_type,
                        details=d.location.details if d.location.details
                                else CifWriter.omitted)
                ordinal += 1

    def dump_other(self, datasets, writer):
        ordinal = 1
        with writer.loop("_ihm_dataset_other",
                         ["id", "dataset_list_id", "data_type",
                          "doi", "content_filename"]) as l:
            for d in datasets:
                l.write(id=ordinal, dataset_list_id=d.id,
                        data_type=d._data_type, doi=d.location.doi,
                        content_filename=d.location.path)
                ordinal += 1


class ExperimentalCrossLink(object):
    def __init__(self, r1, c1, r2, c2, label, length, rdataset):
        self.r1, self.c1, self.r2, self.c2, self.label = r1, c1, r2, c2, label
        self.length, self.rdataset = length, rdataset

class CrossLink(object):
    def __init__(self, ex_xl, p1, p2, sigma1, sigma2, psi):
        self.ex_xl, self.sigma1, self.sigma2 = ex_xl, sigma1, sigma2
        self.p1, self.p2 = p1, p2
        self.psi = psi

class CrossLinkDumper(Dumper):
    def __init__(self, simo):
        super(CrossLinkDumper, self).__init__(simo)
        self.cross_links = []
        self.exp_cross_links = []

    def add_experimental(self, cross_link):
        self.exp_cross_links.append(cross_link)
        cross_link.id = len(self.exp_cross_links)

    def add(self, cross_link):
        self.cross_links.append(cross_link)
        cross_link.id = len(self.cross_links)

    def dump(self, writer):
        self.dump_list(writer)
        self.dump_restraint(writer)
        self.dump_results(writer)

    def dump_list(self, writer):
        with writer.loop("_ihm_cross_link_list",
                         ["id", "group_id", "entity_description_1",
                          "entity_id_1", "seq_id_1", "comp_id_1",
                          "entity_description_2",
                          "entity_id_2", "seq_id_2", "comp_id_2", "type",
                          "dataset_list_id"]) as l:
            for xl in self.exp_cross_links:
                entity1 = self.simo.entities[xl.c1]
                entity2 = self.simo.entities[xl.c2]
                seq1 = entity1.sequence
                seq2 = entity2.sequence
                rt1 = IMP.atom.get_residue_type(seq1[xl.r1-1])
                rt2 = IMP.atom.get_residue_type(seq2[xl.r2-1])
                # todo: handle experimental ambiguity (group_id) properly
                l.write(id=xl.id, group_id=xl.id,
                        entity_description_1=entity1.description,
                        entity_id_1=entity1.id,
                        seq_id_1=xl.r1,
                        comp_id_1=rt1.get_string(),
                        entity_description_2=entity2.description,
                        entity_id_2=entity2.id,
                        seq_id_2=xl.r2,
                        comp_id_2=rt2.get_string(),
                        type=xl.label,
                        dataset_list_id=xl.rdataset.dataset.id)

    def _granularity(self, xl):
        """Determine the granularity of a cross link"""
        if _get_by_residue(xl.p1) and _get_by_residue(xl.p2):
            return 'by-residue'
        else:
            return 'by-feature'

    def dump_restraint(self, writer):
        asym = AsymIDMapper(self.simo.prot)
        with writer.loop("_ihm_cross_link_restraint",
                         ["id", "group_id", "entity_id_1", "asym_id_1",
                          "seq_id_1", "comp_id_1",
                          "entity_id_2", "asym_id_2", "seq_id_2", "comp_id_2",
                          "type", "conditional_crosslink_flag",
                          "model_granularity", "distance_threshold",
                          "psi", "sigma_1", "sigma_2"]) as l:
            for xl in self.cross_links:
                entity1 = self.simo.entities[xl.ex_xl.c1]
                entity2 = self.simo.entities[xl.ex_xl.c2]
                seq1 = entity1.sequence
                seq2 = entity2.sequence
                rt1 = IMP.atom.get_residue_type(seq1[xl.ex_xl.r1-1])
                rt2 = IMP.atom.get_residue_type(seq2[xl.ex_xl.r2-1])
                l.write(id=xl.id,
                        group_id=xl.ex_xl.id,
                        entity_id_1=entity1.id,
                        asym_id_1=asym[xl.p1],
                        seq_id_1=xl.ex_xl.r1,
                        comp_id_1=rt1.get_string(),
                        entity_id_2=entity2.id,
                        asym_id_2=asym[xl.p2],
                        seq_id_2=xl.ex_xl.r2,
                        comp_id_2=rt2.get_string(),
                        type=xl.ex_xl.label,
                        # todo: any circumstances where this could be ANY?
                        conditional_crosslink_flag="ALL",
                        model_granularity=self._granularity(xl),
                        distance_threshold=xl.ex_xl.length,
                        # todo: handle cases where psi is optimized
                        psi=xl.psi.get_scale(),
                        sigma_1=xl.sigma1, sigma_2=xl.sigma2)

    def dump_results(self, writer):
        ordinal = 1
        with writer.loop("_ihm_cross_link_result_parameters",
                         ["ordinal_id", "restraint_id", "model_id",
                          "sigma_1", "sigma_2"]) as l:
            for model in self.models:
                for xl in self.cross_links:
                    # todo: handle resolutions!=1, multiple independent sigmas
                    statname = 'ISDCrossLinkMS_Sigma_1_%s' % xl.ex_xl.label
                    if model.stats and statname in model.stats:
                        sigma = float(model.stats[statname])
                        l.write(ordinal_id=ordinal, restraint_id=xl.id,
                                model_id=model.id, sigma_1=sigma, sigma_2=sigma)
                        ordinal += 1

class EM2DRestraint(object):
    def __init__(self, rdataset, resolution, pixel_size,
                 image_resolution, projection_number):
        self.rdataset, self.resolution = rdataset, resolution
        self.pixel_size, self.image_resolution = pixel_size, image_resolution
        self.projection_number = projection_number

    def get_num_raw_micrographs(self):
        """Return the number of raw micrographs used, if known.
           This is extracted from the EMMicrographsDataset if any."""
        for d in self.rdataset.dataset._primaries.keys():
            if isinstance(d, IMP.pmi.metadata.EMMicrographsDataset):
                return d.number

class EM2DDumper(Dumper):
    def __init__(self, simo):
        super(EM2DDumper, self).__init__(simo)
        self.restraints = []

    def add(self, rsr):
        self.restraints.append(rsr)
        rsr.id = len(self.restraints)

    def dump(self, writer):
        with writer.loop("_ihm_2dem_class_average_restraint",
                         ["id", "dataset_list_id", "number_raw_micrographs",
                          "pixel_size_width", "pixel_size_height",
                          "image_resolution", "image_segment_flag",
                          "number_of_projections", "struct_assembly_id",
                          "details"]) as l:
            for r in self.restraints:
                unk = CifWriter.unknown
                num_raw = r.get_num_raw_micrographs()
                l.write(id=r.id, dataset_list_id=r.rdataset.dataset.id,
                        number_raw_micrographs=num_raw if num_raw else unk,
                        pixel_size_width=r.pixel_size,
                        pixel_size_height=r.pixel_size,
                        image_resolution=r.image_resolution,
                        number_of_projections=r.projection_number,
                        struct_assembly_id=self.simo.modeled_assembly.id,
                        image_segment_flag=False)

class EM3DRestraint(object):
    fitting_method = 'Gaussian mixture models'

    def __init__(self, simo, rdataset, target_ps, densities):
        self.rdataset = rdataset
        self.number_of_gaussians = len(target_ps)
        self.assembly = self.get_assembly(densities, simo)

    def get_assembly(self, densities, simo):
        """Get the Assembly that this restraint acts on"""
        cm = ComponentMapper(simo.prot)
        components = {}
        for d in densities:
            components[cm[d]] = None
        return simo.assembly_dump.get_subassembly(components)


class EM3DDumper(Dumper):
    def __init__(self, simo):
        super(EM3DDumper, self).__init__(simo)
        self.restraints = []

    def add(self, rsr):
        self.restraints.append(rsr)

    def dump(self, writer):
        # todo: support other fields
        ordinal = 1
        with writer.loop("_ihm_3dem_restraint",
                         ["ordinal_id", "dataset_list_id", "fitting_method",
                          "struct_assembly_id",
                          "number_of_gaussians"]) as l:
            for r in self.restraints:
                l.write(ordinal_id=ordinal,
                        dataset_list_id=r.rdataset.dataset.id,
                        fitting_method=r.fitting_method,
                        struct_assembly_id=r.assembly.id,
                        number_of_gaussians=r.number_of_gaussians)
                ordinal += 1

class Assembly(list):
    """A collection of components. Currently simply implemented as a list of
       the component names. These must be in creation order."""
    pass

class AssemblyDumper(Dumper):
    def __init__(self, simo):
        super(AssemblyDumper, self).__init__(simo)
        self.assemblies = []
        self.output = IMP.pmi.output.Output()

    def add(self, a):
        """Add a new assembly. The first such assembly is assumed to contain
           all components."""
        self.assemblies.append(a)
        a.id = len(self.assemblies)
        return a

    def get_subassembly(self, compdict):
        """Get an Assembly consisting of the given components. Should only
           be called after all components are created."""
        # Put components in creation order
        newa = Assembly(c for c in self.assemblies[0] if c in compdict)
        for a in self.assemblies:
            if newa == a: # Note that .id is ignored by ==
                return a
        else:
            return self.add(newa)

    def dump(self, writer):
        ordinal = 1
        with writer.loop("_ihm_struct_assembly",
                         ["ordinal_id", "assembly_id", "entity_description",
                          "entity_id", "asym_id", "seq_id_begin",
                          "seq_id_end"]) as l:
            for a in self.assemblies:
                for comp in a:
                    entity = self.simo.entities[comp]
                    seq = self.simo.sequence_dict[comp]
                    chain_id = self.simo.get_chain_for_component(comp,
                                                                 self.output)
                    l.write(ordinal_id=ordinal, assembly_id=a.id,
                            entity_description=entity.description,
                            entity_id=entity.id,
                            asym_id=chain_id,
                            seq_id_begin=1,
                            seq_id_end=len(seq))
                    ordinal += 1


class Protocol(object):
    pass

class ReplicaExchangeProtocol(Protocol):
    def __init__(self, rex):
        if rex.monte_carlo_sample_objects is not None:
            self.step_method = 'Replica exchange monte carlo'
        else:
            self.step_method = 'Replica exchange molecular dynamics'
        self.num_models_end = rex.vars["number_of_frames"]

class ModelGroup(object):
    """Group sets of models"""
    def __init__(self, name):
        self.name = name

class Model(object):
    def __init__(self, prot, simo, protocol, assembly, group):
        self.group = group
        # The Protocol which produced this model
        self.protocol = protocol
        self.assembly = assembly
        self.stats = None
        o = IMP.pmi.output.Output()
        name = 'cif-output'
        o.dictionary_pdbs[name] = prot
        o._init_dictchain(name, prot)
        (particle_infos_for_pdb,
         self.geometric_center) = o.get_particle_infos_for_pdb_writing(name)
        self.entity_for_chain = {}
        self.comp_for_chain = {}
        for protname, chain_id in o.dictchain[name].items():
            self.entity_for_chain[chain_id] = simo.entities[protname]
            self.comp_for_chain[chain_id] = protname
        self.spheres = [t for t in particle_infos_for_pdb if t[1] is None]
        self.atoms = [t for t in particle_infos_for_pdb if t[1] is not None]
        self.rmsf = {}

    def parse_rmsf_file(self, fname, component):
        self.rmsf[component] = rmsf = {}
        with open(fname) as fh:
            for line in fh:
                resnum, blocknum, val = line.split()
                rmsf[int(resnum)] = (int(blocknum), float(val))

    def get_rmsf(self, component, indexes):
        """Get the RMSF value for the given residue indexes."""
        if not self.rmsf:
            return CifWriter.omitted
        rmsf = self.rmsf[component]
        blocknums = dict.fromkeys(rmsf[ind][0] for ind in indexes)
        if len(blocknums) != 1:
            raise ValueError("Residue indexes %s aren't all in the same block"
                             % str(indexes))
        return rmsf[indexes[0]][1]

class ModelDumper(Dumper):
    def __init__(self, simo):
        super(ModelDumper, self).__init__(simo)
        self.models = []

    def add(self, prot, protocol, assembly, group):
        m = Model(prot, self.simo, protocol, assembly, group)
        self.models.append(m)
        m.id = len(self.models)
        return m

    def dump(self, writer):
        self.dump_model_list(writer)
        num_atoms = sum(len(m.atoms) for m in self.models)
        num_spheres = sum(len(m.spheres) for m in self.models)
        self.dump_atoms(writer)
        self.dump_spheres(writer)

    def dump_model_list(self, writer):
        ordinal = 1
        with writer.loop("_ihm_model_list",
                         ["ordinal_id", "model_id", "model_group_id",
                          "model_group_name", "assembly_id",
                          "protocol_id"]) as l:
            for model in self.models:
                l.write(ordinal_id=ordinal, model_id=model.id,
                        model_group_id=model.group.id,
                        model_group_name=model.group.name,
                        assembly_id=model.assembly.id,
                        protocol_id=model.protocol.id)
                ordinal += 1

    def dump_atoms(self, writer):
        ordinal = 1
        with writer.loop("_atom_site",
                         ["id", "label_atom_id", "label_comp_id",
                          "label_seq_id",
                          "label_asym_id", "Cartn_x",
                          "Cartn_y", "Cartn_z", "label_entity_id",
                          "model_id"]) as l:
            for model in self.models:
                for atom in model.atoms:
                    (xyz, atom_type, residue_type, chain_id, residue_index,
                     all_indexes, radius) = atom
                    l.write(id=ordinal, label_atom_id=atom_type.get_string(),
                            label_comp_id=residue_type.get_string(),
                            label_asym_id=chain_id,
                            label_entity_id=model.entity_for_chain[chain_id].id,
                            label_seq_id=residue_index,
                            Cartn_x=xyz[0] - model.geometric_center[0],
                            Cartn_y=xyz[1] - model.geometric_center[1],
                            Cartn_z=xyz[2] - model.geometric_center[2],
                            model_id=model.id)
                    ordinal += 1

    def dump_spheres(self, writer):
        ordinal = 1
        with writer.loop("_ihm_sphere_obj_site",
                         ["ordinal_id", "entity_id", "seq_id_begin",
                          "seq_id_end", "asym_id", "Cartn_x",
                          "Cartn_y", "Cartn_z", "object_radius", "rmsf",
                          "model_id"]) as l:
            for model in self.models:
                for sphere in model.spheres:
                    (xyz, atom_type, residue_type, chain_id, residue_index,
                     all_indexes, radius) = sphere
                    if all_indexes is None:
                        all_indexes = (residue_index,)
                    l.write(ordinal_id=ordinal,
                            entity_id=model.entity_for_chain[chain_id].id,
                            seq_id_begin = all_indexes[0],
                            seq_id_end = all_indexes[-1],
                            asym_id=chain_id,
                            Cartn_x=xyz[0] - model.geometric_center[0],
                            Cartn_y=xyz[1] - model.geometric_center[1],
                            Cartn_z=xyz[2] - model.geometric_center[2],
                            object_radius=radius,
                            rmsf=model.get_rmsf(model.comp_for_chain[chain_id],
                                                all_indexes),
                            model_id=model.id)
                    ordinal += 1


class ModelProtocolDumper(Dumper):
    def __init__(self, simo):
        super(ModelProtocolDumper, self).__init__(simo)
        self.protocols = []

    def add(self, protocol):
        self.protocols.append(protocol)
        protocol.id = len(self.protocols)
        # Assume that protocol uses all currently-defined datasets
        protocol.dataset_group = self.simo.dataset_dump.get_all_group()

    def get_last_protocol(self):
        """Return the most recently-added Protocol"""
        return self.protocols[-1]

    def dump(self, writer):
        ordinal = 1
        with writer.loop("_ihm_modeling_protocol",
                         ["ordinal_id", "protocol_id", "step_id",
                          "struct_assembly_id", "dataset_group_id",
                          "struct_assembly_description", "protocol_name",
                          "step_name", "step_method", "num_models_begin",
                          "num_models_end", "multi_scale_flag",
                          "multi_state_flag", "time_ordered_flag"]) as l:
            # todo: handle multiple protocols (e.g. sampling then refinement)
            num_models_begin = 0
            for p in self.protocols:
                l.write(ordinal_id=ordinal, protocol_id=1,
                        step_id=p.id, step_method=p.step_method,
                        step_name='Sampling',
                        struct_assembly_id=self.simo.modeled_assembly.id,
                        dataset_group_id=p.dataset_group.id,
                        num_models_begin=num_models_begin,
                        num_models_end=p.num_models_end,
                        # todo: support multiple states, time ordered
                        multi_state_flag=False, time_ordered_flag=False,
                        # all PMI models are multi scale
                        multi_scale_flag=True)
                num_models_begin = p.num_models_end
                ordinal += 1


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


class StartingModelDumper(Dumper):
    def __init__(self, simo):
        super(StartingModelDumper, self).__init__(simo)
        # dict of PDB fragments, ordered by component name
        self.fragments = OrderedDict()
        # dict of starting models (entire PDB files), collected from fragments
        self.models = OrderedDict()
        # mapping from component+pdbname to starting model
        self.starting_model = {}
        self.output = IMP.pmi.output.Output()

    def add_pdb_fragment(self, fragment):
        """Add a starting model PDB fragment."""
        comp = fragment.component
        if comp not in self.fragments:
            self.fragments[comp] = []
            self.models[comp] = []
        self.fragments[comp].append(fragment)
        models = self.models[comp]
        if len(models) == 0 \
           or models[-1].fragments[0].pdbname != fragment.pdbname:
            model = _StartingModel(fragment)
            models.append(model)
            model.sources = self.get_sources(model, fragment.pdbname,
                                             fragment.chain)
        else:
            models[-1].fragments.append(fragment)

    def get_templates(self, pdbname, model):
        templates = []
        tmpre = re.compile('REMARK   6 TEMPLATE: (\S+) .* '
                           'MODELS (\S+):\S+ \- (\S+):\S+ AT (\S+)%')

        with open(pdbname) as fh:
            for line in fh:
                if line.startswith('ATOM'): # Read only the header
                    break
                m = tmpre.match(line)
                if m:
                    templates.append(TemplateSource(m.group(1),
                                                    int(m.group(2)),
                                                    int(m.group(3)),
                                                    m.group(4), model))
        # Sort by starting residue, then ending residue
        return sorted(templates, key=lambda x: (x._seq_id_begin, x._seq_id_end))

    def _parse_pdb(self, fh, first_line):
        """Extract information from an official PDB"""
        metadata = []
        details = ''
        for line in fh:
            if line.startswith('TITLE'):
                details += line[10:].rstrip()
            elif line.startswith('HELIX'):
                metadata.append(PDBHelix(line))
        return (first_line[50:59].strip(),
                details if details else CifWriter.unknown, metadata)

    def get_sources(self, model, pdbname, chain):
        # Attempt to identity PDB file vs. comparative model
        fh = open(pdbname)
        first_line = fh.readline()
        if first_line.startswith('HEADER'):
            version, details, metadata = self._parse_pdb(fh, first_line)
            source = PDBSource(model, first_line[62:66].strip(), chain,
                               metadata)
            l = IMP.pmi.metadata.PDBLocation(source.db_code, version, details)
            d = IMP.pmi.metadata.PDBDataset(l)
            model.dataset = self.simo.dataset_dump.add(d)
            return [source]
        elif first_line.startswith('EXPDTA    THEORETICAL MODEL, MODELLER'):
            self.simo.software_dump.set_modeller_used(
                                        *first_line[38:].split(' ', 1))
            d = IMP.pmi.metadata.ComparativeModelDataset(
                                     self.simo._get_location(pdbname))
            model.dataset = self.simo.dataset_dump.add(d)
            templates = self.get_templates(pdbname, model)
            if templates:
                return templates
            else:
                return [UnknownSource(model)]
        else:
            # todo: extract Modeller-like template info for Phyre models;
            # revisit assumption that all such unknown source PDBs are
            # comparative models
            d = IMP.pmi.metadata.ComparativeModelDataset(
                                      self.simo._get_location(pdbname))
            model.dataset = self.simo.dataset_dump.add(d)
            return [UnknownSource(model)]

    def assign_model_details(self):
        for comp, models in self.models.items():
            for i, model in enumerate(models):
                model.name = "%s-m%d" % (comp, i+1)
                self.starting_model[comp, model.fragments[0].pdbname] = model
                model.seq_id_begin = min(x.start + x.offset
                                         for x in model.fragments)
                model.seq_id_end = max(x.end + x.offset
                                       for x in model.fragments)

    def all_models(self):
        for comp, models in self.models.items():
            for model in models:
                yield model

    def finalize(self):
        self.assign_model_details()

    def dump(self, writer):
        self.dump_details(writer)
        self.dump_coords(writer)

    def dump_details(self, writer):
        writer.write_comment("""IMP will attempt to identify which input models
are crystal structures and which are comparative models, but does not always
have sufficient information to deduce all of the templates used for comparative
modeling. These may need to be added manually below.""")
        with writer.loop("_ihm_starting_model_details",
                     ["ordinal_id", "entity_id", "entity_description",
                      "asym_id", "seq_id_begin",
                      "seq_id_end", "starting_model_source",
                      "starting_model_db_name", "starting_model_db_code",
                      "starting_model_db_pdb_auth_asym_id",
                      "starting_model_sequence_identity",
                      "starting_model_id",
                      "dataset_list_id"]) as l:
            ordinal = 1
            for model in self.all_models():
                f = model.fragments[0]
                entity = self.simo.entities[f.component]
                chain_id = self.simo.get_chain_for_component(f.component,
                                                             self.output)
                for source in model.sources:
                    seq_id_begin, seq_id_end = source.get_seq_id_range(model)
                    l.write(ordinal_id=ordinal,
                      entity_id=entity.id,
                      entity_description=entity.description,
                      asym_id=chain_id,
                      seq_id_begin=seq_id_begin,
                      seq_id_end=seq_id_end,
                      starting_model_db_pdb_auth_asym_id=source.chain_id,
                      starting_model_id=model.name,
                      starting_model_source=source.source,
                      starting_model_db_name=source.db_name,
                      starting_model_db_code=source.db_code,
                      starting_model_sequence_identity=source.sequence_identity,
                      dataset_list_id=model.dataset.id)
                    ordinal += 1

    def dump_coords(self, writer):
        ordinal = 1
        with writer.loop("_ihm_starting_model_coord",
                     ["starting_model_id", "group_PDB", "id", "type_symbol",
                      "atom_id", "comp_id", "entity_id", "asym_id",
                      "seq_id", "Cartn_x",
                      "Cartn_y", "Cartn_z", "B_iso_or_equiv",
                      "ordinal_id"]) as l:
            for model in self.all_models():
                for f in model.fragments:
                    sel = IMP.atom.Selection(f.starting_hier,
                               residue_indexes=list(range(f.start, f.end + 1)))
                    for a in sel.get_selected_particles():
                        coord = IMP.core.XYZ(a).get_coordinates()
                        atom = IMP.atom.Atom(a)
                        element = atom.get_element()
                        element = IMP.atom.get_element_table().get_name(element)
                        atom_name = atom.get_atom_type().get_string()
                        group_pdb = 'ATOM'
                        if atom_name.startswith('HET:'):
                            group_pdb = 'HETATM'
                            atom_name = atom_name[4:]
                        res = IMP.atom.get_residue(atom)
                        res_name = res.get_residue_type().get_string()
                        # MSE in the original PDB is automatically mutated
                        # by IMP to MET, so reflect that in the output.
                        # todo: also add to the notes for starting_model_details
                        # that the sequence was changed
                        if res_name == 'MSE':
                            res_name = 'MET'
                        chain_id = self.simo.get_chain_for_component(
                                            f.component, self.output)
                        entity = self.simo.entities[f.component]
                        l.write(starting_model_id=model.name,
                                group_PDB=group_pdb,
                                id=atom.get_input_index(), type_symbol=element,
                                atom_id=atom_name, comp_id=res_name,
                                entity_id=entity.id,
                                asym_id=chain_id,
                                seq_id=res.get_index(), Cartn_x=coord[0],
                                Cartn_y=coord[1], Cartn_z=coord[2],
                                B_iso_or_equiv=atom.get_temperature_factor(),
                                ordinal_id=ordinal)
                        ordinal += 1

class StructConfDumper(Dumper):
    def all_rigid_fragments(self):
        """Yield all rigid model representation fragments"""
        asym = AsymIDMapper(self.simo.prot)
        model_repr = self.simo.model_repr_dump
        for comp, fragments in model_repr.fragments.items():
            for f in fragments:
                if hasattr(f, 'pdbname') \
                   and model_repr.get_model_mode(f) == 'rigid':
                    yield (f, model_repr.starting_model[comp, f.pdbname],
                           asym[f.hier])

    def all_helices(self):
        """Yield all helices that overlap with rigid model fragments"""
        for f, starting_model, asym_id in self.all_rigid_fragments():
            if len(starting_model.sources) == 1 \
               and isinstance(starting_model.sources[0], PDBSource):
                pdb = starting_model.sources[0]
                for m in pdb.metadata:
                    if isinstance(m, PDBHelix) \
                       and m.start_asym == pdb.chain_id \
                       and m.end_asym == pdb.chain_id \
                       and m.start_resnum >= f.start and m.end_resnum <= f.end:
                        yield (m, max(f.start, m.start_resnum),
                               min(f.end, m.end_resnum), asym_id)

    def dump(self, writer):
        with writer.category("_struct_conf_type") as l:
            l.write(id='HELX_P', criteria=CifWriter.unknown,
                    reference=CifWriter.unknown)
        # Dump helix information for the model. For any model fragment that
        # is rigid, atomic, and uses an experimental PDB structure as the
        # starting model, inherit any helix information from that PDB file.
        # Note that we can't use the helix id from the original PDB, since
        # it has to be unique and this might not be the case if we inherit
        # from multiple PDBs.
        ordinal = 0
        with writer.loop("_struct_conf",
                     ["id", "conf_type_id", "beg_label_comp_id",
                      "beg_label_asym_id", "beg_label_seq_id",
                      "end_label_comp_id", "end_label_asym_id",
                      "end_label_seq_id",]) as l:
            for h, begin, end, asym_id in self.all_helices():
                ordinal += 1
                l.write(id='HELX_P%d' % ordinal, conf_type_id='HELX_P',
                        beg_label_comp_id=h.start_resnam,
                        beg_label_asym_id=asym_id,
                        beg_label_seq_id=begin,
                        end_label_comp_id=h.end_resnam,
                        end_label_asym_id=asym_id,
                        end_label_seq_id=end)


class PostProcess(object):
    """Base class for any post processing"""
    pass


class ReplicaExchangeAnalysisPostProcess(PostProcess):
    """Post processing using AnalysisReplicaExchange0 macro"""
    type = 'cluster'
    feature = 'RMSD'

    def __init__(self, rex, num_models_begin):
        self.rex = rex
        self.num_models_begin = num_models_begin
        self.num_models_end = 0
        for fname in self.get_all_stat_files():
            with open(fname) as fh:
                self.num_models_end += len(fh.readlines())

    def get_stat_file(self, cluster_num):
        return os.path.join(self.rex._outputdir, "cluster.%d" % cluster_num,
                            'stat.out')

    def get_all_stat_files(self):
        for i in range(self.rex._number_of_clusters):
            yield self.get_stat_file(i)


class PostProcessDumper(Dumper):
    def __init__(self, simo):
        super(PostProcessDumper, self).__init__(simo)
        self.postprocs = []

    def add(self, postproc):
        self.postprocs.append(postproc)
        postproc.id = len(self.postprocs)

    def dump(self, writer):
        with writer.loop("_ihm_modeling_post_process",
                         ["id", "protocol_id", "analysis_id", "step_id",
                          "type", "feature", "num_models_begin",
                          "num_models_end"]) as l:
            # todo: handle multiple protocols (e.g. sampling then refinement)
            # and multiple analyses
            for p in self.postprocs:
                l.write(id=p.id, protocol_id=1, analysis_id=1, step_id=p.id,
                        type=p.type, feature=p.feature,
                        num_models_begin=p.num_models_begin,
                        num_models_end=p.num_models_end)

class Ensemble(object):
    """Base class for any ensemble"""
    pass


class ReplicaExchangeAnalysisEnsemble(Ensemble):
    """Ensemble generated using AnalysisReplicaExchange0 macro"""

    def __init__(self, pp, cluster_num, model_group, num_deposit):
        self.model_group = model_group
        self.cluster_num = cluster_num
        self.postproc = pp
        self.num_deposit = num_deposit
        self.localization_density = {}
        with open(pp.get_stat_file(cluster_num)) as fh:
            self.num_models = len(fh.readlines())

    def get_rmsf_file(self, component):
        return os.path.join(self.postproc.rex._outputdir,
                            'cluster.%d' % self.cluster_num,
                            'rmsf.%s.dat' % component)

    def load_rmsf(self, model, component):
        fname = self.get_rmsf_file(component)
        if os.path.exists(fname):
            model.parse_rmsf_file(fname, component)

    def get_localization_density_file(self, component):
        # todo: this assumes that the localization density file name matches
        # the component name and is of the complete residue range (usually
        # this is the case, but it doesn't have to be)
        return os.path.join(self.postproc.rex._outputdir,
                            'cluster.%d' % self.cluster_num,
                            '%s.mrc' % component)

    def load_localization_density(self, mdl, component):
        fname = self.get_localization_density_file(component)
        if os.path.exists(fname):
            dmap = IMP.em.read_map(fname, IMP.em.MRCReaderWriter())
            pts = IMP.isd.sample_points_from_density(dmap, 1000000)
            dmap.set_was_used(True)
            density_ps = []
            ncenters = 50 # todo: make configurable or scale with # of residues
            score, akaikescore = IMP.isd.gmm_tools.fit_gmm_to_points(
                                             pts, ncenters, mdl, density_ps)
            # todo: verify score isn't terrible; if it is try repeating with
            # more centers
            self.localization_density[component] = density_ps

    def load_all_models(self, simo):
        stat_fname = self.postproc.get_stat_file(self.cluster_num)
        model_num = 0
        with open(stat_fname) as fh:
            stats = eval(fh.readline())
            # Correct path
            rmf_file = os.path.join(os.path.dirname(stat_fname),
                                    "%d.rmf3" % model_num)
            for c in simo.all_modeled_components:
                # todo: this only works with PMI 1
                simo._representation.set_coordinates_from_rmf(c, rmf_file, 0,
                                                       force_rigid_update=True)
            # todo: fill in other data from stat file, e.g. crosslink phi/psi
            yield stats
            model_num += 1
            if model_num >= self.num_deposit:
                return

    # todo: also support dRMS precision
    def _get_precision(self):
        precfile = os.path.join(self.postproc.rex._outputdir,
                                "precision.%d.%d.out" % (self.cluster_num,
                                                         self.cluster_num))
        if not os.path.exists(precfile):
            return CifWriter.unknown
        # Fail if the precision.x.x.out file doesn't match the cluster
        r = re.compile('All .*/cluster.%d/ average centroid distance ([\d\.]+)'
                       % self.cluster_num)
        with open(precfile) as fh:
            for line in fh:
                m = r.match(line)
                if m:
                    return float(m.group(1))

    feature = property(lambda self: self.postproc.feature)
    name = property(lambda self: "Cluster %d" % (self.cluster_num + 1))
    precision = property(lambda self: self._get_precision())


class EnsembleDumper(Dumper):
    def __init__(self, simo):
        super(EnsembleDumper, self).__init__(simo)
        self.ensembles = []

    def add(self, ensemble):
        self.ensembles.append(ensemble)
        ensemble.id = len(self.ensembles)

    def dump(self, writer):
        with writer.loop("_ihm_ensemble_info",
                         ["ensemble_id", "ensemble_name", "post_process_id",
                          "model_group_id", "ensemble_clustering_method",
                          "ensemble_clustering_feature",
                          "num_ensemble_models",
                          "num_ensemble_models_deposited",
                          "ensemble_precision_value"]) as l:
            for e in self.ensembles:
                l.write(ensemble_id=e.id, ensemble_name=e.name,
                        post_process_id=e.postproc.id,
                        model_group_id=e.model_group.id,
                        ensemble_clustering_feature=e.feature,
                        num_ensemble_models=e.num_models,
                        num_ensemble_models_deposited=e.num_deposit,
                        ensemble_precision_value=e.precision)

class DensityDumper(Dumper):
    """Output localization densities for ensembles"""

    def __init__(self, simo):
        super(DensityDumper, self).__init__(simo)
        self.ensembles = []
        self.output = IMP.pmi.output.Output()

    def add(self, ensemble):
        self.ensembles.append(ensemble)

    def get_gmm(self, ensemble, component):
        return ensemble.localization_density.get(component, [])

    def dump(self, writer):
        with writer.loop("_ihm_gaussian_obj_ensemble",
                         ["ordinal_id", "entity_id", "seq_id_begin",
                          "seq_id_end", "asym_id", "mean_Cartn_x",
                          "mean_Cartn_y", "mean_Cartn_z", "weight",
                          "covariance_matrix[1][1]", "covariance_matrix[1][2]",
                          "covariance_matrix[1][3]", "covariance_matrix[2][1]",
                          "covariance_matrix[2][2]", "covariance_matrix[2][3]",
                          "covariance_matrix[3][1]", "covariance_matrix[3][2]",
                          "covariance_matrix[3][3]", "ensemble_id"]) as l:
            ordinal = 1
            for ensemble in self.ensembles:
                for comp in self.simo.all_modeled_components:
                    entity = self.simo.entities[comp]
                    lenseq = len(entity.sequence)
                    chain_id = self.simo.get_chain_for_component(comp,
                                                                 self.output)
                    for g in self.get_gmm(ensemble, comp):
                        shape = IMP.core.Gaussian(g).get_gaussian()
                        weight = IMP.atom.Mass(g).get_mass()
                        mean = shape.get_center()
                        cv = IMP.algebra.get_covariance(shape)

                        l.write(ordinal_id=ordinal, entity_id=entity.id,
                            seq_id_begin=1, seq_id_end=lenseq,
                            asym_id=chain_id, mean_Cartn_x=mean[0],
                            mean_Cartn_y=mean[1], mean_Cartn_z=mean[2],
                            weight=weight, ensemble_id=ensemble.id,
                            covariance_matrix11=cv[0][0],
                            covariance_matrix12=cv[0][1],
                            covariance_matrix13=cv[0][2],
                            covariance_matrix21=cv[1][0],
                            covariance_matrix22=cv[1][1],
                            covariance_matrix23=cv[1][2],
                            covariance_matrix31=cv[2][0],
                            covariance_matrix32=cv[2][1],
                            covariance_matrix33=cv[2][2])
                        ordinal += 1

class Entity(object):
    """Represent a CIF entity (a chain with a unique sequence)"""
    def __init__(self, seq, first_component):
        self.sequence = seq
        self.first_component = first_component
        # Use the name of the first component as the description
        self.description = first_component

class _EntityMapper(dict):
    """Handle mapping from IMP components to CIF entities.
       Multiple components may map to the same entity if they share sequence."""
    def __init__(self):
        super(_EntityMapper, self).__init__()
        self._sequence_dict = {}
        self._entities = []

    def add(self, component_name, sequence):
        if sequence not in self._sequence_dict:
            entity = Entity(sequence, component_name)
            self._entities.append(entity)
            entity.id = len(self._entities)
            self._sequence_dict[sequence] = entity
        self[component_name] = self._sequence_dict[sequence]

    def get_all(self):
        """Yield all entities"""
        return self._entities


class RestraintDataset(object):
    """Wrapper around a dataset associated with a restraint.
       This is needed because we need to delay access to the dataset
       in case the writer of the PMI script overrides or changes it
       after creating the restraint."""
    def __init__(self, restraint, num, allow_duplicates):
        self.restraint = restraint
        self.num, self.allow_duplicates = num, allow_duplicates
        self.__dataset = None
    def __get_dataset(self):
        if self.__dataset:
            return self.__dataset
        if self.num is not None:
            d = copy.deepcopy(self.restraint.datasets[self.num])
        else:
            d = copy.deepcopy(self.restraint.dataset)
        if self.allow_duplicates:
            d.location.allow_duplicates = True
        # Don't copy again next time we access self.dataset
        self.__dataset = d
        return d
    dataset = property(__get_dataset)


class ProtocolOutput(IMP.pmi.output.ProtocolOutput):
    def __init__(self, fh):
        self._cif_writer = CifWriter(fh)
        self.entities = _EntityMapper()
        self.chains = {}
        self._all_components = {}
        self.model_groups = []
        self.default_model_group = None
        self.sequence_dict = {}
        self.all_modeled_components = []
        self.model_repr_dump = ModelRepresentationDumper(self)
        self.cross_link_dump = CrossLinkDumper(self)
        self.em2d_dump = EM2DDumper(self)
        self.em3d_dump = EM3DDumper(self)
        self.model_prot_dump = ModelProtocolDumper(self)
        self.dataset_dump = DatasetDumper(self)
        self.starting_model_dump = StartingModelDumper(self)
        self.assembly_dump = AssemblyDumper(self)

        # The assembly of all known components.
        self.complete_assembly = Assembly()
        self.assembly_dump.add(self.complete_assembly)

        # The assembly of all components modeled by IMP
        # This may be smaller than the complete assembly.
        self.modeled_assembly = self.complete_assembly

        self.model_dump = ModelDumper(self)
        self.model_repr_dump.starting_model \
                    = self.starting_model_dump.starting_model
        self.software_dump = SoftwareDumper(self)
        self.post_process_dump = PostProcessDumper(self)
        self.ensemble_dump = EnsembleDumper(self)
        self.density_dump = DensityDumper(self)

        # Some dumpers add per-model information; give them a pointer to
        # the model list
        self.cross_link_dump.models = self.model_dump.models

        self._dumpers = [EntryDumper(self), # should always be first
                         AuditAuthorDumper(self),
                         self.software_dump, CitationDumper(self),
                         ChemCompDumper(self),
                         EntityDumper(self),
                         EntityPolyDumper(self), EntityPolySeqDumper(self),
                         StructAsymDumper(self),
                         self.assembly_dump,
                         self.model_repr_dump, self.dataset_dump,
                         self.cross_link_dump,
                         self.em2d_dump, self.em3d_dump,
                         self.starting_model_dump,
                         # todo: detect atomic models and emit struct_conf
                         #StructConfDumper(self),
                         self.model_prot_dump, self.post_process_dump,
                         self.ensemble_dump, self.density_dump, self.model_dump]

    def get_chain_for_component(self, name, output):
        """Get the chain ID for a component, if any."""
        # todo: handle multiple copies
        if name in self.chains:
            chain = self.chains[name]
            return output.chainids[chain]
        else:
            # A non-modeled component doesn't have a chain ID
            return CifWriter.omitted

    def create_component(self, name, modeled):
        self._all_components[name] = None
        if modeled:
            self.all_modeled_components.append(name)
            self.chains[name] = len(self.chains)
            if self.modeled_assembly is not self.complete_assembly:
                self.modeled_assembly.append(name)
        elif self.complete_assembly is self.modeled_assembly:
            # If this component is not modeled, we need to start tracking
            # the complete and modeled assemblies separately
            self.modeled_assembly = Assembly(self.complete_assembly)
            self.assembly_dump.add(self.modeled_assembly)
        self.complete_assembly.append(name)

    def add_component_sequence(self, name, seq):
        self.sequence_dict[name] = seq
        self.entities.add(name, seq)

    def flush(self):
        for dumper in self._dumpers:
            dumper.finalize()
        for dumper in self._dumpers:
            dumper.dump(self._cif_writer)

    def add_pdb_element(self, name, start, end, offset, pdbname, chain, hier):
        p = _PDBFragment(self.m, name, start, end, offset, pdbname, chain, hier)
        self.model_repr_dump.add_fragment(p)
        self.starting_model_dump.add_pdb_fragment(p)

    def add_bead_element(self, name, start, end, num, hier):
        b = _BeadsFragment(self.m, name, start, end, num, hier)
        self.model_repr_dump.add_fragment(b)

    def get_restraint_dataset(self, r, num=None, allow_duplicates=False):
        """Get a wrapper object for the dataset used by this restraint.
           This is needed because the restraint's dataset may be changed
           in the PMI script before we get to use it."""
        rs = RestraintDataset(r, num, allow_duplicates)
        self.dataset_dump.add(rs)
        return rs

    def add_experimental_cross_link(self, r1, c1, r2, c2, label, length,
                                    dataset):
        if c1 not in self._all_components or c2 not in self._all_components:
            # Crosslink refers to a component we didn't model
            # As a quick hack, just ignore it.
            # todo: need to add an entity for this anyway (so will need the
            # sequence, and add to struct_assembly)
            return None
        xl = ExperimentalCrossLink(r1, c1, r2, c2, label, length, dataset)
        self.cross_link_dump.add_experimental(xl)
        return xl

    def add_cross_link(self, ex_xl, p1, p2, sigma1, sigma2, psi):
        self.cross_link_dump.add(CrossLink(ex_xl, p1, p2, sigma1, sigma2, psi))

    def add_replica_exchange(self, rex):
        # todo: allow for metadata to say how many replicas were used in the
        # actual experiment, and how many independent runs were carried out
        # (use these as multipliers to get the correct total number of
        # output models)
        self.model_prot_dump.add(ReplicaExchangeProtocol(rex))

    def add_model_group(self, group):
        self.model_groups.append(group)
        group.id = len(self.model_groups)
        return group

    def add_replica_exchange_analysis(self, rex):
        # todo: add prefilter as an additional postprocess step (complication:
        # we don't know how many models it filtered)
        # todo: postpone rmsf/density localization extraction until after
        # relevant methods are called (currently it is done from the
        # constructor)
        num_models = self.model_prot_dump.get_last_protocol().num_models_end
        pp = ReplicaExchangeAnalysisPostProcess(rex, num_models)
        self.post_process_dump.add(pp)
        for i in range(rex._number_of_clusters):
            group = self.add_model_group(ModelGroup('Cluster %d' % (i + 1)))
            # todo: make # of models to deposit configurable somewhere
            e = ReplicaExchangeAnalysisEnsemble(pp, i, group, 1)
            self.ensemble_dump.add(e)
            self.density_dump.add(e)
            # Add localization density info if available
            for c in self.all_modeled_components:
                e.load_localization_density(self.m, c)
            for stats in e.load_all_models(self):
                m = self.add_model(group)
                m.stats = stats
                # Don't alter original RMF coordinates
                m.geometric_center = [0,0,0]
                # Add RMSF info if available
                for c in self.all_modeled_components:
                    e.load_rmsf(m, c)

    def add_em2d_restraint(self, r, i, resolution, pixel_size,
                           image_resolution, projection_number):
        d = self.get_restraint_dataset(r, i)
        self.em2d_dump.add(EM2DRestraint(d, resolution, pixel_size,
                                         image_resolution, projection_number))

    def add_em3d_restraint(self, target_ps, densities, r):
        # A 3DEM restraint's dataset ID uniquely defines the restraint, so
        # we need to allow duplicates
        d = self.get_restraint_dataset(r, allow_duplicates=True)
        self.em3d_dump.add(EM3DRestraint(self, d, target_ps, densities))

    def add_model(self, group=None):
        if group is None:
            if self.default_model_group is None:
                self.default_model_group \
                         = self.add_model_group(ModelGroup("All models"))
            group = self.default_model_group
        return self.model_dump.add(self.prot,
                                   self.model_prot_dump.get_last_protocol(),
                                   self.modeled_assembly, group)

    def _get_location(self, path, metadata=[]):
        """Get the location where the given file is deposited, or None.
           If a RepositoryFileLocation object is available, return that.
           Otherwise, if a Repository object is available, construct a
           RepositoryFileLocation object from that and return it."""
        for m in metadata + self._metadata:
            if isinstance(m, IMP.pmi.metadata.RepositoryFileLocation):
                return m
            if isinstance(m, IMP.pmi.metadata.Repository):
                return m.get_path(path)
