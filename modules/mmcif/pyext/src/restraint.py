"""@namespace IMP.mmcif.restraint
   @brief Map IMP restraints to mmCIF categories
"""

import IMP.mmcif.metadata
import ihm.restraint

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


class _GaussianEMRestraint(ihm.restraint.EM3DRestraint):
    """Handle an IMP.isd.GaussianEMRestraint"""

    def __init__(self, imp_restraint, info, modeled_assembly):
        self._imp_restraint = imp_restraint
        p = IMP.mmcif.metadata._GMMParser()
        r = p.parse_file(info['filename'])
        super(_GaussianEMRestraint, self).__init__(
                dataset=r['dataset'],
                assembly=modeled_assembly, # todo: fill in correct assembly
                number_of_gaussians=r['number_of_gaussians'],
                fitting_method='Gaussian mixture model')

    def add_model_fit(self, model):
        info = _parse_restraint_info(self._imp_restraint.get_dynamic_info())
        self.fits[model] = ihm.restraint.EM3DRestraintFit(
                cross_correlation_coefficient=info['cross correlation'])


class _RestraintMapper(object):
    """Map IMP restraints to mmCIF objects"""
    def __init__(self, system):
        self._typemap = {"IMP.isd.GaussianEMRestraint": _GaussianEMRestraint}
        self._system = system

    def handle(self, r, model, modeled_assembly):
        """Handle an individual IMP restraint.
           @return a wrapped version of the restraint if it is handled in
                   mmCIF, otherwise None."""
        info = _parse_restraint_info(r.get_static_info())
        if 'type' in info and info['type'] in self._typemap:
            r = self._typemap[info['type']](r, info, modeled_assembly)
            r.add_model_fit(model)
            return r
