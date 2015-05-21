"""@namespace IMP.pmi.restraints.crosslinking
Restraints for handling crosslinking data.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container
import IMP.pmi.tools

class _NuisancesBase(object):
    ''' This base class is used to provide nuisance setup and interface
    for the ISD cross-link restraints '''

    def create_length(self):
        ''' a nuisance on the length of the cross-link '''
        self.lengthinit = 10.0
        self.lengthissampled = True
        self.lengthminnuis = 0.0000001
        self.lengthmaxnuis = 1000.0
        self.lengthmin = 6.0
        self.lengthmax = 30.0
        self.lengthtrans = 0.2
        self.length = IMP.pmi.tools.SetupNuisance(self.m, self.lengthinit,
                                                  self.lengthminnuis, self.lengthmaxnuis, self.lengthissampled).get_particle()
        self.rslen.add_restraint(
            IMP.isd.UniformPrior(
                self.m,
                self.length,
                1000000000.0,
                self.lengthmax,
                self.lengthmin))

    def create_sigma(self, resolution):
        ''' a nuisance on the structural uncertainty '''
        self.sigmainit = resolution + 2.0
        self.sigmaissampled = True
        self.sigmaminnuis = 0.0000001
        self.sigmamaxnuis = 1000.0
        self.sigmamin = 0.01
        self.sigmamax = 100.0
        self.sigmatrans = 0.5
        self.sigma = IMP.pmi.tools.SetupNuisance(self.m, self.sigmainit,
                                                 self.sigmaminnuis, self.sigmamaxnuis, self.sigmaissampled).get_particle()
        self.sigma_dictionary[resolution] = (
            self.sigma,
            self.sigmatrans,
            self.sigmaissampled)
        self.rssig.add_restraint(
            IMP.isd.UniformPrior(
                self.m,
                self.sigma,
                1000000000.0,
                self.sigmamax,
                self.sigmamin))
        # self.rssig.add_restraint(IMP.isd.JeffreysRestraint(self.sigma))

    def get_sigma(self, resolution):
        if not resolution in self.sigma_dictionary:
            self.create_sigma(resolution)
        return self.sigma_dictionary[resolution]

    def create_psi(self, value):
        ''' a nuisance on the inconsistency '''
        self.psiinit = value
        self.psiissampled = True
        self.psiminnuis = 0.0000001
        self.psimaxnuis = 0.4999999
        self.psimin = 0.01
        self.psimax = 0.49
        self.psitrans = 0.1
        self.psi = IMP.pmi.tools.SetupNuisance(self.m, self.psiinit,
                                               self.psiminnuis, self.psimaxnuis,
                                               self.psiissampled).get_particle()
        self.psi_dictionary[value] = (
            self.psi,
            self.psitrans,
            self.psiissampled)
        self.rspsi.add_restraint(
            IMP.isd.UniformPrior(
                self.m,
                self.psi,
                1000000000.0,
                self.psimax,
                self.psimin))
        self.rspsi.add_restraint(IMP.isd.JeffreysRestraint(self.m, self.psi))

    def get_psi(self, value):
        if not value in self.psi_dictionary:
            self.create_psi(value)
        return self.psi_dictionary[value]


class ConnectivityCrossLinkMS(object):

    '''
    this restraint allows ambiguous crosslinking between multiple copies
    it is a variant of the SimplifiedCrossLinkMS
    '''

    def __init__(
        self,
        representation,
        restraints_file,
        expdistance,
        strength=None,
            resolution=None):

        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'data')
        self.weight = 1.0
        self.label = "None"
        self.pairs = []

        self.outputlevel = "low"
        self.expdistance = expdistance
        self.strength = strength

        fl = IMP.pmi.tools.open_file_or_inline_text(restraints_file)

        for line in fl:

            tokens = line.split()
            # skip character
            if (tokens[0] == "#"):
                continue
            r1 = int(tokens[2])
            c1 = tokens[0]
            r2 = int(tokens[3])
            c2 = tokens[1]

            ps1 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c1,
                name_is_ambiguous=True,
                residue=r1)
            hrc1 = [representation.hier_db.particle_to_name[p] for p in ps1]
            if len(ps1) == 0:
                print("ConnectivityCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r1, c1))
                continue

            ps2 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c2,
                name_is_ambiguous=True,
                residue=r2)
            hrc2 = [representation.hier_db.particle_to_name[p] for p in ps2]
            if len(ps2) == 0:
                print("ConnectivityCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r2, c2))
                continue

            s1 = IMP.atom.Selection(ps1)
            s2 = IMP.atom.Selection(ps2)

            # calculate the radii to estimate the slope of the restraint
            if self.strength is None:
                rad1 = 0
                rad2 = 0
                for p in ps1:
                    rad1 += IMP.pmi.Uncertainty(p).get_uncertainty()

                for p in ps2:
                    rad2 += IMP.pmi.Uncertainty(p).get_uncertainty()

                rad1 = rad1 / len(ps1)
                rad2 = rad2 / len(ps2)

                self.strength = 1 / (rad1 ** 2 + rad2 ** 2)

            sels = [s1, s2]
            cr = IMP.atom.create_connectivity_restraint(
                sels,
                self.expdistance,
                self.strength)

            self.rs.add_restraint(cr)
            self.pairs.append((ps1, hrc1, c1, r1, ps2, hrc2, c2, r2, cr))

    def plot_restraint(
        self,
        uncertainty1,
        uncertainty2,
        maxdist=50,
            npoints=10):
        import IMP.pmi.output as output

        p1 = IMP.Particle(self.m)
        p2 = IMP.Particle(self.m)
        d1 = IMP.core.XYZR.setup_particle(p1)
        d2 = IMP.core.XYZR.setup_particle(p2)
        d1.set_radius(uncertainty1)
        d2.set_radius(uncertainty2)
        s1 = IMP.atom.Selection(p1)
        s2 = IMP.atom.Selection(p2)
        sels = [s1, s2]
        strength = 1 / (uncertainty1 ** 2 + uncertainty2 ** 2)
        cr = IMP.atom.create_connectivity_restraint(
            sels,
            self.expdistance,
            strength)
        dists = []
        scores = []
        for i in range(npoints):
            d2.set_coordinates(
                IMP.algebra.Vector3D(maxdist / npoints * float(i), 0, 0))
            dists.append(IMP.core.get_distance(d1, d2))
            scores.append(cr.unprotected_evaluate(None))
        output.plot_xy_data(dists, scores)

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint_sets(self):
        return self.rs

    def get_restraint(self):
        return self.rs

    def set_output_level(self, level="low"):
            # this might be "low" or "high"
        self.outputlevel = level

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_output(self):
        # content of the crosslink database pairs
        # self.pairs.append((p1,p2,dr,r1,c1,r2,c2))
        self.m.update()

        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ConnectivityCrossLinkMS_Score_" + self.label] = str(score)
        for n, p in enumerate(self.pairs):

            ps1 = p[0]
            hrc1 = p[1]
            c1 = p[2]
            r1 = p[3]
            ps2 = p[4]
            hrc2 = p[5]
            c2 = p[6]
            r2 = p[7]
            cr = p[8]
            for n1, p1 in enumerate(ps1):
                name1 = hrc1[n1]

                for n2, p2 in enumerate(ps2):
                    name2 = hrc2[n2]
                    d1 = IMP.core.XYZR(p1)
                    d2 = IMP.core.XYZR(p2)
                    label = str(r1) + ":" + name1 + "_" + str(r2) + ":" + name2
                    output["ConnectivityCrossLinkMS_Distance_" +
                           label] = str(IMP.core.get_distance(d1, d2))

            label = str(r1) + ":" + c1 + "_" + str(r2) + ":" + c2
            output["ConnectivityCrossLinkMS_Score_" +
                   label] = str(self.weight * cr.unprotected_evaluate(None))

        return output


class SimplifiedCrossLinkMS(object):

    def __init__(
        self,
        representation,
        restraints_file,
        expdistance,
        strength,
        resolution=None,
        columnmapping=None,
        truncated=True,
            spheredistancepairscore=True):
        # columnindexes is a list of column indexes for protein1, protein2, residue1, residue2
        # by default column 0 = protein1; column 1 = protein2; column 2 =
        # residue1; column 3 = residue2

        if columnmapping is None:
            columnmapping = {}
            columnmapping["Protein1"] = 0
            columnmapping["Protein2"] = 1
            columnmapping["Residue1"] = 2
            columnmapping["Residue2"] = 3

        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'data')
        self.weight = 1.0
        self.label = "None"
        self.pairs = []
        self.already_added_pairs = {}

        self.outputlevel = "low"
        self.expdistance = expdistance
        self.strength = strength
        self.truncated = truncated
        self.spheredistancepairscore = spheredistancepairscore

        # fill the cross-linker pmfs
        # to accelerate the init the list listofxlinkertypes might contain only
        # yht needed crosslinks
        protein1 = columnmapping["Protein1"]
        protein2 = columnmapping["Protein2"]
        residue1 = columnmapping["Residue1"]
        residue2 = columnmapping["Residue2"]

        fl = IMP.pmi.tools.open_file_or_inline_text(restraints_file)

        for line in fl:

            tokens = line.split()
            # skip character
            if (tokens[0] == "#"):
                continue
            r1 = int(tokens[residue1])
            c1 = tokens[protein1]
            r2 = int(tokens[residue2])
            c2 = tokens[protein2]

            ps1 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c1,
                name_is_ambiguous=False,
                residue=r1)
            ps2 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c2,
                name_is_ambiguous=False,
                residue=r2)

            if len(ps1) > 1:
                raise ValueError(
                   "residue %d of chain %s selects multiple particles; "
                   "particles are: %s"
                   % (r1, c1, "".join(p.get_name() for p in ps1)))
            elif len(ps1) == 0:
                print("SimplifiedCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r1, c1))
                continue

            if len(ps2) > 1:
                raise ValueError(
                   "residue %d of chain %s selects multiple particles; "
                   "particles are: %s"
                   % (r2, c2, "".join(p.get_name() for p in ps2)))
            elif len(ps2) == 0:
                print("SimplifiedCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r2, c2))
                continue

            p1 = ps1[0]
            p2 = ps2[0]

            if (p1, p2) in self.already_added_pairs:
                dr = self.already_added_pairs[(p1, p2)]
                weight = dr.get_weight()
                dr.set_weight(weight + 1.0)
                print("SimplifiedCrossLinkMS> crosslink %d %s %d %s was already found, adding 1.0 to the weight, weight is now %d" % (r1, c1, r2, c2, weight + 1.0))
                continue

            else:

                if self.truncated:
                    limit = self.strength * (self.expdistance + 15) ** 2 + 10.0
                    hub = IMP.core.TruncatedHarmonicUpperBound(
                        self.expdistance,
                        self.strength,
                        self.expdistance +
                        15.,
                        limit)
                else:
                    hub = IMP.core.HarmonicUpperBound(
                        self.expdistance,
                        self.strength)
                if self.spheredistancepairscore:
                    df = IMP.core.SphereDistancePairScore(hub)
                else:
                    df = IMP.core.DistancePairScore(hub)
                dr = IMP.core.PairRestraint(df, (p1, p2))
                dr.set_name(c1 + ":" + str(r1) + "-" + c2 + ":" + str(r2))

                self.rs.add_restraint(dr)
                self.pairs.append((p1, p2, dr, r1, c1, r2, c2))
                self.already_added_pairs[(p1, p2)] = dr
                self.already_added_pairs[(p2, p1)] = dr

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint_sets(self):
        return self.rs

    def get_restraint(self):
        return self.rs

    def get_restraints(self):
        rlist = []
        for r in self.rs.get_restraints():
            rlist.append(IMP.core.PairRestraint.get_from(r))
        return rlist

    def get_particle_pairs(self):
        ppairs = []
        for i in range(len(self.pairs)):
            p0 = self.pairs[i][0]
            p1 = self.pairs[i][1]
            ppairs.append((p0, p1))
        return ppairs

    def set_output_level(self, level="low"):
            # this might be "low" or "high"
        self.outputlevel = level

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def plot_restraint(self, radius1, radius2, maxdist=50, npoints=10):
        import IMP.pmi.output as output

        p1 = IMP.Particle(self.m)
        p2 = IMP.Particle(self.m)
        d1 = IMP.core.XYZR.setup_particle(p1)
        d2 = IMP.core.XYZR.setup_particle(p2)
        d1.set_radius(radius1)
        d2.set_radius(radius2)
        if self.truncated:
            limit = self.strength * (self.expdistance + 15) ** 2 + 10.0
            hub = IMP.core.TruncatedHarmonicUpperBound(
                self.expdistance,
                self.strength,
                self.expdistance +
                15.,
                limit)
        else:
            hub = IMP.core.HarmonicUpperBound(
                self.expdistance,
                self.strength)
        df = IMP.core.SphereDistancePairScore(hub)
        dr = IMP.core.PairRestraint(df, (p1, p2))
        dists = []
        scores = []
        for i in range(npoints):
            d2.set_coordinates(
                IMP.algebra.Vector3D(maxdist / npoints * float(i), 0, 0))
            dists.append(IMP.core.get_distance(d1, d2))
            scores.append(dr.unprotected_evaluate(None))
        output.plot_xy_data(dists, scores)

    def get_output(self):
        # content of the crosslink database pairs
        # self.pairs.append((p1,p2,dr,r1,c1,r2,c2))
        self.m.update()

        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["SimplifiedCrossLinkMS_Score_" + self.label] = str(score)
        for i in range(len(self.pairs)):

            p0 = self.pairs[i][0]
            p1 = self.pairs[i][1]
            crosslinker = 'standard'
            ln = self.pairs[i][2]
            resid1 = self.pairs[i][3]
            chain1 = self.pairs[i][4]
            resid2 = self.pairs[i][5]
            chain2 = self.pairs[i][6]

            label = str(resid1) + ":" + chain1 + "_" + \
                str(resid2) + ":" + chain2
            output["SimplifiedCrossLinkMS_Score_" + crosslinker + "_" +
                   label] = str(self.weight * ln.unprotected_evaluate(None))

            if self.spheredistancepairscore:
                d0 = IMP.core.XYZR(p0)
                d1 = IMP.core.XYZR(p1)
            else:
                d0 = IMP.core.XYZ(p0)
                d1 = IMP.core.XYZ(p1)
            output["SimplifiedCrossLinkMS_Distance_" +
                   label] = str(IMP.core.get_distance(d0, d1))

        return output

#


class SigmoidalCrossLinkMS(object):

    def __init__(
        self, representation, restraints_file, inflection, slope, amplitude,
        linear_slope, resolution=None, columnmapping=None, csvfile=False,
            filters=None, filelabel="None"):
        # columnindexes is a list of column indexes for protein1, protein2, residue1, residue2
        # by default column 0 = protein1; column 1 = protein2; column 2 = residue1; column 3 = residue2
        # the filters applies to the csvfile, the format is
        # filters=[("Field1",">|<|=|>=|<=",value),("Field2","is","String"),("Field2","in","String")]
        import IMP.pmi.tools as tools

        if columnmapping is None:
            columnmapping = {}
            columnmapping["Protein1"] = 0
            columnmapping["Protein2"] = 1
            columnmapping["Residue1"] = 2
            columnmapping["Residue2"] = 3

        if csvfile:
            # in case the file is a cvs file
            # columnmapping will contain the field names
            # that compare in the first line of the cvs file
            db = tools.get_db_from_csv(restraints_file)
        else:
            db = IMP.pmi.tools.open_file_or_inline_text(restraints_file)

        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'data')
        self.weight = 1.0

        self.label = "None"
        self.pairs = []
        self.already_added_pairs = {}
        self.inflection = inflection
        self.slope = slope
        self.amplitude = amplitude
        self.linear_slope = linear_slope

        self.outputlevel = "low"

        # fill the cross-linker pmfs
        # to accelerate the init the list listofxlinkertypes might contain only
        # yht needed crosslinks
        protein1 = columnmapping["Protein1"]
        protein2 = columnmapping["Protein2"]
        residue1 = columnmapping["Residue1"]
        residue2 = columnmapping["Residue2"]

        indb = open("included." + filelabel + ".xl.db", "w")
        exdb = open("excluded." + filelabel + ".xl.db", "w")
        midb = open("missing." + filelabel + ".xl.db", "w")

        for entry in db:
            if not csvfile:
                tokens = entry.split()
                # skip character
                if (tokens[0] == "#"):
                    continue
                r1 = int(tokens[residue1])
                c1 = tokens[protein1]
                r2 = int(tokens[residue2])
                c2 = tokens[protein2]
            else:
                if filters is not None:
                    if eval(tools.cross_link_db_filter_parser(filters)) == False:
                        exdb.write(str(entry) + "\n")
                        continue
                indb.write(str(entry) + "\n")
                r1 = int(entry[residue1])
                c1 = entry[protein1]
                r2 = int(entry[residue2])
                c2 = entry[protein2]

            ps1 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c1,
                name_is_ambiguous=False,
                residue=r1)
            ps2 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c2,
                name_is_ambiguous=False,
                residue=r2)

            if len(ps1) > 1:
                raise ValueError("residue %d of chain %s selects multiple particles %s" % (r1, c1, str(ps1)))
            elif len(ps1) == 0:
                print("SigmoidalCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r1, c1))
                midb.write(str(entry) + "\n")
                continue

            if len(ps2) > 1:
                raise ValueError("residue %d of chain %s selects multiple particles %s" % (r2, c2, str(ps2)))
            elif len(ps2) == 0:
                print("SigmoidalCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r2, c2))
                midb.write(str(entry) + "\n")
                continue

            p1 = ps1[0]
            p2 = ps2[0]

            if (p1, p2) in self.already_added_pairs:
                dr = self.already_added_pairs[(p1, p2)]
                weight = dr.get_weight()
                dr.increment_amplitude(amplitude)
                print("SigmoidalCrossLinkMS> crosslink %d %s %d %s was already found, adding %d to the amplitude, amplitude is now %d" % (r1, c1, r2, c2, amplitude, dr.get_amplitude()))
                dr.set_name(c1 + ":" + str(r1) + "-" + c2 + ":" + str(r2)
                            + "-ampl:" + str(dr.get_amplitude()))
                continue

            else:

                dr = IMP.pmi.SigmoidRestraintSphere(
                    self.m,
                    p1,
                    p2,
                    self.inflection,
                    self.slope,
                    self.amplitude,
                    self.linear_slope)
                dr.set_name(c1 + ":" + str(r1) + "-" + c2 + ":" + str(r2)
                            + "-ampl:" + str(dr.get_amplitude()))

                self.rs.add_restraint(dr)

                self.pairs.append((p1, p2, dr, r1, c1, r2, c2))
                self.already_added_pairs[(p1, p2)] = dr
                self.already_added_pairs[(p2, p1)] = dr

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint_sets(self):
        return self.rs

    def get_restraint(self):
        return self.rs

    def get_restraints(self):
        rlist = []
        for r in self.rs.get_restraints():
            rlist.append(IMP.core.PairRestraint.get_from(r))
        return rlist

    def get_particle_pairs(self):
        ppairs = []
        for i in range(len(self.pairs)):
            p0 = self.pairs[i][0]
            p1 = self.pairs[i][1]
            ppairs.append((p0, p1))
        return ppairs

    def set_output_level(self, level="low"):
            # this might be "low" or "high"
        self.outputlevel = level

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def plot_restraint(self, radius1, radius2, maxdist=50, npoints=10):
        import IMP.pmi.output as output

        p1 = IMP.Particle(self.m)
        p2 = IMP.Particle(self.m)
        d1 = IMP.core.XYZR.setup_particle(p1)
        d2 = IMP.core.XYZR.setup_particle(p2)
        d1.set_radius(radius1)
        d2.set_radius(radius2)
        dr = IMP.pmi.SigmoidRestraintSphere(
            self.m,
            p1,
            p2,
            self.inflection,
            self.slope,
            self.amplitude,
            self.linear_slope)
        dists = []
        scores = []
        for i in range(npoints):
            d2.set_coordinates(
                IMP.algebra.Vector3D(maxdist / npoints * float(i), 0, 0))
            dists.append(IMP.core.get_distance(d1, d2))
            scores.append(dr.unprotected_evaluate(None))
        output.plot_xy_data(dists, scores)

    def get_output(self):
        # content of the crosslink database pairs
        # self.pairs.append((p1,p2,dr,r1,c1,r2,c2))
        self.m.update()

        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["SigmoidalCrossLinkMS_Score_" + self.label] = str(score)
        for i in range(len(self.pairs)):

            p0 = self.pairs[i][0]
            p1 = self.pairs[i][1]
            crosslinker = 'standard'
            ln = self.pairs[i][2]
            resid1 = self.pairs[i][3]
            chain1 = self.pairs[i][4]
            resid2 = self.pairs[i][5]
            chain2 = self.pairs[i][6]

            label = str(resid1) + ":" + chain1 + "_" + \
                str(resid2) + ":" + chain2
            output["SigmoidalCrossLinkMS_Score_" + crosslinker + "_" +
                   label + "_" + self.label] = str(self.weight * ln.unprotected_evaluate(None))
            d0 = IMP.core.XYZR(p0)
            d1 = IMP.core.XYZR(p1)
            output["SigmoidalCrossLinkMS_Distance_" +
                   label + "_" + self.label] = str(IMP.core.get_distance(d0, d1))

        return output


class ISDCrossLinkMS(_NuisancesBase):
    import IMP.isd
    try:
        import IMP.isd_emxl
        no_isd_emxl = False
    except:
        no_isd_emxl = True
    import IMP.pmi.tools
    from math import log

    def __init__(self, representation,
                 restraints_file,
                 length,
                 jackknifing=None,
                 # jackknifing (Float [0,1]) default=None; percentage of data to be
                 # removed randomly
                 resolution=None,
                 # resolution (Non-negative Integer) default=None; percentage of data
                 # to be removed randomly
                 slope=0.0,
                 inner_slope=0.0,
                 columnmapping=None,
                 rename_dict=None,
                 offset_dict=None,
                 csvfile=False,
                 ids_map=None,
                 radius_map=None,
                 filters=None,
                 label="None",
                 filelabel="None",
                 automatic_sigma_classification=False,
                 attributes_for_label=None):

        # columnindexes is a list of column indexes for protein1, protein2, residue1, residue2,idscore, XL unique id
        # by default column 0 = protein1; column 1 = protein2; column 2 = residue1; column 3 = residue2;
        # column 4 = idscores
        # attributes_for_label: anithing in the csv database that must be added to the label
        # slope is the slope defined on the linear function
        # inner_slope is the slope defined on the restraint dierectly
        # suggestion: do not use both!

        if type(representation) != list:
            representations = [representation]
        else:
            representations = representation

        if columnmapping is None:
            columnmapping = {}
            columnmapping["Protein1"] = 0
            columnmapping["Protein2"] = 1
            columnmapping["Residue1"] = 2
            columnmapping["Residue2"] = 3
            columnmapping["IDScore"] = 4
            columnmapping["XLUniqueID"] = 5

        if csvfile:
            # in case the file is a cvs file
            # columnmapping will contain the field names
            # that compare in the first line of the cvs file
            db = IMP.pmi.tools.get_db_from_csv(restraints_file)
        else:
            db = IMP.pmi.tools.open_file_or_inline_text(restraints_file)

        indb = open("included." + filelabel + ".xl.db", "w")
        exdb = open("excluded." + filelabel + ".xl.db", "w")
        midb = open("missing." + filelabel + ".xl.db", "w")

        self.m = representations[0].prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'data')
        self.rspsi = IMP.RestraintSet(self.m, 'prior_psi')
        self.rssig = IMP.RestraintSet(self.m, 'prior_sigmas')
        self.rslin = IMP.RestraintSet(self.m, 'prior_linear')
        self.rslen = IMP.RestraintSet(self.m, 'prior_length')

        self.label = label
        self.pairs = []
        self.sigma_dictionary = {}
        self.psi_dictionary = {}
        self.psi_is_sampled = True
        self.sigma_is_sampled = True

        # isd_map is a dictionary/map that is used to determine the psi
        # parameter from identity scores (such as ID-Score, or FDR)
        if ids_map is None:
            self.ids_map = IMP.pmi.tools.map()
            self.ids_map.set_map_element(20.0, 0.05)
            self.ids_map.set_map_element(65.0, 0.01)
        else:
            self.ids_map = ids_map

        if radius_map is None:
            self.radius_map = IMP.pmi.tools.map()
            if automatic_sigma_classification:
                self.radius_map.set_map_element(10, 10)
            self.radius_map.set_map_element(1, 1)
        else:
            self.radius_map = radius_map

        self.outputlevel = "low"

        # small linear contribution for long range
        self.linear = IMP.core.Linear(0, 0.0)
        self.linear.set_slope(slope)
        dps2 = IMP.core.DistancePairScore(self.linear)

        protein1 = columnmapping["Protein1"]
        protein2 = columnmapping["Protein2"]
        residue1 = columnmapping["Residue1"]
        residue2 = columnmapping["Residue2"]
        idscore = columnmapping["IDScore"]
        try:
            xluniqueid = columnmapping["XLUniqueID"]
        except:
            xluniqueid = None

        restraints = []

        # we need this dictionary to create ambiguity (i.e., multistate)
        # if one id is already present in the dictionary, add the term to the
        # corresponding already geenrated restraint

        uniqueid_restraints_map = {}

        for nxl, entry in enumerate(db):

            if not jackknifing is None:

                # to be implemented
                # the problem is that in the replica exchange
                # you have to broadcast the same restraints to every
                # replica

                raise NotImplementedError("jackknifing not yet implemented")

            if not csvfile:
                tokens = entry.split()
                if len(tokens)==0:
                    continue

                # skip character
                if (tokens[0] == "#"):
                    continue
                try:
                    r1 = int(tokens[residue1])
                    c1 = tokens[protein1]
                    r2 = int(tokens[residue2])
                    c2 = tokens[protein2]

                    if offset_dict is not None:
                        if c1 in offset_dict: r1+=offset_dict[c1]
                        if c2 in offset_dict: r2+=offset_dict[c2]

                    if rename_dict is not None:
                        if c1 in rename_dict: c1=rename_dict[c1]
                        if c2 in rename_dict: c2=rename_dict[c2]

                    if idscore is None:
                        ids = 1.0
                    else:
                        ids = float(tokens[idscore])
                    if xluniqueid is None:
                        xlid = str(nxl)
                    else:
                        xlid = tokens[xluniqueid]
                except:
                    print("this line was not accessible " + str(entry))
                    if residue1 not in entry: print(str(residue1)+" keyword not in database")
                    if residue2 not in entry: print(str(residue2)+" keyword not in database")
                    if protein1 not in entry: print(str(protein1)+" keyword not in database")
                    if protein2 not in entry: print(str(protein2)+" keyword not in database")
                    if idscore not in entry: print(str(idscore)+" keyword not in database")
                    if xluniqueid not in entry: print(str(xluniqueid)+" keyword not in database")
                    continue

            else:
                if filters is not None:
                    if eval(IMP.pmi.tools.cross_link_db_filter_parser(filters)) == False:
                        exdb.write(str(entry) + "\n")
                        continue

                try:
                    r1 = int(entry[residue1])
                    c1 = entry[protein1]
                    r2 = int(entry[residue2])
                    c2 = entry[protein2]

                    if offset_dict is not None:
                        if c1 in offset_dict: r1+=offset_dict[c1]
                        if c2 in offset_dict: r2+=offset_dict[c2]

                    if rename_dict is not None:
                        if c1 in rename_dict: c1=rename_dict[c1]
                        if c2 in rename_dict: c2=rename_dict[c2]

                    if idscore is None:
                        ids = 1.0
                    else:
                        try:
                            ids = float(entry[idscore])
                        except ValueError:
                            ids = entry[idscore]
                    if xluniqueid is None:
                        xlid = str(nxl)
                    else:
                        xlid = entry[xluniqueid]

                except:
                    print("this line was not accessible " + str(entry))
                    if residue1 not in entry: print(str(residue1)+" keyword not in database")
                    if residue2 not in entry: print(str(residue2)+" keyword not in database")
                    if protein1 not in entry: print(str(protein1)+" keyword not in database")
                    if protein2 not in entry: print(str(protein2)+" keyword not in database")
                    if idscore not in entry: print(str(idscore)+" keyword not in database")
                    if xluniqueid not in entry: print(str(xluniqueid)+" keyword not in database")
                    continue

            for nstate, r in enumerate(representations):
                # loop over every state

                ps1 = IMP.pmi.tools.select(
                    r,
                    resolution=resolution,
                    name=c1,
                    name_is_ambiguous=False,
                    residue=r1)
                ps2 = IMP.pmi.tools.select(
                    r,
                    resolution=resolution,
                    name=c2,
                    name_is_ambiguous=False,
                    residue=r2)

                if len(ps1) > 1:
                    raise ValueError("residue %d of chain %s selects multiple particles %s" % (r1, c1, str(ps1)))
                elif len(ps1) == 0:
                    print("ISDCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r1, c1))
                    midb.write(str(entry) + "\n")
                    continue

                if len(ps2) > 1:
                    raise ValueError("residue %d of chain %s selects multiple particles %s" % (r2, c2, str(ps2)))
                elif len(ps2) == 0:
                    print("ISDCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r2, c2))
                    midb.write(str(entry) + "\n")
                    continue

                p1 = ps1[0]
                p2 = ps2[0]

                # remove in the future!!!
                #if p1 == p2:
                #    continue

                if xlid in uniqueid_restraints_map:
                    print("getting a crosslink restraint from id %s" % str(xlid))
                    dr = uniqueid_restraints_map[xlid]
                else:
                    print("generating a new crosslink restraint")
                    dr = IMP.isd.CrossLinkMSRestraint(
                        self.m,
                        length,
                        inner_slope)
                    restraints.append(dr)
                    uniqueid_restraints_map[xlid] = dr

                mappedr1 = self.radius_map.get_map_element(
                    IMP.pmi.Uncertainty(p1).get_uncertainty())
                sigma1 = self.get_sigma(mappedr1)[0]
                mappedr2 = self.radius_map.get_map_element(
                    IMP.pmi.Uncertainty(p2).get_uncertainty())
                sigma2 = self.get_sigma(mappedr2)[0]

                psival = self.ids_map.get_map_element(ids)
                psi = self.get_psi(psival)[0]


                p1i = p1.get_particle_index()
                p2i = p2.get_particle_index()
                s1i = sigma1.get_particle().get_index()
                s2i = sigma2.get_particle().get_index()

                #print nstate, p1i, p2i, p1.get_name(), p2.get_name()

                psii = psi.get_particle().get_index()

                dr.add_contribution((p1i, p2i), (s1i, s2i), psii)
                print("--------------")
                print("ISDCrossLinkMS: generating cross-link restraint between")
                print("ISDCrossLinkMS: residue %d of chain %s and residue %d of chain %s" % (r1, c1, r2, c2))
                print("ISDCrossLinkMS: with sigma1 %f sigma2 %f psi %s" % (mappedr1, mappedr2, psival))
                print("ISDCrossLinkMS: between particles %s and %s" % (p1.get_name(), p2.get_name()))
                print("==========================================\n")
                indb.write(str(entry) + "\n")

                # check if the two residues belong to the same rigid body
                if(IMP.core.RigidMember.get_is_setup(p1) and
                   IMP.core.RigidMember.get_is_setup(p2) and
                   IMP.core.RigidMember(p1).get_rigid_body() ==
                   IMP.core.RigidMember(p2).get_rigid_body()):
                    xlattribute = "intrarb"
                else:
                    xlattribute = "interrb"

                if csvfile:
                    if not attributes_for_label is None:
                        for a in attributes_for_label:
                            xlattribute = xlattribute + "_" + str(entry[a])

                xlattribute = xlattribute + "-State:" + str(nstate)

                dr.set_name(
                    xlattribute + "-" + c1 + ":" + str(r1) + "-" + c2 + ":" + str(r2) + "_" + self.label)

                pr = IMP.core.PairRestraint(self.m, dps2, (p1, p2))
                pr.set_name(
                    xlattribute + "-" + c1 + ":" + str(r1) + "-" + c2 + ":" + str(r2) + "_" + self.label)
                self.rslin.add_restraint(pr)


                self.pairs.append(
                    (p1,
                     p2,
                     dr,
                     r1,
                     c1,
                     r2,
                     c2,
                     xlattribute,
                     mappedr1,
                     mappedr2,
                     psival,
                     xlid,
                     nstate,
                     ids))

        lw = IMP.isd.LogWrapper(restraints,1.0)
        self.rs.add_restraint(lw)


    def set_slope_linear_term(self, slope):
        self.linear.set_slope(slope)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rspsi)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rssig)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rslen)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rslin)

    def get_hierarchies(self):
        return self.prot

    def get_restraint_sets(self):
        return self.rs

    def get_restraint(self):
        return self.rs

    def get_restraint_for_rmf(self):
        return self.rslin

    def get_restraints(self):
        rlist = []
        for r in self.rs.get_restraints():
            rlist.append(IMP.core.PairRestraint.get_from(r))
        return rlist

    def get_particle_pairs(self):
        ppairs = []
        for i in range(len(self.pairs)):
            p0 = self.pairs[i][0]
            p1 = self.pairs[i][1]
            ppairs.append((p0, p1))
        return ppairs

    def set_output_level(self, level="low"):
            # this might be "low" or "high"
        self.outputlevel = level

    def set_psi_is_sampled(self, is_sampled=True):
        self.psi_is_sampled = is_sampled

    def set_sigma_is_sampled(self, is_sampled=True):
        self.sigma_is_sampled = is_sampled

    def get_label(self,pairs_index):
        resid1 = self.pairs[pairs_index][3]
        chain1 = self.pairs[pairs_index][4]
        resid2 = self.pairs[pairs_index][5]
        chain2 = self.pairs[pairs_index][6]
        attribute = self.pairs[pairs_index][7]
        rad1 = self.pairs[pairs_index][8]
        rad2 = self.pairs[pairs_index][9]
        psi = self.pairs[pairs_index][10]
        xlid= self.pairs[pairs_index][11]
        label = attribute + "-" + \
            str(resid1) + ":" + chain1 + "_" + str(resid2) + ":" + \
            chain2 + "-" + str(rad1) + "-" + str(rad2) + "-" + str(psi)
        return label

    def write_db(self,filename):
        import IMP.pmi.output
        cldb=IMP.pmi.output.CrossLinkIdentifierDatabase()

        for pairs_index in range(len(self.pairs)):

            resid1 = self.pairs[pairs_index][3]
            chain1 = self.pairs[pairs_index][4]
            resid2 = self.pairs[pairs_index][5]
            chain2 = self.pairs[pairs_index][6]
            attribute = self.pairs[pairs_index][7]
            rad1 = self.pairs[pairs_index][8]
            rad2 = self.pairs[pairs_index][9]
            psi = self.pairs[pairs_index][10]
            xlid= self.pairs[pairs_index][11]
            nstate=self.pairs[pairs_index][12]
            ids=self.pairs[pairs_index][13]

            label=self.get_label(pairs_index)
            cldb.set_unique_id(label,xlid)
            cldb.set_protein1(label,chain1)
            cldb.set_protein2(label,chain2)
            cldb.set_residue1(label,resid1)
            cldb.set_residue2(label,resid2)
            cldb.set_idscore(label,ids)
            cldb.set_state(label,nstate)
            cldb.set_sigma1(label,rad1)
            cldb.set_sigma2(label,rad2)
            cldb.set_psi(label,psi)
            cldb.write(filename)


    def get_output(self):
        # content of the crosslink database pairs
        # self.pairs.append((p1,p2,dr,r1,c1,r2,c2))
        self.m.update()

        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ISDCrossLinkMS_Data_Score_" + self.label] = str(score)
        output["ISDCrossLinkMS_PriorSig_Score_" +
               self.label] = self.rssig.unprotected_evaluate(None)
        output["ISDCrossLinkMS_PriorPsi_Score_" +
                   self.label] = self.rspsi.unprotected_evaluate(None)
        output["ISDCrossLinkMS_Linear_Score_" +
               self.label] = self.rslin.unprotected_evaluate(None)
        for i in range(len(self.pairs)):

            label=self.get_label(i)
            ln = self.pairs[i][2]
            p0 = self.pairs[i][0]
            p1 = self.pairs[i][1]
            output["ISDCrossLinkMS_Score_" +
                   label + "_" + self.label] = str(-self.log(ln.unprotected_evaluate(None)))

            d0 = IMP.core.XYZ(p0)
            d1 = IMP.core.XYZ(p1)
            output["ISDCrossLinkMS_Distance_" +
                   label + "_" + self.label] = str(IMP.core.get_distance(d0, d1))


        for psiindex in self.psi_dictionary:
            output["ISDCrossLinkMS_Psi_" +
                    str(psiindex) + "_" + self.label] = str(self.psi_dictionary[psiindex][0].get_scale())

        for resolution in self.sigma_dictionary:
            output["ISDCrossLinkMS_Sigma_" +
                   str(resolution) + "_" + self.label] = str(self.sigma_dictionary[resolution][0].get_scale())


        return output

    def get_particles_to_sample(self):
        ps = {}

        for resolution in self.sigma_dictionary:
            if self.sigma_dictionary[resolution][2] and self.sigma_is_sampled:
                ps["Nuisances_ISDCrossLinkMS_Sigma_" + str(resolution) + "_" + self.label] =\
                    ([self.sigma_dictionary[resolution][0]],
                     self.sigma_dictionary[resolution][1])

        if self.psi_is_sampled:
            for psiindex in self.psi_dictionary:
                if self.psi_dictionary[psiindex][2]:
                    ps["Nuisances_ISDCrossLinkMS_Psi_" +
                        str(psiindex) + "_" + self.label] = ([self.psi_dictionary[psiindex][0]], self.psi_dictionary[psiindex][1])

        return ps

#
class CysteineCrossLinkRestraint(object):

    import IMP.isd
    import IMP.pmi.tools

    def __init__(self, representations, filename, cbeta=False,
                 betatuple=(0.03, 0.1),
                 disttuple=(0.0, 25.0, 1000),
                 omegatuple=(1.0, 1000.0, 50),
                 sigmatuple=(0.3, 0.3, 1),
                 betaissampled=True,
                 sigmaissampled=False,
                 weightissampled=True,
                 epsilonissampled=True
                 ):
    # the file must have residue1 chain1 residue2 chain2 fractionvalue epsilonname
    # epsilonname is a name for the epsilon particle that must be used for that particular
    # residue pair, eg, "Epsilon-Intra-Solvent", or
    # "Epsilon-Solvent-Membrane", etc.

        self.representations = representations
        self.m = self.representations[0].prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'Cysteine_Crosslink')
        self.cbeta = cbeta
        self.epsilonmaxtrans = 0.01
        self.sigmamaxtrans = 0.1
        self.betamaxtrans = 0.01
        self.weightmaxtrans = 0.1
        self.label = "None"
        self.outputlevel = "low"
        self.betaissampled = betaissampled
        self.sigmaissampled = sigmaissampled
        self.weightissampled = weightissampled
        self.epsilonissampled = epsilonissampled

        betalower = betatuple[0]
        betaupper = betatuple[1]
        beta = 0.04
        sigma = 10.0
        betangrid = 30
        crossdataprior = 1

        # beta
        self.beta = IMP.pmi.tools.SetupNuisance(
            self.m,
            beta,
            betalower,
            betaupper,
            betaissampled).get_particle(
        )
        # sigma
        self.sigma = IMP.pmi.tools.SetupNuisance(
            self.m,
            sigma,
            sigmatuple[0],
            sigmatuple[1],
            sigmaissampled).get_particle()
        # population particle
        self.weight = IMP.pmi.tools.SetupWeight(
            self.m,
            weightissampled).get_particle(
        )

        # read the file
        fl = IMP.pmi.tools.open_file_or_inline_text(filename)
        # determine the upperlimit for epsilon
        # and also how many epsilon are needed
        self.epsilons = {}
        data = []
        for line in fl:
            t = line.split()
            if t[0][0] == "#":
                continue
            if t[5] in self.epsilons:
                if 1.0 - float(t[4]) <= self.epsilons[t[5]].get_upper():
                    self.epsilons[t[5]].set_upper(1.0 - float(t[4]))
            else:
                self.epsilons[t[5]] = IMP.pmi.tools.SetupNuisance(self.m,
                                                                  0.01, 0.01, 1.0 - float(t[4]), epsilonissampled).get_particle()
            up = self.epsilons[t[5]].get_upper()
            low = self.epsilons[t[5]].get_lower()
            if up < low:
                self.epsilons[t[5]].set_upper(low)

            data.append((int(t[0]), t[1], int(t[2]), t[3], float(t[4]), t[5]))

        # create CrossLinkData
        if not self.cbeta:
            crossdata = IMP.pmi.tools.get_cross_link_data(
                "cysteine", "cysteine_CA_FES.txt.standard",
                disttuple, omegatuple, sigmatuple, disttuple[1], disttuple[1], 1)
        else:
            crossdata = IMP.pmi.tools.get_cross_link_data(
                "cysteine", "cysteine_CB_FES.txt.standard",
                disttuple, omegatuple, sigmatuple, disttuple[1], disttuple[1], 1)

        # create grids needed by CysteineCrossLinkData
        fmod_grid = IMP.pmi.tools.get_grid(0.0, 1.0, 300, True)
        omega2_grid = IMP.pmi.tools.get_log_grid(0.001, 10000.0, 100)
        beta_grid = IMP.pmi.tools.get_log_grid(betalower, betaupper, betangrid)

        for d in data:
            print("--------------")
            print("CysteineCrossLink: attempting to create a restraint " + str(d))
            resid1 = d[0]
            chain1 = d[1]
            resid2 = d[2]
            chain2 = d[3]
            fexp = d[4]
            epslabel = d[5]

            # CysteineCrossLinkData

            ccldata = IMP.isd.CysteineCrossLinkData(
                fexp,
                fmod_grid,
                omega2_grid,
                beta_grid)

            ccl = IMP.isd.CysteineCrossLinkRestraint(
                self.beta,
                self.sigma,
                self.epsilons[epslabel],
                self.weight,
                crossdata,
                ccldata)

            failed = False
            for i, representation in enumerate(self.representations):

                if not self.cbeta:
                    p1 = None
                    p2 = None

                    p1 = IMP.pmi.tools.select(representation,
                                              resolution=1, name=chain1,
                                              name_is_ambiguous=False, residue=resid1)[0]

                    if p1 is None:
                        failed = True

                    p2 = IMP.pmi.tools.select(representation,
                                              resolution=1, name=chain2,
                                              name_is_ambiguous=False, residue=resid2)[0]

                    if p2 is None:
                        failed = True

                else:
                    # use cbetas
                    p1 = []
                    p2 = []
                    for t in range(-1, 2):
                        p = IMP.pmi.tools.select(representation,
                                                 resolution=1, name=chain1,
                                                 name_is_ambiguous=False, residue=resid1 + t)

                        if len(p) == 1:
                            p1 += p
                        else:
                            failed = True
                            print("\033[93m CysteineCrossLink: missing representation for residue %d of chain %s \033[0m" % (resid1 + t, chain1))

                        p = IMP.pmi.tools.select(representation,
                                                 resolution=1, name=chain2,
                                                 name_is_ambiguous=False, residue=resid2 + t)

                        if len(p) == 1:
                            p2 += p
                        else:
                            failed = True
                            print("\033[93m CysteineCrossLink: missing representation for residue %d of chain %s \033[0m" % (resid2 + t, chain2))

                if not self.cbeta:
                    if (p1 is not None and p2 is not None):
                        ccl.add_contribution(p1, p2)
                        d1 = IMP.core.XYZ(p1)
                        d2 = IMP.core.XYZ(p2)

                        print("Distance_" + str(resid1) + "_" + chain1 + ":" + str(resid2) + "_" + chain2, IMP.core.get_distance(d1, d2))

                else:
                    if (len(p1) == 3 and len(p2) == 3):
                        p11n = p1[0].get_name()
                        p12n = p1[1].get_name()
                        p13n = p1[2].get_name()
                        p21n = p2[0].get_name()
                        p22n = p2[1].get_name()
                        p23n = p2[2].get_name()

                        print("CysteineCrossLink: generating CB cysteine cross-link restraint between")
                        print("CysteineCrossLink: residue %d of chain %s and residue %d of chain %s" % (resid1, chain1, resid2, chain2))
                        print("CysteineCrossLink: between particles %s %s %s and %s %s %s" % (p11n, p12n, p13n, p21n, p22n, p23n))

                        ccl.add_contribution(p1, p2)

            if not failed:
                self.rs.add_restraint(ccl)
                ccl.set_name("CysteineCrossLink_" + str(resid1)
                             + "_" + chain1 + ":" + str(resid2) + "_" + chain2)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_particles_to_sample(self):
        ps = {}
        if self.epsilonissampled:
            for eps in self.epsilons.keys():
                ps["Nuisances_CysteineCrossLinkRestraint_epsilon_" + eps + "_" +
                    self.label] = ([self.epsilons[eps]], self.epsilonmaxtrans)
        if self.betaissampled:
            ps["Nuisances_CysteineCrossLinkRestraint_beta_" +
                self.label] = ([self.beta], self.betamaxtrans)
        if self.weightissampled:
            ps["Weights_CysteineCrossLinkRestraint_" +
                self.label] = ([self.weight], self.weightmaxtrans)
        if self.sigmaissampled:
            ps["Nuisances_CysteineCrossLinkRestraint_" +
                self.label] = ([self.sigma], self.sigmamaxtrans)
        return ps

    def set_output_level(self, level="low"):
                # this might be "low" or "high"
        self.outputlevel = level

    def get_restraint(self):
        return self.rs

    def get_sigma(self):
        return self.sigma

    def get_output(self):
        self.m.update()
        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["CysteineCrossLinkRestraint_Score_" + self.label] = str(score)
        output["CysteineCrossLinkRestraint_sigma_" +
               self.label] = str(self.sigma.get_scale())
        for eps in self.epsilons.keys():
            output["CysteineCrossLinkRestraint_epsilon_" + eps + "_" +
                   self.label] = str(self.epsilons[eps].get_scale())
        output["CysteineCrossLinkRestraint_beta_" +
               self.label] = str(self.beta.get_scale())
        for n in range(self.weight.get_number_of_states()):
            output["CysteineCrossLinkRestraint_weights_" +
                   str(n) + "_" + self.label] = str(self.weight.get_weight(n))

        if self.outputlevel == "high":
            for rst in self.rs.get_restraints():
                output["CysteineCrossLinkRestraint_Total_Frequency_" +
                       IMP.isd.CysteineCrossLinkRestraint.get_from(rst).get_name() +
                       "_" + self.label] = IMP.isd.CysteineCrossLinkRestraint.get_from(rst).get_model_frequency()
                output["CysteineCrossLinkRestraint_Standard_Error_" +
                       IMP.isd.CysteineCrossLinkRestraint.get_from(
                           rst).get_name(
                       ) + "_"
                       + self.label] = IMP.isd.CysteineCrossLinkRestraint.get_from(rst).get_standard_error()
                if len(self.representations) > 1:
                    for i in range(len(self.prots)):
                        output["CysteineCrossLinkRestraint_Frequency_Contribution_" +
                               IMP.isd.CysteineCrossLinkRestraint.get_from(rst).get_name() +
                               "_State_" + str(i) + "_" + self.label] = IMP.isd.CysteineCrossLinkRestraint.get_from(rst).get_frequencies()[i]

        return output
