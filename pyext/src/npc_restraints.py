import IMP.npc
import IMP.pmi.representation

class XYRadialPositionRestraint(object):
    """Create XYRadial Position Restraint
    """
    def __init__(self,
                 representation = None,
                 protein = None,
                 lower_bound = 0.0,
                 upper_bound = 0.0,
                 consider_radius = False,
                 sigma = 1.0,
                 term = 'C',
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
            raise Exception("XYRadialPositionRestraint: must pass hier or representation")

        self.rs = IMP.RestraintSet(self.m, 'XYRadialPositionRestraint')
        self.weight=1.0
        self.label = "None"

        xyr = IMP.npc.XYRadialPositionRestraint(self.m, lower_bound, upper_bound, consider_radius, sigma)
        #terminal_residue = IMP.pmi.tools.get_terminal_residue(representation, representation.hier_dict[protein], terminus="C")
        residues = IMP.pmi.tools.select_by_tuple(representation, protein, resolution=1)
        if (term == 'C'):
            terminal = residues[-1]
            #print (terminal, type(terminal))
            xyr.add_particle(terminal)
        elif (term == 'N'):
            terminal = residues[0]
            #print (terminal, type(terminal))
            xyr.add_particle(terminal)
        else:
            for residue in residues:
                #print (residue, type(residue))
                xyr.add_particle(residue)
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
        output["XYRadialPositionRestraint_" + self.label] = str(score)
        return output

    def evaluate(self):
        return self.weight * self.rs.unprotected_evaluate(None)


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
        #terminal_residue = IMP.pmi.tools.get_terminal_residue(representation, representation.hier_dict[protein], terminus="C")
        residues = IMP.pmi.tools.select_by_tuple(representation, protein, resolution=1)
        cterminal = residues[-1]        #nterminal = residues[0]
        #print (cterminal, type(cterminal))

        xyr.add_particle(cterminal)
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
        #terminal_residue = IMP.pmi.tools.get_terminal_residue(representation, representation.hier_dict[protein], terminus="C")
        residues = IMP.pmi.tools.select_by_tuple(representation, protein, resolution=1)
        cterminal = residues[-1]        #nterminal = residues[0]
        #print (cterminal, type(cterminal))

        xyr.add_particle(cterminal)
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

class ZAxialPositionRestraint(object):
    """Create Z-Axial Position restraints
    """
    def __init__(self,
                 representation = None,
                 protein = None,
                 lower_bound = 0.0,
                 upper_bound = 0.0,
                 consider_radius = False,
                 sigma = 1.0,
                 term = 'C',
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
            raise Exception("ZAxialPositionRestraint: must pass hier or representation")

        self.rs = IMP.RestraintSet(self.m, 'ZAxialPositionRestraint')
        self.weight=1.0
        self.label = "None"

        zax = IMP.npc.ZAxialPositionRestraint(self.m, lower_bound, upper_bound, consider_radius, sigma)
        #terminal_residue = IMP.pmi.tools.get_terminal_residue(representation, representation.hier_dict[protein], terminus="C")
        residues = IMP.pmi.tools.select_by_tuple(representation, protein, resolution=1)
        if (term == 'C'):
            terminal = residues[-1]
            #print (terminal, type(terminal))
            zax.add_particle(terminal)
        elif (term == 'N'):
            terminal = residues[0]
            #print (terminal, type(terminal))
            zax.add_particle(terminal)
        else:
            for residue in residues:
                #print (residue, type(residue))
                zax.add_particle(residue)
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
        output["ZAxialPositionRestraint_" + self.label] = str(score)
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
        #terminal_residue = IMP.pmi.tools.get_terminal_residue(representation, representation.hier_dict[protein], terminus="C")
        residues = IMP.pmi.tools.select_by_tuple(representation, protein, resolution=1)
        cterminal = residues[-1]        #nterminal = residues[0]
        #print (cterminal, type(cterminal))

        zax.add_particle(cterminal)
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
        #terminal_residue = IMP.pmi.tools.get_terminal_residue(representation, representation.hier_dict[protein], terminus="C")
        residues = IMP.pmi.tools.select_by_tuple(representation, protein, resolution=1)
        cterminal = residues[-1]        #nterminal = residues[0]
        #print (cterminal, type(cterminal))

        zax.add_particle(cterminal)
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


class MembraneSurfaceLocationRestraint(object):
    """Create Membrane Surface Location Restraint
    """
    def __init__(self,
                 representation = None,
                 protein = None,
                 tor_R = 540.0,
                 tor_r = 127.5,
                 tor_th = 45.0,
                 sigma = 0.2,
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
            raise Exception("MembraneSurfaceLocationRestraint: must pass hier or representation")

        self.rs = IMP.RestraintSet(self.m, 'MembraneSurfaceLocationRestraint')
        self.weight=1.0
        self.label = "None"

        msl = IMP.npc.MembraneSurfaceLocationRestraint(self.m, tor_R, tor_r, tor_th, sigma)
        residues = IMP.pmi.tools.select_by_tuple(representation, protein, resolution=1)
        for residue in residues:
            #print (residue, type(residue))
            msl.add_particle(residue)
        self.rs.add_restraint(msl)

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
        output["MembraneSurfaceLocationRestraint_" + self.label] = str(score)
        return output

    def evaluate(self):
        return self.weight * self.rs.unprotected_evaluate(None)


class Representation(IMP.pmi.representation.Representation):
    """ NPC specific functions are defined here
    """
    def set_coordinates_from_rmf(self, component_name, rmf_file_name,
                                 rmf_frame_number,
                                 rmf_component_name=None,
                                 check_number_particles=True,
                                 representation_name_to_rmf_name_map=None,
                                 state_number=0,
                                 skip_gaussian_in_rmf=False,
                                 skip_gaussian_in_representation=False,
                                 save_file=False):
        '''Read and replace coordinates from an RMF file.
        Replace the coordinates of particles with the same name.
        It assumes that the RMF and the representation have the particles
        in the same order.
        @param component_name Component name
        @param rmf_component_name Name of the component in the RMF file
                (if not specified, use `component_name`)
        @param representation_name_to_rmf_name_map a dictionary that map
                the original rmf particle name to the recipient particle component name
        @param save_file: save a file with the names of particles of the component

        '''
        import IMP.pmi.analysis

        prots = IMP.pmi.analysis.get_hiers_from_rmf(
            self.m,
            rmf_frame_number,
            rmf_file_name)

        if not prots:
            raise ValueError("cannot read hierarchy from rmf")

        prot=prots[0]

        # if len(self.rigid_bodies)!=0:
        #   print "set_coordinates_from_rmf: cannot proceed if rigid bodies were initialized. Use the function before defining the rigid bodies"
        #   exit()

        allpsrmf = IMP.atom.get_leaves(prot)
        psrmf = []
        for p in allpsrmf:
            (protname, is_a_bead) = IMP.pmi.tools.get_prot_name_from_particle(
                p, self.hier_dict.keys())
            if (protname is None) and (rmf_component_name is not None):
                (protname, is_a_bead) = IMP.pmi.tools.get_prot_name_from_particle(
                    p, rmf_component_name)
            if (skip_gaussian_in_rmf):
                if (IMP.core.Gaussian.get_is_setup(p)) and not (IMP.atom.Fragment.get_is_setup(p) or IMP.atom.Residue.get_is_setup(p)):
                    continue
            ###################################
            # by SJ Kim (06/04/2016)
            ###################################
            """
            if (protname is None):
                continue
            #else:
                #print (p, p.get_parent().get_parent().get_name(), protname, is_a_bead)
            self_names=(p.get_name()).replace("-","_").split("_")
            #print(self_names, is_a_bead)
            #print (self_names, protname[0:8], len(self_names))
            if (len(self_names) > 1) :
                residue_no = int(self_names[1])
            elif (len(self_names) == 1) :
                residue_no = int(self_names[0])
            else:
                residue_no = int(0)

            if (protname[0:6] == "Nup116") and (is_a_bead) :
                if (int(self_names[1]) < 751) :
                    continue
            elif (protname[0:8] == "Nup159.1") :
                #if (int(self_names[1]) > 381) and (int(self_names[1]) < 1117) :
                if (residue_no < 1117) :
                    #print(self_names, protname[0:8], is_a_bead)
                    continue
            elif (protname[0:4] == "Nsp1") and (is_a_bead) :
                if (int(self_names[1]) < 637) :
                    continue

            #if (protname[0:6] == "Nup159"):
            #    print(self_names)
                #exit(0)
            """
            ###################################
            if (rmf_component_name is not None) and (protname == rmf_component_name):
                psrmf.append(p)
            elif (rmf_component_name is None) and (protname == component_name):
                psrmf.append(p)

        psrepr = IMP.atom.get_leaves(self.hier_dict[component_name])
        if (skip_gaussian_in_representation):
            allpsrepr = psrepr
            psrepr = []
            for p in allpsrepr:
                #(protname, is_a_bead) = IMP.pmi.tools.get_prot_name_from_particle(
                #    p, self.hier_dict.keys())
                if (IMP.core.Gaussian.get_is_setup(p)) and not (IMP.atom.Fragment.get_is_setup(p) or IMP.atom.Residue.get_is_setup(p)):
                    continue
                ###################################
                # by SJ Kim (06/04/2016)
                ###################################
                """
                (protname, is_a_bead) = IMP.pmi.tools.get_prot_name_from_particle(
                    p, self.hier_dict.keys())
                if (protname is None) and (rmf_component_name is not None):
                    (protname, is_a_bead) = IMP.pmi.tools.get_prot_name_from_particle(
                        p, rmf_component_name)
                if (protname is None):
                    continue

                self_names=(p.get_name()).replace("-","_").split("_")
                if (len(self_names) > 1) :
                    if (protname[0:6] == "Nup116") and (is_a_bead) :
                        if (int(self_names[1]) < 751) :
                            continue
                    elif (protname[0:6] == "Nup159") and (is_a_bead) :
                        if (int(self_names[1]) > 381) and (int(self_names[1]) < 1117) :
                            continue
                    elif (protname[0:4] == "Nsp1") and (is_a_bead) :
                        if (int(self_names[1]) < 637) :
                            continue
                """
                ###################################
                psrepr.append(p)

        import itertools
        reprnames=[p.get_name() for p in psrepr]
        rmfnames=[p.get_name() for p in psrmf]

        if save_file:
            fl=open(component_name+".txt","w")
            for i in itertools.izip_longest(reprnames,rmfnames): fl.write(str(i[0])+","+str(i[1])+"\n")


        if check_number_particles and not representation_name_to_rmf_name_map:
            if len(psrmf) != len(psrepr):
                fl=open(component_name+".txt","w")
                for i in itertools.izip_longest(reprnames,rmfnames): fl.write(str(i[0])+","+str(i[1])+"\n")
                raise ValueError("%s cannot proceed the rmf and the representation don't have the same number of particles; "
                           "particles in rmf: %s particles in the representation: %s" % (str(component_name), str(len(psrmf)), str(len(psrepr))))


        if not representation_name_to_rmf_name_map:
            for n, prmf in enumerate(psrmf):

                prmfname = prmf.get_name()
                preprname = psrepr[n].get_name()
                if IMP.core.RigidMember.get_is_setup(psrepr[n]):
                    raise ValueError("component %s cannot proceed if rigid bodies were initialized. Use the function before defining the rigid bodies" % component_name)
                if IMP.core.NonRigidMember.get_is_setup(psrepr[n]):
                    raise ValueError("component %s cannot proceed if rigid bodies were initialized. Use the function before defining the rigid bodies" % component_name)

                if prmfname != preprname:
                    print("set_coordinates_from_rmf: WARNING rmf particle and representation particles have not the same name %s %s " % (prmfname, preprname))
                if IMP.core.XYZ.get_is_setup(prmf) and IMP.core.XYZ.get_is_setup(psrepr[n]):
                    xyz = IMP.core.XYZ(prmf).get_coordinates()
                    IMP.core.XYZ(psrepr[n]).set_coordinates(xyz)
                else:
                    print("set_coordinates_from_rmf: WARNING particles are not XYZ decorated %s %s " % (str(IMP.core.XYZ.get_is_setup(prmf)), str(IMP.core.XYZ.get_is_setup(psrepr[n]))))

                if IMP.core.Gaussian.get_is_setup(prmf) and IMP.core.Gaussian.get_is_setup(psrepr[n]):
                    gprmf=IMP.core.Gaussian(prmf)
                    grepr=IMP.core.Gaussian(psrepr[n])
                    g=gprmf.get_gaussian()
                    grepr.set_gaussian(g)

        else:
            repr_name_particle_map={}
            rmf_name_particle_map={}
            for p in psrmf:
                rmf_name_particle_map[p.get_name()]=p
            #for p in psrepr:
            #    repr_name_particle_map[p.get_name()]=p

            for prepr in psrepr:
                try:
                    prmf=rmf_name_particle_map[representation_name_to_rmf_name_map[prepr.get_name()]]
                except KeyError:
                    print("set_coordinates_from_rmf: WARNING missing particle name in representation_name_to_rmf_name_map, skipping...")
                    continue
                xyz = IMP.core.XYZ(prmf).get_coordinates()
                IMP.core.XYZ(prepr).set_coordinates(xyz)
