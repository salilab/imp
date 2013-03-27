import IMP.em2d.imp_general.io as io
import IMP.em2d.Database as Database

import sys
import heapq
import math
import os
import csv
import time
import logging
import glob
import numpy as np

try:
    set = set
except NameError:
    from sets import Set as set

log = logging.getLogger("solutions_io")

unit_delim = "/" # separate units within a field (eg, reference frames).
field_delim = ","

class ClusterRecord(tuple):
    """Simple named tuple class"""

    class _itemgetter(object):
        def __init__(self, ind):
            self.__ind = ind
        def __call__(self, obj):
            return obj[self.__ind]

    def __init__(self, iterable):
        if len(iterable) != self.__n_fields:
            raise TypeError("Expected %d arguments, got %d" \
                            % (self.__n_fields, len(iterable)))
        tuple.__init__(self, iterable)

    __n_fields = 5
    cluster_id = property(_itemgetter(0))
    n_elements = property(_itemgetter(1))
    representative = property(_itemgetter(2))
    elements = property(_itemgetter(3))
    solutions_ids = property(_itemgetter(4))


#################################

# INPUT/OUTPUT OF SOLUTIONS OBTAINED WITH DominoModel

#################################

class HeapRecord(tuple):
    """
        The heapq algorithm is a min-heap. I want a max-heap, that pops the
        larger values out of the heap.
        For that I have to modify the comparison function and also set the
        index that is used for the comparison. The index corresponds to
        the restraint that we desired to order by
    """
    def __new__(self,x,i):
        """
            Build from a tuple and the index used to compare
        """
        self.i = i
        return tuple.__new__(self, x)

    def __lt__(self, other):
        """
            Compare. To convert the min-heap into a max-heap, the lower than
            comparison is transformed into a greater-than
        """
        i = self.i
        if(self[i] > other[i]):
            return True
        return False

    # Need __le__ as well for older Pythons
    def __le__(self, other):
        i = self.i
        return self[i] >= other[i]


def gather_best_solution_results(fns, fn_output, max_number=50000,
                                    raisef=0.1, orderby="em2d"):
    """
       Reads a set of database files and merge them into a single file.

       @param fns List of files with databases
       @param fn_output The database to create
       @param max_number Maximum number of records to keep, sorted according
            to orderby
       @param raisef Ratio of problematic database files tolerated before
            raising an error. This option is to tolerate some files
            of the databases being broken because the cluster fails,
            fill the disks, etc
       @param orderby Criterium used to sort the the records
       NOTE:
       Makes sure to reorder all column names if neccesary before merging
       The record for the native solution is only added once (from first file).
    """
    tbl = "results"
    # Get names and types of the columns from first database file
    db = Database.Database2()
    db.connect(fns[0])
    names = db.get_table_column_names(tbl)
    types = db.get_table_types(tbl)
    indices = get_sorting_indices(names)
    sorted_names = [ names[i] for i in indices]
    sorted_types = [ types[i] for i in indices]

    names.sort()
    ind = names.index(orderby)
    they_are_sorted = field_delim.join(names)
    # Get the native structure data from the first database
    sql_command = """SELECT %s FROM %s
                  WHERE assignment="native" LIMIT 1 """ % (they_are_sorted, tbl)
    native_data = db.retrieve_data(sql_command)
    db.close()
    log.info("Gathering results. Saving to %s", fn_output)
    out_db = Database.Database2()
    out_db.create(fn_output, overwrite=True)
    out_db.connect(fn_output)
    out_db.create_table(tbl, sorted_names, sorted_types)

    best_records = []
    n_problems = 0
    for fn in fns:
        try:
            log.info("Reading %s",fn)
            db.connect(fn)
#            log.debug("Retrieving %s", they_are_sorted)
            sql_command = """SELECT %s FROM %s
                             WHERE assignment<>"native"
                             ORDER BY %s ASC LIMIT %s """ % (
                                    they_are_sorted, tbl,orderby, max_number)
            data = db.retrieve_data(sql_command)
            log.info("%s records read from %s",len(data), fn)
            db.close()
            # Fill heap
            for d in data:
                a = HeapRecord(d, ind)
                if(len(best_records) < max_number):
                    heapq.heappush(best_records, a)
                else:
                    # remember that < here compares for greater em2d value,
                    # as a HeapRecord is used
                    if(best_records[0] < a):
                        heapq.heapreplace(best_records, a)
        except Exception, e:
            log.error("Error for %s: %s",fn, e)
            n_problems += 1

    # If the number of problematic files is too high, report that something
    # big is going on. Otherwise tolerate some errors from some tasks that
    # failed (memory errors, locks, writing errors ...)
    ratio = float(n_problems)/float(len(fns))
    if ratio > raisef:
        raise IOError("There are %8.1f %s of the database "\
                    "files to merge with problems! " % (ratio*100,"%"))
    # append the native data to the best_records
    heapq.heappush(best_records, native_data[0])
    out_db.store_data(tbl, best_records)
    out_db.close()

def gather_solution_results(fns, fn_output, raisef=0.1):
    """
       Reads a set of database files and puts them in a single file
       Makes sure to reorder all column names if neccesary before merging
       @param fns List of database files
       @param fn_output Name of the output database
       @param raisef See help for gather_best_solution_results()
    """
    tbl = "results"
    # Get names and types of the columns from first database file
    db = Database.Database2()
    db.connect(fns[0])
    names = db.get_table_column_names(tbl)
    types = db.get_table_types(tbl)
    indices = get_sorting_indices(names)
    sorted_names = [ names[i] for i in indices]
    sorted_types = [ types[i] for i in indices]
    log.info("Gathering results. Saving to %s", fn_output)
    out_db = Database.Database2()
    out_db.create(fn_output, overwrite=True)
    out_db.connect(fn_output)
    out_db.create_table(tbl, sorted_names, sorted_types)

    n_problems = 0
    for fn in fns:
        try:
            log.info("Reading %s",fn)
            db.connect(fn)
            names = db.get_table_column_names(tbl)
            names.sort()
            they_are_sorted = field_delim.join(names)
            log.debug("Retrieving %s", they_are_sorted)
            sql_command = "SELECT %s FROM %s" % (they_are_sorted, tbl)
            data = db.retrieve_data(sql_command)
            out_db.store_data(tbl, data)
            db.close()
        except Exception, e:
            log.error("Error for file %s: %s",fn, e)
            n_problems += 1
    ratio = float(n_problems)/float(len(fns))
    if ratio > raisef:
        raise IOError("There are %8.1f %s of the database "\
                    "files to merge with problems! " % (ratio*100,"%"))
    out_db.close()

def get_sorting_indices(l):
    """ Return indices that sort the list l """
    pairs = [(element, i) for i,element in enumerate(l)]
    pairs.sort()
    indices = [p[1] for p in pairs]
    return indices

def get_best_solution(fn_database, Nth, fields=False, orderby=False,
                                                                 tbl="results"):
    """
        Recover the reference frame of the n-th best solution from a database.
        The index Nth stars at 0
    """
    f = get_fields_string(fields)
    sql_command = """ SELECT %s FROM %s
                      ORDER BY %s
                      ASC LIMIT 1 OFFSET %d """ % (f, tbl, orderby, Nth)
    data = Database.read_data(fn_database, sql_command)
    if len(data) == 0:
        raise ValueError("The requested %s-th best solution does not exist. "\
                                "Only %s solutions found" % (Nth, len(data) ))
    # the only field  last record is the solution requested
    return data[0][0]

def get_pca(string, delimiter="/"):
    pca = string.split(delimiter)
    pca = [float(p) for p in pca]
    return pca

def get_fields_string(fields):
    """
        Get a list of fields and return a string with them. If there are no
        fields, return an *, indicating SQL that all the fields are requested
        @param fields A list of strings
        @return a string
    """

    if fields:
        return field_delim.join(fields)
    return "*"


class ResultsDB(Database.Database2):
    """
        Class for managing the results of the experiments
    """
    def __init__(self, ):
        self.records = []
        self.native_table_name = "native"
        self.results_table = "results"
        self.placements_table = "placements"
        self.ccc_table_name = "ccc"
        self.cluster_records = []

        # columns describing a solution in the results
        self.results_description_columns = ["solution_id", "assignment",
                                                        "reference_frames"]
        self.results_description_types = [int, str, str]
        # columns describing measures for a result
        self.results_measures_columns = ["drms", "cdrms", "crmsd"]
        self.results_measures_types = [float, float, float]

    def add_results_table(self,restraints_names, add_measures=False):
        """
            Build the table of results
            @param restraints_names The names given to the columns of the table
            @param add_measures If True, add fields for comparing models
            and native conformation
        """
        table_fields = self.results_description_columns + \
                                        ["total_score"] + restraints_names
        table_types = self.results_description_types + \
                                [float] + [float for r in restraints_names]
        if add_measures:
            # Add columns for measures
            table_fields += self.results_measures_columns
            table_types += self.results_measures_types
        log.debug("Creating table %s\n%s",table_fields,table_types)
        self.create_table(self.results_table, table_fields, table_types)
        # create a table for the native assembly if we are benchmarking
        if add_measures :
            self.create_table(self.native_table_name, table_fields, table_types)

    def get_solutions_results_table(self, fields=False,
                                    max_number=None, orderby=False):
        """
            Recovers solutions
            @param fields Fields to recover from the table
            @param max_number Maximum number of solutions to recover
            @param orderby Name of the restraint used for sorting the states
        """
        self.check_if_is_connected()
        log.info("Getting %s from solutions", fields)
        f = self.get_fields_string(fields)
        sql_command = "SELECT %s FROM %s " % (f, self.results_table)
        if orderby:
            sql_command += " ORDER BY %s ASC" % orderby
        if max_number not in (None,False):
            sql_command += " LIMIT %d" % (max_number)
        log.debug("Using %s", sql_command )
        data = self.retrieve_data(sql_command)
        return data

    def get_solutions(self, fields=False,  max_number=None, orderby=False):
        """
            Get solutions from the database.
            @param fields Fields requested. If the fields are in different
            tables, a left join is done. Otherwise get_solutions_results_table()
            is called. See get_solutions_results_table() for the meaning
            of the parameters.
        """
        tables = self.get_tables_names()
        log.debug("tables %s", tables)
        required_tables = set()
        pairs_table_field = []
#        fields_string = self.get_fields_string(fields)
        if not fields:
            fields = ["*",]
        for f,t in [(f,t) for f in fields for t in tables]:
            if t == "native" or f == "solution_id":
                continue
            columns = self.get_table_column_names(t)
            if f in columns:
                required_tables.add(t)
                pairs_table_field.append((t,f))
        required_tables = list(required_tables)
        log.debug("required_tables %s", required_tables)
        log.debug("pairs_table_field %s", pairs_table_field)
        if len(required_tables) == 0:
            data = self.get_solutions_results_table(fields,
                                                max_number, orderby)
            return data
        elif len(required_tables) == 1 and required_tables[0] == "results":
            data = self.get_solutions_results_table(fields,
                                                max_number, orderby)
            return data
        elif len(required_tables) > 1:
            sql_command = self.get_left_join_command( pairs_table_field,
                                                      required_tables)
            if orderby:
                sql_command += " ORDER BY %s ASC" % orderby
            log.debug("Using %s", sql_command )
            data = self.retrieve_data(sql_command)
            return data
        else:
            raise ValueError("Fields not found in the database")

    def get_native_solution(self, fields=False):
        """
            Recover data for the native solution
            @param fields Fields to recover
        """

        f = self.get_fields_string(fields)
        sql_command = "SELECT %s FROM %s " % (f, self.native_table_name)
        data = self.retrieve_data(sql_command)
        return data

    def add_record(self, solution_id, assignment, RFs, total_score,
                                            restraints_scores, measures):
        """
            Add a recorde to the database
            @param solution_id The key for the solution
            @param assignment The assigment for the solution provided by
                              domino
            @param RFs Reference frames of the rigid bodies of the components
            of the assembly in the solution
            @param total_score Total value of the scoring function
            @param restraints_scores A list with all the values for the
                     restraints
            @param measures A list with the values of all the measures for
            benchmark
        """
        words = [io.ReferenceFrameToText(ref).get_text() for ref in RFs]
        RFs_txt = unit_delim.join(words)
        record = [solution_id, assignment, RFs_txt, total_score] + \
                                                        restraints_scores
        if measures != None:
            record = record + measures
        self.records.append(record)

    def add_native_record(self, assignment, RFs, total_score,
                                                        restraints_scores):
        """
            Add a record for the native structure to the database
            see add_record() for the meaning of the parameters
        """
        words = [io.ReferenceFrameToText(ref).get_text() for ref in RFs]
        RFs_txt = unit_delim.join(words)
        solution_id = 0
        record = [solution_id, assignment, RFs_txt, total_score] + \
                                                            restraints_scores
        measures = [0,0,0] # ["drms", "cdrms", "crmsd"]
        record = record + measures
        self.store_data(self.native_table_name, [record])

    def save_records(self,table="results"):
        self.store_data(table, self.records)

    def format_placement_record(self, solution_id, distances, angles):
        """ both distances and angles are expected to be a list of floats """
        return [solution_id] + distances + angles


    def add_placement_scores_table(self, names):
        """
            Creates a table to store the values of the placement scores for the
            models.
            @param names Names of the components of the assembly
        """
        self.check_if_is_connected()
        self.placement_table_name = self.placements_table
        table_fields = ["solution_id"]
        table_fields += ["distance_%s" % name for name in names]
        table_fields += ["angle_%s" % name for name in names]
        table_types = [int] + [float for f in table_fields]
        self.drop_table(self.placement_table_name)
        self.create_table(self.placement_table_name, table_fields, table_types)
        self.add_columns(self.native_table_name,
                                table_fields, table_types,check=True)
        # update all placements scores to 0 for the native assembly
        native_values = [0 for t in table_fields]
        log.debug("%s", self.native_table_name)
        log.debug("table fields %s", table_fields)
        self.update_data(self.native_table_name,
                         table_fields, native_values,
                         ["assignment"], ["\"native\""])

    def get_placement_fields(self):
        """
            Return the names of the placement score fields in the database
        """
        columns = self.get_table_column_names(self.placements_table)
        fields = [col for col in columns if "distance" in col or "angle" in col]
        return fields

    def add_ccc_table(self):
        """
            Add a table to the database for store the values of the cross
            correlation coefficient between a model and the native configuration
        """

        self.check_if_is_connected()
        table_fields = ["solution_id", "ccc"]
        table_types = [int, float]
        self.drop_table(self.ccc_table_name)
        self.create_table(self.ccc_table_name, table_fields, table_types)
        # update values for the native assembly
        self.add_columns(self.native_table_name,
                         table_fields, table_types,check=True)
        self.update_data(self.native_table_name,
                    table_fields, [0,1.00], ["assignment"], ["\"native\""])

    def format_ccc_record(self, solution_id, ccc):
        """ Format for the record to store in the ccc table """
        return [solution_id, ccc]

    def get_ccc(self, solution_id):
        """
            Recover the cross-correlation coefficient for a solution
            @param solution_id
        """
        sql_command = """ SELECT ccc FROM %s
                          WHERE solution_id=%d """ % (self.ccc_table_name,
                                                      solution_id)
        data = self.retrieve_data(sql_command)
        return data[0][0]

    def store_ccc_data(self, ccc_data):
        self.store_data(self.ccc_table_name, ccc_data)

    def store_placement_data(self, data):
        log.debug("store placement table %s",data)
        self.store_data(self.placement_table_name,data)

    def get_left_join_command(self, pairs_table_field, tables_names):
        """
            Format a left join SQL command that recovers all fileds from the
            tables given
            @param pairs_table_field Pairs of (table,field)
            @param tables_names Names of the tables

            E.g. If pairs_table_filed = ((table1,a), (table2,b), (table3,c),
                 (table2,d)) and tables_names = (table1, table2, table3)

            The SQL command is:
            SELECT table1.a, table2.b, table3.c, table2.d FROM table1
            LEFT JOIN table2 ON table1.solution_id = table2.solution_id
            LEFT JOIN table3 ON table1.solution_id = table3.solution_id
            WHERE table1.solution_id IS NOT NULL AND
                    table2.solution_id IS NOT NULL AND
                    table3.solution_id IS NOT NULL
        """

        txt = [ "%s.%s" % (p[0],p[1]) for p in pairs_table_field]
        fields_requested = field_delim.join(txt)
        sql_command = " SELECT %s FROM %s " % (fields_requested,tables_names[0])
        n_tables = len(tables_names)
        for i in range(1, n_tables):
            a = tables_names[i-1]
            b = tables_names[i]
            sql_command += " LEFT JOIN %s " \
                            "ON %s.solution_id = %s.solution_id " % (b,a,b)
        # add the condition of solution_id being not null, so there are not
        # problems if some solutions are missing in one table
        for i in range(n_tables-1):
            sql_command += "WHERE %s.solution_id " \
                                            "IS NOT NULL AND " % tables_names[i]
        sql_command += " %s.solution_id IS NOT NULL " % tables_names[n_tables-1]
        log.debug("%s" %sql_command)
        return sql_command

    def add_clusters_table(self, name):
        """
            Add a table to store information about the clusters of structures
            @param name Name of the table
        """
        self.cluster_table_name = name
        self.check_if_is_connected()
        table_fields = ("cluster_id","n_elements",
                            "representative","elements", "solutions_ids")
        table_types = (int, int, int, str, str)
        self.drop_table(name)
        self.create_table(name, table_fields, table_types)

    def add_cluster_record(self, cluster_id, n_elements, representative,
                            elements, solutions_ids):
        """
            Add a record to the cluster database. Actually, only stores it
            in a list (that will be added later)
            @param cluster_id Number with the id of the cluster
            @param n_elements Number of elements in the cluster
            @param representative Number with the id of the representative
                element
            @param elements List with the number of the elements of the cluster
            @param solutions_ids The numbers above are provided by the
            clustering algorithm. The solutions_ids are the ids of the models
            in "elements".
        """

        record = (cluster_id, n_elements, representative, elements,
                                                            solutions_ids)
        log.debug("Adding cluster record: %s", record)
        self.cluster_records.append(record)

    def store_cluster_data(self):
        """
            Store the data for the clusters
        """
        log.info("Storing data of clusters. Number of records %s",
                                                    len(self.cluster_records) )
        self.store_data(self.cluster_table_name, self.cluster_records)

    def get_solutions_from_list(self, fields=False,  solutions_ids=[]):
        """
            Recover solutions for a specific list of results
            @param fields Fields to recover fro the database
            @param solutions_ids A list with the desired solutions. E.g. [0,3,6]
        """
        sql_command = """ SELECT %s FROM %s WHERE solution_id IN (%s) """
        f = self.get_fields_string(fields)
        str_ids = ",".join(map(str,solutions_ids))
        data = self.retrieve_data( sql_command % (f, self.results_table, str_ids ) )
        return data

    def get_native_rank(self, orderby):
        """
            Get the position of the native configuration
            @param orderby Criterium used to sort the solutions
        """
        import numpy as np

        data = self.get_native_solution([orderby,])
        native_value = data[0][0]
        data = self.get_solutions_results_table(fields=[orderby,],
                                                orderby=orderby)
        values = [row[0] for row in data]
        rank = np.searchsorted(values,native_value)
        return rank

    def get_nth_largest_cluster(self, position, table_name="clusters"):
        """
            Recover the the information about the n-th largest cluster
            @param position Cluster position (by size) requested
            (1 is the largest cluster)
            @param table_name Table where the information about the
                              clusters is stored
        """
        s = """ SELECT * FROM %s ORDER BY n_elements DESC """ % table_name
        data = self.retrieve_data(s)
        record = ClusterRecord(data[position-1])
        return record


    def get_individual_placement_statistics(self, solutions_ids):
        """
            Recovers from the database the placement scores for a set of
            solutions, and returns the mean and standard deviation of the
            placement score for each of the components of the complex being
            scored. This function will be typical used to compute the variation
            of the placement of each component within a cluster of solutions
            @param solutions_ids The ids of the solutions used to compute
                                 the statistics
            @return The output are 4 numpy vectors:
                placement_distances_mean - The mean placement distance for each
                                            component
                placement_distances_stddev - The standardd deviation of the
                                            placement distance for each component
                placement_angles_mean - The mean placement angle for each
                                            component
                placement_angles_stddev - The standard deviation of the placement
                                            angle for each component,
        """

        self.check_if_is_connected()
        table = self.placements_table
        fields = self.get_table_column_names(table)
        distance_fields = filter(lambda x: 'distance' in x, fields)
        angle_fields = filter(lambda x: 'angle' in x, fields)
        sql_command = """ SELECT %s FROM %s WHERE solution_id IN (%s) """
        # string with the solution ids to pass to the sql_command
        str_ids = ",".join(map(str,solutions_ids))
        log.debug("Solutions considered %s", solutions_ids)
        s = sql_command % (",".join(distance_fields), table, str_ids )
        data_distances = self.retrieve_data(s)
        s = sql_command % (",".join(angle_fields), table, str_ids )
        data_angles = self.retrieve_data(s)
        D = np.array(data_distances)
        placement_distances_mean = D.mean(axis=0)
        placement_distances_stddev = D.std(axis=0)
        A = np.array(data_angles)
        placement_angles_mean = A.mean(axis=0)
        placement_angles_stddev = A.std(axis=0)
        return [placement_distances_mean,placement_distances_stddev,
                    placement_angles_mean, placement_angles_stddev]


    def get_placement_statistics(self, solutions_ids):
        """
            Calculate the placement score and its standard deviation for
            the complexes in a set of solutions. The values returned are
            averages, as the placement score for a complex is the average
            of the placement scores of the components. This function is used
            to obtain global placement for a cluster of solutions.
            @param solutions_ids The ids of the solutions used to compute
                                 the statistics
            @return The output are 4 values:
                plcd_mean - Average of the placement distance for the entire
                            complex over all the solutions.
                plcd_std - Standard deviation of the placement distance for
                            the entire complex over all the solutions.
                plca_mean - Average of the placement angle for the entire
                            complex over all the solutions.
                plca_std - Standard deviation of the placement angle for
                            the entire complex over all the solutions.
        """
        [placement_distances_mean,placement_distances_stddev,
            placement_angles_mean, placement_angles_stddev] = \
            self.get_individual_placement_statistics(solutions_ids)
        plcd_mean = placement_distances_mean.mean(axis=0)
        plcd_std  = placement_distances_stddev.mean(axis=0)
        plca_mean = placement_angles_mean.mean(axis=0)
        plca_std  = placement_angles_stddev.mean(axis=0)
        return [plcd_mean, plcd_std, plca_mean, plca_std]
