import IMP.npc


class XYRadialPositionLowerRestraint(object):
    """Create XYRadial Position Lower restraints
    """
    def __init__(self,
                 representation = None,
                 protein = None,
                 lower_bound = 0.0,
                 consider_radius = False,
                 sigma = 1.0,
                 hier = None):
        """Constructor
        @param representation representation
        """

        # PMI1/2 selection
        if representation is None and hier is not None:
            self.m = hier.get_model()
        elif hier is None and representation is not None:
            self.m = representation.prot.get_model()
        else:
            raise Exception("XYRadialPositionLowerRestraint: must pass hier or representation")

        self.rs = IMP.RestraintSet(self.m, 'XYRadialPositionLowerRestraint')
        self.weight=1.0
        self.label = "None"

        xyr = IMP.npc.XYRadialPositionLowerRestraint(self.m, lower_bound, consider_radius, sigma)
        terminal_residue = IMP.pmi.tools.get_terminal_residue(representation, representation.hier_dict[protein], terminus="C")
        #print (terminal_residue, type(terminal_residue))

        xyr.add_particle(terminal_residue)
        self.rs.add_restraint(xyr)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(self.weight)

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["XYRadialPositionLowerRestraint_" + self.label] = str(score)
        return output

    def evaluate(self):
        return self.weight * self.rs.unprotected_evaluate(None)


class XYRadialPositionUpperRestraint(object):
    """Create XYRadial Position Upper restraints
    """
    def __init__(self,
                 representation = None,
                 protein = None,
                 upper_bound = 0.0,
                 consider_radius = False,
                 sigma = 1.0,
                 hier = None):
        """Constructor
        @param representation representation
        """

        # PMI1/2 selection
        if representation is None and hier is not None:
            self.m = hier.get_model()
        elif hier is None and representation is not None:
            self.m = representation.prot.get_model()
        else:
            raise Exception("XYRadialPositionUpperRestraint: must pass hier or representation")

        self.rs = IMP.RestraintSet(self.m, 'XYRadialPositionUpperRestraint')
        self.weight=1.0
        self.label = "None"

        xyr = IMP.npc.XYRadialPositionUpperRestraint(self.m, upper_bound, consider_radius, sigma)
        terminal_residue = IMP.pmi.tools.get_terminal_residue(representation, representation.hier_dict[protein], terminus="C")
        #print (terminal_residue, type(terminal_residue))

        xyr.add_particle(terminal_residue)
        self.rs.add_restraint(xyr)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(self.weight)

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["XYRadialPositionUpperRestraint_" + self.label] = str(score)
        return output

    def evaluate(self):
        return self.weight * self.rs.unprotected_evaluate(None)

class ZAxialPositionLowerRestraint(object):
    """Create Z-Axial Position Lower restraints
    """
    def __init__(self,
                 representation = None,
                 protein = None,
                 lower_bound = 0.0,
                 consider_radius = False,
                 sigma = 1.0,
                 hier = None):
        """Constructor
        @param representation representation
        """

        # PMI1/2 selection
        if representation is None and hier is not None:
            self.m = hier.get_model()
        elif hier is None and representation is not None:
            self.m = representation.prot.get_model()
        else:
            raise Exception("ZAxialPositionLowerRestraint: must pass hier or representation")

        self.rs = IMP.RestraintSet(self.m, 'ZAxialPositionLowerRestraint')
        self.weight=1.0
        self.label = "None"

        zax = IMP.npc.ZAxialPositionLowerRestraint(self.m, lower_bound, consider_radius, sigma)
        terminal_residue = IMP.pmi.tools.get_terminal_residue(representation, representation.hier_dict[protein], terminus="C")
        #print (terminal_residue, type(terminal_residue))

        zax.add_particle(terminal_residue)
        self.rs.add_restraint(zax)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(self.weight)

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ZAxialPositionLowerRestraint_" + self.label] = str(score)
        return output

    def evaluate(self):
        return self.weight * self.rs.unprotected_evaluate(None)

class ZAxialPositionUpperRestraint(object):
    """Create Z-Axial Position Upper restraints
    """
    def __init__(self,
                 representation = None,
                 protein = None,
                 upper_bound = 0.0,
                 consider_radius = False,
                 sigma = 1.0,
                 hier = None):
        """Constructor
        @param representation representation
        """

        # PMI1/2 selection
        if representation is None and hier is not None:
            self.m = hier.get_model()
        elif hier is None and representation is not None:
            self.m = representation.prot.get_model()
        else:
            raise Exception("ZAxialPositionUpperRestraint: must pass hier or representation")

        self.rs = IMP.RestraintSet(self.m, 'ZAxialPositionUpperRestraint')
        self.weight=1.0
        self.label = "None"

        zax = IMP.npc.ZAxialPositionUpperRestraint(self.m, upper_bound, consider_radius, sigma)
        terminal_residue = IMP.pmi.tools.get_terminal_residue(representation, representation.hier_dict[protein], terminus="C")
        #print (terminal_residue, type(terminal_residue))

        zax.add_particle(terminal_residue)
        self.rs.add_restraint(zax)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(self.weight)

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ZAxialPositionUpperRestraint_" + self.label] = str(score)
        return output

    def evaluate(self):
        return self.weight * self.rs.unprotected_evaluate(None)
