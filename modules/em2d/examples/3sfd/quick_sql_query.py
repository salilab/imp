#!/usr/bin/env python

import Jpylib.database.Database as Database

import sys

# The only argument of the script is the name of the database to query.
# You'll change the sql_command to get the results that you want. We have
# included a few examples of common SQL queries

if(len(sys.argv) != 2):
    print "Parameters:"
    print "[1] - File of the database"
    sys.exit()

database_name = sys.argv[1]


################################
# HERE ARE SOME EXAMPLES OF QUERIES TO THE DATABASE OF RESULTS

# Get all results
sql_command = """ SELECT * FROM results  """

# Get the drms of the models respect to the em2d score sorted by em2d
# (try plotting the results)
sql_command = """ SELECT drms,em2d FROM results ORDER BY em2d  """

# Get the reference frames of the 100 best models according to the total_score
sql_command = """ SELECT reference_frames
                  FROM results
                  ORDER BY total_score ASC LIMIT 100
              """

# Get the values of the connectivity restraint cB_A for all models
sql_command = """ SELECT cB_A FROM results """

sql_command = """ SELECT * FROM results  """

################################
db=Database.Database2()
db.connect(database_name)

# This 2 lines will print the names of the columns in the table of results
# They will be the solution_id, assignment, reference_frames, total_score,
# and the names of all the restraints
names = "# " + " ".join([name for name in db.get_table_column_names("results")])
print names

data=db.retrieve_data(sql_command)

# Uncomment this part of the script to see all the data on the screen
"""
for row in data:
    line = []
    for x in row:
        try:
            txt = "%14.4f" % float(x)
        except:
            txt = "%20s" % str(x)
        line.append(txt)
    print " ".join(line)
"""

# write the file of results
f = open("sql_query_file.txt", "w")
Database.write_data(data, f, " ")
