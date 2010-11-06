
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em2d
import IMP.em
import os
from math import *
import random


class ProjectTests(IMP.test.TestCase):

    def test_read_and_write_opencv_images(self):
        """Test read/write for the images using OpenCV data storage"""
        srw = IMP.em2d.SpiderImageReaderWriter()
        img=IMP.em2d.Image()
        img.read_from_floats(
              self.get_input_file_name("1gyt-subject-1-0.5-SNR.spi"),srw);

        rows=int(img.get_header().get_number_of_rows())
        cols=int(img.get_header().get_number_of_columns())

        self.assertEqual(rows,128,"Error reading image")
        self.assertEqual(cols,128,"Error reading image")

        temp=self.get_input_file_name("opencv_test_image.spi")
        img.write_to_floats(temp,srw)

        img2=IMP.em2d.Image()
        img2.read_from_floats(temp,srw);

        for i in range(0,rows):
            for j in range(0,cols):
                self.assertAlmostEqual(img(i,j),img2(i,j),delta=0.001,
                         msg="Generated image is different than stored")

        os.remove(temp)


    def test_variance_filter(self):
        """Test that the variance filter is working"""
        srw = IMP.em2d.SpiderImageReaderWriter()
        img=IMP.em2d.Image()
        img.read_from_floats(
              self.get_input_file_name("1z5s-projection-2.spi"),srw);

        filtered=IMP.em2d.Image()
        kernelsize=7
        IMP.em2d.apply_variance_filter(img,filtered,kernelsize)

        saved=IMP.em2d.Image()
        saved.read_from_floats(
              self.get_input_file_name("filtered_image.spi"),srw)

        rows =int( img.get_header().get_number_of_rows())
        cols = int(img.get_header().get_number_of_columns())
        for i in range(0,rows):
            for j in range(0,cols):
                self.assertAlmostEqual(saved(i,j),filtered(i,j),delta=0.001,
                         msg="Generated image is different than stored")

    def test_substract(self):
        """Test subtracting images"""
        srw = IMP.em2d.SpiderImageReaderWriter()
        rows=int(10)
        cols=int(5)
        img1=IMP.em2d.Image(rows,cols)
        img2=IMP.em2d.Image(rows,cols)
        result=IMP.em2d.Image(rows,cols)
        for i in range(0,rows):
            for j in range(0,cols):
                img1.set_value(i,j,random.uniform(-1,1))
                img2.set_value(i,j,img1(i,j))

        IMP.em2d.subtract_images(img1,img2,result)
        for i in range(0,rows):
            for j in range(0,cols):
                self.assertAlmostEqual(abs(result(i,j)),0,delta=0.001,
                         msg="Subtract images error")

if __name__ == '__main__':
    IMP.test.main()
