"""@namespace IMP.mmcif.metadata

   Classes to extract metadata for various input files.
"""

import IMP.mmcif.dataset
import re
import struct
import os
import sys
import json
try:
    import urllib.request as urllib2
except ImportError:
    import urllib2

class _MetadataParser(object):
    """Base class for all metadata parsers.
       Call parse_file() to extract metadata into self.dataset, etc."""
    def parse_file(self, filename, system):
        pass


class _PDBHelix(object):
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


class _StartingModelSource(object):
    """Base class for all sources for starting models."""
    pass


class _PDBSource(_StartingModelSource):
    """An experimental PDB file used as part of a starting model"""
    source = 'experimental model'
    db_name = 'PDB'
    sequence_identity = 100.0

    def __init__(self, db_code, chain_id, metadata):
        self.db_code = db_code
        self.chain_id = chain_id
        self.metadata = metadata

    def get_seq_id_range(self, starting_model):
        # Assume the structure covers the entire sequence
        return (starting_model.seq_id_begin, starting_model.seq_id_end)


class _TemplateSource(_StartingModelSource):
    """A PDB file used as a template for a comparative starting model"""
    source = 'comparative model'
    db_name = db_code = None
    tm_dataset = None

    def __init__(self, tm_code, tm_seq_id_begin, tm_seq_id_end, seq_id_begin,
                 chain_id, seq_id_end, seq_id):
        # Assume a code of 1abcX refers to a real PDB structure
        if len(tm_code) == 5:
            self._orig_tm_code = None
            self.tm_db_code = tm_code[:4].upper()
            self.tm_chain_id = tm_code[4]
        else:
            # Otherwise, will need to look up in TEMPLATE PATH remarks
            self._orig_tm_code = tm_code
            self.tm_db_code = None
            self.tm_chain_id = tm_code[-1]
        self.sequence_identity = seq_id
        self.tm_seq_id_begin = tm_seq_id_begin
        self.tm_seq_id_end = tm_seq_id_end
        self.chain_id = chain_id
        self._seq_id_begin, self._seq_id_end = seq_id_begin, seq_id_end

    def get_seq_id_range(self, starting_model):
        # The template may cover more than the current starting model
        seq_id_begin = max(starting_model.seq_id_begin, self._seq_id_begin)
        seq_id_end = min(starting_model.seq_id_end, self._seq_id_end)
        return (seq_id_begin, seq_id_end)


class _UnknownSource(_StartingModelSource):
    """Part of a starting model from an unknown source"""
    db_code = None
    db_name = None
    chain_id = None
    sequence_identity = None
    # Map dataset types to starting model sources
    _source_map = {'Comparative model': 'comparative model',
                   'Experimental model': 'experimental model'}

    def __init__(self, dataset, chain):
        self.source = self._source_map[dataset._data_type]
        self.chain_id = chain

    def get_seq_id_range(self, starting_model):
        return (starting_model.seq_id_begin, starting_model.seq_id_end)


class _PDBMetadataParser(_MetadataParser):
    """Extract metadata (e.g. PDB ID, comparative modeling templates) from a
       PDB file."""

    def parse_file(self, filename, chain, system):
        """Extract metadata from `filename`.
           Sets self.dataset to point to the PDB file, self.sources to
           a list of sources (e.g. comparative modeling templates), and
           self.alignment_file to the comparative modeling alignment,
           if available."""
        self.alignment_file = None
        fh = open(filename)
        first_line = fh.readline()
        local_file = IMP.mmcif.dataset.FileLocation(filename,
                                          details="Starting model structure")
        if first_line.startswith('HEADER'):
            self._parse_official_pdb(fh, chain, first_line, system)
        elif first_line.startswith('EXPDTA    DERIVED FROM PDB:'):
            self._parse_derived_from_pdb(fh, chain, first_line, local_file,
                                         system)
        elif first_line.startswith('EXPDTA    DERIVED FROM COMPARATIVE '
                                   'MODEL, DOI:'):
            self._parse_derived_from_model(fh, chain, first_line, local_file,
                                           system)
        elif first_line.startswith('EXPDTA    THEORETICAL MODEL, MODELLER'):
            self._parse_modeller_model(fh, chain, first_line, local_file,
                                       filename, system)
        elif first_line.startswith('REMARK  99  Chain ID :'):
            self._parse_phyre_model(fh, chain, first_line, local_file,
                                    filename, system)
        else:
            self._parse_unknown_model(fh, chain, first_line, local_file,
                                      filename, system)

    def _parse_official_pdb(self, fh, chain, first_line, system):
        """Handle a file that's from the official PDB database."""
        version, details, metadata = self._parse_pdb_records(fh, first_line)
        source = _PDBSource(first_line[62:66].strip(), chain, metadata)
        l = IMP.mmcif.dataset.PDBLocation(source.db_code, version, details)
        d = IMP.mmcif.dataset.PDBDataset(l)
        self.dataset = system.datasets.add(d)
        self.sources = [source]

    def _parse_derived_from_pdb(self, fh, chain, first_line, local_file,
                                system):
        # Model derived from a PDB structure; treat as a local experimental
        # model with the official PDB as a parent
        local_file.details = self._parse_details(fh)
        db_code = first_line[27:].strip()
        d = IMP.mmcif.dataset.PDBDataset(local_file)
        pdb_loc = IMP.mmcif.dataset.PDBLocation(db_code)
        parent = IMP.mmcif.dataset.PDBDataset(pdb_loc)
        d.add_parent(parent)
        self.dataset = system.datasets.add(d)
        self.sources = [_UnknownSource(self.dataset, chain)]

    def _parse_derived_from_model(self, fh, chain, first_line, local_file,
                                  system):
        # Model derived from a comparative model; link back to the original
        # model as a parent
        local_file.details = self._parse_details(fh)
        d = IMP.mmcif.dataset.ComparativeModelDataset(local_file)
        repo = IMP.mmcif.dataset.Repository(doi=first_line[46:].strip())
        # todo: better specify an unknown path
        orig_loc = IMP.mmcif.dataset.FileLocation(repo=repo, path='.',
                          details="Starting comparative model structure")
        parent = IMP.mmcif.dataset.ComparativeModelDataset(orig_loc)
        d.add_parent(parent)
        self.dataset = system.datasets.add(d)
        self.sources = [_UnknownSource(self.dataset, chain)]

    def _parse_modeller_model(self, fh, chain, first_line, local_file,
                              filename, system):
        system._software.set_modeller_used(*first_line[38:].split(' ', 1))
        self._handle_comparative_model(local_file, filename, chain, system)

    def _parse_phyre_model(self, fh, chain, first_line, local_file,
                           filename, system):
        # Model generated by Phyre2
        # todo: extract Modeller-like template info for Phyre models
        system._software.set_phyre2_used()
        self._handle_comparative_model(local_file, filename, chain, system)

    def _parse_unknown_model(self, fh, chain, first_line, local_file, filename,
                             system):
        # todo: revisit assumption that all unknown source PDBs are
        # comparative models
        self._handle_comparative_model(local_file, filename, chain, system)

    def _handle_comparative_model(self, local_file, pdbname, chain, system):
        d = IMP.mmcif.dataset.ComparativeModelDataset(local_file)
        self.dataset = system.datasets.add(d)
        templates, alnfile = self.get_templates(pdbname, system)
        if alnfile:
            self.alignment_file = IMP.mmcif.dataset.FileLocation(alnfile,
                                    details="Alignment for starting "
                                            "comparative model")
            system._external_files.add_input(self.alignment_file)

        if templates:
            self.sources = templates
        else:
            self.sources = [_UnknownSource(self.dataset, chain)]

    def get_templates(self, pdbname, system):
        template_path_map = {}
        templates = []
        alnfile = None
        alnfilere = re.compile('REMARK   6 ALIGNMENT: (\S+)')
        tmppathre = re.compile('REMARK   6 TEMPLATE PATH (\S+) (\S+)')
        tmpre = re.compile('REMARK   6 TEMPLATE: '
                           '(\S+) (\S+):\S+ \- (\S+):\S+ '
                           'MODELS (\S+):(\S+) \- (\S+):\S+ AT (\S+)%')

        with open(pdbname) as fh:
            for line in fh:
                if line.startswith('ATOM'): # Read only the header
                    break
                m = tmppathre.match(line)
                if m:
                    template_path_map[m.group(1)] = \
                              IMP.get_relative_path(pdbname, m.group(2))
                m = alnfilere.match(line)
                if m:
                    # Path to alignment is relative to that of the PDB file
                    alnfile = IMP.get_relative_path(pdbname, m.group(1))
                m = tmpre.match(line)
                if m:
                    templates.append(_TemplateSource(m.group(1),
                                                     int(m.group(2)),
                                                     int(m.group(3)),
                                                     int(m.group(4)),
                                                     m.group(5),
                                                     int(m.group(6)),
                                                     m.group(7)))
        # Add datasets for templates
        for t in templates:
            if t._orig_tm_code:
                fname = template_path_map[t._orig_tm_code]
                l = IMP.mmcif.dataset.FileLocation(fname,
                                 details="Template for comparative modeling")
            else:
                l = IMP.mmcif.dataset.PDBLocation(t.tm_db_code)
            d = IMP.mmcif.dataset.PDBDataset(l)
            d = system.datasets.add(d)
            t.tm_dataset = d
            self.dataset.add_parent(d)

        # Sort by starting residue, then ending residue
        return(sorted(templates,
                      key=lambda x: (x._seq_id_begin, x._seq_id_end)),
               alnfile)

    def _parse_pdb_records(self, fh, first_line):
        """Extract information from an official PDB"""
        metadata = []
        details = ''
        for line in fh:
            if line.startswith('TITLE'):
                details += line[10:].rstrip()
            elif line.startswith('HELIX'):
                metadata.append(_PDBHelix(line))
        return (first_line[50:59].strip(),
                details if details else None, metadata)

    def _parse_details(self, fh):
        """Extract TITLE records from a PDB file"""
        details = ''
        for line in fh:
            if line.startswith('TITLE'):
                details += line[10:].rstrip()
            elif line.startswith('ATOM'):
                break
        return details


class _MRCMetadataParser(_MetadataParser):
    """Extract metadata from an EM density map (MRC file)."""

    def parse_file(self, filename, system):
        """Extract metadata from `filename`.
           Sets self.dataset to point to the MRC file"""
        emdb = self._get_emdb(filename)
        if emdb:
            version, details = self._get_emdb_info(emdb)
            l = IMP.mmcif.dataset.EMDBLocation(emdb, version=version,
                    details=details if details
                            else "Electron microscopy density map")
        else:
            l = IMP.mmcif.dataset.FileLocation(filename,
                    details="Electron microscopy density map")
        d = IMP.mmcif.dataset.EMDensityDataset(l)
        self.dataset = system.datasets.add(d)

    def _get_emdb_info(self, emdb):
        """Query EMDB API and return version & details of a given entry"""
        req = urllib2.Request('https://www.ebi.ac.uk/pdbe/api/emdb/entry/'
                              'summary/%s' % emdb, None, {})
        response = urllib2.urlopen(req)
        contents = json.load(response)
        keys = list(contents.keys())
        info = contents[keys[0]][0]['deposition']
        if sys.version_info[0] < 3:
            return (info['map_release_date'].encode('ascii'),
                    info['title'].encode('ascii'))
        else:
            return info['map_release_date'], info['title']

    def _get_emdb(self, filename):
        """Return the EMDB id of the file, or None."""
        r = re.compile(b'EMDATABANK\.org.*(EMD\-\d+)')
        with open(filename, 'rb') as fh:
            fh.seek(220) # Offset of number of labels
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


class _GMMMetadataParser(_MetadataParser):
    """Extract metadata from an EM density GMM file."""

    def parse_file(self, filename, system):
        """Extract metadata from `filename`.
           Sets self.dataset to point to the GMM file and
           self.number_of_gaussians to the number of GMMs (or None)"""
        l = IMP.mmcif.dataset.FileLocation(filename,
                details="Electron microscopy density map, "
                        "represented as a Gaussian Mixture Model (GMM)")
        # A 3DEM restraint's dataset ID uniquely defines the mmCIF restraint, so
        # we need to allow duplicates
        l._allow_duplicates = True
        d = IMP.mmcif.dataset.EMDensityDataset(l)
        self.dataset = system.datasets.add(d)
        self.number_of_gaussians = None

        with open(filename) as fh:
            for line in fh:
                if line.startswith('# data_fn: '):
                    p = _MRCMetadataParser()
                    fn = line[11:].rstrip('\r\n')
                    p.parse_file(os.path.join(os.path.dirname(filename), fn),
                                 system)
                    self.dataset.add_parent(p.dataset)
                elif line.startswith('# ncenters: '):
                    self.number_of_gaussians = int(line[12:])
