import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em2d
import IMP.em
import os
import random
from math import *


def get_angle_difference_from_rotation(angle,determined_angle):
    difference1=abs(angle-determined_angle)
    difference2=abs(angle+determined_angle-6.28)
    if(difference2<difference1):
        difference1=difference2
    return difference1


class ProjectTests(IMP.test.TestCase):

    def test_rotational_alignment(self):
        """ Test the rotational alignment in 2D routine """
        random.seed()
        name=self.get_input_file_name("1z5s-projection-2.spi")
        srw = IMP.em.SpiderImageReaderWriter()
        image=IMP.em.Image()
        image.read_from_floats(name,srw)
        rotated=IMP.em.Image()
        # random rotation
        angle=random.random()*2*pi
        IMP.em.rotate(image.get_data(),angle,rotated.get_data(),True)
        fn_rotated = self.get_input_file_name("rotated.spi")
        # rotated.write_to_floats(fn_rotated,srw)
        result=IMP.em2d.align2D_rotational(
                image.get_data(),rotated.get_data(),True,0)
        fn_aligned = self.get_input_file_name("aligned.spi")
        # rotated.write_to_floats(fn_aligned,srw)
        determined_angle=result[0].get_rotation().get_angle()
        difference=get_angle_difference_from_rotation(angle,determined_angle)
        # approximately 25 degrees tolerance, 0.5 rad. Some extreme cases go
        # to that but most of them are below 0.1
        self.assertAlmostEqual(difference,0, delta=0.5,
             msg="Angles applied %f and determined %f are different, "\
                 "difference %f" % (angle,determined_angle,difference))


    def test_translational_alignment(self):
        """ Test the translational alignment in 2D routine """
        random.seed()
        name=self.get_input_file_name("1z5s-projection-2.spi")
        srw = IMP.em.SpiderImageReaderWriter()
        image=IMP.em.Image()
        image.read_from_floats(name,srw)
        translated=IMP.em.Image()
        # random translation
        trans=IMP.algebra.Vector2D(random.random()*5,random.random()*5)
        IMP.em.shift(image.get_data(),trans,translated.get_data(),True)
        fn_translated = self.get_input_file_name("translated.spi")
        # translated.write_to_floats(fn_translated,srw)
        result=IMP.em2d.align2D_translational(
                image.get_data(),translated.get_data(),True)
        fn_aligned = self.get_input_file_name("aligned.spi")
        # translated.write_to_floats(fn_aligned,srw)
        # -1 to get the translation applied to reference.
        # Result contains the translation required for align the second matrix
        determined_trans= (-1)*result[0].get_translation()
        # Tolerate 1 pixel error
        self.assertAlmostEqual(abs(determined_trans[0]-trans[0]),0, delta=1,
                msg="1st coordinate is incorrect: Applied %f Determined %f" \
                    % (trans[0], determined_trans[0]))
        self.assertAlmostEqual(abs(determined_trans[1]-trans[1]),0, delta=1,
                msg="2nd coordinate is incorrect: Applied %f Determined %f" \
                    % (trans[1], determined_trans[1]))


    def test_complete_alignment(self):
        """ Test the complete alignment in 2D routine """
        random.seed()
        name=self.get_input_file_name("1z5s-projection-2.spi")
        srw = IMP.em.SpiderImageReaderWriter()
        image=IMP.em.Image()
        image.read_from_floats(name,srw)
        transformed=IMP.em.Image()

        rot=IMP.algebra.Rotation2D(2.5)
        trans=IMP.algebra.Vector2D(5,2)
        T=IMP.algebra.Transformation2D(rot,trans)
        IMP.em.apply_Transformation2D(image.get_data(),T,
                                      transformed.get_data(),True)

        # fn_transformed = self.get_input_file_name("transformed.spi")
        # transformed.write_to_floats(fn_transformed,srw)

        result=IMP.em2d.align2D_complete(image.get_data(),
                                         transformed.get_data(),True,False)
        # fn_aligned = self.get_input_file_name("aligned_complete.spi")
        # transformed.write_to_floats(fn_aligned,srw)

        cross_correlation_coefficient = result.second

        # Tolerate 1 pixel error
        self.assertAlmostEqual(cross_correlation_coefficient,1, delta=0.03,
              msg="Error in the complete aligment routine,"
                  "ccc %f less than 0.97" % (cross_correlation_coefficient))

if __name__ == '__main__':
    IMP.test.main()
