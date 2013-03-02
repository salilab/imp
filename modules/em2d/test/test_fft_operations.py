import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em2d
import IMP.em
import os


class Tests(IMP.test.TestCase):

    def test_get_autocorrelation2d(self):
        """ Test autocorrelation of an image using OpenCV"""
        name=self.get_input_file_name("1z5s-projection-2.spi")
        srw = IMP.em2d.SpiderImageReaderWriter()
        image=IMP.em2d.Image(name,srw)
        corr=IMP.em2d.Image()
        IMP.em2d.get_autocorrelation2d(image,corr)
        IMP.em2d.do_normalize(corr,True)
        rows=image.get_header().get_number_of_rows()
        cols=image.get_header().get_number_of_columns()
        fn_corr=self.get_input_file_name("autocorrelation_img.spi")
        stored_corr=IMP.em2d.Image(fn_corr,srw)
        for i in xrange(0,int(rows)):
            for j in xrange(0,int(cols)):
                self.assertAlmostEqual(corr(i,j),stored_corr(i,j), delta=0.001,
                         msg="Autocorrelation image is different than stored")



    def test_get_correlation2d(self):
        """ Test the correlation between two images using FFT"""
        names=[0,0]
        names[0]=self.get_input_file_name(
                    "1e6v-subject-4-set-16-1.5-apix-0.5-SNR.spi")
        names[1]=self.get_input_file_name(
                    "1e6v-subject-15-set-16-1.5-apix-0.5-SNR.spi")

        srw = IMP.em2d.SpiderImageReaderWriter()
        images=IMP.em2d.read_images(names,srw)
        corr=IMP.em2d.Image()
        IMP.em2d.get_correlation2d(images[0], images[1],corr)
        IMP.em2d.do_normalize(corr)
        name=self.get_input_file_name("correlation_img.spi")
        stored_corr=IMP.em2d.Image(name,srw)
        rows=stored_corr.get_header().get_number_of_rows()
        cols=stored_corr.get_header().get_number_of_columns()
        for i in xrange(0,int(rows)):
            for j in xrange(0,int(cols)):
                self.assertAlmostEqual(corr(i,j),stored_corr(i,j), delta=0.001,
                            msg="Correlation image is different than stored")


if __name__ == '__main__':
    IMP.test.main()
