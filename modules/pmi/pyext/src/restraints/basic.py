"""@namespace IMP.pmi.restraints.basic
Some miscellaneous simple restraints.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container
import IMP.pmi.tools
import IMP.pmi.restraints
import math


class ExternalBarrier(IMP.pmi.restraints.RestraintBase):
    """Keeps all structures inside a sphere."""

    def __init__(self, hierarchies, radius=10.0, resolution=10, weight=1.0,
                 center=None, label=None):
        """Setup external barrier restraint.
        @param hierarchies Can be one of the following inputs: IMP Hierarchy,
               PMI System/State/Molecule/TempResidue, or a list/set of them
        @param radius Size of external barrier
        @param resolution Select which resolutions to act upon
        @param weight Weight of restraint
        @param center Center of the external barrier
               (IMP.algebra.Vector3D object)
        @param label A unique label to be used in outputs and
                     particle/restraint names.
        """
        hiers = IMP.pmi.tools.input_adaptor(hierarchies, resolution,
                                            flatten=True)
        model = hiers[0].get_model()
        particles = [h.get_particle() for h in hiers]

        super(ExternalBarrier, self).__init__(model, label=label,
                                              weight=weight)
        self.radius = radius

        if center is None:
            c3 = IMP.algebra.Vector3D(0, 0, 0)
        elif type(center) is IMP.algebra.Vector3D:
            c3 = center
        else:
            raise Exception(
                "%s: @param center must be an IMP.algebra.Vector3D object" % (
                    self.name))

        ub3 = IMP.core.HarmonicUpperBound(radius, 10.0)
        ss3 = IMP.core.DistanceToSingletonScore(ub3, c3)
        lsc = IMP.container.ListSingletonContainer(self.model)

        lsc.add(particles)
        r3 = IMP.container.SingletonsRestraint(ss3, lsc)
        self.rs.add_restraint(r3)


class DistanceRestraint(IMP.pmi.restraints.RestraintBase):
    """A simple distance restraint"""

    def __init__(self, root_hier, tuple_selection1, tuple_selection2,
                 distancemin=0, distancemax=100, resolution=1.0, kappa=1.0,
                 label=None, weight=1.):
        """Setup distance restraint.
        @param root_hier The hierarchy to select from
        @param tuple_selection1 (resnum, resnum, molecule name, copy
               number (=0))
        @param tuple_selection2 (resnum, resnum, molecule name, copy
               number (=0))
        @param distancemin The minimum dist
        @param distancemax The maximum dist
        @param resolution For selecting particles
        @param kappa The harmonic parameter
        @param label A unique label to be used in outputs and
                     particle/restraint names
        @param weight Weight of restraint
        @note Pass the same resnum twice to each tuple_selection. Optionally
              add a copy number.
        """
        ts1 = IMP.core.HarmonicUpperBound(distancemax, kappa)
        ts2 = IMP.core.HarmonicLowerBound(distancemin, kappa)

        model = root_hier.get_model()
        copy_num1 = 0
        if len(tuple_selection1) > 3:
            copy_num1 = tuple_selection1[3]
        copy_num2 = 0
        if len(tuple_selection2) > 3:
            copy_num2 = tuple_selection2[3]

        sel1 = IMP.atom.Selection(root_hier,
                                  resolution=resolution,
                                  molecule=tuple_selection1[2],
                                  residue_index=tuple_selection1[0],
                                  copy_index=copy_num1)
        particles1 = sel1.get_selected_particles()
        sel2 = IMP.atom.Selection(root_hier,
                                  resolution=resolution,
                                  molecule=tuple_selection2[2],
                                  residue_index=tuple_selection2[0],
                                  copy_index=copy_num2)
        particles2 = sel2.get_selected_particles()

        super(DistanceRestraint, self).__init__(model, label=label,
                                                weight=weight)
        print(self.name)

        print("Created distance restraint between "
              "%s and %s" % (particles1[0].get_name(),
                             particles2[0].get_name()))

        if len(particles1) > 1 or len(particles2) > 1:
            raise ValueError("more than one particle selected")

        self.rs.add_restraint(
            IMP.core.DistanceRestraint(self.model, ts1,
                                       particles1[0],
                                       particles2[0]))
        self.rs.add_restraint(
            IMP.core.DistanceRestraint(self.model, ts2,
                                       particles1[0],
                                       particles2[0]))


class CylinderRestraint(IMP.Restraint):
    """Restrain particles within (or outside) a cylinder.
       The cylinder is aligned along the z-axis and with center x=y=0.
       Optionally, one can restrain the cylindrical angle
    """
    import math

    def __init__(self, m, objects, resolution, radius, mintheta=None,
                 maxtheta=None, repulsive=False, label='None'):
        '''
        @param objects PMI2 objects to restrain
        @param resolution the resolution you want the restraint to be applied
        @param radius the radius of the cylinder
        @param mintheta minimum cylindrical angle in degrees
        @param maxtheta maximum cylindrical angle in degrees
        @param repulsive If True, restrain the particles to be outside
               of the cylinder instead of inside
        @param label A unique label to be used in outputs and
               particle/restraint names
        '''
        IMP.Restraint.__init__(self, m, "CylinderRestraint %1%")
        self.radius = radius
        self.softness = 3.0
        self.softness_angle = 0.5
        self.plateau = 1e-10
        self.weight = 1.0
        self.m = m
        self.mintheta = mintheta
        self.maxtheta = maxtheta
        self.repulsive = repulsive
        hierarchies = IMP.pmi.tools.input_adaptor(objects,
                                                  resolution,
                                                  flatten=True)
        self.particles = [h.get_particle() for h in hierarchies]
        self.label = label

    def get_probability(self, p):
        xyz = IMP.core.XYZ(p)
        r = self.math.sqrt(xyz.get_x()**2+xyz.get_y()**2)
        argvalue = (r-self.radius) / self.softness
        if self.repulsive:
            argvalue = -argvalue
        prob = (1.0 - self.plateau) / (1.0 + self.math.exp(-argvalue))
        return prob

    def get_angle_probability(self, p):
        xyz = IMP.core.XYZ(p)
        angle = self.math.atan2(xyz.get_y(), xyz.get_x())*180.0/self.math.pi
        anglediff = (angle - self.maxtheta + 180 + 360) % 360 - 180
        argvalue1 = anglediff / self.softness_angle
        anglediff = (angle - self.mintheta + 180 + 360) % 360 - 180
        argvalue2 = -anglediff / self.softness_angle
        prob = ((1.0-self.plateau)
                / (1.0 + self.math.exp(-max(argvalue1, argvalue2))))
        return prob

    def unprotected_evaluate(self, da):
        s = 0.0
        for p in self.particles:
            s += -self.math.log(1.0-self.get_probability(p))
            if self.mintheta is not None and self.maxtheta is not None:
                s += -self.math.log(1.0-self.get_angle_probability(p))
        return s

    def do_get_inputs(self):
        return self.particles

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self)

    def get_output(self):
        output = {}
        score = self.weight * self.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["CylinderRestraint_" + self.label] = str(score)
        return output


class BiStableDistanceRestraint(IMP.Restraint):
    '''Distance restraint with bistable potential
    Authors: G. Bouvier, R. Pellarin. Pasteur Institute.
    '''
    import numpy as np
    import math

    def __init__(self, m, p1, p2, dist1, dist2, sigma1, sigma2, weight1,
                 weight2):
        '''
        input two particles, the two equilibrium distances, their amplitudes,
        and their weights (populations)
        '''
        IMP.Restraint.__init__(self, m, "BiStableDistanceRestraint %1%")
        self.dist1 = dist1
        self.dist2 = dist2

        self.sigma1 = sigma1
        self.sigma2 = sigma2

        self.weight1 = weight1
        self.weight2 = weight2

        if self.weight1+self.weight2 != 1:
            raise ValueError("The sum of the weights must be one")

        self.d1 = IMP.core.XYZ(p1)
        self.d2 = IMP.core.XYZ(p2)
        self.particle_list = [p1, p2]

    def gaussian(self, x, mu, sig, w):
        return (w*self.np.exp(-self.np.power(x - mu, 2.)
                / (2 * self.np.power(sig, 2.))))

    def unprotected_evaluate(self, da):
        dist = IMP.core.get_distance(self.d1, self.d2)
        prob = self.gaussian(dist, self.dist1, self.sigma1, self.weight1) + \
            self.gaussian(dist, self.dist2, self.sigma2, self.weight2)
        return -self.math.log(prob)

    def do_get_inputs(self):
        return self.particle_list


class DistanceToPointRestraint(IMP.pmi.restraints.RestraintBase):
    """Anchor a particle to a specific coordinate."""

    def __init__(self, root_hier, tuple_selection,
                 anchor_point=IMP.algebra.Vector3D(0, 0, 0),
                 radius=10.0, kappa=10.0, resolution=1.0, weight=1.0,
                 label=None):
        """Setup distance restraint.
        @param root_hier The hierarchy to select from
        @param tuple_selection (resnum, resnum, molecule name,
               copy number (=0))
        @param anchor_point Point to which to restrain particle
               (IMP.algebra.Vector3D object)
        @param radius Size of the tolerance length
        @param kappa Strength of the harmonic restraint
        @param resolution For selecting a particle
        @param weight Weight of restraint
        @param label A unique label to be used in outputs and
                     particle/restraint names
        @note Pass the same resnum twice to each tuple_selection. Optionally
              add a copy number
        """
        model = root_hier.get_model()
        copy_num1 = 0
        if len(tuple_selection) > 3:
            copy_num1 = tuple_selection[3]

        sel1 = IMP.atom.Selection(root_hier,
                                  resolution=resolution,
                                  molecule=tuple_selection[2],
                                  residue_index=tuple_selection[0],
                                  copy_index=copy_num1)
        ps = sel1.get_selected_particles()
        if len(ps) > 1:
            raise ValueError("More than one particle selected")

        super(DistanceToPointRestraint, self).__init__(model, label=label,
                                                       weight=weight)
        self.radius = radius

        ub3 = IMP.core.HarmonicUpperBound(self.radius, kappa)
        if anchor_point is None:
            c3 = IMP.algebra.Vector3D(0, 0, 0)
        elif isinstance(anchor_point, IMP.algebra.Vector3D):
            c3 = anchor_point
        else:
            raise TypeError("anchor_point must be an algebra.Vector3D object")
        ss3 = IMP.core.DistanceToSingletonScore(ub3, c3)

        lsc = IMP.container.ListSingletonContainer(self.model)
        lsc.add(ps)

        r3 = IMP.container.SingletonsRestraint(ss3, lsc)
        self.rs.add_restraint(r3)

        print("\n%s: Created distance_to_point_restraint between "
              "%s and %s" % (self.name, ps[0].get_name(), c3))


class MembraneRestraint(IMP.pmi.restraints.RestraintBase):
    """Restrain particles to be above, below, or inside a planar membrane.
       The membrane is defined to lie on the xy plane with a given z
       coordinate and thickness, and particles are restrained (by their
       z coordinates) with a simple sigmoid score.
    """

    def __init__(self, hier, objects_above=None, objects_inside=None,
                 objects_below=None, center=0.0, thickness=30.0,
                 softness=3.0, plateau=0.0000000001, resolution=1,
                 weight=1.0, label=None):
        """Setup the restraint.

        @param objects_inside list or tuples of objects in membrane
               (e.g. ['p1', (10, 30,'p2')])
        @param objects_above list or tuples of objects above membrane
        @param objects_below list or tuples of objects below membrane
        @param thickness Thickness of the membrane along the z-axis
        @param softness Softness of the limiter in the sigmoid function
        @param plateau Parameter to set the probability (=1- plateau))
               at the plateau phase of the sigmoid
        @param weight Weight of restraint
        @param label A unique label to be used in outputs and
                     particle/restraint names.
        """

        self.hier = hier
        model = self.hier.get_model()

        super(MembraneRestraint, self).__init__(
            model, name="MembraneRestraint", label=label, weight=weight)

        self.center = center
        self.thickness = thickness
        self.softness = softness
        self.plateau = plateau
        self.linear = 0.02
        self.resolution = resolution

        # Create nuisance particle
        p = IMP.Particle(model)
        z_center = IMP.isd.Nuisance.setup_particle(p)
        z_center.set_nuisance(self.center)

        # Setup restraint
        mr = IMP.pmi.MembraneRestraint(model,
                                       z_center.get_particle_index(),
                                       self.thickness,
                                       self.softness,
                                       self.plateau,
                                       self.linear)

        # Particles above
        if objects_above:
            for obj in objects_above:
                if isinstance(obj, tuple):
                    self.particles_above = self._select_from_tuple(obj)

                elif isinstance(obj, str):
                    self.particles_above = self._select_from_string(obj)
                mr.add_particles_above(self.particles_above)

        # Particles inside
        if objects_inside:
            for obj in objects_inside:
                if isinstance(obj, tuple):
                    self.particles_inside = self._select_from_tuple(obj)

                elif isinstance(obj, str):
                    self.particles_inside = self._select_from_string(obj)
                mr.add_particles_inside(self.particles_inside)

        # Particles below
        if objects_below:
            for obj in objects_below:
                if isinstance(obj, tuple):
                    self.particles_below = self._select_from_tuple(obj)

                elif isinstance(obj, str):
                    self.particles_below = self._select_from_string(obj)
                mr.add_particles_below(self.particles_below)

        self.rs.add_restraint(mr)

    def get_particles_above(self):
        return self.particles_above

    def get_particles_inside(self):
        return self.particles_inside

    def get_particles_below(self):
        return self.particles_below

    def _select_from_tuple(self, obj):
        particles = IMP.atom.Selection(
            self.hier, molecule=obj[2],
            residue_indexes=range(obj[0], obj[1]+1, 1),
            resolution=self.resolution).get_selected_particles()

        return particles

    def _select_from_string(self, obj):
        particles = IMP.atom.Selection(
            self.hier, molecule=obj,
            resolution=self.resolution).get_selected_particles()
        return particles

    def create_membrane_density(self, file_out='membrane_localization.mrc'):
        """Create an MRC density file to visualize the membrane."""
        offset = 5.0 * self.thickness
        apix = 3.0
        resolution = 5.0

        # Create a density header of the requested size
        bbox = IMP.algebra.BoundingBox3D(
            IMP.algebra.Vector3D(-self.center - offset, -self.center - offset,
                                 -self.center - offset),
            IMP.algebra.Vector3D(self.center + offset, self.center + offset,
                                 self.center + offset))
        dheader = IMP.em.create_density_header(bbox, apix)
        dheader.set_resolution(resolution)
        dmap = IMP.em.SampledDensityMap(dheader)

        for vox in range(dmap.get_header().get_number_of_voxels()):
            c = dmap.get_location_by_voxel(vox)
            if self._is_membrane(c[2]) == 1:
                dmap.set_value(c[0], c[1], c[2], 1.0)
            else:
                dmap.set_value(c[0], c[1], c[2], 0.0)

        IMP.em.write_map(dmap, file_out)

    def _is_membrane(self, z):
        if ((z-self.center) < self.thickness/2.0 and
                (z-self.center) >= -self.thickness/2.0):
            return 1
        else:
            return 0


class ResidueProteinProximityRestraint(IMP.pmi.restraints.RestraintBase):
    """Restrain residue/residues to bind to unknown location in a target"""

    def __init__(self, hier, selection, cutoff=6., sigma=3., xi=0.01,
                 resolution=1.0, weight=1.0, label=None):
        """
        Constructor
        @param hier        Hierarchy of the system
        @param section     Selection of residues and target;
                           syntax is (prot, r1, r2, target_prot) or
                           (prot1, r1, r2, target_prot, target_r1, target_r2)
        @param cutoff      Distance cutoff between selected segment and target
                           protein
        @param sigma       Distance variance between selected fragments
        @param xi          Slope of a distance-linear scoring function that
                           funnels the score when the particles are too
                           far away
        @param resolution  Resolution at which to apply restraint
        @param weight      Weight of the restraint
        @param label       Extra text to label the restraint so that it is
                           searchable in the output
        """
        self.hier = hier
        m = self.hier.get_model()

        super(ResidueProteinProximityRestraint, self).__init__(
            m, name="ResidueProteinProximityRestraint", label=label,
            weight=weight)

        self.cutoff = cutoff
        self.sigma = sigma
        self.xi = xi
        self.resolution = resolution

        # Check selection
        print('selection', selection, isinstance(selection, tuple))
        if (not isinstance(selection, tuple)
                and not isinstance(selection, list)):
            raise ValueError("Selection should be a tuple or list")
        if len(selection) < 4:
            raise ValueError(
                "Selection should be (prot, r1, r2, target_prot) or "
                "(prot1, r1, r2, target_prot, target_r1, target_r2)")

        # Selection
        self.prot1 = selection[0]
        self.r1 = int(selection[1])
        self.r2 = int(selection[2])

        self.prot2 = selection[3]
        if len(selection) == 6:
            self.tr1 = int(selection[4])
            self.tr2 = int(selection[5])

        if self.r1 == self.r2:
            sel_resi = IMP.atom.Selection(
                self.hier, molecule=self.prot1, residue_index=self.r1,
                resolution=self.resolution).get_selected_particles()
        else:
            sel_resi = IMP.atom.Selection(
                self.hier, molecule=self.prot1,
                residue_indexes=range(self.r1, self.r2+1, 1),
                resolution=self.resolution).get_selected_particles()

        if len(selection) == 4:
            sel_target = IMP.atom.Selection(
                self.hier, molecule=self.prot2,
                resolution=self.resolution).get_selected_particles()

        elif len(selection) == 6:
            sel_target = IMP.atom.Selection(
                self.hier, molecule=self.prot2,
                residue_indexes=range(self.tr1, self.tr2+1, 1),
                resolution=self.resolution).get_selected_particles()

        self.included_ps = sel_resi + sel_target

        # Setup restraint
        distance = 0.0
        slack = cutoff*2

        br = IMP.isd.ResidueProteinProximityRestraint(
            m, self.cutoff, self.sigma, self.xi, True,
            'ResidueProteinProximityRestraint')

        print('Selected fragment and target lengths:', len(sel_resi),
              len(sel_target))

        # Setup close pair container
        # Find close pair within included_resi and included_target
        lsa_target = IMP.container.ListSingletonContainer(m)
        lsa_target.add(IMP.get_indexes(sel_target))

        lsa_resi = IMP.container.ListSingletonContainer(m)
        lsa_resi.add(IMP.get_indexes(sel_resi))

        self.cpc = IMP.container.CloseBipartitePairContainer(lsa_resi,
                                                             lsa_target,
                                                             distance,
                                                             slack)

        br.add_pairs_container(self.cpc)

        br.add_contribution_particles(sel_resi, sel_target)

        # Compute interpolation paramaters
        yi = ((cutoff**2/(2*sigma**2)
               - math.log(1/math.sqrt(2*math.pi*sigma*sigma))+cutoff*xi/2.)
              / (cutoff/2.))
        interpolation_factor = -(cutoff/2.)*(xi-yi)
        max_p = (math.exp(-((distance+slack)**2)/(2*sigma**2))
                 / math.sqrt(2*math.pi*sigma*sigma))
        max_score = -math.log(max_p)

        # Add interpolation parameters
        br.set_yi(yi)
        br.set_interpolation_factor(interpolation_factor)
        br.set_max_score(max_score)

        self.rs.add_restraint(br)

        self.restraint_sets = [self.rs] + self.restraint_sets[1:]

    def get_container_pairs(self):
        """ Get particles in the close pair container """
        return self.cpc.get_indexes()

    def get_output(self):
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["ResidueProteinProximityRestraint_score_" + self.label] \
            = str(score)

        return output
