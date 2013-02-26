import IMP
import IMP.test
import sys
import os
import random
import logging
log = logging.getLogger("test_mc_modeling")
logging.basicConfig()
logging.root.setLevel(logging.DEBUG)

import IMP.em2d
import IMP.em2d.utility as utility

import IMP.em2d.solutions_io as solutions_io

class TestDominoModeling(IMP.test.ApplicationTestCase):


    def test_generate_model(self):
        """ Test that the DOMINO modeling runs """
        try:
            import networkx, subprocess
        except ImportError, detail:
            self.skipTest(str(detail))
        domino_model = self.import_python_application('emagefit.py')
        IMP.base.set_log_level(IMP.base.SILENT)
        fn = self.get_input_file_name("config.py")
        exp = utility.get_experiment_params(fn)
        fn_output_db = self.get_input_file_name("domino_solutions_temp.db")
        domino_model.generate_domino_model(exp, fn_output_db)
        # assert that a database of results is created
        self.assertTrue(os.path.exists(fn_output_db))
        self.assertGreater(os.path.getsize(fn_output_db),0)
        # check that there are solutions in the database
        db = solutions_io.ResultsDB()
        db.connect(fn_output_db)
        data = db.get_solutions()
        self.assertGreater(len(data), 0)
        columns = db.get_table_column_names("results")
        self.assertTrue("em2d" in columns)
        os.remove(fn_output_db)

if __name__ == '__main__':
    IMP.test.main()
