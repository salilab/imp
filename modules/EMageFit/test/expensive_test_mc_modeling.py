import IMP
import IMP.test
import sys
import os
import random
import logging
log = logging.getLogger("test_mc_modeling")
logging.basicConfig()
logging.root.setLevel(logging.WARNING)
import IMP.EMageFit.utility as utility
import IMP.EMageFit.solutions_io as solutions_io


class TestMonteCarloModeling(IMP.test.ApplicationTestCase):

    def test_generate_model(self):
        """ Test that Monte Carlo modeling runs """
        try:
            import networkx
            import subprocess
        except ImportError as detail:
            self.skipTest(str(detail))
        domino_model = self.import_python_application('emagefit')
        fn_config = self.get_input_file_name("config.py")
        exp = utility.get_experiment_params(fn_config)
        fn_database = "monte_carlo_output_database.db"
        domino_model.generate_monte_carlo_model(exp, fn_database, seed=-1,
                                                write_solution=True)
        # test that the database and pdb files are generated and that they
        # are not empty
        self.assertTrue(os.path.exists(fn_database))
        self.assertGreater(os.path.getsize(fn_database), 0)
        fn_pdb = fn_database + ".pdb"
        self.assertTrue(os.path.exists(fn_pdb))
        self.assertGreater(os.path.getsize(fn_pdb), 0)
        # check that there is one solution in the database
        db = solutions_io.ResultsDB()
        db.connect(fn_database)
        data = db.get_solutions()
        self.assertEqual(len(data), 1)
        os.remove(fn_database)
        os.remove(fn_pdb)

if __name__ == '__main__':
    logging.basicConfig(stream=sys.stdout)
    logging.root.setLevel(logging.DEBUG)
    IMP.test.main()
