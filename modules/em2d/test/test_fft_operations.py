import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em2d
import IMP.em
import os


class FFToperationsTests(IMP.test.TestCase):
    def test_autocorrelation2D(self):
        """ Test autocorrelation of an image using FFT"""
        name=self.get_input_file_name("1z5s-projection-2.spi")
        srw = IMP.em.SpiderImageReaderWriter()
        image=IMP.em.Image()
        image.read_from_floats(name,srw)
        corr=IMP.em.Image()
        IMP.em2d.autocorrelation2D(image.get_data(),corr.get_data() )
        IMP.em.normalize(corr)
        name=self.get_input_file_name("autocorrelation_img.spi")
        stored_corr=IMP.em.Image()
        stored_corr.read_from_floats(name,srw)
        rows=stored_corr.get_data().get_number_of_rows()
        cols=stored_corr.get_data().get_number_of_columns()
        for i in xrange(0,rows):
            for j in xrange(0,cols):
                self.assertAlmostEqual(corr(i,j),stored_corr(i,j), delta=0.001,
                         msg="Autocorrelation image is different than stored")


    def test_correlation2D(self):
        """ Test the correlation between two images using FFT"""
        names=[0,0]
        names[0]=self.get_input_file_name(
                    "1e6v-subject-4-set-16-1.5-apix-0.5-SNR.spi")
        names[1]=self.get_input_file_name(
                    "1e6v-subject-15-set-16-1.5-apix-0.5-SNR.spi")

        srw = IMP.em.SpiderImageReaderWriter()
        images=IMP.em.read_images(names,srw)
        corr=IMP.em.Image()
        IMP.em2d.correlation2D(images[0].get_data(), images[1].get_data(),
                                               corr.get_data() )
        IMP.em.normalize(corr)
        name=self.get_input_file_name("correlation_img.spi")
        stored_corr=IMP.em.Image()
        stored_corr.read_from_floats(name,srw)
        rows=stored_corr.get_data().get_number_of_rows()
        cols=stored_corr.get_data().get_number_of_columns()
        for i in xrange(0,rows):
            for j in xrange(0,cols):
                self.assertAlmostEqual(corr(i,j),stored_corr(i,j), delta=0.001,
                            msg="Correlation image is different than stored")


if __name__ == '__main__':
    IMP.test.main()
