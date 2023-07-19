"""@namespace IMP.pmi.restraints.npc
Specialized restraints for modeling the Nuclear Pore Complex.

These restraints have been used to model the Nuclear Pore Complex (NPC)
but may be useful for other applications too. They model the NPC as lying
in the xy plane, centered at the origin. (Transport through the pore
corresponds to movement along the z axis.) The surface of the nuclear
membrane is represented as the surface of a half torus, also lying in
the xy plane and centered at the origin. Individual proteins
(or parts of proteins, such as the N- or C- termini) are then localized to
different regions of the complex (e.g. near the membrane or pore, or on
the nuclear or cytoplasmic side) by simple restraints on their Cartesian
coordinates.
"""

import IMP.npc


class XYRadialPositionRestraint(IMP.pmi.restraints.RestraintBase):
    """Restrain a protein's distance from the z axis to within a given range.
    """
    _include_in_rmf = True

    def __init__(self, hier, protein, lower_bound=0.0, upper_bound=0.0,
                 consider_radius=False, sigma=1.0, term='C', label=None,
                 weight=1.0):
        """Constructor
        """

        super(XYRadialPositionRestraint, self).__init__(
            hier.get_model(), label=label, weight=weight)

        xyr = IMP.npc.XYRadialPositionRestraint(
            self.model, lower_bound, upper_bound, consider_radius, sigma)
        residues = IMP.pmi.tools.select_by_tuple_2(
            hier, protein, resolution=1)
        if term == 'C':
            terminal = residues[-1]
            # print (terminal, type(terminal))
            xyr.add_particle(terminal)
        elif term == 'N':
            terminal = residues[0]
            # print (terminal, type(terminal))
            xyr.add_particle(terminal)
        else:
            for residue in residues:
                # print (residue, type(residue))
                xyr.add_particle(residue)

        self.dataset = None
        for p, state in IMP.pmi.tools._all_protocol_outputs(hier):
            p.add_xyradial_restraint(state, residues, lower_bound,
                                     upper_bound, sigma, self)

        self.rs.add_restraint(xyr)


class XYRadialPositionLowerRestraint(IMP.pmi.restraints.RestraintBase):
    """Restrain a protein's distance from the z axis to above a lower bound.
    """
    _include_in_rmf = True

    def __init__(self, hier, protein, lower_bound=0.0,
                 consider_radius=False, sigma=1.0, label=None, weight=1.0):
        """Constructor
        """

        super(XYRadialPositionLowerRestraint, self).__init__(
            hier.get_model(), label=label, weight=weight)

        xyr = IMP.npc.XYRadialPositionLowerRestraint(
            self.model, lower_bound, consider_radius, sigma)
        residues = IMP.pmi.tools.select_by_tuple_2(
            hier, protein, resolution=1)
        cterminal = residues[-1]

        xyr.add_particle(cterminal)
        self.rs.add_restraint(xyr)


class XYRadialPositionUpperRestraint(IMP.pmi.restraints.RestraintBase):
    """Restrain a protein's distance from the z axis to below an upper bound.
    """
    _include_in_rmf = True

    def __init__(self, hier, protein, upper_bound=0.0,
                 consider_radius=False, sigma=1.0, label=None, weight=1.0):
        """Constructor
        """

        super(XYRadialPositionUpperRestraint, self).__init__(
            hier.get_model(), label=label, weight=weight)

        xyr = IMP.npc.XYRadialPositionUpperRestraint(
            self.model, upper_bound, consider_radius, sigma)
        residues = IMP.pmi.tools.select_by_tuple_2(
            hier, protein, resolution=1)
        cterminal = residues[-1]

        xyr.add_particle(cterminal)
        self.rs.add_restraint(xyr)


class ZAxialPositionRestraint(IMP.pmi.restraints.RestraintBase):
    """Restrain a protein's z coordinate to within a given range.
    """
    _include_in_rmf = True

    def __init__(self, hier, protein, lower_bound=0.0,
                 upper_bound=0.0, consider_radius=False, sigma=1.0, term='C',
                 label=None, weight=1.0):
        """Constructor
        """

        super(ZAxialPositionRestraint, self).__init__(
            hier.get_model(), label=label, weight=weight)

        zax = IMP.npc.ZAxialPositionRestraint(
            self.model, lower_bound, upper_bound, consider_radius, sigma)
        residues = IMP.pmi.tools.select_by_tuple_2(
            hier, protein, resolution=1)
        if term == 'C':
            terminal = residues[-1]
            # print (terminal, type(terminal))
            zax.add_particle(terminal)
        elif term == 'N':
            terminal = residues[0]
            # print (terminal, type(terminal))
            zax.add_particle(terminal)
        else:
            for residue in residues:
                # print (residue, type(residue))
                zax.add_particle(residue)

        self.dataset = None
        for p, state in IMP.pmi.tools._all_protocol_outputs(hier):
            p.add_zaxial_restraint(state, residues, lower_bound,
                                   upper_bound, sigma, self)

        self.rs.add_restraint(zax)


class ZAxialPositionLowerRestraint(IMP.pmi.restraints.RestraintBase):
    """Restrain a protein's z coordinate to above a lower bound.
    """
    _include_in_rmf = True

    def __init__(self, hier, protein, lower_bound=0.0,
                 consider_radius=False, sigma=1.0, label=None, weight=1.0):
        """Constructor
        """

        super(ZAxialPositionLowerRestraint, self).__init__(
            hier.get_model(), label=label, weight=weight)

        zax = IMP.npc.ZAxialPositionLowerRestraint(
            self.model, lower_bound, consider_radius, sigma)
        residues = IMP.pmi.tools.select_by_tuple_2(
            hier, protein, resolution=1)
        cterminal = residues[-1]

        zax.add_particle(cterminal)
        self.rs.add_restraint(zax)


class ZAxialPositionUpperRestraint(IMP.pmi.restraints.RestraintBase):
    """Restrain a protein's z coordinate to below an upper bound.
    """
    _include_in_rmf = True

    def __init__(self, hier, protein, upper_bound=0.0,
                 consider_radius=False, sigma=1.0, label=None, weight=1.0):
        """Constructor
        """

        super(ZAxialPositionUpperRestraint, self).__init__(
            hier.get_model(), label=label, weight=weight)

        zax = IMP.npc.ZAxialPositionUpperRestraint(
            self.model, upper_bound, consider_radius, sigma)
        residues = IMP.pmi.tools.select_by_tuple_2(
            hier, protein, resolution=1)
        cterminal = residues[-1]

        zax.add_particle(cterminal)
        self.rs.add_restraint(zax)


class YAxialPositionRestraint(IMP.pmi.restraints.RestraintBase):
    """Restrain a protein's y coordinate to within a given range.
    """
    _include_in_rmf = True

    def __init__(self, hier, protein, lower_bound=0.0,
                 upper_bound=0.0, consider_radius=False, sigma=1.0, term='C',
                 label=None, weight=1.0):
        """Constructor
        """

        super(YAxialPositionRestraint, self).__init__(
            hier.get_model(), label=label, weight=weight)

        yax = IMP.npc.YAxialPositionRestraint(
            self.model, lower_bound, upper_bound, consider_radius, sigma)
        residues = IMP.pmi.tools.select_by_tuple_2(
            hier, protein, resolution=1)
        if term == 'C':
            terminal = residues[-1]
            # print (terminal, type(terminal))
            yax.add_particle(terminal)
        elif term == 'N':
            terminal = residues[0]
            # print (terminal, type(terminal))
            yax.add_particle(terminal)
        else:
            for residue in residues:
                # print (residue, type(residue))
                yax.add_particle(residue)

        self.dataset = None
        for p, state in IMP.pmi.tools._all_protocol_outputs(hier):
            p.add_yaxial_restraint(state, residues, lower_bound,
                                   upper_bound, sigma, self)

        self.rs.add_restraint(yax)


class YAxialPositionLowerRestraint(IMP.pmi.restraints.RestraintBase):
    """Restrain a protein's y coordinate to above a lower bound.
    """
    _include_in_rmf = True

    def __init__(self, hier, protein, lower_bound=0.0,
                 consider_radius=False, sigma=1.0, label=None, weight=1.0):
        """Constructor
        """

        super(YAxialPositionLowerRestraint, self).__init__(
            hier.get_model(), label=label, weight=weight)

        yax = IMP.npc.YAxialPositionLowerRestraint(
            self.model, lower_bound, consider_radius, sigma)
        residues = IMP.pmi.tools.select_by_tuple_2(
            hier, protein, resolution=1)
        cterminal = residues[-1]

        yax.add_particle(cterminal)
        self.rs.add_restraint(yax)


class YAxialPositionUpperRestraint(IMP.pmi.restraints.RestraintBase):
    """Restrain a protein's y coordinate to below an upper bound.
    """
    _include_in_rmf = True

    def __init__(self, hier, protein, upper_bound=0.0,
                 consider_radius=False, sigma=1.0, label=None, weight=1.0):
        """Constructor
        """

        super(YAxialPositionUpperRestraint, self).__init__(
            hier.get_model(), label=label, weight=weight)

        yax = IMP.npc.YAxialPositionUpperRestraint(
            self.model, upper_bound, consider_radius, sigma)
        residues = IMP.pmi.tools.select_by_tuple_2(
            hier, protein, resolution=1)
        cterminal = residues[-1]

        yax.add_particle(cterminal)
        self.rs.add_restraint(yax)


class MembraneSurfaceLocationRestraint(IMP.pmi.restraints.RestraintBase):
    """Localize protein on the surface of a half torus in the xy plane.
    """
    _include_in_rmf = True

    def __init__(self, hier, protein,
                 tor_R=540.0, tor_r=127.5, tor_th=45.0, sigma=0.2,
                 resolution=1, label=None, weight=1.0):
        """Constructor
        """

        super(MembraneSurfaceLocationRestraint, self).__init__(
            hier.get_model(), label=label, weight=weight)

        msl = IMP.npc.MembraneSurfaceLocationRestraint(
            self.model, tor_R, tor_r, tor_th, sigma)
        residues = IMP.pmi.tools.select_by_tuple_2(
            hier, protein, resolution=resolution)
        for residue in residues:
            # print (residue, type(residue))
            msl.add_particle(residue)

        self.dataset = None
        for p, state in IMP.pmi.tools._all_protocol_outputs(hier):
            p.add_membrane_surface_location_restraint(
                state, residues, tor_R, tor_r, tor_th, sigma, self)

        self.rs.add_restraint(msl)


class MembraneSurfaceLocationConditionalRestraint(
        IMP.pmi.restraints.RestraintBase):
    """Localize one protein on the surface of a half torus in the xy plane.

       Create Membrane Surface Location CONDITIONAL Restraint
       for Nup120 ALPS Motifs - Mutually Exclusive from (135,152,'Nup120')
       and (197,216,'Nup120').
       It returns a minimum penalty score from two potential ALPS motifs.
    """
    _include_in_rmf = True

    def __init__(self, hier, protein1, protein2,
                 tor_R=540.0, tor_r=127.5, tor_th=45.0, sigma=0.2,
                 resolution=1, label=None, weight=1.0):
        """Constructor
        """

        super(MembraneSurfaceLocationConditionalRestraint, self).__init__(
            hier.get_model(), label=label, weight=weight)

        msl = IMP.npc.MembraneSurfaceLocationConditionalRestraint(
            self.model, tor_R, tor_r, tor_th, sigma)
        residues1 = IMP.pmi.tools.select_by_tuple_2(
            hier, protein1, resolution=resolution)
        for residue in residues1:
            # print (residue, type(residue))
            msl.add_particle1(residue)
        residues2 = IMP.pmi.tools.select_by_tuple_2(
            hier, protein2, resolution=resolution)
        for residue in residues2:
            # print (residue, type(residue))
            msl.add_particle2(residue)

        # Approximate as two membrane restraints
        self.dataset = None
        for p, state in IMP.pmi.tools._all_protocol_outputs(hier):
            for residues in residues1, residues2:
                p.add_membrane_surface_location_restraint(
                    state, residues, tor_R, tor_r, tor_th, sigma, self)

        self.rs.add_restraint(msl)


class MembraneExclusionRestraint(IMP.pmi.restraints.RestraintBase):
    """Keep protein away from a half torus in the xy plane.
    """
    _include_in_rmf = True

    def __init__(self, hier, protein=None,
                 tor_R=540.0, tor_r=127.5, tor_th=45.0, sigma=0.2,
                 resolution=1, label=None, weight=1.0):
        """Constructor
        """

        super(MembraneExclusionRestraint, self).__init__(
            hier.get_model(), label=label, weight=weight)

        mex = IMP.npc.MembraneExclusionRestraint(
            self.model, tor_R, tor_r, tor_th, sigma)
        residues = IMP.pmi.tools.select_by_tuple_2(
            hier, protein, resolution=resolution)
        for residue in residues:
            mex.add_particle(residue)

        self.dataset = None
        for p, state in IMP.pmi.tools._all_protocol_outputs(hier):
            p.add_membrane_exclusion_restraint(
                state, residues, tor_R, tor_r, tor_th, sigma, self)

        self.rs.add_restraint(mex)
