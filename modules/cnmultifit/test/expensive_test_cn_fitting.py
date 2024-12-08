import IMP
import IMP.test
import sys
import IMP.em
import IMP.cnmultifit
import os
import random
import time


class CnFittingTests(IMP.test.TestCase):

    """Tests for generating cyclic models"""

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.density_filename = self.get_input_file_name("1oel_10.mrc")
        mrw = IMP.em.MRCReaderWriter()
        self.dmap = IMP.em.read_map(self.density_filename, mrw)
        self.dmap.get_header_writable().set_resolution(10)
        self.cn_symm_deg = 7
        self.dens_threshold = 50
        self.ref_prot_fn = self.get_input_file_name("1oel.pdb")

        # load the assembly
        self.mdl = IMP.Model()
        self.ref = IMP.atom.read_pdb(self.ref_prot_fn, self.mdl)
        self.ref_chains = IMP.atom.get_by_type(self.ref, IMP.atom.CHAIN_TYPE)
        self.ref_rbs = [IMP.atom.create_rigid_body(c) for c in self.ref_chains]

    def test_consistent_fitting(self):
        mh_pca = IMP.cnmultifit.MolCnSymmAxisDetector(self.cn_symm_deg,
                                                      self.ref_chains)
        all_trans = IMP.algebra.get_alignments_from_first_to_second(
            mh_pca.get_pca(), mh_pca.get_pca())
        aligner = IMP.cnmultifit.AlignSymmetric(self.dmap, self.dens_threshold,
                                                self.cn_symm_deg)
        # Enumerating all (~25) the transformations would take way too long,
        # so instead just pick a random subset
        for i, t in random.sample(list(enumerate(all_trans)), 3):
            for rb in self.ref_rbs:
                IMP.core.transform(rb, t)
            sols = IMP.cnmultifit.fit_cn_assembly(self.ref_chains, 1,
                                                  self.dmap, self.dens_threshold,
                                                  aligner, False, False)
            print("FOR trans ", i, " FIT VALUE:", sols.get_score(0))
            # Since the fit is done in 'fast' mode, the density map is not
            # resampled, and particles can end up outside the density, giving
            # relatively poor scores. So relax the score tolerance from
            # 0.05 to 0.15.
            self.assertLess(sols.get_score(0), 0.15)
#           IMP.atom.write_pdb(self.ref_chains,"mdl_"+str(i)+".pdb")
            for rb in self.ref_rbs:
                IMP.core.transform(rb, sols.get_transformation(0))
#           IMP.atom.write_pdb(self.ref_chains,"mdl_fitted_"+str(i)+".pdb")
            for rb in self.ref_rbs:
                IMP.core.transform(rb, sols.get_transformation(0).get_inverse())
            for rb in self.ref_rbs:
                IMP.core.transform(rb, t.get_inverse())

if __name__ == '__main__':
    IMP.test.main()
