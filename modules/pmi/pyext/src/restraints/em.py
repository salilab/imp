"""@namespace IMP.pmi.restraints.em
Restraints for handling electron microscopy maps.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container
import IMP.isd
import IMP.pmi.tools
import IMP.isd.gmm_tools
import sys
from math import sqrt

class GaussianEMRestraint(object):
    """Fit Gaussian-decorated particles to an EM map
    (also represented with a set of Gaussians)
    \note This class wraps an isd::GaussianEMRestraint
    """
    def __init__(self, densities,
                 target_fn='',
                 target_ps=[],
                 cutoff_dist_model_model=0.0,
                 cutoff_dist_model_data=0.0,
                 target_mass_scale=1.0,
                 target_radii_scale=3.0,
                 model_radii_scale=1.0,
                 slope=0.0,
                 spherical_gaussians=False,
                 close_pair_container=None,
                 backbone_slope=False,
                 scale_target_to_mass=False,
                 weight=1.0,
                 target_is_rigid_body=False,
                 local=False,
                 representation=None,
                 emdb=None):
        """Constructor.
        @param densities The Gaussian-decorated particles to be restrained
        @param target_fn GMM file of the target density map
                         (alternatively, pass the ps)
        @param target_ps List of Gaussians of the target map
                         (alternatively, pass the filename)
        @param cutoff_dist_model_model Distance in model-model close
               pair container
        @param cutoff_dist_model_data  Distance in model-data close pair
               container. Usually can set to zero because we multiply the
               target radii
        @param target_mass_scale Scale up the target densities so that
               the mass is accurate.
               Needed if the GMM you generated was not already scaled.
               To make it the same as model mass, set scale_to_target_mass=True
        @param target_radii_scale Scale the target density radii -
               only used for the close pair container.
               If you keep this at 3.0 or so you don't have to use cutoff dist.
        @param model_radii_scale Scale the model density radii - only used
               for the close pair container
        @param slope Linear term added to help bring model into the density
        @param spherical_gaussians     Set to True for a speed bonus when
               the model densities are spheres. (This means you don't have
               to do a matrix multiplication if they rotate.)
        @param close_pair_container    Pass a close pair container for
               the model if you already have one (e.g. for an excluded
               volume restraint.) May give a speed bonus.
        @param backbone_slope Only apply slope to backbone particles -
               only matters for atomic
        @param scale_target_to_mass    Set True if you would need to scale
               target to EXACTLY the model mass
        @param weight                  The restraint weight
        @param target_is_rigid_body Set True if you want to put the target density particles
               into a rigid body that need to be sampled (e.g.,when you need to fit one density
               against another one). Default is False.
        @param local Only consider density particles that are within the
                specified model-density cutoff (experimental)
        @param emdb The EMDB identifier of the map, if known.
        """

        # some parameters
        self.label = "None"
        self.sigmaissampled = False
        self.sigmamaxtrans = 0.3
        self.sigmamin = 1.0
        self.sigmamax = 100.0
        self.sigmainit = 2.0
        self.label = "None"
        self.densities = densities
        self.em_root_hier = None

        # setup target GMM
        self.m = self.densities[0].get_model()
        if scale_target_to_mass:
            def hierarchy_mass(h):
                leaves = IMP.atom.get_leaves(h)
                return sum(IMP.atom.Mass(p).get_mass() for p in leaves)
            target_mass_scale = sum(hierarchy_mass(h) for h in densities)
        print('will scale target mass by', target_mass_scale)

        if target_fn != '':
            self.target_ps = []
            IMP.isd.gmm_tools.decorate_gmm_from_text(
                target_fn,
                self.target_ps,
                self.m,
                radius_scale=target_radii_scale,
                mass_scale=target_mass_scale)
        elif target_ps != []:
            self.target_ps = target_ps
        else:
            print('Gaussian EM restraint: must provide target density file or properly set up target densities')
            return

        if representation:
            for p in representation._protocol_output:
                p.add_em3d_restraint(self.target_ps, emdb)

        # setup model GMM
        self.model_ps = []
        for h in self.densities:
            self.model_ps += [ k.get_particle() for k in IMP.atom.get_leaves(h) ]
        if model_radii_scale != 1.0:
            for p in self.model_ps:
                rmax = sqrt(max(IMP.core.Gaussian(p).get_variances())) * \
                    model_radii_scale
                if not IMP.core.XYZR.get_is_setup(p):
                    IMP.core.XYZR.setup_particle(p, rmax)
                else:
                    IMP.core.XYZR(p).set_radius(rmax)
        #wrap target particles in rigid body if requested
        if target_is_rigid_body:
            #p = IMP.Particle(self.m)
            #self.rb=IMP.core.RigidBody.setup_particle(p,self.target_ps)
            self.rb=IMP.atom.create_rigid_body(self.target_ps)
        else:
            self.rb=None

        # sigma particle
        self.sigmaglobal = IMP.pmi.tools.SetupNuisance(self.m, self.sigmainit,
                                               self.sigmamin, self.sigmamax,
                                               self.sigmaissampled).get_particle()

        # create restraint
        print('target num particles', len(self.target_ps), \
            'total weight', sum([IMP.atom.Mass(p).get_mass()
                                for p in self.target_ps]))
        print('model num particles', len(self.model_ps), \
            'total weight', sum([IMP.atom.Mass(p).get_mass()
                                for p in self.model_ps]))

        update_model=not spherical_gaussians
        log_score=False
        self.gaussianEM_restraint = IMP.isd.GaussianEMRestraint(
            self.m,
            IMP.get_indexes(self.model_ps),
            IMP.get_indexes(self.target_ps),
            self.sigmaglobal.get_particle().get_index(),
            cutoff_dist_model_model,
            cutoff_dist_model_data,
            slope,
            update_model, backbone_slope, local)

        print('done EM setup')
        self.rs = IMP.RestraintSet(self.m, 'GaussianEMRestraint')
        self.rs.add_restraint(self.gaussianEM_restraint)
        self.set_weight(weight)

    def center_target_density_on_model(self):
        target_com = IMP.algebra.Vector3D(0, 0, 0)
        target_mass = 0.0
        for p in self.target_ps:
            mass = IMP.atom.Mass(p).get_mass()
            pos = IMP.core.XYZ(p).get_coordinates()
            target_com += pos * mass
            target_mass += mass
        target_com /= target_mass
        print('target com', target_com)
        model_com = IMP.algebra.Vector3D(0, 0, 0)
        model_mass = 0.0
        for p in self.model_ps:
            mass = IMP.atom.Mass(p).get_mass()
            pos = IMP.core.XYZ(p).get_coordinates()
            model_com += pos * mass
            model_mass += mass
        model_com /= model_mass
        print('model com', model_com)

        v = target_com - model_com
        print('translating with', -v)
        transformation = IMP.algebra.Transformation3D(IMP.algebra.Vector3D(-v))
        for p in self.target_ps:
            IMP.core.transform(IMP.core.RigidBody(p), transformation)
        # IMP.pmi.tools.translate_hierarchies(self.densities,v)

    def get_center_of_mass(self, target=True):
        '''Returns the geometric center of the GMM particles
        @param target = True - returns target map gmm COM
        @param target = False - returns model gmm COM'''
        com = IMP.algebra.Vector3D(0, 0, 0)
        total_mass = 0.0
        if target:
            ps = self.target_ps
        else:
            ps = self.model_ps
        for p in ps:
            mass = IMP.atom.Mass(p).get_mass()
            pos = IMP.core.XYZ(p).get_coordinates()
            com += pos * mass
            total_mass += mass
        com /= total_mass
        return com

    def center_target_density_on_origin(self):
        target_com = self.get_center_of_mass()
        print('target com', target_com)
        model_com = IMP.algebra.Vector3D(0, 0, 0)
        print('model com', model_com)
        v = target_com - model_com
        print('translating with', -v)
        transformation = IMP.algebra.Transformation3D(IMP.algebra.Vector3D(-v))
        for p in self.target_ps:
            IMP.core.transform(IMP.core.RigidBody(p), transformation)
        # IMP.pmi.tools.translate_hierarchies(self.densities,v)

    def center_model_on_target_density(self, input_object):
        if type(input_object) is IMP.pmi.representation.Representation:
            hier = input_object.prot
        elif type(input_object) is IMP.pmi.topology.State:
            hier = input_object.get_hierarchy()
        else:
            raise Exception("Input must be a Representation or topology.State object")
        target_com = self.get_center_of_mass()
        print('target com', target_com)
        model_com = self.get_center_of_mass(target=False)
        print('model com', model_com)
        v = target_com - model_com
        print('translating with', v)
        transformation = IMP.algebra.Transformation3D(IMP.algebra.Vector3D(v))

        rigid_bodies = set()
        XYZRs = set()

        for p in IMP.atom.get_leaves(hier):
            if IMP.core.RigidBodyMember.get_is_setup(p):
                rb = IMP.core.RigidBodyMember(p).get_rigid_body()
                rigid_bodies.add(rb)
            elif IMP.core.XYZR.get_is_setup(p):
                XYZRs.add(p)

        for rb in list(rigid_bodies):
            IMP.core.transform(rb, transformation)

        for p in list(XYZRs):
            IMP.core.transform(IMP.core.XYZ(p), transformation)


    def set_weight(self,weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_particles_to_sample(self):
        ps = {}
        if self.sigmaissampled:
            ps["Nuisances_GaussianEMRestraint_sigma_" +
                self.label] = ([self.sigmaglobal], self.sigmamaxtrans)
        if self.rb:
            ps["Rigid_Bodies_GaussianEMRestraint"] = (
                [self.rb],
                4,
                0.03)
        return ps

    def get_rigid_body(self):
        if self.rb is None:
            raise Exception("No rigid body created for GMM particles. Ensure target_is_rigid_body is set to True")
        return self.rb

    def get_density_as_hierarchy(self):
        if self.em_root_hier is None:
            self.em_root_hier = IMP.atom.Copy.setup_particle(IMP.Particle(self.m),0)
            self.em_root_hier.set_name("GaussianEMRestraint_density_"+self.label)
            for p in self.target_ps:
                self.em_root_hier.add_child(p)
        return self.em_root_hier

    def add_target_density_to_hierarchy(self,inp):
        ''' Can add a target GMM to a Hierarchy.
        For PMI2 a state object may also be passed'''
        if type(inp) is IMP.pmi.topology.State:
            inp.get_hierarchy().add_child(self.get_density_as_hierarchy())
        elif type(inp) is IMP.atom.Hierarchy:
            inp.add_child(self.get_density_as_hierarchy())
        else:
            raise Exception("Can only add a density to a PMI State object or IMP.atom.Hierarchy. You passed a", type(inp))

    def get_restraint_set(self):
        return self.rs

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["GaussianEMRestraint_" +
               self.label] = str(score)
        output["GaussianEMRestraint_sigma_" +
               self.label] = str(self.sigmaglobal.get_scale())
        return output

    def evaluate(self):
        return self.weight * self.rs.unprotected_evaluate(None)

    def write_target_gmm_to_mrc(self, fileout=None, voxel_size=5.0):
        '''Writes target GMM file to MRC'''
        if fileout is None:
            fileout="Gaussian_map_" + self.label + ".mrc"
        IMP.isd.gmm_tools.write_gmm_to_map(self.target_ps, fileout, voxel_size)
        return fileout


#-------------------------------------------

class CrossCorrelationRestraint(object):
    """Fit particles to an EM map. This creates a simulate density map and updates them every eval.
    \note Wraps an em::FitRestraint
    """
    def __init__(self,
                 ps,
                 dmap,
                 resolution,
                 origin=None,
                 voxel_size=None,
                 weight=1.0,
                 label=""):
        """Constructor
        @param ps The particles to restrain. Currently these must be atomic particles.
        @param map_fn The EM density map to fit to
        @param resolution Map resolution
        @param origin In case you need to tell IMP the correct origin
        @param voxel_size In case you need to tell IMP the angstroms per pixel
        @param weight The data weight
        @param label Extra PMI label
        """
        print('FitRestraint: setup')
        #print('\tmap_fn',map_fn)
        print('\tresolution',resolution)
        print('\tvoxel_size',voxel_size)
        print('\torigin',origin)
        print('\tweight',weight)

        # some parameters
        self.mdl = ps[0].get_model()
        self.label = label
        self.dmap = dmap #IMP.em.read_map(map_fn,IMP.em.MRCReaderWriter())
        #dh = self.dmap.get_header()
        #dh.set_resolution(resolution)
        if voxel_size:
            self.dmap.update_voxel_size(voxel_size)
        if origin is not None:
            if type(origin)==IMP.algebra.Vector3D:
                self.dmap.set_origin(origin)
            elif type(origin)==list:
                self.dmap.set_origin(*origin)
            else:
                print('FitRestraint did not recognize format of origin')
                exit()
        fr = IMP.em.FitRestraint(ps,self.dmap)
        self.rs = IMP.RestraintSet(self.mdl,weight,"FitRestraint")
        self.rs.add_restraint(fr)
        self.set_weight(weight)

    def set_weight(self,weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint_set(self):
        return self.rs

    def get_output(self):
        self.mdl.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["EMRestraint_" + self.label] = str(score)
        return output

    def evaluate(self):
        return self.weight * self.rs.unprotected_evaluate(None)


#-------------------------------------------

class ElectronMicroscopy2D(object):

    def __init__(
        self,
        representation,
        images,
        resolution=None):

        self.weight=1.0
        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'em2d')
        self.label = "None"

        # IMP.atom.get_by_type
        particles = IMP.pmi.tools.select(
            representation,
            resolution=resolution)

        em2d = None
        self.rs.add_restraint(em2d)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self,weight):
        self.weight=weight
        self.rs.set_weigth(self.weight)

    def get_output(self):
        self.m.update()
        output = {}
        score = self.weight*self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ElectronMicroscopy2D_" + self.label] = str(score)
        return output
