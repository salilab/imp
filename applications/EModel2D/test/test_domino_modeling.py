import IMP
import IMP.test
import sys
import os
import random
import logging
log = logging.getLogger("test_mc_modeling")
logging.basicConfig(stream=sys.stdout)
logging.root.setLevel(logging.DEBUG)

import IMP.em2d
import IMP.em2d.utility as utility

import IMP.em2d.solutions_io as solutions_io

class TestDominoModeling(IMP.test.ApplicationTestCase):


    def test_generate_model(self):
        """ Test that the DOMINO modeling runs """

        domino_model = self.import_python_application('em2d_domino_model.py')
        IMP.set_log_level(IMP.SILENT)
        fn = self.get_input_file_name("config.py")
        directory = self.get_input_file_name("")
        os.chdir(directory)
        exp = utility.get_experiment_params(fn)
        fn_output_db = self.get_input_file_name("domino_solutions.db")
        domino_model.generate_domino_model(fn, fn_output_db)
        # assert that a database of results is created
        self.assertTrue(os.path.exists(fn_output_db))
        self.assertGreater(os.path.getsize(fn_output_db),0)
        # check that there are solutions in the database
        db = solutions_io.ResultsDB()
        db.connect(fn_output_db)
        data = db.get_solutions()
        self.assertGreater(len(data), 0)
        os.remove(fn_output_db)

if __name__ == '__main__':
    IMP.test.main()
