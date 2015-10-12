import IMP
import IMP.test
import IMP.core
import IMP.container
import IMP.algebra
import math

def make_histogram(values, bin_low, bin_high, bin_size):
    hist = [0] * (int)((bin_high - bin_low) / bin_size)
    for v in values:
        bin_index = (int)((v - bin_low) / bin_size)
        hist[bin_index] += 1
    return hist

class Tests(IMP.test.TestCase):

    def setup(self, m):
        # Make reference particles
        ref = [IMP.Particle(m) for x in range(3)]
        IMP.core.XYZ.setup_particle(ref[0], IMP.algebra.Vector3D(10,10,0))
        IMP.core.XYZ.setup_particle(ref[1], IMP.algebra.Vector3D(10,14,0))
        IMP.core.XYZ.setup_particle(ref[2], IMP.algebra.Vector3D(10,7,0))
        # Make images
        im = [IMP.Particle(m) for x in range(3)]
        for i in im:
            IMP.core.XYZ.setup_particle(i, IMP.algebra.Vector3D(0,0,0))
        # Tell each image what its reference is
        for i, r in zip(im, ref):
            IMP.core.Reference.setup_particle(i, r)
        # Create transformation
        tr = IMP.algebra.Transformation3D(
                 IMP.algebra.get_rotation_about_axis(
                           IMP.algebra.get_basis_vector_3d(2), math.pi/6.0),
                 IMP.algebra.Vector3D(8, 0, 0))
        sm = IMP.core.TransformationSymmetry(tr)
        # Create the constraint
        lsc = IMP.container.ListSingletonContainer(m, im)
        c = IMP.container.SingletonsConstraint(sm, None, lsc)
        m.add_score_state(c)
        return ref, im, sm

    def test_rotation(self):
        """Test TransformationSymmetryMover rotation"""
        m = IMP.Model()
        ref, im, sm = self.setup(m)
        m.update()
        orig_im_coor = [IMP.core.XYZ(x).get_coordinates() for x in im]
        mover = IMP.core.TransformationSymmetryMover(m, sm, ref[0], 0.,
                                                     math.pi)
        angles = [[], []]
        for iteration in range(50):
            mover.propose()
            m.update()
            im_coor = [IMP.core.XYZ(x).get_coordinates() for x in im]
            # im[0] should not move from original position
            self.assertLess(IMP.algebra.get_distance(im_coor[0],
                                                     orig_im_coor[0]), 1e-6)
            # distances between im[i] and im[j] should not change
            for i in range(3):
                for j in range(i, 3):
                    orig_d = IMP.algebra.get_distance(orig_im_coor[i],
                                                      orig_im_coor[j])
                    d = IMP.algebra.get_distance(im_coor[i], im_coor[j])
                    self.assertAlmostEqual(orig_d, d, delta=1e-6)
            # im[1] and im[2] should fully explore circle around im[0]
            for num, i in enumerate((1,2)):
                angle = math.atan2(im_coor[i][0] - im_coor[0][0],
                                   im_coor[i][1] - im_coor[0][1])
                angles[num].append(angle)
            mover.accept()
        for i in range(2):
            hist = make_histogram(angles[i], -math.pi, math.pi, math.pi / 4.)
            # Histogram should be flat, but we don't have enough samples
            # to assert that
            print(hist)

    def test_translation(self):
        """Test TransformationSymmetryMover translation"""
        m = IMP.Model()
        ref, im, sm = self.setup(m)
        m.update()
        orig_im_coor = [IMP.core.XYZ(x).get_coordinates() for x in im]
        mover = IMP.core.TransformationSymmetryMover(m, sm, ref[0], 5., 0.)
        displacement = [[], [], []]
        for iteration in range(50):
            mover.propose()
            m.update()
            im_coor = [IMP.core.XYZ(x).get_coordinates() for x in im]
            # vectors between im[i] and im[j] should not change
            for i in range(3):
                for j in range(i, 3):
                    orig_d = orig_im_coor[i] - orig_im_coor[j]
                    d = im_coor[i] - im_coor[j]
                    self.assertLess(IMP.algebra.get_distance(orig_d, d), 1e-5)
            # each im should fully explore sphere around original position
            for i in range(3):
                displacement[i].append(IMP.algebra.get_distance(
                                            im_coor[i], orig_im_coor[i]))
            mover.reject()
        for i in range(3):
            hist = make_histogram(displacement[i], 0, 5.0, 1.0)
            def volume_for_bin(i):
                high_val = i + 1.0
                low_val = i
                high_volume = 4./3. * math.pi * high_val * high_val * high_val
                low_volume = 4./3. * math.pi * low_val * low_val * low_val
                return high_volume - low_volume
            # Account for increasing volume of sphere with radius
            hist = [hist[i] / volume_for_bin(i) for i in range(len(hist))]
            # Histogram should be flat, but we don't have enough samples
            # to assert that
            print(hist)

if __name__ == '__main__':
    IMP.test.main()
