"""@namespace IMP.em2d.Database
   Utility functions to manage SQL databases with sqlite3.
"""

import sqlite3 as sqlite
import os
import csv
import logging

log = logging.getLogger("Database")


class Database2:
    """ Class to manage a SQL database built with sqlite3 """

    def __init__(self):
        # Connection to the database
        self.connection = None
        # Cursor of actions
        self.cursor = None
        # Dictionary of tablenames and types (used to convert values when storing data)

    def create(self,filename,overwrite=False):
        """ Creates a database by simply connecting to the file """
        log.info("Creating database")
        if overwrite and os.path.exists(filename):
            os.remove(filename)
        sqlite.connect(filename)

    def connect(self,filename):
        """ Connects to the database in filename """
        if not os.path.isfile(filename):
            raise IOError,"Database file not found: %s" % filename
        self.connection = sqlite.connect(filename)
        self.cursor = self.connection.cursor()

    def check_if_is_connected(self):
        """ Checks if the class is connected to the database filename """
        if self.connection == None:
            raise ValueError,"The database has not been created " \
            "or connection not established "

    def create_table(self, table_name, column_names, column_types):
        """ Creates a table. It expects a sorted dictionary
            of (data_field,typename) entries """
        log.info("Creating table %s",table_name)
        self.check_if_is_connected()
        sql_command = "CREATE TABLE %s (" % (table_name)
        for name, data_type in zip(column_names, column_types):
            sql_typename = get_sql_type_name(data_type)
            sql_command += "%s %s," % (name, sql_typename)
        # replace last comma for a parenthesis
        n = len(sql_command)
        sql_command = sql_command[0:n-1] + ")"
        log.debug(sql_command)
        self.cursor.execute(sql_command)
        self.connection.commit()

    def drop_table(self, table_name):
        """
            Delete a table if it exists
        """
        log.info("Deleting table %s",table_name)
        self.check_if_is_connected()
        sql_command = "DROP TABLE IF EXISTS %s" % (table_name)
        log.debug(sql_command)
        self.cursor.execute(sql_command)
        self.connection.commit()

    def store_dataV1(self,table_name,data):
        """ Inserts information in a given table of the database.
        The info must be a list of tuples containing as many values
        as columns in the table
            Conversion of values is done AUTOMATICALLY after checking the
            types stored in the table
        """
        self.check_if_is_connected()
        n = len(data[0]) # number of columns for each row inserted
        tuple_format="("+"?,"*(n-1)+"?)"
        sql_command="INSERT INTO %s VALUES %s " % (table_name, tuple_format)
        # Fill the table with the info in the tuples
        types = self.get_table_types(table_name)
#        log.debug("Storing types: %s", types)
        for x in data:
#            log.debug("DATA %s", x)
            # convert (applies the types stored in the table dictionary to each value in x
            y = [apply_type(i) for i,apply_type in zip(x, types)]
            self.cursor.execute(sql_command, y)
        self.connection.commit()

    def store_data(self,table_name,data):
        """ Inserts information in a given table of the database.
        The info must be a list of tuples containing as many values
        as columns in the table
            Conversion of values is done AUTOMATICALLY after checking the
            types stored in the table
        """
        if len(data) == 0:
            log.warning("Inserting empty data")
            return
        self.check_if_is_connected()
        n = len(data[0]) # number of columns for each row inserted
        tuple_format="("+"?,"*(n-1)+"?)"
        sql_command="INSERT INTO %s VALUES %s " % (table_name, tuple_format)
        # Fill the table with the info in the tuples
        types = self.get_table_types(table_name)
#        log.debug("Storing types: %s", types)
        for i in xrange(len(data)):
            data[i] = [apply_type(d) for d,apply_type in zip(data[i], types)]
        self.cursor.executemany(sql_command, data)
        self.connection.commit()

    def retrieve_data(self,sql_command):
        """ Retrieves data from the database using the sql_command
        returns the records as a list of tuples"""
        self.check_if_is_connected()
        log.debug("Retrieving data: %s" % sql_command)
        self.cursor.execute(sql_command)
        return self.cursor.fetchall()

    def update_data(self, table_name,
                    updated_fields,
                    updated_values,
                    condition_fields,
                    condition_values):
        """ updates the register in the table identified by the condition
            values for the condition fields
        """
        self.check_if_is_connected()
        sql_command = "UPDATE %s SET " % (table_name)
        for field, value in zip(updated_fields, updated_values):
            sql_command += "%s=%s," % (field, value)
        sql_command = sql_command.rstrip(",") + " WHERE "
        s = self.get_condition_string(condition_fields, condition_values)
        sql_command = sql_command + s
        #print sql_command
        log.debug("Updating %s: %s",table_name, sql_command)
        self.cursor.execute(sql_command)
        self.connection.commit()

    def create_view(self,view_name,table_name,
                                condition_fields, condition_values):
        """ creates a view of the given table where the values are selected
            using the condition values. See the help for update_data()
        """
        try: # if this fails is because the view already exist
            self.drop_view(view_name)
        except:
            pass
        sql_command = 'CREATE VIEW %s AS SELECT * FROM %s WHERE ' % (view_name, table_name)
        condition = self.get_condition_string(condition_fields, condition_values)
        sql_command += condition
        log.info("Creating view %s", sql_command)
        self.cursor.execute(sql_command)

    def create_view_of_best_records(self, view_name, table_name, orderby, n_records):
        try: # if this fails is because the view already exist
            self.drop_view(view_name)
        except:
            pass
        sql_command = """CREATE VIEW %s AS SELECT * FROM %s
                         ORDER BY %s ASC LIMIT %d """ % (view_name, table_name, orderby, n_records)
        log.info("Creating view %s", sql_command)
        self.cursor.execute(sql_command)

    def drop_view(self,view_name):
        """ Removes a view from the database """
        self.cursor.execute('DROP VIEW %s' % view_name)

    def get_table(self, table_name, fields=False, orderby=False):
        """ Returns th fields requested from the table """
        fields = self.get_fields_string(fields)
        sql_command = "SELECT %s  FROM %s " % (fields, table_name)
        if orderby:
            sql_command += " ORDER BY %s ASC" % orderby
        data = self.retrieve_data(sql_command)
        return data

    def get_fields_string(self, fields, field_delim=","):
        if fields:
            return field_delim.join(fields)
        return "*"

    def close(self):
        """ Closes the database """
        self.check_if_is_connected()
        self.cursor.close()
        self.connection.close()

    def get_condition_string(self, fields, values):
        """ creates a condition applying each value to each field
        """
        s = ""
        for field,value in zip(fields,values):
            s += "%s=%s AND " % (field, value)
        # remove last AND
        n = len(s)
        s = s[0:n-5]
        return s

    def get_table_types(self, name):
        """
            Gets info about a table and retuns all the types in it
        """
        self.check_if_is_connected()
        sql_command = "PRAGMA table_info(%s)" % name
        self.cursor.execute(sql_command)
        info = self.cursor.fetchall()
        types = []
        for row in info:
            if row[2] == "INT":
                types.append(int)
            elif row[2] == "DOUBLE":
                types.append(float)
            elif row[2][0:7] == "VARCHAR":
                types.append(str)
        return types

    def get_table_column_names(self, name):
        """
            Get the names of the columns for a given table
        """
        self.check_if_is_connected()
        sql_command = "PRAGMA table_info(%s)" % name
        self.cursor.execute(sql_command)
        info = self.cursor.fetchall()
        return [ row[1] for row in info]

    def execute_sql_command(self, sql_command):
        self.check_if_is_connected()
        self.cursor.execute(sql_command)
        self.connection.commit()


    def add_column(self,table,column, data_type):
        """
            Add a column to a table
            column - the name of the column.
            data_type - the type: int, float, str
        """
        sql_typename = get_sql_type_name(data_type)
        sql_command = "ALTER TABLE %s ADD %s %s" % (table, column, sql_typename)
        self.execute_sql_command(sql_command)

    def add_columns(self, table, names, types, check=True):
        """
            Add columns to the database. If check=True, columns with names
            already in the database are skipped. If check=False no check
            is done and trying to add a column that already exists will
            raise and exception
        """
        col_names = self.get_table_column_names(table)
        if check:
            for name, dtype in zip(names, types):
                if name not in col_names:
                    self.add_column(table, name, dtype)
        else:
            for name, dtype in zip(names, types):
                self.add_column(table, name, dtype)

    def get_tables_names(self):
        sql_command = """ SELECT tbl_name FROM sqlite_master """
        data = self.retrieve_data(sql_command)
        names = [d[0] for d in data]
        return names


    def select_table(self):
        """
            Prompt for tables so the user can choose one
        """
        table_name = ""
        self.check_if_is_connected()
        tables = self.get_tables_names()
        for t in tables:
            say = ''
            while say not in ('n','y'):
                say = raw_input("Use table %s (y/n) " % t)
            if say == 'y':
                table_name = t
                columns = self.get_table_column_names(t)
                break
        return table_name, columns


    def drop_columns(self, table, columns):

        cnames = self.get_table_column_names(table)
        for name in columns:
            cnames.remove(name)
        names_txt = ", ".join(cnames)
        sql_command = [
        "CREATE TEMPORARY TABLE backup(%s);" % names_txt,
        "INSERT INTO backup SELECT %s FROM %s" % (names_txt, table),
        "DROP TABLE %s;" % table,
        "CREATE TABLE %s(%s);" % (table, names_txt),
        "INSERT INTO %s SELECT * FROM backup;" % table,
        "DROP TABLE backup;",
        ]
        for command in sql_command:
            log.debug(command)
#            print command
            self.cursor.execute(command)

def print_data(data, delimiter=" "):
    """ Prints the data recovered from a database """
    for row in data:
        line = delimiter.join([str(x) for x in row])
        print line

def write_data(data,output_file,delimiter=" "):
    """writes data to a file. The output file is expected to be a python
    file object """
    w = csv.writer(output_file, delimiter=delimiter)
    for row in data:
        w.writerow(row)

def get_sql_type_name(data_type):
    if(data_type == int):
        return "INT"
    elif(data_type == float):
        return "DOUBLE"
    elif(data_type == str):
        return "VARCHAR(10)" # 10 is a random number, SQLITE does not chop strings

def open(fn_database):
    db = Database2()
    db.connect(fn_database)
    return db

def read_data(fn_database, sql_command):
    db = Database2()
    db.connect(fn_database)
    data = db.retrieve_data(sql_command)
    db.close()
    return data

def get_sorting_indices(l):
    """ Return indices that sort the list l """
    pairs = [(element, i) for i,element in enumerate(l)]
    pairs.sort()
    indices = [p[1] for p in pairs]
    return indices

def merge_databases(fns, fn_output, tbl):
    """
       Reads a table from a set of database files into a single file
       Makes sure to reorder all column names if neccesary before merging
    """
    # Get names and types of the columns from first database file
    db = Database2()
    db.connect(fns[0])
    names = db.get_table_column_names(tbl)
    types = db.get_table_types(tbl)
    indices = get_sorting_indices(names)
    sorted_names = [ names[i] for i in indices]
    sorted_types = [ types[i] for i in indices]
    log.info("Merging databases. Saving to %s", fn_output)
    out_db = Database2()
    out_db.create(fn_output, overwrite=True)
    out_db.connect(fn_output)
    out_db.create_table(tbl, sorted_names, sorted_types)
    for fn in fns:
        log.debug("Reading %s",fn)
        db.connect(fn)
        names = db.get_table_column_names(tbl)
        names.sort()
        they_are_sorted = ",".join(names)
        log.debug("Retrieving %s", they_are_sorted)
        sql_command = "SELECT %s FROM %s" % (they_are_sorted, tbl)
        data = db.retrieve_data(sql_command)
        out_db.store_data(tbl, data)
        db.close()
    out_db.close()
