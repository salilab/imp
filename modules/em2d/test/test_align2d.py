import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em2d
import IMP.em
import random
import math


class Tests(IMP.test.TestCase):

    def test_rotational_alignment(self):
        """ Test the rotational alignment in 2D routine (new) """
        random.seed()
        name = self.get_input_file_name("1z5s-projection-2.spi")
        srw = IMP.em2d.SpiderImageReaderWriter()
        image = IMP.em2d.Image()
        image.read(name, srw)
        rotated = IMP.em2d.Image()
        # random rotation
        angle = random.random() * 2 * math.pi
        rot = IMP.algebra.Rotation2D(angle)
        transformation = IMP.algebra.Transformation2D(rot)
        IMP.em2d.get_transformed(image.get_data(), rotated.get_data(),
                                 transformation)

        result = IMP.em2d.get_rotational_alignment(
            image.get_data(), rotated.get_data(), True)
        determined_angle = result[0].get_rotation().get_angle()
        # approximately 6 degrees tolerance, 0.1 rad.
        x = angle + determined_angle
        modulo = (
            abs(x %
                (2 * math.pi)) < 0.1) or (abs(x %
                                          (2 * math.pi) - 2 * math.pi) < 0.1)
        self.assertEqual(
            modulo, True, msg="Angles applied %f and "
            "determined %f are different, difference %f"
            % (angle, determined_angle, x))

    def test_translational_alignment(self):
        """ Test the translational alignment in 2D routine """
        random.seed()
        name = self.get_input_file_name("1z5s-projection-2.spi")
        srw = IMP.em2d.SpiderImageReaderWriter()
        image = IMP.em2d.Image()
        image.read(name, srw)
        translated = IMP.em2d.Image()
        # random translation
        trans = IMP.algebra.Vector2D(
            random.random() * 10,
            random.random() * 10)
        transformation = IMP.algebra.Transformation2D(trans)
        IMP.em2d.get_transformed(image.get_data(), translated.get_data(),
                                 transformation)
        result = IMP.em2d.get_translational_alignment(
            image.get_data(), translated.get_data(), True)
        # -1 to get the translation applied to reference.
        # Result contains the translation required for align the second matrix
        determined_trans = (-1) * result[0].get_translation()
        # Tolerate 1 pixel error
        self.assertAlmostEqual(
            abs(determined_trans[0] - trans[0]), 0, delta=1,
            msg="1st coordinate is incorrect: Applied %f Determined %f"
                % (trans[0], determined_trans[0]))
        self.assertAlmostEqual(
            abs(determined_trans[1] - trans[1]), 0, delta=1,
            msg="2nd coordinate is incorrect: Applied %f Determined %f"
                % (trans[1], determined_trans[1]))

    def test_complete_alignment(self):
        """ Test the complete alignment in 2D routine (new) """
        random.seed()
        name = self.get_input_file_name("1z5s-projection-2.spi")
        srw = IMP.em2d.SpiderImageReaderWriter()
        image = IMP.em2d.Image()
        image.read(name, srw)
        transformed = IMP.em2d.Image()

        rot = IMP.algebra.Rotation2D(random.random() * 2 * math.pi)
        trans = IMP.algebra.Vector2D(
            random.random() * 10,
            random.random() * 10)

        T = IMP.algebra.Transformation2D(rot, trans)
        IMP.em2d.get_transformed(image.get_data(), transformed.get_data(), T)

        result = IMP.em2d.get_complete_alignment(image.get_data(),
                                                 transformed.get_data(), True)
        cross_correlation_coefficient = result.second
        # Tolerate 1 pixel error
        self.assertAlmostEqual(
            cross_correlation_coefficient, 1, delta=0.035,
            msg="Error in the complete alignment routine,"
                "ccc %f less than 0.965" % (cross_correlation_coefficient))


if __name__ == '__main__':
    IMP.test.main()
