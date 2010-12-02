import IMP
import IMP.test
import IMP.em2d
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

    def test_polar_resampling(self):
        """Test of polar resampling of images"""
        srw = IMP.em2d.SpiderImageReaderWriter()
        fn_input=self.get_input_file_name("1gyt-subject-1-0.5-SNR.spi")
        img=IMP.em2d.Image(fn_input,srw)
        polar_params = IMP.em2d.PolarResamplingParameters()
        polar=IMP.em2d.Image()
        IMP.em2d.resample_polar(img,polar,polar_params)
        fn_saved = self.get_input_file_name("1gyt-subject-1-0.5-SNR-polar.spi")
        saved=IMP.em2d.Image(fn_saved,srw)
        rows=int(polar.get_header().get_number_of_rows())
        cols=int(polar.get_header().get_number_of_columns())
        for i in range(0,rows):
            for j in range(0,cols):
                self.assertAlmostEqual(saved(i,j),polar(i,j),delta=0.001,
                         msg="Generated polar image is different than stored"\
                         " row %d col %d" % (i,j))


    def test_read_jpg(self):
        """Test of JPGReaderWriter reading"""
        srw = IMP.em2d.SpiderImageReaderWriter()
        jrw = IMP.em2d.JPGImageReaderWriter()
        fn_jpg_img = self.get_input_file_name("lena-256x256.jpg")
        jpg_img=IMP.em2d.Image(fn_jpg_img,jrw)
        fn_spider_img = self.get_input_file_name("lena-256x256.spi")
        spider_img=IMP.em2d.Image(fn_spider_img,srw)

        rows =int( jpg_img.get_header().get_number_of_rows())
        cols = int(jpg_img.get_header().get_number_of_columns())

        self.assertEqual(spider_img.get_header().get_number_of_rows(),rows);
        self.assertEqual(spider_img.get_header().get_number_of_columns(),cols);

        for i in range(0,rows):
            for j in range(0,cols):
                # due to rounding, integer numbers in the jpg file can vary
                # to the next integer. Allow delta 1
                self.assertAlmostEqual(abs(spider_img(i,j)-jpg_img(i,j)),0,
                delta=1,msg="JPG image is not equal to spider image " \
                    "at pixel (%d,%d)" % (i,j))

    def test_write_jpg(self):
        """Test of JPGReaderWriter writing"""
        jrw = IMP.em2d.JPGImageReaderWriter()
        fn_img1 = self.get_input_file_name("lena-256x256.jpg")
        img1=IMP.em2d.Image(fn_img1,jrw)
        fn_img2 = self.get_input_file_name("temp.jpg")
        img1.write_to_floats(fn_img2,jrw)
        img2 = IMP.em2d.Image(fn_img2,jrw)

        rows = int( img1.get_header().get_number_of_rows())
        cols = int(img1.get_header().get_number_of_columns())
        for i in range(0,rows):
            for j in range(0,cols):
                # due to rounding, integer numbers in the jpg file can vary
                # to the next integer. Allow delta 1
                self.assertAlmostEqual(abs(img1(i,j)-img2(i,j)),0,
                delta=1,msg="Written JPG image is not equal to read " \
                " at pixel (%d,%d)" % (i,j))
        os.remove(fn_img2)

    def test_write_error_jpg(self):
        """Test that writing with JPGReaderWriter fails with bad extension"""
        jrw = IMP.em2d.JPGImageReaderWriter()
        fn_img1 = self.get_input_file_name("lena-256x256.jpg")
        img1=IMP.em2d.Image(fn_img1,jrw)
        try:
            fn_img2 = self.get_input_file_name("temp.xxx")
            img1.write_to_floats(fn_img2,jrw)
        except:
            # Make sure a exception is sent
            self.assertTrue(True);
            return
        self.assertTrue(False);

if __name__ == '__main__':
    IMP.test.main()
