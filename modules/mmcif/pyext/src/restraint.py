"""@namespace IMP.mmcif.restraint
   @brief Map IMP restraints to mmCIF categories
"""

import IMP.mmcif.metadata
import ihm.restraint


def _get_by_residue(m, p):
    """Determine whether the given particle represents a specific residue
       or a more coarse-grained object."""
    return (IMP.atom.Residue.get_is_setup(m, p)
            or IMP.atom.Atom.get_is_setup(m, p))


def _get_scale(m, p):
    """Get the numerical value of the Scale particle"""
    if not IMP.isd.Scale.get_is_setup(m, p):
        raise ValueError("not scale particle")
    return IMP.isd.Scale(m, p).get_scale()


def _get_asym(m, pi, comp):
    """Get the ihm.AsymUnit for a given particle"""
    # Walk up the hierarchy until we find the Chain, then map that to AsymUnit
    if IMP.atom.Hierarchy.get_is_setup(m, pi):
        h = IMP.atom.Hierarchy(m, pi)
        while h:
            if IMP.atom.Chain.get_is_setup(m, h):
                return comp[IMP.atom.Chain(h)].asym_unit
            h = h.get_parent()
    raise ValueError("Could not find top-level Chain for "
                     + m.get_particle_name(pi))


def _parse_restraint_info(info):
    """Convert RestraintInfo object to Python dict"""
    d = {}
    if info is None:
        return d
    info.set_was_used(True)
    for typ in ('int', 'float', 'string', 'filename', 'floats', 'filenames',
                'particle_indexes'):
        for i in range(getattr(info, 'get_number_of_' + typ)()):
            key = getattr(info, 'get_%s_key' % typ)(i)
            value = getattr(info, 'get_%s_value' % typ)(i)
            d[key] = value
    return d


class _GaussianEMRestraint(ihm.restraint.EM3DRestraint):
    """Handle an IMP.isd.GaussianEMRestraint"""

    def __init__(self, imp_restraint, info, modeled_assembly, system):
        self._imp_restraint = imp_restraint
        p = IMP.mmcif.metadata._GMMParser()
        r = p.parse_file(info['filename'])
        super(_GaussianEMRestraint, self).__init__(
                dataset=r['dataset'],
                assembly=modeled_assembly,  # todo: fill in correct assembly
                number_of_gaussians=r['number_of_gaussians'],
                fitting_method='Gaussian mixture model')

    def add_model_fit(self, model):
        info = _parse_restraint_info(self._imp_restraint.get_dynamic_info())
        self.fits[model] = ihm.restraint.EM3DRestraintFit(
                cross_correlation_coefficient=info['cross correlation'])


class _CrossLinkRestraint(ihm.restraint.CrossLinkRestraint):
    def __init__(self, imp_restraint, info, modeled_assembly, system):
        self._imp_restraint = imp_restraint
        loc = ihm.location.InputFileLocation(
            info['filename'], details='Crosslinks')
        dataset = ihm.dataset.CXMSDataset(loc)
        linker = ihm.ChemDescriptor(
            auth_name=info['linker author name'],
            chemical_name=info.get('linker chemical name'),
            smiles=info.get('linker smiles'),
            smiles_canonical=info.get('linker smiles canonical'),
            inchi=info.get('linker inchi'),
            inchi_key=info.get('linker inchi key'))
        super(_CrossLinkRestraint, self).__init__(
                dataset=dataset, linker=linker)
        # Map from IMP/RMF chain names to ihm.Entity
        cmap = dict((e.description, e) for e in system.entities.get_all())
        dist = ihm.restraint.UpperBoundDistanceRestraint(info['linker length'])
        self._add_all_links(IMP.RestraintSet.get_from(imp_restraint), cmap,
                            system.components, dist)

    def _add_all_links(self, rset, cmap, comp, dist):
        """Add info for each cross-link in the given RestraintSet"""
        for link in rset.restraints:
            # Recurse into any child RestraintSets
            try:
                child_rs = IMP.RestraintSet.get_from(link)
            except ValueError:
                child_rs = None
            if child_rs:
                self._add_all_links(child_rs, cmap, comp, dist)
            else:
                info = _parse_restraint_info(link.get_static_info())
                # todo: handle ambiguous cross-links, fix residue numbering
                r1 = cmap[info['protein1']].residue(info['residue1'])
                r2 = cmap[info['protein2']].residue(info['residue2'])
                ex_xl = ihm.restraint.ExperimentalCrossLink(residue1=r1,
                                                            residue2=r2)
                self.experimental_cross_links.append([ex_xl])
                # todo: handle multiple contributions
                m = link.get_model()
                endp1, endp2 = info['endpoints']
                asym1 = _get_asym(m, endp1, comp)
                asym2 = _get_asym(m, endp2, comp)
                if _get_by_residue(m, endp1) and _get_by_residue(m, endp2):
                    cls = ihm.restraint.ResidueCrossLink
                else:
                    cls = ihm.restraint.FeatureCrossLink
                xl = cls(ex_xl, asym1=asym1, asym2=asym2, distance=dist,
                         restrain_all=False,
                         psi=_get_scale(m, info['psis'][0]),
                         sigma1=_get_scale(m, info['sigmas'][0]),
                         sigma2=_get_scale(m, info['sigmas'][1]))
                self.cross_links.append(xl)

    def add_model_fit(self, model):
        pass  # todo


class _SAXSRestraint(ihm.restraint.SASRestraint):
    """Handle an IMP.saxs.Restraint"""

    def __init__(self, imp_restraint, info, modeled_assembly, system):
        self._imp_restraint = imp_restraint
        loc = ihm.location.InputFileLocation(
            info['filename'], details='SAXS profile')
        dataset = ihm.dataset.SASDataset(loc)
        super(_SAXSRestraint, self).__init__(
                dataset=dataset,
                assembly=modeled_assembly,  # todo: fill in correct assembly
                segment=False, fitting_method='IMP SAXS restraint',
                fitting_atom_type=info['form factor type'],
                multi_state=False)

    def add_model_fit(self, model):
        # We don't know the chi value; we only report a score
        self.fits[model] = ihm.restraint.SASRestraintFit(chi_value=None)


class _RestraintMapper(object):
    """Map IMP restraints to mmCIF objects"""
    def __init__(self, system):
        self._typemap = {
            "IMP.isd.GaussianEMRestraint": _GaussianEMRestraint,
            "IMP.pmi.CrossLinkingMassSpectrometryRestraint":
            _CrossLinkRestraint,
            "IMP.saxs.Restraint": _SAXSRestraint}
        self._system = system

    def handle(self, r, model, modeled_assembly):
        """Handle an individual IMP restraint.
           @return a wrapped version of the restraint if it is handled in
                   mmCIF, otherwise None."""
        info = _parse_restraint_info(r.get_static_info())
        if 'type' in info and info['type'] in self._typemap:
            r = self._typemap[info['type']](r, info, modeled_assembly,
                                            self._system)
            r.add_model_fit(model)
            return r
