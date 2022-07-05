import IMP.npc


class XYRadialPositionRestraint(IMP.pmi.restraints.RestraintBase):
    """Create XYRadial Position Restraint
    """
    def __init__(self, hier, protein, lower_bound=0.0, upper_bound=0.0,
                 consider_radius=False, sigma=1.0, term='C', label=None,
                 weight=1.0):
        """Constructor
        @param representation representation
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
        self.rs.add_restraint(xyr)


class XYRadialPositionLowerRestraint(IMP.pmi.restraints.RestraintBase):
    """Create XYRadial Position Lower restraints
    """
    def __init__(self, hier, protein, lower_bound=0.0,
                 consider_radius=False, sigma=1.0, label=None, weight=1.0):
        """Constructor
        @param representation representation
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
    """Create XYRadial Position Upper restraints
    """
    def __init__(self, hier, protein, upper_bound=0.0,
                 consider_radius=False, sigma=1.0, label=None, weight=1.0):
        """Constructor
        @param representation representation
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
    """Create Z-Axial Position restraints
    """
    def __init__(self, hier, protein, lower_bound=0.0,
                 upper_bound=0.0, consider_radius=False, sigma=1.0, term='C',
                 label=None, weight=1.0):
        """Constructor
        @param representation representation
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
        self.rs.add_restraint(zax)


class ZAxialPositionLowerRestraint(IMP.pmi.restraints.RestraintBase):
    """Create Z-Axial Position Lower restraints
    """
    def __init__(self, hier, protein, lower_bound=0.0,
                 consider_radius=False, sigma=1.0, label=None, weight=1.0):
        """Constructor
        @param representation representation
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
    """Create Z-Axial Position Upper restraints
    """
    def __init__(self, hier, protein, upper_bound=0.0,
                 consider_radius=False, sigma=1.0, label=None, weight=1.0):
        """Constructor
        @param representation representation
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
    """Create Y-Axial Position restraints
    """
    def __init__(self, hier, protein, lower_bound=0.0,
                 upper_bound=0.0, consider_radius=False, sigma=1.0, term='C',
                 label=None, weight=1.0):
        """Constructor
        @param representation representation
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
        self.rs.add_restraint(yax)


class YAxialPositionLowerRestraint(IMP.pmi.restraints.RestraintBase):
    """Create Y-Axial Position Lower restraints
    """
    def __init__(self, hier, protein, lower_bound=0.0,
                 consider_radius=False, sigma=1.0, label=None, weight=1.0):
        """Constructor
        @param representation representation
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
    """Create Y-Axial Position Upper restraints
    """
    def __init__(self, hier, protein, upper_bound=0.0,
                 consider_radius=False, sigma=1.0, label=None, weight=1.0):
        """Constructor
        @param representation representation
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
    """Create Membrane Surface Location Restraint
    """
    def __init__(self, hier, protein,
                 tor_R=540.0, tor_r=127.5, tor_th=45.0, sigma=0.2,
                 resolution=1, label=None, weight=1.0):
        """Constructor
        @param representation representation
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
        self.rs.add_restraint(msl)


class MembraneSurfaceLocationConditionalRestraint(
        IMP.pmi.restraints.RestraintBase):
    """Create Membrane Surface Location CONDITIONAL Restraint
       for Nup120 ALPS Motifs - Mutually Exclusive from (135,152,'Nup120')
       and (197,216,'Nup120').
       It returns a minimum penalty score from two potential ALPS motifs.
    """
    def __init__(self, hier, protein1, protein2,
                 tor_R=540.0, tor_r=127.5, tor_th=45.0, sigma=0.2,
                 resolution=1, label=None, weight=1.0):
        """Constructor
        @param representation representation
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
        self.rs.add_restraint(msl)


class MembraneExclusionRestraint(IMP.pmi.restraints.RestraintBase):
    """Create Membrane Exclusion Restraint
    """
    def __init__(self, hier, protein=None,
                 tor_R=540.0, tor_r=127.5, tor_th=45.0, sigma=0.2,
                 resolution=1, label=None, weight=1.0):
        """Constructor
        @param representation representation
        """

        super(MembraneExclusionRestraint, self).__init__(
            hier.get_model(), label=label, weight=weight)

        mex = IMP.npc.MembraneExclusionRestraint(
            self.model, tor_R, tor_r, tor_th, sigma)
        residues = IMP.pmi.tools.select_by_tuple_2(
            hier, protein, resolution=resolution)
        for residue in residues:
            mex.add_particle(residue)
        self.rs.add_restraint(mex)
