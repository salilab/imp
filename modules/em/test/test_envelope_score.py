import IMP.em
import IMP.test
import IMP.core


class Tests(IMP.test.TestCase):

    def test_envelope_score(self):
        """Test EnvelopeScore class"""
        dmap = IMP.em.read_map(self.get_input_file_name('3points.mrc'),
                               IMP.em.MRCReaderWriter())
        mdt = IMP.em.MapDistanceTransform(dmap, 0.1, 5.0)
        d = mdt.get_distance_from_envelope(IMP.algebra.Vector3D(0,0,0))
        self.assertAlmostEqual(d, 0.0, delta=1e-6)
        d = mdt.get_distance_from_envelope(IMP.algebra.Vector3D(0,4,0))
        self.assertAlmostEqual(d, -2.67, delta=0.01)

        es = IMP.em.EnvelopeScore(mdt)
        idt = IMP.algebra.get_identity_transformation_3d()
        in_pts = [IMP.algebra.Vector3D(0,4,0)]
        out_pts = [IMP.algebra.Vector3D(0,0,0)]
        self.assertTrue(es.is_penetrating(in_pts, 0.))
        self.assertTrue(es.is_penetrating(in_pts, idt, 0.))
        self.assertFalse(es.is_penetrating(out_pts, 0.))
        self.assertFalse(es.is_penetrating(out_pts, idt, 0.))

        self.assertAlmostEqual(es.score(in_pts), -4.0, delta=1e-6)
        self.assertAlmostEqual(es.score(in_pts, idt), -4.0, delta=1e-6)

        self.assertAlmostEqual(es.score(out_pts), 1.0, delta=1e-6)
        self.assertAlmostEqual(es.score(out_pts, idt), 1.0, delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
