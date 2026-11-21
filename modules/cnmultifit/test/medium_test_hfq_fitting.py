import IMP
import IMP.test
import IMP.cnmultifit
import IMP.multifit
import IMP.atom
import os


class Tests(IMP.test.TestCase):

    def write_params(self, monomer):
        with open('hfq.param', 'w') as fh:
            fh.write("""[files]
monomer = %s
surface = %s.ms
prot_lib = %s
output = multifit.output
model = asmb.model

[symmetry]
cn = 6
dn = 1

[scoring]
small_interface_ratio = 0.1
max_penetration = -5.0
threshold = 0.5
weight1 = -8
weight2 = -4
weight3 = 0
weight4 = 1
weight5 = 0

[density]
map = %s
resolution = 15.0
spacing = 5.0
origin_x = -84.0
origin_y = -82.0
origin_z = -66.0
threshold = 0.240663
pca_matching_threshold = 11.25

[clustering]
axis_angle_threshold = 18
min_size = 1
distance = 2.0

[base]
min_distance = 5.0
max_distance = 50.0

[grid]
step = 0.5
max_distance = 6.0
volume_radius = 6.0

[surface]
threshold = 1.5

[fitting]
solutions = 5
""" % (monomer, monomer, IMP.multifit.get_data_path('chem.lib'),
                self.get_input_file_name('hfq-15.0.mrc')))

    def test_hfq_fitting(self):
        """Test do_all_fitting on the Hfq hexamer (C6) at 15A"""
        with IMP.test.temporary_working_directory():
            # Use a copy of the monomer so that its surface is written in
            # the temporary directory
            monomer = os.path.abspath('hfq-monomer.pdb')
            with open(self.get_input_file_name('hfq-monomer.pdb')) as fin:
                with open(monomer, 'w') as fout:
                    fout.write(fin.read())
            # libTAU needs the molecular surface
            m = IMP.Model()
            h = IMP.atom.read_pdb(
                monomer, m, IMP.atom.NonWaterNonHydrogenPDBSelector())
            IMP.multifit.write_connolly_surface(
                IMP.atom.get_leaves(h), monomer + '.ms', 10.0, 1.8)
            self.write_params(monomer)
            IMP.cnmultifit.do_all_fitting('hfq.param')
            with open('multifit.output') as fh:
                self.assertEqual(len(fh.readlines()), 6)

            m = IMP.Model()
            ref = IMP.atom.read_pdb(self.get_input_file_name('hfq-ref.pdb'),
                                    m, IMP.atom.CAlphaPDBSelector())
            ref_chains = IMP.atom.get_by_type(ref, IMP.atom.CHAIN_TYPE)
            self.assertEqual(len(ref_chains), 6)
            model = IMP.atom.read_pdb('asmb.model.0.pdb', m,
                                      IMP.atom.CAlphaPDBSelector())
            model_chains = IMP.atom.get_by_type(model, IMP.atom.CHAIN_TYPE)
            self.assertEqual(len(model_chains), 6)
            # The best scoring model should be close to native
            rmsd = IMP.cnmultifit.get_cn_rmsd(model_chains, ref_chains)
            self.assertLess(rmsd, 4.0)


if __name__ == '__main__':
    IMP.test.main()
