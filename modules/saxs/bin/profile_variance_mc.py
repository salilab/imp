#!/usr/bin/env python
import IMP
import IMP.atom
import IMP.core
import IMP.saxs
import os,sys
from numpy import *

tau = .1
niter = 1000

m = IMP.Model()

#! read PDB
mp= IMP.atom.read_pdb('6lyz.pdb', m,
                      IMP.atom.CAlphaPDBSelector())

class Variance():
    def __init__(self, tau, niter, ext=''):
        self.tau = tau
        self.niter = niter
        self.ext = ext
        #create folders
        if not os.path.isdir('data'):
            os.mkdir('data')
        #! read experimental profile and get bounds
        #exp_profile = IMP.saxs.Profile('lyzexp_med.dat')
        #self.exp_profile = exp_profile
        #qmin = exp_profile.get_min_q()
        #qmax = exp_profile.get_max_q()
        #dq = exp_profile.get_delta_q()
        qmin = 0.04
        qmax = 0.5
        dq = 0.02
        #! select particles from the model
        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.particles = particles
        #store reference coordinates and theoretical profile
        self.refpos = [ IMP.core.XYZ(p).get_coordinates() for p in particles ]
        self.model_profile = IMP.saxs.Profile(qmin,qmax,dq)

    def perturb_particles(self):
        for i,p in enumerate(self.particles):
            newpos = array(self.refpos[i])
            newpos += random.normal(0,self.tau,3)
            newpos = IMP.algebra.Vector3D(newpos)
            IMP.core.XYZ(p).set_coordinates(newpos)

    def get_profile(self):
        model_profile = self.model_profile
        p=model_profile.calculate_profile(self.particles, IMP.saxs.CA_ATOMS)
        return array( [ model_profile.get_intensity(i) for i in
                        xrange(model_profile.size()) ] )

    def init_variances(self):
        #create placeholders
        N = self.model_profile.size()
        a = self.profiles[0][:]
        self.m = matrix(a).T # Nx1
        self.V = self.m * self.m.T
        self.normm = linalg.norm(self.m)
        self.normV = linalg.norm(self.V)

    def update_variances(self):
        a = matrix(self.profiles[-1]) #1xN
        n = float(len(self.profiles))
        self.m = a.T/n + (n-1)/n * self.m
        self.V = a.T*a + self.V
        self.oldnormm = self.normm
        self.oldnormV = self.normV
        self.normm = linalg.norm(self.m)
        self.normV = linalg.norm(self.V)
        self.diffm = (self.oldnormm-self.normm)/self.oldnormm
        self.diffV = (self.oldnormV-self.normV)/self.oldnormV

    def get_direct_stats(self, a):
        nq = len(a[0])
        nprof = len(a)
        m = [0]*nq
        for prof in a:
            for q,I in enumerate(prof):
                m[q] += I
        m = array(m)/nprof
        V = matrix(a)
        V = V.T*V
        Sigma = (matrix(a-m))
        Sigma = Sigma.T*Sigma/(nprof-1)
        return m,V,Sigma

    def store_data(self):
        profiles = matrix(self.profiles)
        self.directm, self.directV, Sigma = \
                self.get_direct_stats(array(profiles))
        directV = self.directV
        print "V comparison",(linalg.norm(directV-self.V)/self.normV)
        save('data/profiles', profiles)
        #profiles
        fl=open('data/profiles.dat','w')
        for i,l in enumerate(array(profiles).T):
            self.model_profile.get_q(i)
            fl.write('%s ' % i)
            for k in l:
                fl.write('%s ' % (k-self.directm[i]))
            fl.write('\n')
        save('data/m', self.m)
        save('data/V', self.V)
        self.Sigma = Sigma
        save('data/Sigma', Sigma)
        #Sigma matrix
        fl=open('data/Sigma.dat', 'w')
        model_profile = self.model_profile
        for i in xrange(model_profile.size()):
            qi = model_profile.get_q(i)
            for j in xrange(model_profile.size()):
                qj = model_profile.get_q(j)
                vij = self.Sigma[i,j]
                fl.write('%s %s %s\n' % (qi,qj,vij))
            fl.write('\n')
        #Sigma eigenvalues
        fl=open('data/eigenvals','w')
        #for i in linalg.eigvalsh(Sigma+matrix(eye(len(Sigma)))*0.1):
        for i in linalg.eigvalsh(Sigma):
            fl.write('%s\n' % i)
        #mean profile
        fl=open('data/mean.dat','w')
        for i in xrange(len(self.m)):
            qi = self.model_profile.get_q(i)
            fl.write('%s ' % qi)
            fl.write('%s ' % self.m[i,0])
            fl.write('%s ' % sqrt(self.Sigma[i,i]))
            fl.write('\n')

    def try_chol(self, jitter):
        Sigma=self.Sigma
        try:
            linalg.cholesky(Sigma+matrix(eye(len(Sigma)))*jitter)
        except linalg.LinAlgError:
            print "Decomposition failed with jitter =",jitter
            return
        print "Successful decomposition with jitter =",jitter

    def run(self):
        self.profiles = [self.get_profile()]
        self.init_variances()
        for n in xrange(self.niter):
            self.perturb_particles()
            self.profiles.append(self.get_profile())
            self.update_variances()
            profiles = matrix(self.profiles)
            print n,self.diffm,self.diffV
        print
        #
        self.store_data()
        self.try_chol(0.)
        for i in logspace(-7,0,num=8):
            self.try_chol(i)


if __name__ == '__main__':
    v= Variance(tau,niter)
    v.run()
