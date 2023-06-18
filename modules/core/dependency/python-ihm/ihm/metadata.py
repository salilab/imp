"""Classes to extract metadata from various input files.

   Often input files contain metadata that would be useful to include in
   the mmCIF file, but the metadata is stored in a different way for each
   domain-specific file type. For example, MRC files used for electron
   microscopy maps may contain an EMDB identifier, which the mmCIF file
   can point to in preference to the local file.

   This module provides classes for each file type to extract suitable
   metadata where available.
"""

import ihm
from . import location, dataset, startmodel, util
from .startmodel import SequenceIdentityDenominator
import ihm.source
import ihm.citations
import ihm.reader
import ihm.format

import operator
import struct
import json
import warnings
import sys
import re

# Handle different naming of urllib in Python 2/3
try:
    import urllib.request
    import urllib.error
except ImportError:
    class MockUrlLib(object):
        pass
    urllib = MockUrlLib()
    urllib.request = urllib.error = __import__('urllib2')


class Parser(object):
    """Base class for all metadata parsers."""

    def parse_file(self, filename):
        """Extract metadata from the given file.

           :param str filename: the file to extract metadata from.
           :return: a dict with extracted metadata (generally including
                    a :class:`~ihm.dataset.Dataset`)."""
        pass


class MRCParser(Parser):
    """Extract metadata from an EM density map (MRC file)."""

    def parse_file(self, filename):
        """Extract metadata. See :meth:`Parser.parse_file` for details.

           :return: a dict with key `dataset` pointing to the density map,
                    as an EMDB entry if the file contains EMDB headers,
                    otherwise to the file itself.

           If the file turns out to be an EMDB entry, this will also query
           the EMDB web API (if available) to extract version information
           and details for the dataset.
        """
        emdb = self._get_emdb(filename)
        if emdb:
            loc = _ParsedEMDBLocation(emdb)
        else:
            loc = location.InputFileLocation(
                filename, details="Electron microscopy density map")
        return {'dataset': dataset.EMDensityDataset(loc)}

    def _get_emdb(self, filename):
        """Return the EMDB id of the file, or None."""
        r = re.compile(b'EMDATABANK\\.org.*(EMD\\-\\d+)')
        with open(filename, 'rb') as fh:
            fh.seek(220)  # Offset of number of labels
            num_labels_raw = fh.read(4)
            # Number of labels in MRC is usually a very small number, so it's
            # very likely to be the smaller of the big-endian and little-endian
            # interpretations of this field
            num_labels_big, = struct.unpack_from('>i', num_labels_raw)
            num_labels_little, = struct.unpack_from('<i', num_labels_raw)
            num_labels = min(num_labels_big, num_labels_little)
            for i in range(num_labels):
                label = fh.read(80).strip()
                m = r.search(label)
                if m:
                    if sys.version_info[0] < 3:
                        return m.group(1)
                    else:
                        return m.group(1).decode('ascii')


class _ParsedEMDBLocation(location.EMDBLocation):
    """Like an EMDBLocation, but looks up version and details from EMDB
       when they are requested (unless they are set to other values)."""
    def __init__(self, emdb):
        self.__emdb_info = None
        super(_ParsedEMDBLocation, self).__init__(
            db_code=emdb, version=None, details=None)
        self.__emdb_info = None

    def __get_version(self):
        self._get_emdb_info()
        return self.__emdb_info[0]

    def __set_version(self, val):
        if self.__emdb_info is None:
            self.__emdb_info = [None, None]
        self.__emdb_info[0] = val

    def __get_details(self):
        self._get_emdb_info()
        return self.__emdb_info[1] or "Electron microscopy density map"

    def __set_details(self, val):
        if self.__emdb_info is None:
            self.__emdb_info = [None, None]
        self.__emdb_info[1] = val

    def _get_emdb_info(self):
        """Query EMDB API and get version & details of a given entry"""
        if self.__emdb_info is not None:
            return
        req = urllib.request.Request(
            'https://www.ebi.ac.uk/pdbe/api/emdb/entry/summary/%s'
            % self.access_code, None, {})
        try:
            response = urllib.request.urlopen(req, timeout=10)
        except urllib.error.URLError as err:
            warnings.warn("EMDB API query failed; using default metadata "
                          "for MRC file; %s" % str(err))
            self.__emdb_info = [None, None]
            return
        contents = json.load(response)
        keys = list(contents.keys())
        info = contents[keys[0]][0]['deposition']
        # JSON values are always Unicode, but on Python 2 we want non-Unicode
        # strings, so convert to ASCII
        if sys.version_info[0] < 3:
            self.__emdb_info = [info['map_release_date'].encode('ascii'),
                                info['title'].encode('ascii')]
        else:
            self.__emdb_info = [info['map_release_date'], info['title']]

    version = property(__get_version, __set_version)
    details = property(__get_details, __set_details)


def _get_swiss_model_metadata(filename):
    """Extract and return metadata from SWISS-MODEL PDB REMARK headers"""
    meta = {}
    with open(filename) as fh:
        in_header = None
        for line in fh:
            if line.startswith('ATOM'):
                break
            if line.startswith('REMARK   3 '):
                if line.startswith('REMARK   3 MODEL INFORMATION'):
                    in_header = {}
                    meta['info'] = in_header
                elif line.startswith('REMARK   3 TEMPLATE'):
                    in_header = {}
                    meta[line[11:].rstrip('\r\n  ')] = in_header
                elif in_header is not None:
                    linedata = line[11:].rstrip('\r\n ')
                    if linedata:
                        key, val = linedata.split(None, 1)
                        if key == 'ALN':
                            chain, tpltgt, seq = val.split()
                            key = (chain, tpltgt)
                            in_header[key] = in_header.get(key, '') + seq
                        elif key in ('CHAIN', 'MMCIF', 'LIGND'):
                            in_header.setdefault(key, []).append(val)
                        else:
                            in_header[key] = val
    return meta


def _parse_seq(seq):
    """Get a primary sequence and its length (without gaps)"""
    return seq, len(seq.replace('-', ''))


def _get_aligned_region(tgt_seq, tmpl_seq):
    """Given two primary sequences, return the range of each that is
       aligned (i.e. from the first aligned residue in both sequences to
       the last)"""
    first = True
    tgt_pos = 0
    tmpl_pos = 0
    start_align = end_align = None
    for tgt, tmpl in zip(tgt_seq, tmpl_seq):
        if tgt != '-':
            tgt_pos += 1
        if tmpl != '-':
            tmpl_pos += 1
            if tgt != '-':
                end_align = (tgt_pos, tmpl_pos)
                if first:
                    start_align = end_align
                    first = False
    if first:
        raise ValueError("Cannot parse empty alignment")
    return (start_align[0], end_align[0]), (start_align[1], end_align[1])


class PDBParser(Parser):
    """Extract metadata (e.g. PDB ID, comparative modeling templates) from a
       PDB file. This handles PDB headers added by the PDB database itself,
       comparative modeling packages such as MODELLER and Phyre2, and also
       some custom headers that can be used to indicate that a file has been
       locally modified in some way.

       See also :class:`CIFParser` for coordinate files in mmCIF format.
    """

    def parse_file(self, filename):
        """Extract metadata. See :meth:`Parser.parse_file` for details.

           :param str filename: the file to extract metadata from.
           :return: a dict with key `dataset` pointing to the PDB dataset;
                    'templates' pointing to a dict with keys the asym (chain)
                    IDs in the PDB file and values the list of comparative
                    model templates used to model that chain as
                    :class:`ihm.startmodel.Template` objects;
                    'entity_source' pointing to a dict with keys the asym IDs
                    and values :class:`ihm.source.Source` objects;
                    'software' pointing to a list of software used to generate
                    the file (as :class:`ihm.Software` objects);
                    'script' pointing to the script used to generate the
                    file, if any (as :class:`ihm.location.WorkflowFileLocation`
                    objects);
                    'metadata' a list of PDB metadata records.

           This parser looks at PDB headers. Standard PDB database headers are
           recognized, plus some added by common comparative modeling
           packages such as MODELLER and Phyre2, as well as some custom headers
           that can be used to denote that a PDB file is a locally-modified
           version of some other resource. Additional details will be extracted
           from other PDB headers if available, such as ``TITLE`` records.

           If the first line of the file starts with ``HEADER`` then the file
           is assumed to live in the PDB database. For example, the following
           will be interpreted as PDB entry 2HBJ::

               HEADER    HYDROLASE, GENE REGULATION              14-JUN-06   2HBJ

           If the first line starts with ``EXPDTA    DERIVED FROM`` then the
           file is assumed to derive from a given PDB ID or a comparative
           or integrative model available at a given DOI. ``TITLE`` records
           are expected to describe the nature of the transformation::

               EXPDTA    DERIVED FROM PDB:1YKH
               EXPDTA    DERIVED FROM COMPARATIVE MODEL, DOI:10.1093/nar/gkt704
               EXPDTA    DERIVED FROM INTEGRATIVE MODEL, DOI:10.1016/j.str.2017.01.006

           A first line starting with ``REMARK  99  Chain ID :`` is assumed to
           be a model generated by Phyre2. Template information can be added
           using Modeller-style headers, as below, if desired.

           A first line starting with ``EXPDTA    THEORETICAL MODEL, MODELLER``
           is assumed to be a model generated by Modeller. Headers generated
           by modern versions of Modeller are parsed to extract information
           about the comparative modeling script, plus the templates used and
           their alignment.
           Templates named ``1abcX`` or ``1abcX_N`` are assumed to be
           structures deposited in PDB (in this case, chain X in
           structure 1ABC).
           A custom ``TEMPLATE PATH`` header can be used to point to templates
           that are not deposited in the PDB database. For example, the model
           below is assumed to be constructed using templates from PDB codes
           3JRO and 3F3F, plus another template in ``my_custom_pdb_file.pdb``,
           and the given alignment::

               EXPDTA    THEORETICAL MODEL, MODELLER 9.18 2017/02/10 22:21:34
               REMARK   6 ALIGNMENT: modeller_model.ali
               REMARK   6 SCRIPT: model-default.py
               REMARK   6 TEMPLATE PATH custom1 ../inputs/my_custom_pdb_file.pdb
               REMARK   6 TEMPLATE: 3jroC 33:C - 424:C MODELS 33:A - 424:A AT 100.0%
               REMARK   6 TEMPLATE: 3f3fG 482:G - 551:G MODELS 429:A - 488:A AT 10.0%
               REMARK   6 TEMPLATE: custom1 9:A - 352:A MODELS 80:A - 414:A AT 32.0%

           A first line starting with ``TITLE     SWISS-MODEL SERVER``
           is assumed to be a model generated by SWISS-MODEL, and information
           about the template(s) is extracted from ``REMARK    3`` records.
        """  # noqa:  E501
        ret = {'templates': {}, 'software': [], 'metadata': [], 'script': None,
               'entity_source': {}}
        with open(filename) as fh:
            first_line = fh.readline()
            local_file = location.InputFileLocation(
                filename, details="Starting model structure")
            if first_line.startswith('HEADER'):
                self._parse_official_pdb(fh, first_line, ret)
            elif first_line.startswith('EXPDTA    DERIVED FROM PDB:'):
                self._parse_derived_from_pdb(fh, first_line, local_file,
                                             ret)
            elif first_line.startswith('EXPDTA    DERIVED FROM COMPARATIVE '
                                       'MODEL, DOI:'):
                self._parse_derived_from_comp_model(fh, first_line, local_file,
                                                    ret)
            elif first_line.startswith('EXPDTA    DERIVED FROM INTEGRATIVE '
                                       'MODEL, DOI:'):
                self._parse_derived_from_int_model(fh, first_line, local_file,
                                                   ret)
            elif first_line.startswith(
                    'EXPDTA    THEORETICAL MODEL, MODELLER'):
                self._parse_modeller_model(fh, first_line, local_file,
                                           filename, ret)
            elif first_line.startswith('REMARK  99  Chain ID :'):
                self._parse_phyre_model(fh, first_line, local_file,
                                        filename, ret)
            elif first_line.startswith('TITLE     SWISS-MODEL SERVER'):
                self._parse_swiss_model(fh, first_line, local_file,
                                        filename, ret)
            else:
                self._parse_unknown_model(fh, first_line, local_file,
                                          filename, ret)
        return ret

    def _parse_official_pdb(self, fh, first_line, ret):
        """Handle a file that's from the official PDB database."""
        version, details, metadata, entity_source \
            = self._parse_pdb_records(fh, first_line)
        loc = location.PDBLocation(first_line[62:66].strip(), version, details)
        ret['entity_source'] = entity_source
        ret['metadata'] = metadata
        ret['dataset'] = dataset.PDBDataset(loc, details=loc.details)

    def _parse_derived_from_pdb(self, fh, first_line, local_file, ret):
        # Model derived from a PDB structure; treat as a local experimental
        # model with the official PDB as a parent
        local_file.details = self._parse_details(fh)
        db_code = first_line[27:].strip()
        d = dataset.PDBDataset(local_file, details=local_file.details)
        d.parents.append(dataset.PDBDataset(location.PDBLocation(db_code)))
        ret['dataset'] = d

    def _parse_derived_from_comp_model(self, fh, first_line, local_file, ret):
        """Model derived from a comparative model; link back to the original
           model as a parent"""
        self._parse_derived_from_model(
            fh, first_line, local_file, ret, dataset.ComparativeModelDataset,
            'comparative')

    def _parse_derived_from_int_model(self, fh, first_line, local_file, ret):
        """Model derived from an integrative model; link back to the original
           model as a parent"""
        self._parse_derived_from_model(
            fh, first_line, local_file, ret, dataset.IntegrativeModelDataset,
            'integrative')

    def _parse_derived_from_model(self, fh, first_line, local_file, ret,
                                  dataset_class, model_type):
        local_file.details = self._parse_details(fh)
        d = dataset_class(local_file)
        repo = location.Repository(doi=first_line[46:].strip())
        # todo: better specify an unknown path
        orig_loc = location.InputFileLocation(
            repo=repo, path='.',
            details="Starting %s model structure" % model_type)
        d.parents.append(dataset_class(orig_loc))
        ret['dataset'] = d

    def _parse_modeller_model(self, fh, first_line, local_file, filename, ret):
        version, date = first_line[38:].rstrip('\r\n').split(' ', 1)
        s = ihm.Software(
            name='MODELLER', classification='comparative modeling',
            description='Comparative modeling by satisfaction '
                        'of spatial restraints, build ' + date,
            location='https://salilab.org/modeller/',
            version=version,
            citation=ihm.citations.modeller)
        ret['software'].append(s)
        self._handle_comparative_model(local_file, filename, ret)

    def _parse_phyre_model(self, fh, first_line, local_file, filename, ret):
        # Model generated by Phyre2
        s = ihm.Software(
            name='Phyre2', classification='protein homology modeling',
            description='Protein Homology/analogY Recognition '
                        'Engine V 2.0',
            version='2.0', location='http://www.sbg.bio.ic.ac.uk/~phyre2/',
            citation=ihm.citations.phyre2)
        ret['software'].append(s)
        self._handle_comparative_model(local_file, filename, ret)

    def _parse_swiss_model(self, fh, first_line, local_file, filename, ret):
        # Model generated by SWISS-MODEL
        meta = _get_swiss_model_metadata(filename)
        s = ihm.Software(
            name='SWISS-MODEL', classification='protein homology modeling',
            description='SWISS-MODEL: homology modelling of protein '
                        'structures and complexes, using %s engine'
                        % meta.get('info', {}).get('ENGIN', 'unknown'),
            version=meta.get('info', {}).get('VERSN', ihm.unknown),
            location='https://swissmodel.expasy.org/',
            citation=ihm.citations.swiss_model)
        ret['software'].append(s)
        comp_model_ds = dataset.ComparativeModelDataset(local_file)
        ret['dataset'] = comp_model_ds

        ret['templates'] = self._add_swiss_model_templates(
            local_file, meta, comp_model_ds, ret)

    def _add_swiss_model_templates(self, local_file, meta, comp_model_ds, ret):
        """Add template information extracted from SWISS-MODEL PDB metadata"""
        ret_templates = {}
        templates = [v for k, v in sorted(((k, v) for k, v in meta.items()
                                          if k.startswith('TEMPLATE')),
                                          key=operator.itemgetter(0))]
        for t in templates:
            loc = location.PDBLocation(t['PDBID'])
            d = dataset.PDBDataset(loc)
            # Make the comparative model dataset derive from the template's
            comp_model_ds.parents.append(d)
            for chain in t['MMCIF']:
                # todo: check we're using the right chain ID and that target
                # and template chain IDs really are always the same
                offset = int(t[chain, 'OFF'])
                tgt_seq, tgt_len = _parse_seq(t[chain, 'TRG'])
                tmpl_seq, tmpl_len = _parse_seq(t[chain, 'TPL'])
                tgt_rng, tmpl_rng = _get_aligned_region(tgt_seq, tmpl_seq)

                # apply offset
                tmpl_rng = (tmpl_rng[0] + offset, tmpl_rng[1] + offset)

                seq_id = float(t['SID'])
                seq_id = startmodel.SequenceIdentity(
                    float(t['SID']),
                    SequenceIdentityDenominator.NUM_ALIGNED_WITHOUT_GAPS)
                tmpl = startmodel.Template(
                    dataset=d, asym_id=chain, seq_id_range=tgt_rng,
                    template_seq_id_range=tmpl_rng, sequence_identity=seq_id,
                    alignment_file=local_file)
                ret_templates[chain] = [tmpl]
        return ret_templates

    def _parse_unknown_model(self, fh, first_line, local_file, filename, ret):
        # todo: revisit assumption that all unknown source PDBs are
        # comparative models
        self._handle_comparative_model(local_file, filename, ret)

    def _handle_comparative_model(self, local_file, pdbname, ret):
        d = dataset.ComparativeModelDataset(local_file)
        ret['dataset'] = d
        ret['templates'], ret['script'] \
            = self._get_templates_script(pdbname, d)

    def _get_templates_script(self, pdbname, target_dataset):
        template_path_map = {}
        alnfile = None
        script = None
        alnfilere = re.compile(r'REMARK   6 ALIGNMENT: (\S+)')
        scriptre = re.compile(r'REMARK   6 SCRIPT: (\S+)')
        tmppathre = re.compile(r'REMARK   6 TEMPLATE PATH (\S+) (\S+)')
        tmpre = re.compile(r'REMARK   6 TEMPLATE: '
                           r'(\S+) (\S+):(\S+) \- (\S+):\S+ '
                           r'MODELS (\S+):(\S+) \- (\S+):\S+ AT (\S+)%')
        template_info = []

        with open(pdbname) as fh:
            for line in fh:
                if line.startswith('ATOM'):  # Read only the header
                    break
                m = tmppathre.match(line)
                if m:
                    template_path_map[m.group(1)] = \
                        util._get_relative_path(pdbname, m.group(2))
                m = alnfilere.match(line)
                if m:
                    # Path to alignment is relative to that of the PDB file
                    fname = util._get_relative_path(pdbname, m.group(1))
                    alnfile = location.InputFileLocation(
                        fname,
                        details="Alignment for starting comparative model")
                m = scriptre.match(line)
                if m:
                    # Path to script is relative to that of the PDB file
                    fname = util._get_relative_path(pdbname, m.group(1))
                    script = location.WorkflowFileLocation(
                        fname, details="Script for starting comparative model")
                m = tmpre.match(line)
                if m:
                    template_info.append(m)

        templates = {}
        for t in template_info:
            chain, template = self._handle_template(t, template_path_map,
                                                    target_dataset, alnfile)
            if chain not in templates:
                templates[chain] = []
            templates[chain].append(template)
        # Sort templates by starting residue, then ending residue
        for chain in templates.keys():
            templates[chain] = sorted(templates[chain],
                                      key=operator.attrgetter('seq_id_range'))
        return templates, script

    def _handle_template(self, info, template_path_map, target_dataset,
                         alnfile):
        """Create a Template object from Modeller PDB header information."""
        template_code = info.group(1)
        template_seq_id_range = (int(info.group(2)), int(info.group(4)))
        template_asym_id = info.group(3)
        seq_id_range = (int(info.group(5)), int(info.group(7)))
        target_asym_id = info.group(6)
        sequence_identity = startmodel.SequenceIdentity(
            float(info.group(8)), SequenceIdentityDenominator.SHORTER_LENGTH)

        # Assume a code of 1abc, 1abc_N, 1abcX, or 1abcX_N refers
        # to a real PDB structure
        m = re.match(r'(\d[a-zA-Z0-9]{3})[a-zA-Z]?(_.*)?$', template_code)
        if m:
            template_db_code = m.group(1).upper()
            loc = location.PDBLocation(template_db_code)
        else:
            # Otherwise, look up the PDB file in TEMPLATE PATH remarks
            fname = template_path_map[template_code]
            loc = location.InputFileLocation(
                fname, details="Template for comparative modeling")
        d = dataset.PDBDataset(loc, details=loc.details)

        # Make the comparative model dataset derive from the template's
        target_dataset.parents.append(d)

        return (target_asym_id,
                startmodel.Template(
                    dataset=d, asym_id=template_asym_id,
                    seq_id_range=seq_id_range,
                    template_seq_id_range=template_seq_id_range,
                    sequence_identity=sequence_identity,
                    alignment_file=alnfile))

    def _parse_pdb_records(self, fh, first_line):
        """Extract information from an official PDB"""
        metadata = []
        details = ''
        compnd = ''
        source = ''
        for line in fh:
            if line.startswith('TITLE'):
                details += line[10:].rstrip()
            elif line.startswith('COMPND'):
                compnd += line[10:].rstrip()
            elif line.startswith('SOURCE'):
                source += line[10:].rstrip()
            elif line.startswith('HELIX'):
                metadata.append(startmodel.PDBHelix(line))
        return (first_line[50:59].strip(),
                details if details else None, metadata,
                self._make_entity_source(compnd, source))

    def _make_one_entity_source(self, compnd, source):
        """Make a single ihm.source.Source object"""
        def make_from_source(cls):
            return cls(scientific_name=source.get('ORGANISM_SCIENTIFIC'),
                       common_name=source.get('ORGANISM_COMMON'),
                       strain=source.get('STRAIN'),
                       ncbi_taxonomy_id=source.get('ORGANISM_TAXID'))
        if compnd.get('ENGINEERED', None) == 'YES':
            gene = make_from_source(ihm.source.Details)
            host = ihm.source.Details(
                scientific_name=source.get('EXPRESSION_SYSTEM'),
                common_name=source.get('EXPRESSION_SYSTEM_COMMON'),
                strain=source.get('EXPRESSION_SYSTEM_STRAIN'),
                ncbi_taxonomy_id=source.get('EXPRESSION_SYSTEM_TAXID'))
            return ihm.source.Manipulated(gene=gene, host=host)
        else:
            if source.get('SYNTHETIC', None) == 'YES':
                cls = ihm.source.Synthetic
            else:
                cls = ihm.source.Natural
            return make_from_source(cls)

    def _make_entity_source(self, compnd, source):
        """Make ihm.source.Source objects given PDB COMPND and SOURCE lines"""
        entity_source = {}
        # Convert each string into dict of mol_id vs keys
        compnd = self._parse_pdb_mol_id(compnd)
        source = self._parse_pdb_mol_id(source)
        for mol_id, c in compnd.items():
            if mol_id in source and 'CHAIN' in c:
                s = self._make_one_entity_source(c, source[mol_id])
                for chain in c['CHAIN'].split(','):
                    entity_source[chain.strip()] = s
        return entity_source

    def _parse_pdb_mol_id(self, txt):
        """Convert text COMPND or SOURCE records to a dict of mol_id vs keys"""
        d = {}
        mol_id = None
        for pair in txt.split(';'):
            spl = pair.split(':')
            if len(spl) == 2:
                key = spl[0].upper().strip()
                val = spl[1].upper().strip()
                if key == 'MOL_ID':
                    mol_id = d[val] = {}
                elif mol_id is not None:
                    mol_id[key] = val
        return d

    def _parse_details(self, fh):
        """Extract TITLE records from a PDB file"""
        details = ''
        for line in fh:
            if line.startswith('TITLE'):
                details += line[10:].rstrip()
            elif line.startswith('ATOM'):
                break
        return details


class _Database2Handler(ihm.reader.Handler):
    def __init__(self, m):
        self.m = m

    def __call__(self, database_id, database_code):
        self.m['db'][database_id.upper()] = database_code


class _StructHandler(ihm.reader.Handler):
    def __init__(self, m):
        self.m = m

    def __call__(self, title):
        self.m['title'] = title


class _AuditRevHistHandler(ihm.reader.Handler):
    def __init__(self, m):
        self.m = m

    def __call__(self, revision_date):
        self.m['version'] = revision_date


class CIFParser(Parser):
    """Extract metadata from an mmCIF file. Currently, this does not handle
       information from comparative modeling packages such as MODELLER
       (see :class:`PDBParser`).

       See also :class:`PDBParser` for coordinate files in legacy PDB format.
    """
    dbmap = {'PDB': (location.PDBLocation, dataset.PDBDataset),
             'MODELARCHIVE': (location.ModelArchiveLocation,
                              dataset.DeNovoModelDataset)}

    def parse_file(self, filename):
        """Extract metadata. See :meth:`Parser.parse_file` for details.

           :param str filename: the file to extract metadata from.
           :return: a dict with key `dataset` pointing to the coordinate file,
                    as an entry in the PDB or Model Archive databases if the
                    file contains appropriate headers, otherwise to the
                    file itself.
        """
        dset = self._get_dataset(filename)
        return {'dataset': dset}

    def _get_dataset(self, filename):
        m = {'db': {}, 'title': 'Starting model structure'}
        with open(filename) as fh:
            dbh = _Database2Handler(m)
            structh = _StructHandler(m)
            arevhisth = _AuditRevHistHandler(m)
            r = ihm.format.CifReader(
                fh, {'_database_2': dbh, '_struct': structh,
                     '_pdbx_audit_revision_history': arevhisth})
            r.read_file()
        # Check for known databases. Note that if a file is in multiple
        # databases, we currently return one "at random"
        for dbid, dbcode in m['db'].items():
            if dbid in self.dbmap:
                loccls, dsetcls = self.dbmap[dbid]
                loc = loccls(db_code=dbcode, version=m.get('version'),
                             details=m['title'])
                return dsetcls(location=loc, details=loc.details)
        # Fall back to a local file
        loc = location.InputFileLocation(filename, details=m['title'])
        return dataset.ComparativeModelDataset(
            location=loc, details=loc.details)
