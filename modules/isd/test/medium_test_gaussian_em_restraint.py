#!/usr/bin/env python

from __future__ import print_function, division
import IMP
import IMP.core
import IMP.isd
import IMP.isd.gmm_tools
import IMP.algebra
import IMP.test
import numpy as np
from math import cos, sin, pi, sqrt, exp, log
from copy import deepcopy
import itertools


def create_test_points(mu, radii):
    testers = [[mu[0], mu[1], mu[2]]]
    for i in range(3):
        t = mu[:]
        t[i] += radii[i] + 1  # kluge to ensure good order at the end
        testers.append(t)
        t = mu[:]
        t[i] -= radii[i]
        testers.append(t)
    return testers

def score_gaussian_overlap(p0, p1):
    g0 = IMP.core.Gaussian(p0).get_gaussian()
    g1 = IMP.core.Gaussian(p1).get_gaussian()
    mass0 = IMP.atom.Mass(p0).get_mass()
    mass1 = IMP.atom.Mass(p1).get_mass()
    c0 = np.reshape(np.array(IMP.algebra.get_covariance(g0)), (3, 3))
    c1 = np.reshape(np.array(IMP.algebra.get_covariance(g1)), (3, 3))
    u0 = np.array(list(g0.get_center()))
    u1 = np.array(list(g1.get_center()))

    c = c0 + c1
    u = u1 - u0
    det = np.linalg.det(c)
    inv = np.linalg.inv(c)
    score = mass0 * mass1 * 1 / \
        sqrt((2.0 * pi) ** 3 * det) * \
        exp(-0.5 * np.dot(u.transpose(), np.dot(inv, u)))
    return score


def gem_score(model_ps, density_ps,slope=0.0):
    mm_score = 0.0
    md_score = 0.0
    dd_score = 0.0

    nm = len(model_ps)
    nd = len(density_ps)
    slope_score=0.0
    for nd1 in range(len(density_ps)):
        for nd2 in range(len(density_ps)):
            dd_score += score_gaussian_overlap(density_ps[nd1], density_ps[nd2])

    for nm1 in range(len(model_ps)):
        for nm2 in range(len(model_ps)):
            mm_score += score_gaussian_overlap(model_ps[nm1], model_ps[nm2])

        for nd in range(len(density_ps)):
            md_score += score_gaussian_overlap(model_ps[nm1], density_ps[nd])
            dist = IMP.algebra.get_distance(IMP.core.XYZ(model_ps[nm1]).get_coordinates(),
                                            IMP.core.XYZ(density_ps[nd]).get_coordinates())
            slope_score+=dist*slope
    cc = 2*md_score/(mm_score+dd_score)
    dist = -log(cc) + slope_score
    return cc, dist

def create_random_gaussians(m,randstate,num,spherical,rad_scale=1.0):
    ret=[]
    for n in range(num):
        if spherical:
            std=[1,1,1]
        else:
            std=randstate.random_sample(3,) * 5
        center=randstate.random_sample(3,) * 5 - [2.5,2.5,2.5]
        var=[s**2 for s in std]
        rot=IMP.algebra.get_random_rotation_3d()
        trans=IMP.algebra.Transformation3D(rot,center)
        shape=IMP.algebra.Gaussian3D(IMP.algebra.ReferenceFrame3D(trans),var)
        p=IMP.Particle(m)
        IMP.core.Gaussian.setup_particle(p,shape)
        IMP.atom.Mass.setup_particle(p,1.0/num)
        IMP.core.XYZR.setup_particle(p)
        IMP.core.XYZR(p).set_radius(max(std)*rad_scale)
        ret.append(p)
    return ret

def shuffle_particles(ps,t=2.0,r=0.01):
    for np,p in enumerate(ps):
        trans=IMP.algebra.get_random_local_transformation(IMP.algebra.Vector3D(0,0,0),
                                                          t,r)
        d=IMP.core.RigidBody(p)
        IMP.core.transform(d,trans)

def reset_coords(ps,orig_coords):
    for p,c in zip(ps,orig_coords):
        IMP.core.XYZ(p).set_coordinates(c)

class Tests(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)

        # setup problem
        ndensity=4
        nmodel=10
        rs=np.random.RandomState()

        self.m = IMP.Model()
        itrans = IMP.algebra.get_identity_transformation_3d()
        self.density_ps=create_random_gaussians(self.m,rs,ndensity,spherical=False)
        self.model_ps=create_random_gaussians(self.m,rs,nmodel,spherical=False)

        psigma=IMP.Particle(self.m)
        si = IMP.isd.Scale.setup_particle(psigma,1.0)
        slope=0.0
        model_cutoff_dist=1e8
        density_cutoff_dist=1e8
        update_model=True
        self.gem=IMP.isd.GaussianEMRestraint(self.m,self.model_ps,
                                             self.density_ps,psigma,
                                             model_cutoff_dist,density_cutoff_dist,
                                             slope,
                                             update_model,False)
        self.sf = IMP.core.RestraintsScoringFunction([self.gem])
        self.orig_coords=[IMP.core.XYZ(p).get_coordinates() for p in self.model_ps]

    def test_gem_score(self):
        """test accuracy of GMM score"""
        for nt in range(10):
            shuffle_particles(self.model_ps)
            score = self.sf.evaluate(False)
            cc = self.gem.get_cross_correlation_coefficient()
            pycc, pyscore = gem_score(self.model_ps, self.density_ps)
            self.assertAlmostEqual(score, pyscore, delta=0.02)
            self.assertAlmostEqual(cc, pycc, delta=0.02)

    def test_gem_score_with_slope(self):
        """test accuracy of GMM score using slope"""
        reset_coords(self.model_ps,self.orig_coords)
        slope=0.1
        self.gem.set_slope(slope)
        for nt in range(10):
            shuffle_particles(self.model_ps)
            score = self.sf.evaluate(False)
            cc = self.gem.get_cross_correlation_coefficient()
            pycc, pyscore = gem_score(self.model_ps, self.density_ps,
                                      slope=slope)
            self.assertAlmostEqual(score, pyscore, delta=0.02)
            self.assertAlmostEqual(cc, pycc, delta=0.02)
        self.gem.set_slope(0.0)


    def test_gem_derivatives(self):
        """test accuracy of GMM derivatives"""
        reset_coords(self.model_ps,self.orig_coords)
        for i in range(10):
            shuffle_particles(self.model_ps)
            self.gem.evaluate(True)
            for np, p in enumerate(self.model_ps):
                d = IMP.core.XYZ(p)
                #print 'n', IMP.test.xyz_numerical_derivatives(self.m, d, 0.01), 'a', d.get_derivatives()
                self.assertXYZDerivativesInTolerance(self.sf, d, tolerance = 1e-2,percentage=10.0)
    def test_gem_derivatives_with_slope(self):
        """test accuracy of GMM derivatives"""
        self.gem.set_slope(0.1)
        reset_coords(self.model_ps,self.orig_coords)
        for i in range(10):
            shuffle_particles(self.model_ps)
            self.gem.evaluate(True)
            for np, p in enumerate(self.model_ps):
                d = IMP.core.XYZ(p)
                #print 'n', IMP.test.xyz_numerical_derivatives(self.m, d, 0.01), 'a', d.get_derivatives()
                self.assertXYZDerivativesInTolerance(self.sf, d, tolerance = 1e-2,percentage=10.0)
        self.gem.set_slope(0.0)

    def test_rasterize(self):
        """Test making a map from a GMM"""
        dmap = IMP.isd.gmm_tools.gmm2map(self.model_ps,1.0,fast=False)

class LocalTests(IMP.test.TestCase):
    def test_local_score(self):
        ndensity=10
        nmodel=10
        rs=np.random.RandomState()

        self.m = IMP.Model()
        itrans = IMP.algebra.get_identity_transformation_3d()
        self.density_ps=create_random_gaussians(self.m,rs,ndensity,spherical=False)
        self.model_ps=create_random_gaussians(self.m,rs,nmodel,spherical=False)

        psigma=IMP.Particle(self.m)
        si = IMP.isd.Scale.setup_particle(psigma,1.0)
        slope=0.0
        model_cutoff_dist = 1e8
        density_cutoff_dist = 0.0
        update_model=True
        backbone_slope=False
        local=True
        self.gem=IMP.isd.GaussianEMRestraint(self.m,self.model_ps,
                                             self.density_ps,psigma,
                                             model_cutoff_dist,density_cutoff_dist,
                                             slope,
                                             update_model,backbone_slope,local)
        self.sf = IMP.core.RestraintsScoringFunction([self.gem])
        self.orig_coords=[IMP.core.XYZ(p).get_coordinates() for p in self.model_ps]

        for nt in range(10):
            shuffle_particles(self.model_ps,5.0,1.5)
            score = self.sf.evaluate(False)
            pycc, pyscore = gem_score(self.model_ps, self.density_ps)
            print(score,pycc,pyscore)

if __name__ == '__main__':
    IMP.test.main()
