import IMP.em
import IMP.test
import IMP.core


class Tests(IMP.test.TestCase):

    def test_local_rigid_fitting(self):
        """Test local_rigid_fitting()"""
        resolution = 8.
        dmap = IMP.em.read_map(self.get_input_file_name('3points.mrc'),
                               IMP.em.MRCReaderWriter())
        dmap.get_header_writable().set_resolution(resolution)

        m = IMP.kernel.Model()

        sel = IMP.atom.CAlphaPDBSelector()
        mh = IMP.atom.read_pdb(self.get_input_file_name('3points.pdb'), m, sel)
        IMP.atom.add_radii(mh)
        ps = IMP.core.get_leaves(mh)
        IMP.atom.create_rigid_body(mh)

        refiner = IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())

        fitting_sols = IMP.em.local_rigid_fitting(
            mh.get_particle(), refiner,
            IMP.atom.Mass.get_mass_key(),
            dmap, [], 1, 10, 100)
        self.assertEqual(fitting_sols.get_number_of_solutions(), 1)

if __name__ == '__main__':
    IMP.test.main()
