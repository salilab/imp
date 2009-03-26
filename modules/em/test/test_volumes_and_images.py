import unittest
import IMP.test
import IMP.algebra as alg
import EM
import IMP.em as em
import math
import random

class VolumeTest(IMP.test.TestCase):

    def test_image(self):
        """Check image reading and writing"""
        img=em.imageEM()
        rw=em.spiderImageReaderWriter("input/flipY-nup84-0.spi",
                                                  False,False,True)
        img.read("input/flipY-nup84-0.spi",rw)
        img.write("test_image.spi",rw)
        img2=em.imageEM()
        img2.read("test_image.spi",rw)
        for j in xrange(0,img.get_data().get_rows()):
            for i in xrange(0,img.get_data().get_columns()):
                self.assertEqual(img.get_data()[i,j],img2.get_data()[i,j])

    def test_em_maps(self):
        """Check volume reading and writing"""
        # Read in Xmipp format
        rw1=em.SpiderMapReaderWriter("input/media_mon_iter3.xmp",
                                                    False,False,True)
        rw2=EM.MRCReaderWriter()
        m=EM.DensityMap()
        m.Read("input/media_mon_iter3.xmp",rw1)
        m.Write("test.mrc",rw2)
        m.Write("test.xmp",rw1)
        m2=EM.DensityMap()
        m2.Read("test.xmp",rw1)
        # Check that the two maps have the same values
        for k in xrange(0,m.get_header().nz):
            for j in xrange(0,m.get_header().ny):
                for i in xrange(0,m.get_header().nx):
                    self.assertEqual(m.get_value(m.xyz_ind2voxel(i,j,k)),
                                    m2.get_value(m.xyz_ind2voxel(i,j,k)))

if __name__ == '__main__':
    unittest.main()
