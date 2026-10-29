import math
import IMP
import IMP.test
import IMP.em2d


def make_image(rows, cols, transpose=False):
    """Smooth non-square test image (or its transpose)"""
    if transpose:
        img = IMP.em2d.Image(cols, rows)
    else:
        img = IMP.em2d.Image(rows, cols)
    for i in range(rows):
        for j in range(cols):
            d1 = (i - 0.3 * rows) ** 2 + (j - 0.7 * cols) ** 2
            d2 = (i - 0.6 * rows) ** 2 + (j - 0.3 * cols) ** 2
            v = (math.exp(-d1 / 30.) - 0.5 * math.exp(-d2 / 50.)
                 + 0.1 * math.sin(i * 0.7 + j * 0.3))
            if transpose:
                img.set_value(j, i, v)
            else:
                img.set_value(i, j, v)
    return img


class Tests(IMP.test.TestCase):

    def get_size(self, img):
        h = img.get_header()
        return (int(h.get_number_of_rows()), int(h.get_number_of_columns()))

    def test_fill_holes(self):
        """Test filling holes in non-square images"""
        for rows, cols in ((20, 60), (60, 20)):
            img = make_image(rows, cols)
            result = IMP.em2d.Image()
            IMP.em2d.do_fill_holes(img, result, 1.0)
            self.assertEqual(self.get_size(result), (rows, cols))
            # Filling holes never lowers values
            for i in range(rows):
                for j in range(cols):
                    self.assertGreaterEqual(result(i, j), img(i, j) - 1e-9)

    def test_diffusion_filter(self):
        """Test diffusion filter on non-square images"""
        rows, cols = 20, 60
        img, img_t = make_image(rows, cols), make_image(rows, cols, True)
        result, result_t = IMP.em2d.Image(), IMP.em2d.Image()
        IMP.em2d.apply_diffusion_filter(img, result, 45, 1.0, 3)
        IMP.em2d.apply_diffusion_filter(img_t, result_t, 45, 1.0, 3)
        self.assertEqual(self.get_size(result), (rows, cols))
        # The filter treats rows and columns the same way
        for i in range(rows):
            for j in range(cols):
                self.assertAlmostEqual(result(i, j), result_t(j, i),
                                       delta=1e-9)


if __name__ == '__main__':
    IMP.test.main()
