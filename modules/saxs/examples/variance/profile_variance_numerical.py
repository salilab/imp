# \example saxs/profile.py
## In this example, we read a protein from a PDB file and experimental profile file. Next we compute the theoretical profile from the PDB file and fit it to the experimental one.
##
## This application is available as a web service at salilab.org/foxs. It is also available as C++ code in IMP/applications.
##
## The experimental data for lysozyme is taken from crysol program (www.embl-hamburg.de/ExternalInfo/Research/Sax/crysol.html)
##

import IMP
import IMP.atom
import IMP.core
import IMP.saxs
import os
import sys
import numpy
import copy

class Variance():
    def __init__(self, model, tau, niter, prot, th_profile):
        self.model = model
        self.tau = tau
        self.niter = niter
        #! select particles from the model
        particles=IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)
        self.particles = particles
        #store reference coordinates and theoretical profile
        self.refpos = array(
                [ IMP.core.XYZ(p).get_coordinates() for p in particles ])
        self.model_profile = th_profile

    def perturb_particles(self, perturb=True):
        for i,p in enumerate(self.particles):
            newpos = copy.deepcopy(self.refpos[i])
            if perturb:
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
        mi = matrix(diag(1./m))
        Sigmarel = mi.T*Sigma*mi
        return m,V,Sigma,Sigmarel

    def store_data(self):
        if not os.path.isdir('data'):
            os.mkdir('data')
        profiles = matrix(self.profiles)
        self.directm, self.directV, self.Sigma, self.Sigmarel = \
                self.get_direct_stats(array(profiles))
        directV = self.directV
        #print "V comparison",(linalg.norm(directV-self.V)/self.normV)
        save('data/profiles', profiles)
        #absolute profile differences
        fl=open('data/profiles.dat','w')
        for i,l in enumerate(array(profiles).T):
            self.model_profile.get_q(i)
            fl.write('%s ' % i)
            for k in l:
                fl.write('%s ' % (k-self.directm[i]))
            fl.write('\n')
        #relative profile differences
        fl=open('data/profiles_rel.dat','w')
        for i,l in enumerate(array(profiles).T):
            self.model_profile.get_q(i)
            fl.write('%s ' % i)
            for k in l:
                fl.write('%s ' % ((k-self.directm[i])/self.directm[i]))
            fl.write('\n')
        #absolute profile differences (1st 20 profiles)
        fl=open('data/profiles_20.dat','w')
        for i,l in enumerate((array(profiles).T)[:,:20]):
            self.model_profile.get_q(i)
            fl.write('%s ' % i)
            for k in l:
                fl.write('%s ' % (k-self.directm[i]))
            fl.write('\n')
        #relative profile differences (1st 20 profiles)
        fl=open('data/profiles_rel_20.dat','w')
        for i,l in enumerate((array(profiles).T)[:,:20]):
            self.model_profile.get_q(i)
            fl.write('%s ' % i)
            for k in l:
                fl.write('%s ' % ((k-self.directm[i])/self.directm[i]))
            fl.write('\n')
        save('data/m', self.directm)
        save('data/V', self.directV)
        Sigma = self.Sigma
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
        for i in linalg.eigvalsh(Sigma):
            fl.write('%s\n' % i)
        Sigmarel = self.Sigmarel
        save('data/Sigmarel', Sigmarel)
        #Sigmarel matrix
        fl=open('data/Sigmarel.dat', 'w')
        model_profile = self.model_profile
        for i in xrange(model_profile.size()):
            qi = model_profile.get_q(i)
            for j in xrange(model_profile.size()):
                qj = model_profile.get_q(j)
                vij = self.Sigmarel[i,j]
                fl.write('%s %s %s\n' % (qi,qj,vij))
            fl.write('\n')
        #Sigma eigenvalues
        fl=open('data/eigenvals_rel','w')
        for i in linalg.eigvalsh(Sigmarel):
            fl.write('%s\n' % i)
        #mean profile
        fl=open('data/mean.dat','w')
        for i in xrange(len(self.directm)):
            qi = self.model_profile.get_q(i)
            fl.write('%s ' % qi)
            fl.write('%s ' % self.directm[i])
            fl.write('%s ' % sqrt(self.Sigma[i,i]))
            fl.write('\n')
        #Covariance matrix
        fl=open('data/Corr.dat', 'w')
        model_profile = self.model_profile
        for i in xrange(model_profile.size()):
            qi = model_profile.get_q(i)
            if qi == 0:
                continue
            for j in xrange(model_profile.size()):
                qj = model_profile.get_q(j)
                if qj == 0:
                    continue
                cij = self.Sigma[i,j]/sqrt(self.Sigma[i,i]*self.Sigma[j,j])
                fl.write('%s %s %s\n' % (qi,qj,cij))
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
        #self.init_variances()
        for n in xrange(self.niter):
            self.perturb_particles()
            self.profiles.append(self.get_profile())
            #self.update_variances()
            #profiles = matrix(self.profiles)
            #print n,self.diffm,self.diffV
            if n % 10 == 0:
                print n
        #print
        #
        self.store_data()
        #self.try_chol(0.)
        #for i in logspace(-7,0,num=8):
        #    self.try_chol(i)

    def get_cov(self, relative=True):
        if not relative:
            return self.Sigma
        else:
            return self.Sigmarel


m = IMP.Model()

#! read PDB
mp= IMP.atom.read_pdb(IMP.saxs.get_example_path('6lyz.pdb'), m,
                      IMP.atom.NonWaterNonHydrogenPDBSelector())

#! read experimental profile
exp_profile = IMP.saxs.Profile(IMP.saxs.get_example_path('lyzexp.dat'))

print 'min_q = ' + str(exp_profile.get_min_q())
print 'max_q = ' + str(exp_profile.get_max_q())
print 'delta_q = ' + str(exp_profile.get_delta_q())

#! select particles from the model
particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

#! calculate SAXS profile
model_profile = IMP.saxs.Profile()
model_profile.calculate_profile(particles,IMP.saxs.HEAVY_ATOMS,False,False)
model_profile.write_SAXS_file('6lyz.dat')
"""
fl=open('variance_cross.dat','w')
for i in xrange(model_profile.size()):
    qi = model_profile.get_q(i)
    Ii = model_profile.get_intensity(i)
    for j in xrange(model_profile.size()):
        qj = model_profile.get_q(j)
        Ij = model_profile.get_intensity(j)
        vij = model_profile.get_variance(i,j)
        fl.write('%s %s %s\n' % (qi,qj,vij))
    fl.write('\n')
fl.close()
"""
v=Variance(m,0.1,1000,mp, model_profile)
v.run()

sys.exit()

#! calculate chi score (should be ~0.5 for this example)
saxs_score = IMP.saxs.ProfileFitterChi(exp_profile)
chi = saxs_score.compute_score(model_profile)
print 'Chi = ' + str(chi)


#! convert to p(r)
pr = IMP.saxs.RadialDistributionFunction(0.5)
model_profile.profile_2_distribution(pr, 48.0)
pr.normalize()
pr.show()
