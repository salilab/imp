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
            self._check_restraint_info(pca_fit_restraint)
            c = pca_fit_restraint.get_cross_correlation_coefficient(0)
            self.assertGreater(c, 0.9)
            # Test that transformation puts model on the image
            t = pca_fit_restraint.get_transformation(0)
            pgm = PGM(110, 110, 2.2)
            for p in particles:
                tp = t.get_transformed(IMP.core.XYZ(p).get_coordinates())
                pgm.add(tp)
            #pgm.write_file(open('transformed_orig.pgm', 'w'))
            self.assertGreater(pgm.correlate(orig_pgm), 1400000.0)
            os.unlink('images.pgm')

    def _check_restraint_info(self, r):
        self._check_static_restraint_info(r)
        self._check_dynamic_restraint_info(r)

    def _check_static_restraint_info(self, r):
        info = r.get_static_info()

        self.assertEqual(info.get_number_of_int(), 1)
        self.assertEqual(info.get_int_key(0), "projection number")
        self.assertEqual(info.get_int_value(0), 100)

        self.assertEqual(info.get_number_of_float(), 2)
        self.assertEqual(info.get_float_key(0), "pixel size")
        self.assertAlmostEqual(info.get_float_value(0), 2.2, delta=1e-6)
        self.assertEqual(info.get_float_key(1), "resolution")
        self.assertAlmostEqual(info.get_float_value(1), 20., delta=1e-6)

        self.assertEqual(info.get_number_of_string(), 1)
        self.assertEqual(info.get_string_key(0), "type")
        self.assertEqual(info.get_string_value(0), "IMP.em2d.PCAFitRestraint")

        self.assertEqual(info.get_number_of_filenames(), 1)
        self.assertEqual(info.get_filenames_key(0), "image files")
        self.assertEqual(len(info.get_filenames_value(0)), 1)

    def _check_dynamic_restraint_info(self, r):
        info = r.get_dynamic_info()

        self.assertEqual(info.get_number_of_floats(), 3)
        self.assertEqual(info.get_floats_key(0), "cross correlation")
        v = info.get_floats_value(0)
        self.assertEqual(len(v), 1)
        self.assertAlmostEqual(v[0], r.get_cross_correlation_coefficient(0),
                               delta=1e-6)

        self.assertEqual(info.get_floats_key(1), "rotation")
        self.assertEqual(len(info.get_floats_value(1)), 4)
        self.assertEqual(info.get_floats_key(2), "translation")
        self.assertEqual(len(info.get_floats_value(2)), 3)


if __name__ == '__main__':
    IMP.test.main()
