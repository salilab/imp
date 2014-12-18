import IMP
import os
import IMP.test
import IMP.em2d
import IMP.EMageFit.solutions_io
import IMP.EMageFit.database as database


class Tests(IMP.test.TestCase):

    def make_db_with_table(self, fname, tbl):
        db = database.Database2()
        db.create(fname, overwrite=True)
        db.connect(fname)
        db.create_table(tbl, ["foo", "bar"], [int, float])
        db.close()

    def make_db_with_data(self, fname, tbl, data):
        db = database.Database2()
        db.create(fname, overwrite=True)
        db.connect(fname)
        db.create_table(tbl, ["assignment", "em2d"], [str, float])
        db.store_data(tbl, data)
        db.close()

    def test_gather_best_solution_results(self):
        """Test gather_best_solution_results()"""
        tbl = "results"
        self.make_db_with_data("test1.db", tbl,
                               [("native", 500.0), ("nonnative", 10.0),
                                ("nonnative", 20.0), ("nonnative", 30.0)])
        IMP.EMageFit.solutions_io.gather_best_solution_results(
            ["test1.db", "dummy.db"], "out.db",
            max_number=2, raisef=1.0)
        # Make sure output db contains best 2 results, plus native
        data = IMP.EMageFit.database.read_data('out.db',
                                           'SELECT assignment,em2d from results')
        self.assertEqual(len(data), 3)
        self.assertEqual(data[0][0], 'native')
        self.assertAlmostEqual(data[0][1], 500.0, delta=1e-6)
        self.assertEqual(data[1][0], 'nonnative')
        self.assertAlmostEqual(data[1][1], 10.0, delta=1e-6)
        self.assertEqual(data[2][0], 'nonnative')
        self.assertAlmostEqual(data[2][1], 20.0, delta=1e-6)
        self.assertRaises(IOError,
                        IMP.EMageFit.solutions_io.gather_best_solution_results,
                        ["test1.db", "dummy.db"], "out.db", raisef=0.0)
        os.unlink('out.db')
        os.unlink('test1.db')

    def test_cluster_record(self):
        """Test ClusterRecord class"""
        it = ["testid", 42, "testrep", 23, ["foo", "bar"]]
        c = IMP.EMageFit.solutions_io.ClusterRecord(it)
        self.assertRaises(TypeError,
                          IMP.EMageFit.solutions_io.ClusterRecord, it[:3])
        self.assertEqual([x for x in c], it)
        self.assertEqual(c.cluster_id, "testid")
        self.assertEqual(c.n_elements, 42)
        self.assertEqual(c.representative, "testrep")
        self.assertEqual(c.elements, 23)
        self.assertEqual(c.solutions_ids, ["foo", "bar"])

if __name__ == '__main__':
    IMP.test.main()
