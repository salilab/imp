"""@namespace IMP.mmcif.restraint
   @brief Map IMP restraints to mmCIF categories
"""

import IMP.mmcif.metadata
import ihm.restraint
import operator


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


class _AsymMapper(object):
    """Map ParticleIndexes to ihm.AsymUnit"""
    def __init__(self, m, components, ignore_non_structural=False):
        self.m = m
        self.components = components
        self._seen_ranges = {}
        self._ignore_non_structural = ignore_non_structural

    def __getitem__(self, pi):
        m = self.m
        # Walk up the hierarchy until we find the Chain,
        # then map that to AsymUnit
        if IMP.atom.Hierarchy.get_is_setup(m, pi):
            h = IMP.atom.Hierarchy(m, pi)
            while h:
                if IMP.atom.Chain.get_is_setup(m, h):
                    return self.components[IMP.atom.Chain(h)].asym_unit
                h = h.get_parent()
        if not self._ignore_non_structural:
            raise KeyError("Could not find top-level Chain for "
                           + m.get_particle_name(pi))

    def get_feature(self, ps):
        """Get an ihm.restraint.Feature that covers the given particles"""
        # todo: handle things other than residues
        m = self.m
        rngs = []
        for p in ps:
            asym = self[p]
            # todo: handle overlapping ranges
            if IMP.atom.Residue.get_is_setup(m, p):
                rind = IMP.atom.Residue(m, p).get_index()
                rng = asym(rind, rind)
            elif IMP.atom.Fragment.get_is_setup(m, p):
                # PMI Fragments always contain contiguous residues
                rinds = IMP.atom.Fragment(m, p).get_residue_indexes()
                rng = asym(rinds[0], rinds[-1])
            else:
                raise ValueError("Unsupported particle type %s" % str(p))
            # Join contiguous ranges
            if len(rngs) > 0 and rngs[-1].asym == asym \
               and rngs[-1].seq_id_range[1] == rng.seq_id_range[0] - 1:
                rngs[-1].seq_id_range = (rngs[-1].seq_id_range[0],
                                         rng.seq_id_range[1])
            else:
                rngs.append(rng)
        # If an identical feature already exists, return that
        # todo: python-ihm should handle this automatically for us
        hrngs = tuple(rngs)
        if hrngs in self._seen_ranges:
            return self._seen_ranges[hrngs]
        else:
            feat = ihm.restraint.ResidueFeature(rngs)
            self._seen_ranges[hrngs] = feat
            return feat


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


def _get_restraint_assembly(imp_restraint, components):
    """Get the assembly corresponding to all input particles for
       the restraint"""
    asym_map = _AsymMapper(imp_restraint.get_model(), components,
                           ignore_non_structural=True)
    asyms = frozenset(
        asym_map[p]
        for p in IMP.get_input_particles(imp_restraint.get_inputs()))
    asyms = sorted((a for a in asyms if a is not None),
                   key=operator.attrgetter('id'))
    return asyms


class _EM3DRestraint(ihm.restraint.EM3DRestraint):
    def __init__(self, imp_restraint, info, components, system):
        # If a subunit contains any density, add the entire subunit to
        # this restraint's assembly
        asyms = _get_restraint_assembly(imp_restraint, components)

        assembly = ihm.Assembly(
            asyms, name="EM subassembly",
            description="All components that fit the EM map")

        self._filename = info['filename']
        self._asyms = tuple(asyms)
        p = IMP.mmcif.metadata._GMMParser()
        r = p.parse_file(info['filename'])
        super(_EM3DRestraint, self).__init__(
            dataset=r['dataset'], assembly=assembly,
            number_of_gaussians=r['number_of_gaussians'],
            fitting_method='Gaussian mixture model')

    def _get_signature(self):
        return ("EM3DRestraint", self._filename, self._asyms,
                self.number_of_gaussians, self.fitting_method)

    def add_model_fit(self, imp_restraint, model):
        info = _parse_restraint_info(imp_restraint.get_dynamic_info())
        self.fits[model] = ihm.restraint.EM3DRestraintFit(
                cross_correlation_coefficient=info['cross correlation'])


def _make_em2d_restraint(imp_restraint, info, components, system):
    for i in range(len(info['image files'])):
        yield _EM2DRestraint(imp_restraint, info, components, i)


class _EM2DRestraint(ihm.restraint.EM2DRestraint):
    def __init__(self, imp_restraint, info, components, image_number):
        asyms = _get_restraint_assembly(imp_restraint, components)

        assembly = ihm.Assembly(
            asyms, name="2D EM subassembly",
            description="All components that fit the EM images")

        self._image_number = image_number
        self._filename = info['image files'][image_number]
        self._asyms = tuple(asyms)

        loc = ihm.location.InputFileLocation(
                self._filename,
                details="Electron microscopy class average")
        dataset = ihm.dataset.EM2DClassDataset(loc)

        super(_EM2DRestraint, self).__init__(
            dataset=dataset, assembly=assembly,
            segment=False,
            number_raw_micrographs=info['micrographs number'] or None,
            pixel_size_width=info['pixel size'],
            pixel_size_height=info['pixel size'],
            image_resolution=info['resolution'],
            number_of_projections=info['projection number'])

    def _get_signature(self):
        return ("EM2DRestraint", self._filename, self._asyms,
                self.number_raw_micrographs, self.pixel_size_width,
                self.pixel_size_height, self.image_resolution,
                self.number_of_projections)

    def add_model_fit(self, imp_restraint, model):
        info = _parse_restraint_info(imp_restraint.get_dynamic_info())
        ccc = info['cross correlation'][self._image_number]
        transform = self._get_transformation(model, info, self._image_number)
        rot = transform.get_rotation()
        rm = [[e for e in rot.get_rotation_matrix_row(i)] for i in range(3)]
        self.fits[model] = ihm.restraint.EM2DRestraintFit(
            cross_correlation_coefficient=ccc, rot_matrix=rm,
            tr_vector=transform.get_translation())

    def _get_transformation(self, model, info, nimage):
        """Get the transformation that places the model on image nimage"""
        r = info['rotation'][nimage * 4: nimage * 4 + 4]
        t = info['translation'][nimage * 3: nimage * 3 + 3]
        return IMP.algebra.Transformation3D(IMP.algebra.Rotation3D(*r),
                                            IMP.algebra.Vector3D(*t))


class _SAXSRestraint(ihm.restraint.SASRestraint):
    def __init__(self, imp_restraint, info, components, system):
        asyms = _get_restraint_assembly(imp_restraint, components)

        assembly = ihm.Assembly(
            asyms, name="SAXS subassembly",
            description="All components that fit the SAXS profile")

        self._filename = info['filename']
        self._asyms = tuple(asyms)
        loc = ihm.location.InputFileLocation(
            info['filename'], details='SAXS profile')
        dataset = ihm.dataset.SASDataset(loc)
        super(_SAXSRestraint, self).__init__(
            dataset=dataset, assembly=assembly,
            segment=False, fitting_method='IMP SAXS restraint',
            fitting_atom_type=info['form factor type'],
            multi_state=False)

    def _get_signature(self):
        return ("SAXSRestraint", self._filename, self._asyms,
                self.segment, self.fitting_method, self.fitting_atom_type,
                self.multi_state)

    def add_model_fit(self, imp_restraint, model):
        # We don't know the chi value; we only report a score
        self.fits[model] = ihm.restraint.SASRestraintFit(chi_value=None)


class _CrossLinkRestraint(ihm.restraint.CrossLinkRestraint):
    def __init__(self, imp_restraint, info, components, system):
        self._info = info
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
        cmap = {e.description: e for e in system.entities}
        dist = ihm.restraint.UpperBoundDistanceRestraint(info['linker length'])
        asym = _AsymMapper(imp_restraint.get_model(), components)
        self._add_all_links(IMP.RestraintSet.get_from(imp_restraint), cmap,
                            asym, dist)

    def _add_all_links(self, rset, cmap, asym, dist):
        """Add info for each cross-link in the given RestraintSet"""
        for link in rset.restraints:
            # Recurse into any child RestraintSets
            try:
                child_rs = IMP.RestraintSet.get_from(link)
            except ValueError:
                child_rs = None
            if child_rs:
                self._add_all_links(child_rs, cmap, asym, dist)
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
                asym1 = asym[endp1]
                asym2 = asym[endp2]
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

    def _get_signature(self):
        # Assume that if we have the same restraint info (linker, csv file)
        # and the same number of links, it is the same restraint.
        # dict is not hashable, but a tuple of its items is.
        return ("CXMSRestraint", tuple(self._info.items()),
                len(self.cross_links), len(self.experimental_cross_links))

    def add_model_fit(self, imp_restraint, model):
        pass  # todo


class _GeometricRestraint(ihm.restraint.GeometricRestraint):
    """Base for all geometric restraints"""

    def __init__(self, imp_restraint, info, components, system):
        self._info = info
        asym = _AsymMapper(imp_restraint.get_model(), components)
        super(_GeometricRestraint, self).__init__(
            dataset=None,
            geometric_object=self._geom_object,
            feature=asym.get_feature(
                IMP.get_input_particles(imp_restraint.get_inputs())),
            distance=self._get_distance(info),
            harmonic_force_constant=1. / info['sigma'],
            restrain_all=True)

    def _get_distance(self, info):
        pass

    def add_model_fit(self, imp_restraint, model):
        pass

    def _get_signature(self):
        return ("GeometricRestraint", self.feature, self.geometric_object,
                tuple(self._info.items()))


class _ZAxialRestraint(_GeometricRestraint):
    """Handle an IMP.npc.ZAxialRestraint"""
    _geom_object = ihm.geometry.XYPlane()

    def _get_distance(self, info):
        return ihm.restraint.LowerUpperBoundDistanceRestraint(
            info['lower bound'], info['upper bound'])


class _AllRestraints(object):
    """Map IMP restraints to mmCIF objects"""
    _typemap = {
        "IMP.isd.GaussianEMRestraint": _EM3DRestraint,
        "IMP.pmi.CrossLinkingMassSpectrometryRestraint": _CrossLinkRestraint,
        "IMP.em2d.PCAFitRestraint": _make_em2d_restraint,
        "IMP.npc.ZAxialPositionRestraint": _ZAxialRestraint,
        "IMP.saxs.Restraint": _SAXSRestraint}

    def __init__(self, system, components):
        self._system = system
        self._components = components
        self._seen_restraints = {}

    def add(self, restraint):
        """Add and return a new restraint"""
        sig = restraint._get_signature()
        if sig not in self._seen_restraints:
            self._system.restraints.append(restraint)
            self._seen_restraints[sig] = restraint
        return self._seen_restraints[sig]

    def handle(self, restraint, ihm_models):
        """Handle an individual IMP restraint.
           Yield wrapped version(s) of the restraint if it is handled in
           mmCIF (one IMP restraint may map to multiple mmCIF restraints).
           These may be new or existing restraint objects."""
        info = _parse_restraint_info(restraint.get_static_info())
        if 'type' in info and info['type'] in self._typemap:
            cif_rs = self._typemap[info['type']](restraint,
                                                 info, self._components,
                                                 self._system)
            try:
                _ = iter(cif_rs)
            except TypeError:
                cif_rs = [cif_rs]
            for cif_r in cif_rs:
                cif_r = self.add(cif_r)
                for model in ihm_models:
                    cif_r.add_model_fit(restraint, model)
                yield cif_r
