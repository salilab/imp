from __future__ import print_function, division
import IMP
import IMP.test
import IMP.atom
import IMP.em2d
import sys
import os
import numpy

class PGM(object):
    def __init__(self, width, height, scale):
        self.width, self.height, self.scale = width, height, scale
        self._data = numpy.zeros((width, height))
    def correlate(self, other_pgm):
        assert(self.width == other_pgm.width
               and self.height == other_pgm.height)
        return numpy.sum(self._data * other_pgm._data)
    def read_file(self, fh):
        """Read a PGM file. This is a very naive parser."""
        # Skip header lines
        for _ in range(3):
            fh.readline()
        x = y = 0
        for line in fh:
            for v in [int(s) for s in line.split()]:
                self._data[x,y] = v
                x += 1
                if x == self.width:
                    x = 0
                    y += 1
        assert(x == 0 and y == self.height)
    def add(self, v):
        x = int(v[0] / self.scale)
        y = int(v[1] / self.scale)
        if 0 <= x < self.width and 0 <= y < self.height:
            self._data[x,y] += 1.
    def write_file(self, fh):
        m = numpy.max(self._data)
        factor = 0. if m == 0. else 255. / m
        fh.write('P2\n%d %d\n255\n' % (self.width, self.height))
        for y in range(self.height):
            for x in range(self.width):
                fh.write("%d\n" % (self._data[x,y] * factor))

class EM2DFitRestraintTest(IMP.test.TestCase):

    def test_simple(self):
        """Simple test of EM2D single PDB score application"""
        orig_pgm = PGM(110, 110, 2.2)
        with open(self.get_input_file_name('image_1_text.pgm')) as fh:
            orig_pgm.read_file(fh)
        #! read PDB
        for image_name in ('image_1_binary.pgm', 'image_1_text.pgm'):
            m = IMP.Model()
            mp = IMP.atom.read_pdb(self.get_input_file_name('complex-2d.pdb'), m,
                               IMP.atom.NonWaterNonHydrogenPDBSelector())

            particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

            image_list = [self.get_input_file_name(image_name)]

            pca_fit_restraint = IMP.em2d.PCAFitRestraint(
                particles, image_list, 2.2, 20, 100)
            self.check_standard_object_methods(pca_fit_restraint)
            score = pca_fit_restraint.evaluate(False)
            print('initial score = ' + str(score))
            self.assertAlmostEqual(score, 0.052, delta=0.01)
            # Test that transformation puts model on the image
            t = pca_fit_restraint.get_transformation(0)
            pgm = PGM(110, 110, 2.2)
            for p in particles:
                tp = t.get_transformed(IMP.core.XYZ(p).get_coordinates())
                pgm.add(tp)
            #pgm.write_file(open('transformed_orig.pgm', 'w'))
            self.assertGreater(pgm.correlate(orig_pgm), 1400000.0)
            os.unlink('images.pgm')


if __name__ == '__main__':
    IMP.test.main()
