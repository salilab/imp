
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em2d
import IMP.em
import os
from math import *



class ProjectTests(IMP.test.TestCase):

    def test_read_and_write_opencv_images(self):
        """Test read/write for the images using OpenCV data storage"""
        srw = IMP.em2d.SpiderImageReaderWriter()
        img=IMP.em2d.Image()
        img.read_from_floats(
              self.get_input_file_name("1gyt-subject-1-0.5-SNR.spi"),srw);

#        rows=img.get_data().rows
#        cols=img.get_data().cols

#        self.assertEqual(rows,128,"Error reading image")
#        self.assertEqual(cols,128,"Error reading image")

        temp=self.get_input_file_name("opencv_test_image.spi")
        img.write_to_floats(temp,srw)

        img2=IMP.em2d.Image()
        img2.read_from_floats(temp,srw);

#        for i in xrange(0,rows):
#            for j in xrange(0,cols):
#                self.assertAlmostEqual(img.get_data()[i][j],
#                                       img2.get_data()[i][j],
#                                       delta=0.001,
#                         msg="Generated image is different than stored")


        os.remove(temp)

if __name__ == '__main__':
    IMP.test.main()
