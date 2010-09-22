import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.helper
import math

class SimpleExcludedVolumeTests(IMP.test.TestCase):
    """Tests for simple excluded volume"""

    def setup_system(self):
        m= IMP.Model()
        # Read in two chains
        sel = IMP.atom.CAlphaPDBSelector()
        p0= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"), m, sel)
        p1= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"), m, sel)
        mhs = IMP.atom.Hierarchies()
        mhs.append(p0)
        mhs.append(p1)
        ps = IMP.Particles()
        for mh in mhs:
            ps.append(mh.get_particle())

        rbs = IMP.helper.set_rigid_bodies(mhs)
        # Set radius of each atom for excluded volume
        for p in IMP.core.get_leaves(p0)+IMP.core.get_leaves(p1):
            d= IMP.core.XYZR.setup_particle(p.get_particle())
            d.set_radius(1)

        # Randomize the position of each chain
        for p in ps:
            d= IMP.core.XYZ(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(
                    IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                              IMP.algebra.Vector3D(20,20,20))))
        return m, mhs, rbs

    def optimize_system(self, m, mhs, simple_exvol):
        r = simple_exvol.get_restraint()

        m.add_restraint(r)

        # Make sure that refcounting is working correctly
        # refs from Python, the SimpleExcludedVolume object, and the Model
        self.assertEqual(r.get_ref_count(), 3)

        o= IMP.core.ConjugateGradients(m)
        o.optimize(100)

        # Make sure that the excluded volume restraint separated the two chains
        sas = IMP.core.XYZRs(IMP.core.get_leaves(mhs[1]))
        sbs = IMP.core.XYZRs(IMP.core.get_leaves(mhs[0]))
        for sa in sas:
            for sb in sbs:
                if sa != sb:
                    d = IMP.core.get_distance(sa, sb)
                    self.assert_(d > -.1,
                                 "Excluded volume did not separate spheres "
                                 "%s and %s (surface-surface distance = %f)" \
                                 % (sa, sb, d))

    def test_ev_on_molecules(self):
        """Test excluded volume restraint on molecules"""
        m, mhs, rbs = self.setup_system()
        self.optimize_system(m, mhs,
                IMP.helper.create_simple_excluded_volume_on_molecules(mhs))

    def test_ev_on_rigid_bodies(self):
        """Test excluded volume restraint on rigid bodies"""
        m, mhs, rbs = self.setup_system()
        self.optimize_system(m, mhs,
                IMP.helper.create_simple_excluded_volume_on_rigid_bodies(rbs))

if __name__ == '__main__':
    IMP.test.main()
