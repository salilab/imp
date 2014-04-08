import os
import math

list_dir = os.listdir("./CrossLinkPMFs")

'''
pmf=-kT log g(r)
g(r)=exp(-fes/kT)/4/pi/r/r
'''


global_dict = {}
for ld in list_dir:

    files = os.listdir("./CrossLinkPMFs/" + ld)

    file_dict = {}
    for f in files:

        of = open("./CrossLinkPMFs/" + ld + "/" + f)
        allvalues = []

        for i, line in enumerate(of):
            # read the header
            if i == 0:
                labels = []
                tokens = line.split()
                for t in tokens:
                    labels.append(t)
                listoflists = []
                for l in labels:
                    listoflists.append([])
                if not ("fes" in labels or "gofr" in labels or "pmf" in labels or "pfes" in labels):
                    print "error: 'fes' (free energy surface in kcal/mol) or gofr (the radial distribution function) or pmf (the potential of mean force) or pfes (expfes) labels missing in file " + ld + "/" + f
                    exit()
                if not ("distance" in labels):
                    print "error: 'distance' (in Angstrom) missing in file " + ld + "/" + f
                    exit()
                if not ("kbt" in labels) and not ("pfes" in labels):
                    print "error: 'kbt' (temperature in kcal/mol) missing in file " + ld + "/" + f
                    exit()
            else:

                tokens = line.split()

                if len(tokens) != len(labels):
                    print "error: not the same number of labels and entry values, line " + str(i) + " in file " + ld + "/" + f
                    exit()

                for k, t in enumerate(tokens):
                    try:
                        a = float(t)
                    except:
                        print "error: non-numeric entry, line " + str(i) + " in file " + ld + "/" + f
                        exit()

                    listoflists[k].append(a)

            # allvalues.append(values)
            label_dict = {}
            for k, l in enumerate(labels):
                label_dict[l] = listoflists[k]

            # calculate the radial distribution function from pmf or fes"
            if not "gofr" in labels:

                if "pfes" in labels:

                    label_dict["gofr"] = []

                    for j, pfes in enumerate(label_dict["pfes"]):
                        dist = label_dict["distance"][j]
                        label_dict["gofr"].append(
                            pfes / 4 / math.pi / dist / dist)

                elif "fes" in labels:

                    label_dict["gofr"] = []

                    for j, fes in enumerate(label_dict["fes"]):
                        dist = label_dict["distance"][j]
                        kbt = label_dict["kbt"][j]
                        label_dict["gofr"].append(
                            math.exp(-fes / kbt) / 4 / math.pi / dist / dist)

                elif "pmf" in labels:

                    label_dict["gofr"] = []

                    for j, pmf in enumerate(label_dict["pmf"]):
                        dist = label_dict["distance"][j]
                        kbt = label_dict["kbt"][j]
                        label_dict["gofr"].append(math.exp(-pmf / kbt))

                else:
                    print "error: cannot calculate the radial distribution function, no pmf, pfes or fes specified"

        file_dict[f] = label_dict

    global_dict[ld] = file_dict


ofile = 'CrossLinkPMFs.dict'
outfile = open(ofile, "w")
outfile.write("%s" % global_dict)
outfile.close()

print "CrossLinkPMFs"
for i in global_dict:
    print "--", i
    for j in global_dict[i]:
        print "----", j
        for l in global_dict[i][j]:
            print "------", l
            print "------", global_dict[i][j][l]
            print "......"
