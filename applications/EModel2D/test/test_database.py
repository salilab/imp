import IMP
import IMP.test
import IMP.core as core
import IMP.atom as atom
import IMP.em2d as em2d
import sys
import os
from math import *
try:
    import sqlite3 as sqlite
except ImportError:
    raise ImportError("The Python module SQlite is not present")
import add_parent
add_parent.add_parent_to_python_path()

import IMP.em2d.Database as Database



class TestsDatabase(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.db = Database.Database2()
        self.fn = self.get_input_file_name("test.db")
        self.db.create(self.fn, True)
        self.db.connect(self.fn)
        self.tables = ["mytable1", "mytable2"]
        self.column_names = ["id","property","value"]
        self.column_types = [int, str, float]
        for t in self.tables:
            self.db.create_table(t, self.column_names , self.column_types)

    def test_tables(self):
        """
        Test the names of the tables
        """
        tbls = self.db.get_tables_names()
        for tab, tbl in zip(self.tables, tbls):
            self.assertEqual(tab,tbl, "Names of the tables are not equal")
        os.remove(self.fn)

    def test_columns(self):
        """
            Test the names of the columns
        """
        tbls = self.db.get_tables_names()
        for t in tbls:
            cols = self.db.get_table_column_names(t)
            types = self.db.get_table_types(t)
            for c, n, t, s in zip(cols, self.column_names,
                                    types,  self.column_types):
                self.assertEqual(c,n,
                            "Names of the columns in tables do not match")
        os.remove(self.fn)

    def test_data(self):
        """
            Test data entry and recovery
        """
        data = [ (0, "width", 1.8), (1, "length", "3.54")]
        mytable = self.tables[0]
        self.db.store_data(mytable, data)
        sql_command = """ SELECT * FROM %s """ % mytable
        recovered = self.db.retrieve_data(sql_command)
        for d, r in zip(data, recovered):
            for field, field_recovered in zip(d,r):
                self.assertEqual(field, field_recovered)
        updated_value = 80
        self.db.update_data(mytable, ["value",],[updated_value,],
                                     ['property',],['"width"',])

        sql_command = """ SELECT value FROM %s
                          WHERE property = "width" """ % mytable

        recovered = self.db.retrieve_data(sql_command)
        self.assertEqual(recovered[0][0], updated_value)
        os.remove(self.fn)


if __name__ == '__main__':
    IMP.test.main()
