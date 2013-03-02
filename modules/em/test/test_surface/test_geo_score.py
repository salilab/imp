import IMP.test
import IMP.em

class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        # Initial values and names of files
        self.mdl=IMP.Model()
        sel=IMP.atom.NonAlternativePDBSelector()
        mh1_fn = self.get_input_file_name('1z5s_A.pdb')
        self.mh1=IMP.atom.read_pdb(mh1_fn,self.mdl,sel)
        IMP.atom.add_radii(self.mh1)
        good_mh2_fn = self.get_input_file_name('1z5s_C.pdb')
        self.good_mh2=IMP.atom.read_pdb(good_mh2_fn,self.mdl,sel)
        IMP.atom.add_radii(self.good_mh2)
        bad_mh2_fn = self.get_input_file_name('1z5s_C_wrong.pdb')
        self.bad_mh2=IMP.atom.read_pdb(bad_mh2_fn,self.mdl,sel)
        IMP.atom.add_radii(self.bad_mh2)
        self.mh1_shell_map = IMP.em.SurfaceShellDensityMap(IMP.core.get_leaves(self.mh1),
                                                             2.0)
        self.good_mh2_shell_map = IMP.em.SurfaceShellDensityMap(IMP.core.get_leaves(self.good_mh2),
                                                             2.0)
        self.bad_mh2_shell_map = IMP.em.SurfaceShellDensityMap(IMP.core.get_leaves(self.bad_mh2),
                                                             2.0)
        self.mh1_shell_map.calcRMS()
        self.good_mh2_shell_map.calcRMS()
        self.bad_mh2_shell_map.calcRMS()
    def test_good_geo_comp(self):
        """Check good geometric complementarity score"""
        erw = IMP.em.EMReaderWriter()
         #fix map dimensions
        h1=self.mh1_shell_map.get_header()
        h2=self.good_mh2_shell_map.get_header()
        nx=max(h1.get_nx(),h2.get_nx())
        ny=max(h1.get_ny(),h2.get_ny())
        nz=max(h1.get_nz(),h2.get_nz())

        self.mh1_shell_map.pad(nx,ny,nz)
        self.good_mh2_shell_map.pad(nx,ny,nz)
        self.mh1_shell_map.calcRMS()
        self.good_mh2_shell_map.calcRMS()
        conv = IMP.em.CoarseCC()
        score = conv.cross_correlation_coefficient(self.mh1_shell_map,
                                                   self.good_mh2_shell_map,
                                                   0.0,False)
        print "SCORE : " + str(score)
    def test_bad_geo_comp(self):
        """Check bad geometric complementarity score"""
        erw = IMP.em.EMReaderWriter()
        #        self.prot1_shell_map.Write("prot1_shell.em",erw)
        #        self.prot2_shell_map.Write("prot2_shell.em",erw)
        #fix map dimensions
        h1=self.mh1_shell_map.get_header()
        h2=self.bad_mh2_shell_map.get_header()
        nx=max(h1.get_nx(),h2.get_nx())
        ny=max(h1.get_ny(),h2.get_ny())
        nz=max(h1.get_nz(),h2.get_nz())

        self.mh1_shell_map.pad(nx,ny,nz)
        self.bad_mh2_shell_map.pad(nx,ny,nz)
        self.mh1_shell_map.calcRMS()
        self.bad_mh2_shell_map.calcRMS()
        conv = IMP.em.CoarseCC()
        score = conv.cross_correlation_coefficient(self.mh1_shell_map,
                                                   self.bad_mh2_shell_map,
                                                   0.0,False)
        print "SCORE BAD : " + str(score)

if __name__=='__main__':
    IMP.test.main()
