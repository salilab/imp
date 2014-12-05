"""@namespace IMP.isd.gmm_tools
   Tools for handling Gaussian Mixture Models.
"""

import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.em
import numpy as np
import numpy.linalg
import sys,os

try:
    import sklearn.mixture
    nosklearn=False
except:
    nosklearn=True
from math import exp,sqrt,copysign

def decorate_gmm_from_text(in_fn,ps,mdl,transform=None,radius_scale=1.0,mass_scale=1.0):
    ''' read the output from write_gmm_to_text, decorate as Gaussian and Mass'''
    inf=open(in_fn,'r')
    ncomp=0
    for l in inf:
        if l[0]!='#':
            fields=l.split('|')
            weight=float(fields[2])
            center=map(float,fields[3].split())
            covar=np.array(map(float,fields[4].split())).reshape((3,3))
            if ncomp>=len(ps):
                ps.append(IMP.Particle(mdl))
            shape=IMP.algebra.get_gaussian_from_covariance(covar.tolist(),
                                                           IMP.algebra.Vector3D(center))
            g=IMP.core.Gaussian.setup_particle(ps[ncomp],shape)
            m=IMP.atom.Mass.setup_particle(ps[ncomp],weight*mass_scale)
            rmax=sqrt(max(g.get_variances()))*radius_scale
            IMP.core.XYZR.setup_particle(ps[ncomp],rmax)
            if not transform is None:
                IMP.core.transform(IMP.core.RigidBody(ps[ncomp]),transform)
            ncomp+=1
def write_gmm_to_text(ps,out_fn):
    '''write a list of gaussians to text. must be decorated as Gaussian and Mass'''
    print 'will write GMM text to',out_fn
    outf=open(out_fn,'w')
    outf.write('#|num|weight|mean|covariance matrix|\n')
    for ng,g in enumerate(ps):
        shape=IMP.core.Gaussian(g).get_gaussian()
        weight=IMP.atom.Mass(g).get_mass()
        covar=[c for row in IMP.algebra.get_covariance(shape) for c in row]
        mean=list(shape.get_center())
        fm=[ng,weight]+mean+covar
        try:
            #python 2.7 format
            outf.write('|{}|{}|{} {} {}|{} {} {} {} {} {} {} {} {}|\n'.format(*fm))
        except ValueError:
            #python 2.6 and below
            outf.write('|{0}|{1}|{2} {3} {4}|{5} {6} {7} {8} {9} {10} {11} {12} {13}|\n'.format(*fm))
        outf.close()

def write_gmm_to_map(to_draw,out_fn,voxel_size,bounding_box=None):
    '''write density map from GMM. input can be either particles or gaussians'''
    if type(to_draw[0]) in (IMP.Particle,IMP.atom.Hierarchy,IMP.core.Hierarchy):
        ps=to_draw
    elif type(to_draw[0])==IMP.core.Gaussian:
        ps=[g.get_particle() for g in to_draw]
    else:
        print 'ps must be Particles or Gaussians'
        return
    print 'will write GMM map to',out_fn
    if bounding_box is None:
        if len(ps)>1:
            s=IMP.algebra.get_enclosing_sphere([IMP.core.XYZ(p).get_coordinates() for p in ps])
            s2=IMP.algebra.Sphere3D(s.get_center(),s.get_radius()*3)
        else:
            g=IMP.core.Gaussian(ps[0]).get_gaussian()
            s2=IMP.algebra.Sphere3D(g.get_center(),max(g.get_variances())*3)
        bounding_box=IMP.algebra.get_bounding_box(s2)
    shapes=[]
    weights=[]
    for p in ps:
        shapes.append(IMP.core.Gaussian(p).get_gaussian())
        weights.append(IMP.atom.Mass(p).get_mass())
    print 'rasterizing'
    grid=IMP.algebra.get_rasterized(shapes,weights,voxel_size,bounding_box)
    print 'creating map'
    d1=IMP.em.create_density_map(grid)
    print 'writing'
    IMP.em.write_map(d1,out_fn,IMP.em.MRCReaderWriter())
    del d1

def write_sklearn_gmm_to_map(gmm,out_fn,apix=0,bbox=None,dmap_model=None):
    '''write density map directly from sklearn GMM (kinda slow) '''
    ### create density
    if not dmap_model is None:
        d1=IMP.em.create_density_map(dmap_model)
    else:
        d1=IMP.em.create_density_map(bbox,apix)

    ### fill it with values from the GMM
    print 'getting coords'
    nvox=d1.get_number_of_voxels()
    apos=[list(d1.get_location_by_voxel(nv)) for nv in xrange(nvox)]

    print 'scoring'
    scores=gmm.score(apos)

    print 'assigning'
    map(lambda nv: d1.set_value(nv,exp(scores[nv])),xrange(nvox))
    print 'will write GMM map to',out_fn
    IMP.em.write_map(d1,out_fn,IMP.em.MRCReaderWriter())

def draw_points(pts,out_fn,trans=IMP.algebra.get_identity_transformation_3d(),
                                use_colors=False):
    ''' given some points (and optional transform), write them to chimera 'bild' format
    colors flag only applies to ellipses, otherwise it'll be weird'''
    outf=open(out_fn,'w')
    #print 'will draw',len(pts),'points'
    # write first point in red
    pt=trans.get_transformed(IMP.algebra.Vector3D(pts[0]))
    start=0
    if use_colors:
        outf.write('.color 1 0 0\n.dotat %.2f %.2f %.2f\n' %(pt[0],pt[1],pt[2]))
        start=1

    # write remaining points in green
    if use_colors:
        outf.write('.color 0 1 0\n')
        colors=['0 1 0','0 0 1','0 1 1']
    for nt,t in enumerate(pts[start:]):
        if use_colors and nt%2==0:
            outf.write('.color %s\n' % colors[nt/2])
        pt=trans.get_transformed(IMP.algebra.Vector3D(t))
        outf.write('.dotat %.2f %.2f %.2f\n' %(pt[0],pt[1],pt[2]))
    outf.close()



def create_gmm_for_bead(mdl,
                        particle,
                        n_components,
                        sampled_points=100000,
                        num_iter=100):
    print 'fitting bead with',n_components,'gaussians'
    dmap=IMP.em.SampledDensityMap([particle],1.0,1.0,
                                  IMP.atom.Mass.get_mass_key(),3,IMP.em.SPHERE)
    IMP.em.write_map(dmap,'test_intermed.mrc')
    pts=IMP.isd.sample_points_from_density(dmap,sampled_points)
    draw_points(pts,'pts.bild')
    density_particles=[]
    fit_gmm_to_points(pts,n_components,mdl,
                      density_particles,
                      num_iter,'full',
                      mass_multiplier=IMP.atom.Mass(particle).get_mass())
    return density_particles,dmap

def sample_and_fit_to_particles(model,
                                fragment_particles,
                                num_components,
                                sampled_points=1000000,
                                simulation_res=0.5,
                                voxel_size=1.0,
                                num_iter=100,
                                covariance_type='full',
                                multiply_by_total_mass=True,
                                output_map=None,
                                output_txt=None):
    density_particles=[]
    if multiply_by_total_mass:
        mass_multiplier=sum((IMP.atom.Mass(p).get_mass() for p in set(fragment_particles)))
        print 'add_component_density: will multiply by mass',mass_multiplier

    # simulate density from ps, then calculate points to fit
    print 'add_component_density: sampling points'
    dmap=IMP.em.SampledDensityMap(fragment_particles,simulation_res,voxel_size,
                                 IMP.atom.Mass.get_mass_key(),3)
    dmap.calcRMS()
    #if not intermediate_map_fn is None:
    #   IMP.em.write_map(dmap,intermediate_map_fn)
    pts=IMP.isd.sample_points_from_density(dmap,sampled_points)

    # fit GMM
    print 'add_component_density: fitting GMM to',len(pts),'points'
    fit_gmm_to_points(points=pts,
                      n_components=num_components,
                      mdl=model,
                      ps=density_particles,
                      num_iter=num_iter,
                      covariance_type=covariance_type,
                      mass_multiplier=mass_multiplier)

    if not output_txt is None:
        write_gmm_to_text(density_particles,output_txt)
    if not output_map is None:
        write_gmm_to_map(to_draw=density_particles,
                         out_fn=output_map,
                         voxel_size=voxel_size,
                         bounding_box=IMP.em.get_bounding_box(dmap))

    return density_particles

def fit_gmm_to_points(points,
                      n_components,
                      mdl,
                      ps=[],
                      num_iter=100,
                      covariance_type='full',
                      init_centers=[],
                      force_radii=-1.0,
                      force_weight=-1.0,
                      mass_multiplier=1.0):
    '''fit a GMM to some points. Will return core::Gaussians.
    if no particles are provided, they will be created

    points:            list of coordinates (python)
    n_components:      number of gaussians to create
    mdl:               IMP Model
    ps:                list of particles to be decorated. if empty, will add
    num_iter:          number of EM iterations
    covariance_type:   covar type for the gaussians. options: 'full', 'diagonal', 'sphereical'
    init_centers:      initial coordinates of the GMM
    force_radii:       fix the radii (spheres only)
    force_weight:      fix the weights
    mass_multiplier:   multiply the weights of all the gaussians by this value
    dirichlet:         use the DGMM fitting (can reduce number of components, takes longer)
    '''


    import sklearn.mixture

    params='m'
    init_params='m'
    if force_radii==-1.0:
        params+='c'
        init_params+='c'
    else:
        covariance_type='spherical'
        print 'forcing spherical with radii',force_radii

    if force_weight==-1.0:
        params+='w'
        init_params+='w'
    else:
        print 'forcing weights to be',force_weight

    print 'creating GMM with params',params,'init params',init_params,'n_components',n_components,'n_iter',num_iter,'covar type',covariance_type
    gmm=sklearn.mixture.GMM(n_components=n_components,
                          n_iter=num_iter,
                          covariance_type=covariance_type,
                          params=params,
                          init_params=init_params)

    if force_weight!=-1.0:
        gmm.weights_=np.array([force_weight]*n_components)
    if force_radii!=-1.0:
        gmm.covars_=np.array([[force_radii]*3 for i in xrange(n_components)])
    if init_centers!=[]:
        gmm.means_=init_centers
    print 'fitting'
    gmm.fit(points)

    print '>>> GMM score',gmm.score(points)

    ### convert format to core::Gaussian
    for ng in xrange(n_components):
        covar=gmm.covars_[ng]
        if covar.size==3:
            covar=np.diag(covar).tolist()
        else:
            covar=covar.tolist()
        center=list(gmm.means_[ng])
        weight=mass_multiplier*gmm.weights_[ng]
        if ng>=len(ps):
            ps.append(IMP.Particle(mdl))
        shape=IMP.algebra.get_gaussian_from_covariance(covar,IMP.algebra.Vector3D(center))
        g=IMP.core.Gaussian.setup_particle(ps[ng],shape)
        IMP.atom.Mass.setup_particle(ps[ng],weight)
        IMP.core.XYZR.setup_particle(ps[ng],sqrt(max(g.get_variances())))

def fit_dirichlet_gmm_to_points(points,
                      n_components,
                      mdl,
                      ps=[],
                      num_iter=100,
                      covariance_type='full',
                      mass_multiplier=1.0):
    '''fit a GMM to some points. Will return core::Gaussians.
    if no particles are provided, they will be created

    points:            list of coordinates (python)
    n_components:      number of gaussians to create
    mdl:               IMP Model
    ps:                list of particles to be decorated. if empty, will add
    num_iter:          number of EM iterations
    covariance_type:   covar type for the gaussians. options: 'full', 'diagonal', 'sphereical'
    init_centers:      initial coordinates of the GMM
    force_radii:       fix the radii (spheres only)
    force_weight:      fix the weights
    mass_multiplier:   multiply the weights of all the gaussians by this value
'''


    import sklearn.mixture

    ### create and fit GMM
    print 'using dirichlet prior'
    gmm=sklearn.mixture.DPGMM(n_components=n_components,
                              n_iter=num_iter,
                              covariance_type=covariance_type)

    gmm.fit(points)

    print '>>> GMM score',gmm.score(points)

    #print gmm.covars_
    #print gmm.weights_
    #print gmm.means_
    ### convert format to core::Gaussian
    for ng in xrange(n_components):
        invcovar=gmm.precs_[ng]
        covar=np.linalg.inv(invcovar)
        if covar.size==3:
            covar=np.diag(covar).tolist()
        else:
            covar=covar.tolist()
        center=list(gmm.means_[ng])
        weight=mass_multiplier*gmm.weights_[ng]
        if ng>=len(ps):
            ps.append(IMP.Particle(mdl))
        shape=IMP.algebra.get_gaussian_from_covariance(covar,IMP.algebra.Vector3D(center))
        g=IMP.core.Gaussian.setup_particle(ps[ng],shape)
        IMP.atom.Mass.setup_particle(ps[ng],weight)
        IMP.core.XYZR.setup_particle(ps[ng],sqrt(max(g.get_variances())))
