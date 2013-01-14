import IMP
import IMP.test
import IMP.multifit

class Tests(IMP.test.TestCase):

    def test_no_file(self):
        """Check AlignmentParams() with non-existing input file"""
        self.skipTest("Causes an infinite loop with current code")
        p = IMP.multifit.AlignmentParams("bad_input_file")
        p.process_parameters()

    def test_read(self):
        """Check AlignmentParams()"""
        p = IMP.multifit.AlignmentParams(
                               self.get_input_file_name("test.align.param"))
        p.process_parameters()
        d = p.get_domino_params()
        self.assertAlmostEqual(d.max_value_threshold_, 10000., delta=1e-6)
        self.assertEqual(d.max_num_states_for_subset_, 1000)
        self.assertAlmostEqual(d.max_anchor_penetration_, 0.0, delta=1e-6)
        self.assertEqual(d.heap_size_, 500000)
        self.assertEqual(d.cache_size_, 50000)
        f = p.get_fitting_params()
        self.assertAlmostEqual(f.pca_max_angle_diff_, 15., delta=1e-6)
        self.assertAlmostEqual(f.pca_max_size_diff_, 10., delta=1e-6)
        self.assertAlmostEqual(f.pca_max_cent_dist_diff_, 10., delta=1e-6)
        self.assertAlmostEqual(f.max_asmb_fit_score_, 0.5, delta=1e-6)
        c = p.get_complementarity_params()
        self.assertAlmostEqual(c.max_score_, 100000, delta=1e-6)
        self.assertAlmostEqual(c.max_penetration_, 200, delta=1e-6)
        self.assertAlmostEqual(c.interior_layer_thickness_, 2, delta=1e-6)
        self.assertAlmostEqual(c.boundary_coef_, -3, delta=1e-6)
        self.assertAlmostEqual(c.comp_coef_, 1, delta=1e-6)
        self.assertAlmostEqual(c.penetration_coef_, 2, delta=1e-6)
        x = p.get_xlink_params()
        self.assertAlmostEqual(x.upper_bound_, 20, delta=1e-6)
        self.assertAlmostEqual(x.k_, 0.0236, delta=1e-6)
        self.assertAlmostEqual(x.max_xlink_val_, 0.3, delta=1e-6)
        self.assertEqual(x.treat_between_residues_, False)
        c = p.get_connectivity_params()
        self.assertAlmostEqual(c.upper_bound_, 10, delta=1e-6)
        self.assertAlmostEqual(c.k_, 0.0236, delta=1e-6)
        self.assertAlmostEqual(c.max_conn_rest_val_, 0.3, delta=1e-6)
        r = p.get_rog_params()
        self.assertAlmostEqual(r.get_max_score(), 10, delta=1e-6)
        self.assertAlmostEqual(r.get_scale(), 1.5, delta=1e-6)
        f = p.get_fragments_params()
        self.assertEqual(f.frag_len_, 1)
        self.assertAlmostEqual(f.bead_radius_scale_, 1., delta=1e-6)
        self.assertEqual(f.load_atomic_, True)
        self.assertEqual(f.subunit_rigid_, True)
        f = p.get_filters_params()
        self.assertEqual(f.max_num_violated_xlink_, 4)
        self.assertEqual(f.max_num_violated_conn_, 4)
        self.assertEqual(f.max_num_violated_ev_, 3)
        e = p.get_ev_params()
        self.assertAlmostEqual(e.pair_distance_, 3., delta=1e-6)
        self.assertAlmostEqual(e.pair_slack_, 1., delta=1e-6)
        self.assertAlmostEqual(e.hlb_mean_, 2., delta=1e-6)
        self.assertAlmostEqual(e.hlb_k_, 0.59, delta=1e-6)
        self.assertAlmostEqual(e.maximum_ev_score_for_pair_, 0.295, delta=1e-6)
        self.assertAlmostEqual(e.allowed_percentage_of_bad_pairs_,
                               0.01, delta=1e-6)
        self.assertEqual(e.scoring_mode_, 2)

if __name__ == '__main__':
    IMP.test.main()
