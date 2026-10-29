import IMP
import IMP.test
import IMP.em2d
import pickle
import random


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of MatchTemplateResult"""
        r = IMP.em2d.MatchTemplateResult((1, 2), 42.0)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertEqual(newr.pair, (1, 2))
        self.assertAlmostEqual(newr.cross_correlation, 42.0, delta=1e-4)

    def test_best_template_matches(self):
        """Test finding the best matches of a template in an image"""
        srw = IMP.em2d.SpiderImageReaderWriter()
        image = IMP.em2d.Image(
            self.get_input_file_name("1z5s-projection-0.spi"), srw)
        # Template from the image (rows 20-35, columns 26-41), plus noise
        row, col, size = 20, 26, 16
        rng = random.Random(42)
        templ = IMP.em2d.Image(size, size)
        for i in range(size):
            for j in range(size):
                templ.set_value(i, j, image(row + i, col + j)
                                + rng.gauss(0., 0.05))
        matches = IMP.em2d.get_best_template_matches(
            image.get_data(), templ.get_data(), 3)
        self.assertEqual(len(matches), 3)
        self.assertEqual(matches[0].pair, (row, col))
        self.assertGreater(matches[0].cross_correlation, 0.99)
        self.assertLessEqual(matches[0].cross_correlation, 1.)
        # Matches are sorted by decreasing correlation
        self.assertGreaterEqual(matches[0].cross_correlation,
                                matches[1].cross_correlation)
        self.assertGreaterEqual(matches[1].cross_correlation,
                                matches[2].cross_correlation)


if __name__ == '__main__':
    IMP.test.main()
