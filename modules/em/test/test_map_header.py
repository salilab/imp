import os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.algebra


class Tests(IMP.test.TestCase):
    """Class for testing Map header data"""

    def make_map(self, voxel_size):
        m = IMP.kernel.Model()
        ps = [IMP.Particle(m)]
        IMP.core.XYZR.setup_particle(ps[0],
                        IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1,2,3), 4))
        IMP.atom.Mass.setup_particle(ps[0], 1.0)
        dmap = IMP.em.SampledDensityMap(ps, 10.0, voxel_size)
        return m, dmap

    def test_header_values(self):
        """Check values in DensityMap header"""
        voxel_size = 2.0
        m, dmap = self.make_map(voxel_size)
        self.assertAlmostEqual(dmap.get_spacing(), voxel_size, delta=1e-6)

    def test_write_read_mrc(self):
        """Test mrc format read/write"""
        voxel_size = 2.0
        m, dmap = self.make_map(voxel_size)
        out_filename = "input1.mrc"
        mrw = IMP.em.MRCReaderWriter()
        IMP.em.write_map(dmap, out_filename, mrw)
        scene = IMP.em.read_map(out_filename, mrw)
        # Check header size
        self.assertAlmostEqual(scene.get_spacing(), voxel_size, delta=1e-6)
        os.unlink(out_filename)

    def test_write_read_em(self):
        """Test em format read/write"""
        voxel_size = 2.0
        m, dmap = self.make_map(voxel_size)
        out_filename = "input1.em"
        mrw = IMP.em.EMReaderWriter()
        IMP.em.write_map(dmap, out_filename, mrw)
        scene = IMP.em.read_map(out_filename, mrw)
        # Check header size
        self.assertAlmostEqual(scene.get_spacing(), voxel_size, delta=1e-6)
        os.unlink(out_filename)

    def test_get_header_memory(self):
        """Check memory management of DensityHeader"""
        voxel_size = 2.0
        model, m = self.make_map(voxel_size)
        hr = m.get_header()
        hw = m.get_header_writable()
        del m
        # Since DensityHeaders are returned as pointers into the DensityMap
        # object m accessing hr or hw after the deletion of m will cause
        # invalid memory access. The Python wrappers should ensure that
        # hr and hw keep a reference to m to prevent this from occurring.
        for h in hr, hw:
            a, b = h.alpha, h.beta
            h.alpha, h.beta = 45., 90.


if __name__ == '__main__':
    IMP.test.main()
