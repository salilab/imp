"""@namespace IMP.mmcif.metadata

   Classes to extract metadata for various input files.
"""

import os
import ihm.dataset
import ihm.location
import ihm.metadata


class _GMMParser(ihm.metadata.Parser):
    """Extract metadata from an EM density GMM file."""

    def parse_file(self, filename):
        """Extract metadata from `filename`.
           @return a dict with key `dataset` pointing to the GMM file and
           `number_of_gaussians` to the number of GMMs (or None)"""
        loc = ihm.location.InputFileLocation(
            filename, details="Electron microscopy density map, "
                              "represented as a Gaussian Mixture Model (GMM)")
        # A 3DEM restraint's dataset ID uniquely defines the mmCIF restraint,
        # so we need to allow duplicates
        loc._allow_duplicates = True
        d = ihm.dataset.EMDensityDataset(loc)
        ret = {'dataset': d, 'number_of_gaussians': None}

        with open(filename) as fh:
            for line in fh:
                if line.startswith('# data_fn: '):
                    p = ihm.metadata.MRCParser()
                    fn = line[11:].rstrip('\r\n')
                    dataset = p.parse_file(os.path.join(
                                     os.path.dirname(filename), fn))['dataset']
                    ret['dataset'].parents.append(dataset)
                elif line.startswith('# ncenters: '):
                    ret['number_of_gaussians'] = int(line[12:])
        return ret
