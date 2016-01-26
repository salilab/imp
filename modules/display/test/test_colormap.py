from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.display
import io
import re


class Tests(IMP.test.TestCase):

    def assertColorEqual(self, c, red, green, blue, delta=1e-6):
        self.assertAlmostEqual(c.get_red(), red, delta=delta)
        self.assertAlmostEqual(c.get_green(), green, delta=delta)
        self.assertAlmostEqual(c.get_blue(), blue, delta=delta)

    def test_color(self):
        """Test Color class"""
        c = IMP.display.Color()
        self.assertColorEqual(c, -1.0, -1.0, -1.0)
        c = IMP.display.Color(0.1, 0.2, 0.3)
        self.assertColorEqual(c, 0.1, 0.2, 0.3)
        c.show()
        for bad in range(3):
            rgb = [0.5, 0.5, 0.5]
            rgb[bad] = -1.0
            self.assertRaisesUsageException(IMP.display.Color, *rgb)
            rgb[bad] = 2.0
            self.assertRaisesUsageException(IMP.display.Color, *rgb)

    def test_get_interpolated_rgb(self):
        """Test get_interpolated_rgb()"""
        a = IMP.display.Color(0.1, 0.2, 0.3)
        b = IMP.display.Color(0.4, 0.9, 0.8)

        # c == a when f=0
        c = IMP.display.get_interpolated_rgb(a, b, 0.)
        self.assertColorEqual(c, 0.1, 0.2, 0.3)

        # c == b when f=1
        c = IMP.display.get_interpolated_rgb(a, b, 1.)
        self.assertColorEqual(c, 0.4, 0.9, 0.8)

        c = IMP.display.get_interpolated_rgb(a, b, 0.4)
        self.assertColorEqual(c, 0.22, 0.48, 0.5)

    def test_get_linear_color_map_value(self):
        """Test get_linear_color_map_value()"""
        self.assertAlmostEqual(IMP.display.get_linear_color_map_value(
                                                 10, 40, 30), 0.66, delta=0.1)
        self.assertAlmostEqual(IMP.display.get_linear_color_map_value(
                                                 10, 40, 50), 1.0, delta=0.1)
        self.assertAlmostEqual(IMP.display.get_linear_color_map_value(
                                                 10, 40, -50), 0.0, delta=0.1)
        self.assertRaisesUsageException(
                       IMP.display.get_linear_color_map_value, 100, 50, 70)

    def test_get_display_color(self):
        """Test get_display_color()"""
        self.assertColorEqual(IMP.display.get_display_color(0),
                              166./255., 206./255., 227./255.)
        self.assertColorEqual(IMP.display.get_display_color(105),
                               253./255., 191./255., 111./255.)

    def test_get_jet_color(self):
        """Test the jet color map"""
        self.assertColorEqual(IMP.display.get_jet_color(0.), 0., 0., 1.)
        self.assertColorEqual(IMP.display.get_jet_color(1.), 0., 0., 1.)
        self.assertColorEqual(IMP.display.get_jet_color(0.5), 1., 0.5, 0.)
        # Some rounding error over 1.0 should be OK
        self.assertColorEqual(IMP.display.get_jet_color(1.0001), 0., 0., 1.)
        # Check out of range condition
        self.assertRaisesUsageException(IMP.display.get_jet_color, -1.0)
        self.assertRaisesUsageException(IMP.display.get_jet_color, 1.1)

    def test_get_rgb_color(self):
        """Test the rgb color map"""
        self.assertColorEqual(IMP.display.get_rgb_color(0.), 0., 0., 1.)
        self.assertColorEqual(IMP.display.get_rgb_color(1.), 1., 0., 0.)
        self.assertColorEqual(IMP.display.get_rgb_color(0.5), 0., 1., 0.)

    def test_get_hot_color(self):
        """Test the hot color map"""
        self.assertColorEqual(IMP.display.get_hot_color(0.), 0., 0., 0.)
        self.assertColorEqual(IMP.display.get_hot_color(1.), 1., 1., 1.)
        self.assertColorEqual(IMP.display.get_hot_color(0.5), 1., 0.5, 0.)

    def test_get_gray_color(self):
        """Test the gray color map"""
        self.assertColorEqual(IMP.display.get_gray_color(0.), 0., 0., 0.)
        self.assertColorEqual(IMP.display.get_gray_color(1.), 1., 1., 1.)
        self.assertColorEqual(IMP.display.get_gray_color(0.5), 0.5, 0.5, 0.5)

    def test_get_gnuplot_color(self):
        """Test the gnuplot color map"""
        self.assertColorEqual(IMP.display.get_gnuplot_color(0.), 0., 0., 0.)
        self.assertColorEqual(IMP.display.get_gnuplot_color(1.), 1., 1., 0.)
        self.assertColorEqual(IMP.display.get_gnuplot_color(0.5),
                              0.675, 0.125, 0.3)

if __name__ == '__main__':
    IMP.test.main()
