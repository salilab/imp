"""Classes to extract metadata from various input files.

   Often input files contain metadata that would be useful to include in
   the mmCIF file, but the metadata is stored in a different way for each
   domain-specific file type. For example, MRC files used for electron
   microscopy maps may contain an EMDB identifier, which the mmCIF file
   can point to in preference to the local file.

   This module provides classes for each file type to extract suitable
   metadata where available.
"""

from . import location, dataset, startmodel, util
from .format import CifWriter

import struct
import json
import sys
import re

# Handle different naming of urllib in Python 2/3
try:
    import urllib.request as urllib2
except ImportError:
    import urllib2

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
        """
        emdb = self._get_emdb(filename)
        if emdb:
            version, details = self._get_emdb_info(emdb)
            l = location.EMDBLocation(emdb, version=version,
                                      details=details if details
                                         else "Electron microscopy density map")
        else:
            l = location.InputFileLocation(filename,
                                      details="Electron microscopy density map")
        return {'dataset': dataset.EMDensityDataset(l)}

    def _get_emdb_info(self, emdb):
        """Query EMDB API and return version & details of a given entry"""
        req = urllib2.Request('https://www.ebi.ac.uk/pdbe/api/emdb/entry/'
                              'summary/%s' % emdb, None, {})
        response = urllib2.urlopen(req)
        contents = json.load(response)
        keys = list(contents.keys())
        info = contents[keys[0]][0]['deposition']
        # JSON values are always Unicode, but on Python 2 we want non-Unicode
        # strings, so convert to ASCII
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


class PDBParser(Parser):
    """Extract metadata (e.g. PDB ID, comparative modeling templates) from a
       PDB file. This handles PDB headers added by the PDB database itself,
       comparative modeling packages such as MODELLER and Phyre2, and also
       some custom headers that can be used to indicate that a file has been
       locally modified in some way."""

    def parse_file(self, filename, chain):
        """Extract metadata. See :meth:`Parser.parse_file` for details.

           :param str filename: the file to extract metadata from.
           :param str chain: the ID of the chain to use from the PDB file.
           :return: a dict with key `dataset` pointing to the PDB dataset;
                    'sources' pointing to a list of model sources (such as
                    comparative model templates) as
                    :class:`ihm.startmodel.Source` objects;
                    'software' pointing to a dict with keys the name of
                    comparative modeling packages used and values their
                    versions;
                    'alignment' pointing to the comparative modeling
                    alignment if available
                    (as a :class:`ihm.location.Location`).
        """
        ret = {'alignment':None, 'software':{}}
        with open(filename) as fh:
            first_line = fh.readline()
            local_file = location.InputFileLocation(filename,
                                          details="Starting model structure")
            if first_line.startswith('HEADER'):
                self._parse_official_pdb(fh, chain, first_line, ret)
            elif first_line.startswith('EXPDTA    DERIVED FROM PDB:'):
                self._parse_derived_from_pdb(fh, chain, first_line, local_file,
                                             ret)
            elif first_line.startswith('EXPDTA    DERIVED FROM COMPARATIVE '
                                       'MODEL, DOI:'):
                self._parse_derived_from_model(fh, chain, first_line,
                                               local_file, ret)
            elif first_line.startswith('EXPDTA    THEORETICAL MODEL, MODELLER'):
                self._parse_modeller_model(fh, chain, first_line, local_file,
                                           filename, ret)
            elif first_line.startswith('REMARK  99  Chain ID :'):
                self._parse_phyre_model(fh, chain, first_line, local_file,
                                        filename, ret)
            else:
                self._parse_unknown_model(fh, chain, first_line, local_file,
                                          filename, ret)
        return ret

    def _parse_official_pdb(self, fh, chain, first_line, ret):
        """Handle a file that's from the official PDB database."""
        version, details, metadata = self._parse_pdb_records(fh, first_line)
        source = startmodel.PDBSource(first_line[62:66].strip(), chain,
                                      metadata)
        l = location.PDBLocation(source.db_code, version, details)
        ret['dataset'] = dataset.PDBDataset(l)
        ret['sources'] = [source]

    def _parse_derived_from_pdb(self, fh, chain, first_line, local_file,
                                ret):
        # Model derived from a PDB structure; treat as a local experimental
        # model with the official PDB as a parent
        local_file.details = self._parse_details(fh)
        db_code = first_line[27:].strip()
        d = dataset.PDBDataset(local_file)
        d.parents.append(dataset.PDBDataset(location.PDBLocation(db_code)))
        ret['dataset'] = d
        ret['sources'] = [startmodel.UnknownSource(d, chain)]

    def _parse_derived_from_model(self, fh, chain, first_line, local_file,
                                  ret):
        # Model derived from a comparative model; link back to the original
        # model as a parent
        local_file.details = self._parse_details(fh)
        d = dataset.ComparativeModelDataset(local_file)
        repo = location.Repository(doi=first_line[46:].strip())
        # todo: better specify an unknown path
        orig_loc = location.InputFileLocation(repo=repo, path='.',
                          details="Starting comparative model structure")
        d.parents.append(dataset.ComparativeModelDataset(orig_loc))
        ret['dataset'] = d
        ret['sources'] = [startmodel.UnknownSource(d, chain)]

    def _parse_modeller_model(self, fh, chain, first_line, local_file,
                              filename, ret):
        # todo: handle this more cleanly (generate Software object?)
        ret['software']['modeller'] = first_line[38:].split(' ', 1)
        self._handle_comparative_model(local_file, filename, chain, ret)

    def _parse_phyre_model(self, fh, chain, first_line, local_file,
                           filename, ret):
        # Model generated by Phyre2
        # todo: extract Modeller-like template info for Phyre models
        ret['software']['phyre2'] = CifWriter.unknown
        self._handle_comparative_model(local_file, filename, chain, ret)

    def _parse_unknown_model(self, fh, chain, first_line, local_file, filename,
                             ret):
        # todo: revisit assumption that all unknown source PDBs are
        # comparative models
        self._handle_comparative_model(local_file, filename, chain, ret)

    def _handle_comparative_model(self, local_file, pdbname, chain, ret):
        d = dataset.ComparativeModelDataset(local_file)
        ret['dataset'] = d
        templates, alnfile = self.get_templates(pdbname, ret)
        if alnfile:
            ret['alignment'] = location.InputFileLocation(alnfile,
                                    details="Alignment for starting "
                                            "comparative model")
        if templates:
            ret['sources'] = templates
        else:
            ret['sources'] = [startmodel.UnknownSource(ret['dataset'], chain)]

    def get_templates(self, pdbname, ret):
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
                              util._get_relative_path(pdbname, m.group(2))
                m = alnfilere.match(line)
                if m:
                    # Path to alignment is relative to that of the PDB file
                    alnfile = util._get_relative_path(pdbname, m.group(1))
                m = tmpre.match(line)
                if m:
                    templates.append(startmodel.TemplateSource(m.group(1),
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
                l = location.InputFileLocation(fname,
                                 details="Template for comparative modeling")
            else:
                l = location.PDBLocation(t.tm_db_code)
            d = dataset.PDBDataset(l)
            t.tm_dataset = d
            ret['dataset'].parents.append(d)

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
                metadata.append(startmodel.PDBHelix(line))
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
