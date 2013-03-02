import IMP
import IMP.test
import sys
import IMP.em
import IMP.cnmultifit
import os
import time

class Tests(IMP.test.TestCase):
    """Tests for generating cyclic models"""

    def test_cn_rmsd(self):
        """Test get_cn_rmsd()"""
        def check_rmsd(mh1, mh2, expected_rmsd):
            meth = IMP.cnmultifit.get_cn_rmsd
            # Check 1:1 correspondence of chains
            self.assertAlmostEqual(meth(mh1, mh2), expected_rmsd, delta=1e-2)

            # Check chains in same order (clockwise vs. counterclockwise)
            # but offset
            self.assertAlmostEqual(meth(mh1, mh2[2:] + mh2[:2]),
                                   expected_rmsd, delta=1e-2)

            # Check chains in opposite order
            self.assertAlmostEqual(meth(mh1, mh2[::-1]),
                                   expected_rmsd, delta=1e-2)

        m = IMP.Model()
        mh_ref = IMP.atom.read_pdb(self.get_input_file_name('mini.pdb'), m)
        mh = IMP.atom.read_pdb(self.get_input_file_name('mini.pdb'), m)

        mh_ref_chains = IMP.atom.get_by_type(mh_ref, IMP.atom.CHAIN_TYPE)
        mh_chains = IMP.atom.get_by_type(mh, IMP.atom.CHAIN_TYPE)

        # RMSD of self should be zero
        check_rmsd(mh_ref_chains, mh_chains, 0.0)

        # Offset one atom to make sure method doesn't always return zero!
        atoms = IMP.atom.get_by_type(mh_chains[0], IMP.atom.ATOM_TYPE)
        d = IMP.core.XYZ(atoms[0])
        d.set_coordinates(d.get_coordinates() \
                          + IMP.algebra.Vector3D(0.0, 0.0, 10.0))
        check_rmsd(mh_ref_chains, mh_chains, 1.690)

    def test_symmetry_score_calculation_for_molecule(self):
        m = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('trimer-ref.pdb'), m)
        mhs = IMP.atom.get_by_type(mh, IMP.atom.CHAIN_TYPE)
        dec = IMP.cnmultifit.MolCnSymmAxisDetector(3, mhs)
        dec.get_pca().show()
        min_score=999
        for i in range(3):
            min_score=min(dec.calc_symm_score(i),min_score)
        #one of the them should be the symm axis
        self.assertLess(min_score,1)

    def test_symmetry_score_calculation_for_density(self):
        """Test CnSymmAxisDetector with density"""
        dmap = IMP.em.read_map(self.get_input_file_name('1oel_10.mrc'),
                               IMP.em.MRCReaderWriter())
        dec = IMP.cnmultifit.CnSymmAxisDetector(7, dmap, 5.0)
        min_score=999.
        for i in range(3):
            min_score=min(dec.calc_symm_score(i), min_score)
        #one of them should be the symm axis
        self.assertLess(min_score, 10.)

    def _test_score_by_pca(self):
        aligner = IMP.cnmultifit.AlignSymmetric(self.dmap,self.cn_symm_deg)
        for sol_ind in [0,1,2]:
            IMP.cnmultifit.transform_cn_assembly(self.mhs,
                                                  self.asmb_sols[sol_ind].get_transformation(),
                                                  self.ccw)
            print aligner.score_alignment(self.mhs)
            IMP.cnmultifit.transform_cn_assembly(self.mhs,
                                                  self.asmb_sols[sol_ind].get_transformation().get_inverse(),
                                                  self.ccw)


if __name__ == '__main__':
    IMP.test.main()
