import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.restrainer
import math

class Tests(IMP.test.TestCase):
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
        ps = []
        for mh in mhs:
            ps.append(mh.get_particle())

        # Randomize position of the first particle
        t0 = IMP.algebra.Transformation3D(
            IMP.algebra.get_identity_rotation_3d(),
            IMP.algebra.get_random_vector_in(
                    IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                              IMP.algebra.Vector3D(20,20,20))))
        for p in IMP.core.get_leaves(p0):
            d= IMP.core.XYZR(p.get_particle())
            d.set_radius(1)
            coords = t0.get_transformed(d.get_coordinates())
            d.set_coordinates(coords)

        # Randomize position of the second particle
        t1 = IMP.algebra.Transformation3D(
            IMP.algebra.get_identity_rotation_3d(),
            IMP.algebra.get_random_vector_in(
                    IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                              IMP.algebra.Vector3D(20,20,20))))
        for p in IMP.core.get_leaves(p1):
            d= IMP.core.XYZR(p.get_particle())
            d.set_radius(1)
            coords = t1.get_transformed(d.get_coordinates())
            d.set_coordinates(coords)


        # Setup rigid bodies
        rbs = IMP.restrainer.set_rigid_bodies(mhs)

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
                    self.assertGreater(d, -.1,
                                 "Excluded volume did not separate spheres "
                                 "%s and %s (surface-surface distance = %f)" \
                                 % (sa, sb, d))

    def test_ev_on_molecules(self):
        """Test excluded volume restraint on molecules"""
        m, mhs, rbs = self.setup_system()
        self.optimize_system(m, mhs,
                IMP.restrainer.create_simple_excluded_volume_on_molecules(mhs))

    def test_ev_on_rigid_bodies(self):
        """Test excluded volume restraint on rigid bodies"""
        m, mhs, rbs = self.setup_system()
        self.optimize_system(m, mhs,
                IMP.restrainer.create_simple_excluded_volume_on_rigid_bodies(rbs))

if __name__ == '__main__':
    IMP.test.main()
