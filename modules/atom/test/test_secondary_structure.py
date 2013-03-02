import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.algebra
from operator import itemgetter
from math import sqrt

class Tests(IMP.test.TestCase):
    """Test the SecondaryStructureResidue decorator and reader"""
    def test_get_set(self):
        m = IMP.Model()
        p = IMP.Particle(m)
        ssr = IMP.atom.SecondaryStructureResidue.setup_particle(p,0.5,0.25,0.25)
        self.assertAlmostEqual(ssr.get_prob_helix(), 0.5, delta=1e-6)
        self.assertAlmostEqual(ssr.get_prob_strand(), 0.25, delta=1e-6)
        self.assertAlmostEqual(ssr.get_prob_coil(), 0.25, delta=1e-6)

    def test_coarsen(self):
        """Test coarsening a set of SecondaryStructureResidues"""
        m = IMP.Model()
        avs=[0.0,0.0,0.0]
        helices=[0.6,0.4,0.05]
        coils=[0.25,0.35,0.15]
        strands=[0.15,0.25,0.8]
        av=[0.0,0.0,0.0]
        av_w=[0.0,0.0,0.0]
        ps=[]

        for i in range(3):
            p=IMP.Particle(m)
            ssr=IMP.atom.SecondaryStructureResidue.setup_particle(p,
                                           helices[i],coils[i],strands[i])
            av[0]+=helices[i]/3
            av[1]+=coils[i]/3
            av[2]+=strands[i]/3
            sses=[helices[i],coils[i],strands[i]]
            nmax,vmax=max(enumerate(sses),key=itemgetter(1))
            av_w[nmax]+=1.0/3.0
            ps.append(p)
        coarse_ssr=IMP.atom.setup_coarse_secondary_structure_residue(ps,m,False)
        self.assertAlmostEqual(coarse_ssr.get_prob_helix(),
                               av[0], delta=1e-6)
        self.assertAlmostEqual(coarse_ssr.get_prob_strand(),
                               av[1], delta=1e-6)
        self.assertAlmostEqual(coarse_ssr.get_prob_coil(),
                               av[2], delta=1e-6)

        coarse_ssr_w=IMP.atom.setup_coarse_secondary_structure_residue(ps,m,True)
        self.assertAlmostEqual(coarse_ssr_w.get_prob_helix(),
                               av_w[0], delta=1e-6)
        self.assertAlmostEqual(coarse_ssr_w.get_prob_strand(),
                               av_w[1], delta=1e-6)
        self.assertAlmostEqual(coarse_ssr_w.get_prob_coil(),
                               av_w[2], delta=1e-6)

    def test_match_score(self):
        """Test if SecondaryStructureResidue scores are compared correctly"""
        m = IMP.Model()
        ssr_vals=[[0.5,0.25,0.25],
                  [0.9,0.05,0.05]]
        p1 = IMP.Particle(m)
        ssr1 = IMP.atom.SecondaryStructureResidue.setup_particle(p1,
                                   ssr_vals[0][0],ssr_vals[0][1],ssr_vals[0][2])
        p2 = IMP.Particle(m)
        ssr2 = IMP.atom.SecondaryStructureResidue.setup_particle(p2,
                                   ssr_vals[1][0],ssr_vals[1][1],ssr_vals[1][2])
        rmsd=0.0
        for i in range(3):
            rmsd+=(ssr_vals[1][i]-ssr_vals[0][i])**2
        rmsd=sqrt(rmsd)
        self.assertAlmostEqual(IMP.atom.get_secondary_structure_match_score(ssr1,ssr2),
                               rmsd,delta=1e-6)
    def test_psipred_reader(self):
        """Test if psipred file is read into SecondaryStructureResidues"""
        m = IMP.Model()
        ssres=IMP.atom.read_psipred(self.open_input_file("yGCP2.psipred"),m)
        self.assertEqual(len(ssres),769)
        self.assertAlmostEqual(ssres[0].get_prob_coil(),1.0,delta=1e-6)
        self.assertAlmostEqual(ssres[0].get_prob_helix(),0.0,delta=1e-6)
        self.assertAlmostEqual(ssres[0].get_prob_strand(),0.0,delta=1e-6)

        self.assertAlmostEqual(ssres[1].get_prob_coil(),0.0,delta=1e-6)
        self.assertAlmostEqual(ssres[1].get_prob_helix(),8.0/10.0,delta=1e-6)
        self.assertAlmostEqual(ssres[1].get_prob_strand(),0.0,delta=1e-6)

    def test_psipred_reader_provided_particles(self):
        """Test if psipred file (+particles) are read into SecondaryStructureResidues"""
        m = IMP.Model()
        ps=[]
        for i in range(769):
            p=IMP.Particle(m)
            ps.append(p)

        ssres=IMP.atom.read_psipred(self.open_input_file("yGCP2.psipred"),ps)
        self.assertEqual(len(ssres),769)
        self.assertAlmostEqual(ssres[0].get_prob_coil(),1.0,delta=1e-6)
        self.assertAlmostEqual(ssres[0].get_prob_helix(),0.0,delta=1e-6)
        self.assertAlmostEqual(ssres[0].get_prob_strand(),0.0,delta=1e-6)

        self.assertAlmostEqual(ssres[1].get_prob_coil(),0.0,delta=1e-6)
        self.assertAlmostEqual(ssres[1].get_prob_helix(),8.0/10.0,delta=1e-6)
        self.assertAlmostEqual(ssres[1].get_prob_strand(),0.0,delta=1e-6)

if __name__ == '__main__':
    IMP.test.main()
