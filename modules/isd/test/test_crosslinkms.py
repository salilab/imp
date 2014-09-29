import IMP
import IMP.core
import IMP.isd
import IMP.test
from random import sample
from math import pi, log, exp


def setupnuisance(m, initialvalue, minvalue, maxvalue, isoptimized=True):

        nuisance = IMP.isd.Scale.setup_particle(IMP.Particle(m), initialvalue)
        if minvalue:
            nuisance.set_lower(minvalue)
        if maxvalue:
            nuisance.set_upper(maxvalue)

        nuisance.set_is_optimized(nuisance.get_nuisance_key(), isoptimized)

        return nuisance


class CrossLinkMS(object):
    def __init__(self,length,slope):
        self.pairs=[]
        self.length=length
        self.slope=slope
    
    def add_contribution(self,xyz1,xyz2,sigma1,sigma2,psi):
        self.pairs.append((xyz1,xyz2,sigma1,sigma2,psi))
    
    def get_probability(self):
    
        onemprob = 1.0
    
        for (xyz1,xyz2,sigma1,sigma2,psi) in self.pairs:
            dist = IMP.core.get_distance(xyz1, xyz2)
            psi = psi.get_scale()
            sigmai = sigma1.get_scale()
            sigmaj = sigma2.get_scale()
    
            voli = 4.0 / 3.0 * pi * sigmai * sigmai * sigmai
            volj = 4.0 / 3.0 * pi * sigmaj * sigmaj * sigmaj
    
            fi = 0
            fj = 0
    
            if dist < sigmai + sigmaj:
                xlvol = 4.0 / 3.0 * pi * (self.length / 2) * (self.length / 2) * \
                    (self.length / 2)
                fi = min(voli, xlvol)
                fj = min(volj, xlvol)
            else:
                di = dist - sigmaj - self.length / 2
                dj = dist - sigmai - self.length / 2
                fi = self.sphere_cap(sigmai, self.length / 2, abs(di))
                fj = self.sphere_cap(sigmaj, self.length / 2, abs(dj))
    
            pofr = fi * fj / voli / volj
            onemprob = onemprob * \
                (1.0 - (psi * (1.0 - pofr) + pofr * (1 - psi))
                 * exp(-self.slope * dist))
    
        prob = 1.0 - onemprob
    
        return prob


    def sphere_cap(self,r1, r2, d):
        sc = 0.0
        if d <= max(r1, r2) - min(r1, r2):
            sc = min(4.0 / 3 * pi * r1 * r1 * r1,
                     4.0 / 3 * pi * r2 * r2 * r2)
        elif d >= r1 + r2:
            sc = 0
        else:
            sc = (pi / 12 / d * (r1 + r2 - d) * (r1 + r2 - d)) * \
                 (d * d + 2 * d * r1 - 3 * r1 * r1 + 2 * d * r2 + 6 * r1 * r2 -
                  3 * r2 * r2)
        return sc
    

class TestXLRestraintSimple(IMP.test.TestCase):
    def test_score_simple(self):
        IMP.test.TestCase.setUp(self)

        m = IMP.Model()
        p1 = IMP.kernel.Particle(m)
        p2 = IMP.kernel.Particle(m)
        #p3 = IMP.kernel.Particle(m)

        self.slope = 0.01
        self.length = 10

        self.xyz1 = IMP.core.XYZ.setup_particle(p1)
        self.xyz2 = IMP.core.XYZ.setup_particle(p2)
        
        self.xyz1.set_coordinates((0, 0, 0))
        self.xyz2.set_coordinates((0, 0, 0))

        
        self.sigma1 = setupnuisance(m, 5, 0, 100, False)
        self.sigma2 = setupnuisance(m, 5, 0, 100, False)
        self.psi = setupnuisance(m, 0.1, 0.0, 0.5, False)

        dr = IMP.isd.CrossLinkMSRestraint(m, self.length, self.slope)
        dr.add_contribution((p1.get_index(), p2.get_index()),
                            (self.sigma1.get_particle_index(), self.sigma2.get_particle_index()),
                            self.psi.get_particle_index())
        
        self.testdr= CrossLinkMS(self.length, self.slope)
        self.testdr.add_contribution(self.xyz1,self.xyz2,self.sigma1,self.sigma2,self.psi)

        self.lw = IMP.isd.LogWrapper([dr],1.0)
      
   
        maxdist = 40.0
        npoints = 100
        
        sigmas1=sample([0.01,0.1,0.5,1.0,5.0,10.0,50.0,100.0],5)
        sigmas2=sample([0.01,0.1,0.5,1.0,5.0,10.0,50.0,100.0],5)
        psis=sample([0.01,0.05,0.1,0.15,0.2,0.25,0.3,0.35,0.4,0.45,0.49],5)
        
        for s1 in sigmas1:
            self.sigma1.set_scale(s1)
            for s2 in sigmas2:
                self.sigma2.set_scale(s2)
                for p1 in psis: 
                    self.psi.set_scale(p1)
                    for i in range(npoints):
                        self.xyz2.set_coordinates(
                            IMP.algebra.Vector3D(maxdist / npoints * float(i), 0.0, 0.0))
                        dist = IMP.core.get_distance(self.xyz1, self.xyz2)
                        scoretest = - \
                            log(self.testdr.get_probability())
                        score = self.lw.unprotected_evaluate(None)
                        self.assertAlmostEqual(score,scoretest,places=5)

    def test_score_two_fold_ambiguity(self):
        IMP.test.TestCase.setUp(self)

        m = IMP.Model()
        p1 = IMP.kernel.Particle(m)
        p2 = IMP.kernel.Particle(m)
        p3 = IMP.kernel.Particle(m)

        self.slope = 0.01
        self.length = 10

        self.xyz1 = IMP.core.XYZ.setup_particle(p1)
        self.xyz2 = IMP.core.XYZ.setup_particle(p2)
        self.xyz3 = IMP.core.XYZ.setup_particle(p3)
        
        self.xyz1.set_coordinates((0, 0, 0))
        self.xyz2.set_coordinates((0, 0, 0))
        self.xyz3.set_coordinates((40, 0, 0))

        
        self.sigma1 = setupnuisance(m, 5, 0, 100, False)
        self.sigma2 = setupnuisance(m, 5, 0, 100, False)
        self.sigma3 = setupnuisance(m, 5, 0, 100, False)
        self.psi = setupnuisance(m, 0.1, 0.0, 0.5, False)

        dr = IMP.isd.CrossLinkMSRestraint(m, self.length, self.slope)
        dr.add_contribution((p1.get_index(), p2.get_index()),
                            (self.sigma1.get_particle_index(), self.sigma2.get_particle_index()),
                            self.psi.get_particle_index())
        dr.add_contribution((p3.get_index(), p2.get_index()),
                            (self.sigma3.get_particle_index(), self.sigma2.get_particle_index()),
                            self.psi.get_particle_index())
        
        self.testdr= CrossLinkMS(self.length, self.slope)
        self.testdr.add_contribution(self.xyz1,self.xyz2,self.sigma1,self.sigma2,self.psi)
        self.testdr.add_contribution(self.xyz3,self.xyz2,self.sigma3,self.sigma2,self.psi)
        
        self.lw = IMP.isd.LogWrapper([dr],1.0)
      
   
        maxdist = 40.0
        npoints = 100
        
        sigmas1=sample([0.01,0.1,0.5,1.0,5.0,10.0,50.0,100.0],5)
        sigmas2=sample([0.01,0.1,0.5,1.0,5.0,10.0,50.0,100.0],5)
        sigmas3=sample([0.01,0.1,0.5,1.0,5.0,10.0,50.0,100.0],5) 
        psis=sample([0.01,0.05,0.1,0.15,0.2,0.25,0.3,0.35,0.4,0.45,0.49],5)
        
        for s1 in sigmas1:
            self.sigma1.set_scale(s1)
            for s2 in sigmas2:
                self.sigma2.set_scale(s2)
                for s3 in sigmas3:
                    self.sigma3.set_scale(s3)                
                    for p1 in psis: 
                        self.psi.set_scale(p1)
                        for i in range(npoints):
                            self.xyz2.set_coordinates(
                                IMP.algebra.Vector3D(maxdist / npoints * float(i), 0.0, 0.0))
                            dist = IMP.core.get_distance(self.xyz1, self.xyz2)
                            scoretest = - \
                                log(self.testdr.get_probability())
                            score = self.lw.unprotected_evaluate(None)
                            self.assertAlmostEqual(score,scoretest,delta=1.e-5)


if __name__ == '__main__':
    IMP.test.main()
