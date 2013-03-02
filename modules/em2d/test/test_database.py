import IMP
import IMP.test
import IMP.core as core
import IMP.atom as atom
import IMP.em2d as em2d
import sys
import os
from math import *
import IMP.em2d.Database as Database



class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.db = Database.Database2()
        self.fn = "test.db"
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

        self.db.drop_table(self.tables[0])
        tbls = self.db.get_tables_names()
        self.assertTrue(not self.tables[0] in tbls)

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

    def test_data(self):
        """
            Test data entry and recovery
        """
        mytable = self.tables[0]
        # store empty data (it should be ignored)
        self.db.store_data(mytable, [])

        data = [ (0, "width", 1.8), (1, "length", "3.54")]
        self.db.store_data(mytable, data)
        sql_command = """ SELECT * FROM %s """ % mytable
        recovered = self.db.retrieve_data(sql_command)
        whole_table = self.db.get_table(mytable)
        for d, r, wt in zip(data, recovered, whole_table):
            for field, field_recovered, field_wt in zip(d,r, wt):
                self.assertEqual(field, field_recovered)
                self.assertEqual(field, field_wt)

        updated_value = 80
        self.db.update_data(mytable, ["value",],[updated_value,],
                                     ['property',],['"width"',])

        sql_command = """ SELECT value FROM %s
                          WHERE property = "width" """ % mytable

        recovered = self.db.retrieve_data(sql_command)
        self.assertEqual(recovered[0][0], updated_value)

    def test_add_column(self):
        """ Test of adding a column """
        my_table = self.tables[0]
        name = "newcol"
        cols = self.db.get_table_column_names(my_table)
        self.assertTrue(not name in cols)

        self.db.add_column(my_table, name, float)
        cols = self.db.get_table_column_names(my_table)
        self.assertTrue(name in cols)

        self.db.drop_columns(my_table, [name])
        cols = self.db.get_table_column_names(my_table)
        self.assertTrue(not name in cols)


    def test_merging(self):
        """ of merging databases """

        db = Database.Database2()
        my_table = "mytable"
        column_names = ["id","property","value"]
        column_types = [int, str, float]
        fns = ["file1.db", "file2.pdb"]
        for fn in fns:
            db.create(fn, True)
            db.connect(fn)
            db.create_table(my_table, column_names , column_types)
            data = [ (0, "width", 5.6), (1, "length", "34")]
            db.store_data( my_table, data)
            db.close()
        fn_output = "file3.db"
        Database.merge_databases(fns, fn_output, my_table)
        db.connect(fn_output)
        data = db.get_table(my_table)
        self.assertEqual(len(data),4)
        db.close()
        for fn in fns:
            os.remove(fn)
        os.remove(fn_output)

    def tearDown(self):
        self.db.close()
        os.remove(self.fn)

if __name__ == '__main__':
    IMP.test.main()
