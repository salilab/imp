import IMP
import IMP.test
import IMP.em2d as em2d
import os
import random

class Tests(IMP.test.TestCase):

    def test_read_and_write_opencv_images(self):
        """Test read/write for the images using OpenCV data storage"""
        srw = em2d.SpiderImageReaderWriter()
        img=em2d.Image()
        img.read(
              self.get_input_file_name("1gyt-subject-1-0.5-SNR.spi"),srw)

        rows=int(img.get_header().get_number_of_rows())
        cols=int(img.get_header().get_number_of_columns())

        self.assertEqual(rows,128,"Error reading image")
        self.assertEqual(cols,128,"Error reading image")

        temp=self.get_input_file_name("opencv_test_image.spi")
        img.write(temp,srw)

        img2=em2d.Image()
        img2.read(temp,srw)

        for i in range(0,rows):
            for j in range(0,cols):
                self.assertAlmostEqual(img(i,j),img2(i,j),delta=0.001,
                         msg="Generated image is different from stored")

        os.remove(temp)


    def test_variance_filter(self):
        """Test that the variance filter is working"""
        srw = em2d.SpiderImageReaderWriter()
        img=em2d.Image()
        img.read(
              self.get_input_file_name("1z5s-projection-2.spi"),srw)

        filtered=em2d.Image()
        kernelsize=7
        em2d.apply_variance_filter(img,filtered,kernelsize)

        saved=em2d.Image()
        saved.read(
              self.get_input_file_name("filtered_image.spi"),srw)

        rows =int( img.get_header().get_number_of_rows())
        cols = int(img.get_header().get_number_of_columns())
        for i in range(0,rows):
            for j in range(0,cols):
                self.assertAlmostEqual(saved(i,j),filtered(i,j),delta=0.001,
                         msg="Generated image is different from stored")

    def test_substract(self):
        """Test subtracting images"""
        srw = em2d.SpiderImageReaderWriter()
        rows=int(10)
        cols=int(5)
        img1=em2d.Image(rows,cols)
        img2=em2d.Image(rows,cols)
        result=em2d.Image(rows,cols)
        for i in range(0,rows):
            for j in range(0,cols):
                img1.set_value(i,j,random.uniform(-1,1))
                img2.set_value(i,j,img1(i,j))

        em2d.do_subtract_images(img1,img2,result)
        for i in range(0,rows):
            for j in range(0,cols):
                self.assertAlmostEqual(abs(result(i,j)),0,delta=0.001,
                         msg="Subtract images error")

    def test_polar_resampling(self):
        """Test of polar resampling of images"""
        srw = em2d.SpiderImageReaderWriter()
        fn_input=self.get_input_file_name("1gyt-subject-1-0.5-SNR.spi")
        img=em2d.Image(fn_input,srw)
        polar_params = em2d.PolarResamplingParameters()
        polar=em2d.Image()
        em2d.do_resample_polar(img,polar,polar_params)
        fn_saved = self.get_input_file_name("1gyt-subject-1-0.5-SNR-polar.spi")
        saved=em2d.Image(fn_saved,srw)
        rows=int(polar.get_header().get_number_of_rows())
        cols=int(polar.get_header().get_number_of_columns())
        for i in range(0,rows):
            for j in range(0,cols):
                self.assertAlmostEqual(saved(i,j),polar(i,j),delta=0.001,
                         msg="Generated polar image is different from stored"\
                         " row %d col %d" % (i,j))


    def test_read_jpg(self):
        """Test of JPGReaderWriter reading"""
        srw = em2d.SpiderImageReaderWriter()
        jrw = em2d.JPGImageReaderWriter()
        fn_jpg_img = self.get_input_file_name("lena-256x256.jpg")
        jpg_img=em2d.Image(fn_jpg_img,jrw)
        fn_spider_img = self.get_input_file_name("lena-256x256.spi")
        spider_img=em2d.Image(fn_spider_img,srw)

        rows =int( jpg_img.get_header().get_number_of_rows())
        cols = int(jpg_img.get_header().get_number_of_columns())

        self.assertEqual(spider_img.get_header().get_number_of_rows(),rows)
        self.assertEqual(spider_img.get_header().get_number_of_columns(),cols)

        for i in range(0,rows):
            for j in range(0,cols):
                # due to rounding, integer numbers in the jpg file can vary
                # to the next integer. Allow delta 1
                self.assertAlmostEqual(abs(spider_img(i,j)-jpg_img(i,j)),0,
                delta=1,msg="JPG image is not equal to spider image " \
                    "at pixel (%d,%d)" % (i,j))

    def test_write_jpg(self):
        """Test of JPGReaderWriter writing"""
        jrw = em2d.JPGImageReaderWriter()
        fn_img1 = self.get_input_file_name("lena-256x256.jpg")
        img1=em2d.Image(fn_img1,jrw)
        fn_img2 = self.get_input_file_name("temp.jpg")
        img1.write(fn_img2,jrw)
        img2 = em2d.Image(fn_img2,jrw)
        # Use the ccc for testing instead of the pixel values. The matrix
        # in img2 is transformed from floats to ints son it can be written.
        # Values can change, but the ccc has to be very close to 1.
        ccc= em2d.get_cross_correlation_coefficient(img1.get_data(),
                                           img2.get_data())
        self.assertAlmostEqual(ccc,1,delta=0.05,
        msg="Written JPG image is not equal to read ")
        os.remove(fn_img2)

    def test_write_error_jpg(self):
        """Test that writing with JPGReaderWriter fails with bad extension"""
        jrw = em2d.JPGImageReaderWriter()
        fn_img1 = self.get_input_file_name("lena-256x256.jpg")
        img1=em2d.Image(fn_img1,jrw)
        fn_img2 = self.get_input_file_name("temp.xxx")
        self.assertRaises(IOError, img1.write, fn_img2, jrw)

    def test_read_tiff(self):
        """Test of TIFFReaderWriter reading"""
        srw = em2d.SpiderImageReaderWriter()
        trw = em2d.TIFFImageReaderWriter()
        fn_tif_img = self.get_input_file_name("lena-256x256.tif")
        tif_img=em2d.Image(fn_tif_img,trw)
        fn_spider_img = self.get_input_file_name("lena-256x256.spi")
        spider_img=em2d.Image(fn_spider_img,srw)
        rows =int( tif_img.get_header().get_number_of_rows())
        cols = int(tif_img.get_header().get_number_of_columns())

        self.assertEqual(spider_img.get_header().get_number_of_rows(),rows)
        self.assertEqual(spider_img.get_header().get_number_of_columns(),cols)
        ccc=em2d.get_cross_correlation_coefficient(tif_img.get_data(),
                                                  spider_img.get_data())
        self.assertAlmostEqual(ccc,1,delta=0.01,msg="ccc ins not 1")

    def test_write_tiff(self):
        """Test of TIFFReaderWriter writing"""
        trw = em2d.TIFFImageReaderWriter()
        srw = em2d.SpiderImageReaderWriter()
        fn_img1 = self.get_input_file_name("lena-256x256.tif")
        img1=em2d.Image(fn_img1,trw)
        fn_img2 = self.get_input_file_name("temp.tif")
        img1.write(fn_img2,trw)
        img2 = em2d.Image(fn_img2,trw)
        # Use the ccc for testing instead of the pixel values. The matrix
        # in img2 is transformed from floats to ints son it can be written.
        # Values can change, but the ccc has to be very close to 1.
        ccc= em2d.get_cross_correlation_coefficient(img1.get_data(),
                                           img2.get_data())
        print ccc
        self.assertAlmostEqual(ccc,1,delta=0.01,
        msg="Written TIFF image is not equal to read ")
        os.remove(fn_img2)

    def test_write_error_tiff(self):
        """Test that writing with TIFFReaderWriter fails with bad extension"""
        trw = em2d.TIFFImageReaderWriter()
        fn_img1 = self.get_input_file_name("lena-256x256.tif")
        img1=em2d.Image(fn_img1,trw)
        fn_img2 = self.get_input_file_name("temp.xxx")
        self.assertRaises(IOError, img1.write, fn_img2, trw)

    def test_do_extend_borders(self):
        """Test that extending the borders of an image is done correctly"""
        srw = em2d.SpiderImageReaderWriter()
        fn_img1 = self.get_input_file_name("lena-256x256.spi")
        img1=em2d.Image(fn_img1,srw)
        img2=em2d.Image()
        border = 10
        em2d.do_extend_borders(img1,img2,border)
#        fn_img2 = self.get_input_file_name("lena-256x256-extended.spi")
#        img2.write(fn_img2,srw)
        rows2=int(img2.get_header().get_number_of_rows())
        cols2=int(img2.get_header().get_number_of_columns())
        self.assertEqual(rows2,256+2*border,
                        "Border rows are not extended properly")
        self.assertEqual(cols2,256+2*border,
                         "Border columns are not extended properly")
        for i in range(0,rows2):
            for j in range(0,cols2):
                if(i < border or i > (rows2-border)):
                    self.assertAlmostEqual(img2(i,j),0,delta=0.001,
                         msg="Borders are not zero at row %d col %d" % (i,j))
                if(j< border or i >  (cols2-border)):
                    self.assertAlmostEqual(img2(i,j),0,delta=0.001,
                         msg="Borders are not zero at row %d col %d" % (i,j))

    def test_get_overlap_percentage(self):
        """ Test that the function returns > 0 when there is overlap, and 0
          otherwise """
        # TODO
        pass

    def test_crop(self):
        img=em2d.Image()
        img.set_size(20,20)
        em2d.crop(img,[10,10], 10);
        self.assertEqual(img.get_header().get_number_of_rows(), 10,
                "Crop size is incorrect")
        #em2d.crop(img,[10,10], 20);

    def test_fill_outside_circle(self):
        img = em2d.Image()
        size = 20
        radius = 6
        center = size/2
        img.set_size(size, size)
        img.set_zeros()
        n = 0
        mean = 0
        border = 4
        for i in range(border, size-border+1):
            for j in range(border, size-border+1):
                if ((i-center)**2 + (j-center)**2)**0.5 <= radius:
                    val = i * j
                    img.set_value(i,j, val)
                    n += 1
                    mean += val
        mean /= n
        em2d.apply_mean_outside_mask(img, radius)
        pix = range(0,size)
        for i in pix:
            for j in pix:
                if ((i-center)**2 + (j-center)**2)**0.5 <= radius:
                    val = i * j
                    self.assertEqual(img(i,j), val)
                else:
                    self.assertAlmostEqual(img(i,j), mean, delta=0.01)


if __name__ == '__main__' :
    IMP.test.main()
