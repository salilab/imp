import unittest
import random
from math import *
import IMP.test
import IMP.algebra as algebra
import IMP.em as em

class EMrotateTests(IMP.test.TestCase):
    def test_rotate_image(self):
        """ Test for rotating an image """
        rw  = em.SpiderImageReaderWriter()
        img = em.Image()
        original = em.Image()
        original.read('input/image.xmp',rw)
        ydim = original.get_data().get_rows()
        xdim = original.get_data().get_columns()
        img.read('input/image.xmp',rw)
        img.rotate(pi/2)
        img.rotate(pi/2)
        img.rotate(pi/2)
        img.rotate(pi/2)
#        img.write('tmp.xmp',rw)
        # After a complete rotation, the images should be almost the same
        for i in range(0,ydim):
            for j in range(0,xdim):
                self.assertInTolerance(original.get_data()[i,j],
                                       img.get_data()[i,j],1e-1)


if __name__ == '__main__':
    unittest.main()
