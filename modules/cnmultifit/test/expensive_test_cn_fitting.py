import IMP
import IMP.test
import IMP.em
import IMP.cnmultifit
import random


class CnFittingTests(IMP.test.TestCase):

    """Tests for generating cyclic models"""

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.density_filename = self.get_input_file_name("1oel_10.mrc")
        mrw = IMP.em.MRCReaderWriter()
        self.dmap = IMP.em.read_map(self.density_filename, mrw)
        self.dmap.get_header_writable().set_resolution(10)
        # Use a coarser grid (3A rather than 1.5A spacing) for speed; this
        # is still sufficient for a 10A map
        self.dmap = IMP.em.get_resampled(self.dmap, 2.0)
        self.cn_symm_deg = 7
        self.dens_threshold = 50
        self.ref_prot_fn = self.get_input_file_name("1oel.pdb")

        # load the assembly
        self.mdl = IMP.Model()
        # Only CA atoms are used in the fitting, so don't read the others
        self.ref = IMP.atom.read_pdb(self.ref_prot_fn, self.mdl,
                                     IMP.atom.CAlphaPDBSelector())
        self.ref_chains = IMP.atom.get_by_type(self.ref, IMP.atom.CHAIN_TYPE)
        self.ref_rbs = [IMP.atom.create_rigid_body(c) for c in self.ref_chains]

    def get_cn_rmsd(self, ref_coords):
        """Get the CA RMSD between the current structure and the reference,
           allowing for the subunits to be relabeled by the Cn symmetry
           (cyclic permutation, in either direction)"""
        coords = [[a.get_coordinates() for a in c] for c in self.ca_xyzs]
        n = len(coords)
        best = None
        for shift in range(n):
            for direction in (1, -1):
                sumsq = npt = 0
                for i in range(n):
                    j = (direction * i + shift) % n
                    for a, b in zip(coords[i], ref_coords[j]):
                        sumsq += IMP.algebra.get_squared_distance(a, b)
                        npt += 1
                rmsd = (sumsq / npt) ** 0.5
                if best is None or rmsd < best:
                    best = rmsd
        return best

    def test_consistent_fitting(self):
        """Test that fitting gives the same answer from any starting point"""
        self.ca_xyzs = [
            [IMP.core.XYZ(p) for p in IMP.atom.Selection(
                c, atom_type=IMP.atom.AT_CA).get_selected_particles()]
            for c in self.ref_chains]
        all_cas = [a.get_particle() for c in self.ca_xyzs for a in c]
        ref_coords = [[a.get_coordinates() for a in c] for c in self.ca_xyzs]
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
            # Use fine rotational sampling around the symmetry axis; with
            # coarse sampling, for some starting orientations no sampled
            # rotation is close enough to the correct one
            sols = IMP.cnmultifit.fit_cn_assembly(
                self.ref_chains, 1, self.dmap, self.dens_threshold, aligner,
                False, True)
            for rb in self.ref_rbs:
                IMP.core.transform(rb, sols.get_transformation(0))
            # The best fit should match the original structure
            rmsd = self.get_cn_rmsd(ref_coords)
            # The fit is done in 'fast' mode, where the model density is not
            # resampled for each transformation; any density from the starting
            # structure that lies outside of the map is lost, so the score
            # depends on the starting orientation. Instead, check the score
            # of the fitted structure computed with a resampled model density.
            score = IMP.em.compute_fitting_scores(
                all_cas, self.dmap,
                [IMP.algebra.get_identity_transformation_3d()],
                False).get_score(0)
            print("FOR trans ", i, " FIT VALUE:", sols.get_score(0),
                  "RESAMPLED FIT VALUE:", score, "RMSD:", rmsd)
            self.assertLess(rmsd, 2.0)
            self.assertLess(score, 0.11)
            fit_inv = sols.get_transformation(0).get_inverse()
            for rb in self.ref_rbs:
                IMP.core.transform(rb, fit_inv)
            for rb in self.ref_rbs:
                IMP.core.transform(rb, t.get_inverse())


if __name__ == '__main__':
    IMP.test.main()
