import IMP
import IMP.test
import IMP.em2d


class Tests(IMP.test.TestCase):

    def make_image(self, values):
        img = IMP.em2d.Image(len(values), len(values[0]))
        for i, row in enumerate(values):
            for j, v in enumerate(row):
                img.set_value(i, j, v)
        return img

    def test_histogram(self):
        """Test histogram of an image, including its maximum value"""
        img = self.make_image([[0., 1., 2., 3.],
                               [4., 5., 6., 7.],
                               [8., 9., 9., 9.]])
        h = IMP.em2d.get_histogram(img, 3)
        self.assertEqual(len(h), 3)
        # Bins are [0,3), [3,6), [6,9]; the maximum value is in the last bin
        for got, expected in zip(h, [3. / 12, 3. / 12, 6. / 12]):
            self.assertAlmostEqual(got, expected, delta=1e-6)

    def test_histogram_constant(self):
        """Test histogram of an image where all values are the same"""
        img = self.make_image([[2.5, 2.5], [2.5, 2.5]])
        h = IMP.em2d.get_histogram(img, 4)
        self.assertEqual(len(h), 4)
        for got, expected in zip(h, [1., 0., 0., 0.]):
            self.assertAlmostEqual(got, expected, delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
