import IMP.test
import IMP
import IMP.statistics
import IMP.algebra
import random


class Tests(IMP.test.TestCase):

    def test_add_configuration(self):
        "Test add configuration"
        nitems = random.randrange(1, 10)
        data_exp = [random.uniform(0.0, 1.0) for i in range(nitems)]
        data_ref = [random.uniform(0.0, 1.0) for i in range(nitems)]
        sigma_ref = [random.uniform(0.0, 0.2) for i in range(nitems)]
        w_ref = random.uniform(0.0, 1.0)
        chi2 = IMP.statistics.ChiSquareMetric(data_exp)
        chi2.add_configuration(data_ref, sigma_ref, w_ref)
        self.assertAlmostEqual(w_ref, chi2.get_weight(0), delta=0.00001)
        self.assertEqual(1, chi2.get_number_of_items())
        data = chi2.get_data(0)
        sigma = chi2.get_standard_deviation(0)
        for i in range(nitems):
            self.assertAlmostEqual(data[i], data_ref[i], delta=0.00001)
            self.assertAlmostEqual(sigma[i], sigma_ref[i], delta=0.00001)

    def test_get_distance(self):
        "Test get distance"
        for i in range(50):
            nitems = random.randrange(1, 10)
            data_exp = [random.uniform(0.0, 1.0) for i in range(nitems)]
            chi2 = IMP.statistics.ChiSquareMetric(data_exp)
            data0 = [random.uniform(0.0, 1.0) for i in range(nitems)]
            sigma0 = [random.uniform(0.0, 0.2) for i in range(nitems)]
            chi2.add_configuration(data0, sigma0, 1.0)
            data1 = [random.uniform(0.0, 1.0) for i in range(nitems)]
            sigma1 = [random.uniform(0.0, 0.2) for i in range(nitems)]
            chi2.add_configuration(data1, sigma1, 1.0)
            dist_ref = 0.
            for i in range(nitems):
                dist_ref += (data0[i] - data1[i]) * (data0[i] - data1[
                    i]) / (sigma0[i] * sigma0[i] + sigma1[i] * sigma1[i])
            dist_ref /= float(nitems)
            dist = chi2.get_distance(0, 1)
            self.assertAlmostEqual(dist_ref, dist, delta=0.00001)

    def test_get_exp_chi2(self):
        "Test get chi2 from exp"
        for i in range(50):
            nitems = random.randrange(1, 10)
            data_exp = [random.uniform(0.0, 1.0) for i in range(nitems)]
            chi2 = IMP.statistics.ChiSquareMetric(data_exp)
            data = [random.uniform(0.0, 1.0) for i in range(nitems)]
            sigma = [random.uniform(0.0, 0.2) for i in range(nitems)]
            chi2.add_configuration(data, sigma, 1.0)
            dist_ref = 0.
            for i in range(nitems):
                dist_ref += (data[i] - data_exp[i]) * \
                    (data[i] - data_exp[i]) / sigma[i] / sigma[i]
            dist_ref /= float(nitems)
            dist = chi2.get_chi_square_experimental(0)
            self.assertAlmostEqual(dist_ref, dist, delta=0.00001)

if __name__ == '__main__':
    IMP.test.main()
