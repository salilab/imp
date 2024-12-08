import IMP
import IMP.test
import IMP.atom
import IMP.core
import IMP.saxs


class Tests(IMP.test.TestCase):

    def test_surface_area(self):
        """Check protein surface computation"""
        m = IMP.Model()

        # read PDB
        mp = IMP.atom.read_pdb(self.get_input_file_name('6lyz.pdb'), m,
                               IMP.atom.NonWaterNonHydrogenPDBSelector())
        IMP.atom.add_radii(mp)

        # select atom particles from the model
        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

        # calculate surface accessibility
        s = IMP.saxs.SolventAccessibleSurface()
        surface_area = s.get_solvent_accessibility(IMP.core.XYZRs(particles))

        self.assertAlmostEqual(sum(surface_area), 37.728, delta=0.1)

    def test_surface_area2(self):
        """Atom radii and probe radius parameters that work for SOAP"""
        m = IMP.Model()

        # read PDB
        mp = IMP.atom.read_pdb(self.get_input_file_name('6lyz.pdb'), m,
                               IMP.atom.NonWaterNonHydrogenPDBSelector())
        IMP.atom.add_radii(mp)

        # select atom particles from the model
        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

        for p in particles:
            xyzrp = IMP.core.XYZR(p)
            xyzrp.set_radius(0.7 * xyzrp.get_radius())

        # calculate surface accessibility
        s = IMP.saxs.SolventAccessibleSurface()
        surface_area = s.get_solvent_accessibility(
            IMP.core.XYZRs(particles), 1.4)

        self.assertAlmostEqual(sum(surface_area), 73.53, delta=0.1)

    def test_corner_case(self):
        """Check the surface area handle points on boundary"""
        # this test could be simplified probably, but it is fast enough
        # if we move to non-grid based SA, it should go away
        ensemble = ["./433.pdb", "./434.pdb"]
        m = IMP.Model()
        # read PDBs
        for struc in ensemble:
            print(" ... Fitting structure %s" % struc)
            mp = IMP.atom.read_pdb(self.get_input_file_name(struc), m,
                                   IMP.atom.NonWaterNonHydrogenPDBSelector())

            # select particles from the model
            particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
            # add radius for water layer computation
            ft = IMP.saxs.get_default_form_factor_table()
            for i in range(0, len(particles)):
                radius = ft.get_radius(particles[i])
            IMP.core.XYZR(particles[i]).set_radius(radius)
            # compute surface accessibility
            s = IMP.saxs.SolventAccessibleSurface()
            _ = s.get_solvent_accessibility(IMP.core.XYZRs(particles))


if __name__ == '__main__':
    IMP.test.main()
