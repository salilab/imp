"""@namespace IMP.mmcif.restraint
   @brief Map IMP restraints to mmCIF categories
"""

import IMP.mmcif.dataset

def _parse_restraint_info(info):
    """Convert RestraintInfo object to Python dict"""
    d = {}
    if info is None:
        return d
    info.set_was_used(True)
    for typ in ('int', 'float', 'string', 'filename', 'floats', 'filenames'):
        for i in range(getattr(info, 'get_number_of_' + typ)()):
            key = getattr(info, 'get_%s_key' % typ)(i)
            value = getattr(info, 'get_%s_value' % typ)(i)
            d[key] = value
    return d

class _MappedRestraint(object):
    """Map specific IMP restraint types to mmCIF output"""
    def __init__(self, restraint, restraint_info, system):
        self._restraint = restraint
        self._info = restraint_info
        self._setup_dataset(system)
        self._frame_info = {}

    def _get_frame_info(self, frame):
        """Store any restraint info specific to this frame"""
        info = _parse_restraint_info(self._restraint.get_dynamic_info())
        self._frame_info[frame] = info

    def _setup_dataset(self, system):
        """Create any datasets used by this restraint"""
        pass


class _GaussianEMRestraint(_MappedRestraint):
    """Handle an IMP.isd.GaussianEMRestraint"""

    def _setup_dataset(self, system):
        l = IMP.mmcif.dataset.FileLocation(self._info['filename'],
                details="Electron microscopy density map, "
                        "represented as a Gaussian Mixture Model (GMM)")
        # A 3DEM restraint's dataset ID uniquely defines the mmCIF restraint, so
        # we need to allow duplicates
        l._allow_duplicates = True
        d = IMP.mmcif.dataset.EMDensityDataset(l)
        self.dataset = system.datasets.add(d)
        # todo: parse file and pull out original MRC file


class _RestraintMapper(object):
    """Map IMP restraints to mmCIF objects"""
    def __init__(self, system):
        self._typemap = {"IMP.isd.GaussianEMRestraint": _GaussianEMRestraint}
        self._system = system

    def handle(self, r, frame):
        """Handle an individual IMP restraint.
           @return a wrapped version of the restraint if it is handled in
                   mmCIF, otherwise None."""
        info = _parse_restraint_info(r.get_static_info())
        if 'type' in info and info['type'] in self._typemap:
            r = self._typemap[info['type']](r, info, self._system)
            r._get_frame_info(frame)
            return r
