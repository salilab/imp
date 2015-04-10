"""@namespace IMP.pmi.output
   Classes for writing output files and processing them.
"""

from __future__ import print_function, division
import IMP
import IMP.atom
import IMP.core
import IMP.pmi
import IMP.pmi.tools
import os
import RMF
import numpy as np
import operator
try:
    import cPickle as pickle
except ImportError:
    import pickle

class Output(object):
    """Class for easy writing of PDBs, RMFs, and stat files"""
    def __init__(self, ascii=True,atomistic=False):
        self.dictionary_pdbs = {}
        self.dictionary_rmfs = {}
        self.dictionary_stats = {}
        self.dictionary_stats2 = {}
        self.best_score_list = None
        self.nbestscoring = None
        self.suffixes = []
        self.replica_exchange = False
        self.ascii = ascii
        self.initoutput = {}
        self.residuetypekey = IMP.StringKey("ResidueName")
        self.chainids = "ABCDEFGHIJKLMNOPQRSTUVXYWZabcdefghijklmnopqrstuvxywz"
        self.dictchain = {}
        self.particle_infos_for_pdb = {}
        self.atomistic=atomistic

    def get_pdb_names(self):
        return list(self.dictionary_pdbs.keys())

    def get_rmf_names(self):
        return list(self.dictionary_rmfs.keys())

    def get_stat_names(self):
        return list(self.dictionary_stats.keys())

    def init_pdb(self, name, prot):
        flpdb = open(name, 'w')
        flpdb.close()
        self.dictionary_pdbs[name] = prot
        self.dictchain[name] = {}

        for n, i in enumerate(self.dictionary_pdbs[name].get_children()):
            self.dictchain[name][i.get_name()] = self.chainids[n]

    def write_psf(self,filename,name):
        flpsf=open(filename,'w')
        flpsf.write("PSF CMAP CHEQ"+"\n")
        index_residue_pair_list={}
        (particle_infos_for_pdb, geometric_center)=self.get_particle_infos_for_pdb_writing(name)
        nparticles=len(particle_infos_for_pdb)
        flpsf.write(str(nparticles)+" !NATOM"+"\n")
        for n,p in enumerate(particle_infos_for_pdb):
            atom_index=n+1
            atomtype=p[1]
            residue_type=p[2]
            chain=p[3]
            resid=p[4]
            flpsf.write('{0:8d}{1:1s}{2:4s}{3:1s}{4:4s}{5:1s}{6:4s}{7:1s}{8:4s}{9:1s}{10:4s}{11:14.6f}{12:14.6f}{13:8d}{14:14.6f}{15:14.6f}'.format(atom_index," ",chain," ",str(resid)," ",'"'+residue_type.get_string()+'"'," ","C"," ","C",1.0,0.0,0,0.0,0.0))
            flpsf.write('\n')
            #flpsf.write(str(atom_index)+" "+str(chain)+" "+str(resid)+" "+str(residue_type).replace('"','')+" C C "+"1.0 0.0 0 0.0 0.0\n")
            if chain not in index_residue_pair_list:
                index_residue_pair_list[chain]=[(atom_index,resid)]
            else:
                index_residue_pair_list[chain].append((atom_index,resid))


        #now write the connectivity
        indexes_pairs=[]
        for chain in sorted(index_residue_pair_list.keys()):

            ls=index_residue_pair_list[chain]
            #sort by residue
            ls=sorted(ls, key=lambda tup: tup[1])
            #get the index list
            indexes=[x[0] for x in ls]
            # get the contiguous pairs
            indexes_pairs+=list(IMP.pmi.tools.sublist_iterator(indexes,lmin=2,lmax=2))
        nbonds=len(indexes_pairs)
        flpsf.write(str(nbonds)+" !NBOND: bonds"+"\n")

        sublists=[indexes_pairs[i:i+4] for i in range(0,len(indexes_pairs),4)]

        # save bonds in fized column format
        for ip in sublists:
            if len(ip)==4:
                flpsf.write('{0:8d}{1:8d}{2:8d}{3:8d}{4:8d}{5:8d}{6:8d}{7:8d}'.format(ip[0][0],ip[0][1],
                             ip[1][0],ip[1][1],ip[2][0],ip[2][1],ip[3][0],ip[3][1]))
            elif len(ip)==3:
                flpsf.write('{0:8d}{1:8d}{2:8d}{3:8d}{4:8d}{5:8d}'.format(ip[0][0],ip[0][1],ip[1][0],
                             ip[1][1],ip[2][0],ip[2][1]))
            elif len(ip)==2:
                flpsf.write('{0:8d}{1:8d}{2:8d}{3:8d}'.format(ip[0][0],ip[0][1],ip[1][0],ip[1][1]))
            elif len(ip)==1:
                flpsf.write('{0:8d}{1:8d}'.format(ip[0][0],ip[0][1]))
            flpsf.write('\n')

        del particle_infos_for_pdb
        flpsf.close()

    def write_pdb(self,name,appendmode=True,
                  translate_to_geometric_center=False):
        if appendmode:
            flpdb = open(name, 'a')
        else:
            flpdb = open(name, 'w')

        (particle_infos_for_pdb,
         geometric_center) = self.get_particle_infos_for_pdb_writing(name)

        if not translate_to_geometric_center:
            geometric_center = (0, 0, 0)

        for n,tupl in enumerate(particle_infos_for_pdb):

            (xyz, atom_type, residue_type,
             chain_id, residue_index,radius) = tupl

            flpdb.write(IMP.atom.get_pdb_string((xyz[0] - geometric_center[0],
                                                xyz[1] - geometric_center[1],
                                                xyz[2] - geometric_center[2]),
                                               n+1, atom_type, residue_type,
                                               chain_id, residue_index,' ',1.00,radius))

        flpdb.write("ENDMDL\n")
        flpdb.close()

        del particle_infos_for_pdb

    def get_particle_infos_for_pdb_writing(self, name):
        # index_residue_pair_list={}

        # the resindexes dictionary keep track of residues that have been already
        # added to avoid duplication
        # highest resolution have highest priority
        resindexes_dict = {}

        # this dictionary dill contain the sequence of tuples needed to
        # write the pdb
        particle_infos_for_pdb = []

        geometric_center = [0, 0, 0]
        atom_count = 0
        atom_index = 0

        for n, p in enumerate(IMP.atom.get_leaves(self.dictionary_pdbs[name])):

            # this loop gets the protein name from the
            # particle leave by descending into the hierarchy

            (protname, is_a_bead) = IMP.pmi.tools.get_prot_name_from_particle(
                p, self.dictchain[name])

            if protname not in resindexes_dict:
                resindexes_dict[protname] = []

            if IMP.atom.Atom.get_is_setup(p) and self.atomistic:
                residue = IMP.atom.Residue(IMP.atom.Atom(p).get_parent())
                rt = residue.get_residue_type()
                resind = residue.get_index()
                atomtype = IMP.atom.Atom(p).get_atom_type()
                xyz = list(IMP.core.XYZ(p).get_coordinates())
                radius = IMP.core.XYZR(p).get_radius()
                geometric_center[0] += xyz[0]
                geometric_center[1] += xyz[1]
                geometric_center[2] += xyz[2]
                atom_count += 1
                particle_infos_for_pdb.append((xyz,
                                               atomtype, rt, self.dictchain[name][protname], resind,radius))
                resindexes_dict[protname].append(resind)

            elif IMP.atom.Residue.get_is_setup(p):

                residue = IMP.atom.Residue(p)
                resind = residue.get_index()
                # skip if the residue was already added by atomistic resolution
                # 0
                if resind in resindexes_dict[protname]:
                    continue
                else:
                    resindexes_dict[protname].append(resind)
                rt = residue.get_residue_type()
                xyz = IMP.core.XYZ(p).get_coordinates()
                radius = IMP.core.XYZR(p).get_radius()
                geometric_center[0] += xyz[0]
                geometric_center[1] += xyz[1]
                geometric_center[2] += xyz[2]
                atom_count += 1
                particle_infos_for_pdb.append((xyz,
                                               IMP.atom.AT_CA, rt, self.dictchain[name][protname], resind,radius))

            elif IMP.atom.Fragment.get_is_setup(p) and not is_a_bead:
                resindexes = IMP.pmi.tools.get_residue_indexes(p)
                resind = resindexes[len(resindexes) // 2]
                if resind in resindexes_dict[protname]:
                    continue
                else:
                    resindexes_dict[protname].append(resind)
                rt = IMP.atom.ResidueType('BEA')
                xyz = IMP.core.XYZ(p).get_coordinates()
                radius = IMP.core.XYZR(p).get_radius()
                geometric_center[0] += xyz[0]
                geometric_center[1] += xyz[1]
                geometric_center[2] += xyz[2]
                atom_count += 1
                particle_infos_for_pdb.append((xyz,
                                               IMP.atom.AT_CA, rt, self.dictchain[name][protname], resind,radius))

            else:
                if is_a_bead:
                    rt = IMP.atom.ResidueType('BEA')
                    resindexes = IMP.pmi.tools.get_residue_indexes(p)
                    resind = resindexes[len(resindexes) // 2]
                    xyz = IMP.core.XYZ(p).get_coordinates()
                    radius = IMP.core.XYZR(p).get_radius()
                    geometric_center[0] += xyz[0]
                    geometric_center[1] += xyz[1]
                    geometric_center[2] += xyz[2]
                    atom_count += 1
                    particle_infos_for_pdb.append((xyz,
                                                   IMP.atom.AT_CA, rt, self.dictchain[name][protname], resind,radius))

        if atom_count > 0:
            geometric_center = (geometric_center[0] / atom_count,
                                geometric_center[1] / atom_count,
                                geometric_center[2] / atom_count)

        particle_infos_for_pdb = sorted(particle_infos_for_pdb, key=operator.itemgetter(3, 4))

        return (particle_infos_for_pdb, geometric_center)


    def write_pdbs(self, appendmode=True):
        for pdb in self.dictionary_pdbs.keys():
            self.write_pdb(pdb, appendmode)

    def init_pdb_best_scoring(
        self,
        suffix,
        prot,
        nbestscoring,
            replica_exchange=False):
        # save only the nbestscoring conformations
        # create as many pdbs as needed

        self.suffixes.append(suffix)
        self.replica_exchange = replica_exchange
        if not self.replica_exchange:
            # common usage
            # if you are not in replica exchange mode
            # initialize the array of scores internally
            self.best_score_list = []
        else:
            # otherwise the replicas must cominucate
            # through a common file to know what are the best scores
            self.best_score_file_name = "best.scores.rex.py"
            self.best_score_list = []
            best_score_file = open(self.best_score_file_name, "w")
            best_score_file.write(
                "self.best_score_list=" + str(self.best_score_list))
            best_score_file.close()

        self.nbestscoring = nbestscoring
        for i in range(self.nbestscoring):
            name = suffix + "." + str(i) + ".pdb"
            flpdb = open(name, 'w')
            flpdb.close()
            self.dictionary_pdbs[name] = prot
            self.dictchain[name] = {}
            for n, i in enumerate(self.dictionary_pdbs[name].get_children()):
                self.dictchain[name][i.get_name()] = self.chainids[n]

    def write_pdb_best_scoring(self, score):
        if self.nbestscoring is None:
            print("Output.write_pdb_best_scoring: init_pdb_best_scoring not run")

        # update the score list
        if self.replica_exchange:
            # read the self.best_score_list from the file
            exec(open(self.best_score_file_name).read())

        if len(self.best_score_list) < self.nbestscoring:
            self.best_score_list.append(score)
            self.best_score_list.sort()
            index = self.best_score_list.index(score)
            for suffix in self.suffixes:
                for i in range(len(self.best_score_list) - 2, index - 1, -1):
                    oldname = suffix + "." + str(i) + ".pdb"
                    newname = suffix + "." + str(i + 1) + ".pdb"
                    # rename on Windows fails if newname already exists
                    if os.path.exists(newname):
                        os.unlink(newname)
                    os.rename(oldname, newname)
                filetoadd = suffix + "." + str(index) + ".pdb"
                self.write_pdb(filetoadd, appendmode=False)

        else:
            if score < self.best_score_list[-1]:
                self.best_score_list.append(score)
                self.best_score_list.sort()
                self.best_score_list.pop(-1)
                index = self.best_score_list.index(score)
                for suffix in self.suffixes:
                    for i in range(len(self.best_score_list) - 1, index - 1, -1):
                        oldname = suffix + "." + str(i) + ".pdb"
                        newname = suffix + "." + str(i + 1) + ".pdb"
                        os.rename(oldname, newname)
                    filenametoremove = suffix + \
                        "." + str(self.nbestscoring) + ".pdb"
                    os.remove(filenametoremove)
                    filetoadd = suffix + "." + str(index) + ".pdb"
                    self.write_pdb(filetoadd, appendmode=False)

        if self.replica_exchange:
            # write the self.best_score_list to the file
            best_score_file = open(self.best_score_file_name, "w")
            best_score_file.write(
                "self.best_score_list=" + str(self.best_score_list))
            best_score_file.close()

    def init_rmf(self, name, hierarchies,rs=None):
        rh = RMF.create_rmf_file(name)
        IMP.rmf.add_hierarchies(rh, hierarchies)
        if rs is not None:
            IMP.rmf.add_restraints(rh,rs)
        self.dictionary_rmfs[name] = rh

    def add_restraints_to_rmf(self, name, objectlist):
        for o in objectlist:
            try:
                rs = o.get_restraint_for_rmf()
            except:
                rs = o.get_restraint()
            IMP.rmf.add_restraints(
                self.dictionary_rmfs[name],
                rs.get_restraints())

    def add_geometries_to_rmf(self, name, objectlist):
        for o in objectlist:
            geos = o.get_geometries()
            IMP.rmf.add_geometries(self.dictionary_rmfs[name], geos)

    def add_particle_pair_from_restraints_to_rmf(self, name, objectlist):
        for o in objectlist:

            pps = o.get_particle_pairs()
            for pp in pps:
                IMP.rmf.add_geometry(
                    self.dictionary_rmfs[name],
                    IMP.core.EdgePairGeometry(pp))

    def write_rmf(self, name):
        IMP.rmf.save_frame(self.dictionary_rmfs[name])
        self.dictionary_rmfs[name].flush()

    def close_rmf(self, name):
        del self.dictionary_rmfs[name]

    def write_rmfs(self):
        for rmf in self.dictionary_rmfs.keys():
            self.write_rmf(rmf)

    def init_stat(self, name, listofobjects):
        if self.ascii:
            flstat = open(name, 'w')
            flstat.close()
        else:
            flstat = open(name, 'wb')
            flstat.close()

        # check that all objects in listofobjects have a get_output method
        for l in listofobjects:
            if not "get_output" in dir(l):
                raise ValueError("Output: object %s doesn't have get_output() method" % str(l))
        self.dictionary_stats[name] = listofobjects

    def set_output_entry(self, key, value):
        self.initoutput.update({key: value})

    def write_stat(self, name, appendmode=True):
        output = self.initoutput
        for obj in self.dictionary_stats[name]:
            d = obj.get_output()
            # remove all entries that begin with _ (private entries)
            dfiltered = dict((k, v) for k, v in d.items() if k[0] != "_")
            output.update(dfiltered)

        if appendmode:
            writeflag = 'a'
        else:
            writeflag = 'w'

        if self.ascii:
            flstat = open(name, writeflag)
            flstat.write("%s \n" % output)
            flstat.close()
        else:
            flstat = open(name, writeflag + 'b')
            cPickle.dump(output, flstat, 2)
            flstat.close()

    def write_stats(self):
        for stat in self.dictionary_stats.keys():
            self.write_stat(stat)

    def get_stat(self, name):
        output = {}
        for obj in self.dictionary_stats[name]:
            output.update(obj.get_output())
        return output

    def write_test(self, name, listofobjects):
#       write the test:
#       output=output.Output()
#       output.write_test("test_modeling11_models.rmf_45492_11Sep13_veena_imp-020713.dat",outputobjects)
#       run the test:
#       output=output.Output()
#       output.test("test_modeling11_models.rmf_45492_11Sep13_veena_imp-020713.dat",outputobjects)
        flstat = open(name, 'w')
        output = self.initoutput
        for l in listofobjects:
            if not "get_test_output" in dir(l) and not "get_output" in dir(l):
                raise ValueError("Output: object %s doesn't have get_output() or get_test_output() method" % str(l))
        self.dictionary_stats[name] = listofobjects

        for obj in self.dictionary_stats[name]:
            try:
                d = obj.get_test_output()
            except:
                d = obj.get_output()
            # remove all entries that begin with _ (private entries)
            dfiltered = dict((k, v) for k, v in d.items() if k[0] != "_")
            output.update(dfiltered)
        #output.update({"ENVIRONMENT": str(self.get_environment_variables())})
        #output.update(
        #    {"IMP_VERSIONS": str(self.get_versions_of_relevant_modules())})
        flstat.write("%s \n" % output)
        flstat.close()

    def test(self, name, listofobjects):
        from numpy.testing import assert_approx_equal as aae
        output = self.initoutput
        for l in listofobjects:
            if not "get_test_output" in dir(l) and not "get_output" in dir(l):
                raise ValueError("Output: object %s doesn't have get_output() or get_test_output() method" % str(l))
        for obj in listofobjects:
            try:
                output.update(obj.get_test_output())
            except:
                output.update(obj.get_output())
        #output.update({"ENVIRONMENT": str(self.get_environment_variables())})
        #output.update(
        #    {"IMP_VERSIONS": str(self.get_versions_of_relevant_modules())})

        flstat = open(name, 'r')

        passed=True
        for l in flstat:
            test_dict = eval(l)
        for k in test_dict:
            if k in output:
                old_value = str(test_dict[k])
                new_value = str(output[k])

                if test_dict[k] != output[k]:
                    if len(old_value) < 50 and len(new_value) < 50:
                        print(str(k) + ": test failed, old value: " + old_value + " new value " + new_value)
                        passed=False
                    else:
                        print(str(k) + ": test failed, omitting results (too long)")
                        passed=False

            else:
                print(str(k) + " from old objects (file " + str(name) + ") not in new objects")
        return passed

    def get_environment_variables(self):
        import os
        return str(os.environ)

    def get_versions_of_relevant_modules(self):
        import IMP
        versions = {}
        versions["IMP_VERSION"] = IMP.kernel.get_module_version()
        try:
            import IMP.pmi
            versions["PMI_VERSION"] = IMP.pmi.get_module_version()
        except (ImportError):
            pass
        try:
            import IMP.isd2
            versions["ISD2_VERSION"] = IMP.isd2.get_module_version()
        except (ImportError):
            pass
        try:
            import IMP.isd_emxl
            versions["ISD_EMXL_VERSION"] = IMP.isd_emxl.get_module_version()
        except (ImportError):
            pass
        return versions

#-------------------
    def init_stat2(
        self,
        name,
        listofobjects,
        extralabels=None,
            listofsummedobjects=None):
        # this is a new stat file that should be less
        # space greedy!
        # listofsummedobjects must be in the form [([obj1,obj2,obj3,obj4...],label)]
        # extralabels

        if listofsummedobjects is None:
            listofsummedobjects = []
        if extralabels is None:
            extralabels = []
        flstat = open(name, 'w')
        output = {}
        stat2_keywords = {"STAT2HEADER": "STAT2HEADER"}
        stat2_keywords.update(
            {"STAT2HEADER_ENVIRON": str(self.get_environment_variables())})
        stat2_keywords.update(
            {"STAT2HEADER_IMP_VERSIONS": str(self.get_versions_of_relevant_modules())})
        stat2_inverse = {}

        for l in listofobjects:
            if not "get_output" in dir(l):
                raise ValueError("Output: object %s doesn't have get_output() method" % str(l))
            else:
                d = l.get_output()
                # remove all entries that begin with _ (private entries)
                dfiltered = dict((k, v)
                                 for k, v in d.items() if k[0] != "_")
                output.update(dfiltered)

        # check for customizable entries
        for l in listofsummedobjects:
            for t in l[0]:
                if not "get_output" in dir(t):
                    raise ValueError("Output: object %s doesn't have get_output() method" % str(t))
                else:
                    if "_TotalScore" not in t.get_output():
                        raise ValueError("Output: object %s doesn't have _TotalScore entry to be summed" % str(t))
                    else:
                        output.update({l[1]: 0.0})

        for k in extralabels:
            output.update({k: 0.0})

        for n, k in enumerate(output):
            stat2_keywords.update({n: k})
            stat2_inverse.update({k: n})

        flstat.write("%s \n" % stat2_keywords)
        flstat.close()
        self.dictionary_stats2[name] = (
            listofobjects,
            stat2_inverse,
            listofsummedobjects,
            extralabels)

    def write_stat2(self, name, appendmode=True):
        output = {}
        (listofobjects, stat2_inverse, listofsummedobjects,
         extralabels) = self.dictionary_stats2[name]

        # writing objects
        for obj in listofobjects:
            od = obj.get_output()
            dfiltered = dict((k, v) for k, v in od.items() if k[0] != "_")
            for k in dfiltered:
                output.update({stat2_inverse[k]: od[k]})

        # writing summedobjects
        for l in listofsummedobjects:
            partial_score = 0.0
            for t in l[0]:
                d = t.get_output()
                partial_score += float(d["_TotalScore"])
            output.update({stat2_inverse[l[1]]: str(partial_score)})

        # writing extralabels
        for k in extralabels:
            if k in self.initoutput:
                output.update({stat2_inverse[k]: self.initoutput[k]})
            else:
                output.update({stat2_inverse[k]: "None"})

        if appendmode:
            writeflag = 'a'
        else:
            writeflag = 'w'

        flstat = open(name, writeflag)
        flstat.write("%s \n" % output)
        flstat.close()

    def write_stats2(self):
        for stat in self.dictionary_stats2.keys():
            self.write_stat2(stat)


class ProcessOutput(object):
    """A class for reading stat files"""
    def __init__(self, filename):
        self.filename = filename
        self.isstat1 = False
        self.isstat2 = False

        # open the file
        if not self.filename is None:
            f = open(self.filename, "r")
        else:
            raise ValueError("No file name provided. Use -h for help")

        # get the keys from the first line
        for line in f.readlines():
            d = eval(line)
            self.klist = list(d.keys())
            # check if it is a stat2 file
            if "STAT2HEADER" in self.klist:
                self.isstat2 = True
                for k in self.klist:
                    if "STAT2HEADER" in str(k):
                        # if print_header: print k, d[k]
                        del d[k]
                stat2_dict = d
                # get the list of keys sorted by value
                kkeys = [k[0]
                         for k in sorted(stat2_dict.items(), key=operator.itemgetter(1))]
                self.klist = [k[1]
                              for k in sorted(stat2_dict.items(), key=operator.itemgetter(1))]
                self.invstat2_dict = {}
                for k in kkeys:
                    self.invstat2_dict.update({stat2_dict[k]: k})
            else:
                self.isstat1 = True
                self.klist.sort()

            break
        f.close()

    def get_keys(self):
        return self.klist

    def show_keys(self, ncolumns=2, truncate=65):
        IMP.pmi.tools.print_multicolumn(self.get_keys(), ncolumns, truncate)

    def get_fields(self, fields, filtertuple=None, filterout=None, get_every=1):
        '''
        Get the desired field names, and return a dictionary.

        @param filterout specify if you want to "grep" out something from
                         the file, so that it is faster
        @param filtertuple a tuple that contains
                     ("TheKeyToBeFiltered",relationship,value)
                     where relationship = "<", "==", or ">"
        '''

        outdict = {}
        for field in fields:
            outdict[field] = []

        # print fields values
        f = open(self.filename, "r")
        line_number = 0

        for line in f.readlines():
            if not filterout is None:
                if filterout in line:
                    continue
            line_number += 1

            if line_number % get_every != 0:
                continue
            #if line_number % 1000 == 0:
            #    print "ProcessOutput.get_fields: read line %s from file %s" % (str(line_number), self.filename)
            try:
                d = eval(line)
            except:
                print("# Warning: skipped line number " + str(line_number) + " not a valid line")
                continue

            if self.isstat1:

                if not filtertuple is None:
                    keytobefiltered = filtertuple[0]
                    relationship = filtertuple[1]
                    value = filtertuple[2]
                    if relationship == "<":
                        if float(d[keytobefiltered]) >= value:
                            continue
                    if relationship == ">":
                        if float(d[keytobefiltered]) <= value:
                            continue
                    if relationship == "==":
                        if float(d[keytobefiltered]) != value:
                            continue
                [outdict[field].append(d[field]) for field in fields]

            elif self.isstat2:
                if line_number == 1:
                    continue

                if not filtertuple is None:
                    keytobefiltered = filtertuple[0]
                    relationship = filtertuple[1]
                    value = filtertuple[2]
                    if relationship == "<":
                        if float(d[self.invstat2_dict[keytobefiltered]]) >= value:
                            continue
                    if relationship == ">":
                        if float(d[self.invstat2_dict[keytobefiltered]]) <= value:
                            continue
                    if relationship == "==":
                        if float(d[self.invstat2_dict[keytobefiltered]]) != value:
                            continue

                [outdict[field].append(d[self.invstat2_dict[field]])
                 for field in fields]
        f.close()
        return outdict


def plot_fields(fields, framemin=None, framemax=None):
    import matplotlib as mpl
    mpl.use('Agg')
    import matplotlib.pyplot as plt

    plt.rc('lines', linewidth=4)
    fig, axs = plt.subplots(nrows=len(fields))
    fig.set_size_inches(10.5, 5.5 * len(fields))
    plt.rc('axes', color_cycle=['r'])

    n = 0
    for key in fields:
        if framemin is None:
            framemin = 0
        if framemax is None:
            framemax = len(fields[key])
        x = list(range(framemin, framemax))
        y = [float(y) for y in fields[key][framemin:framemax]]
        if len(fields) > 1:
            axs[n].plot(x, y)
            axs[n].set_title(key, size="xx-large")
            axs[n].tick_params(labelsize=18, pad=10)
        else:
            axs.plot(x, y)
            axs.set_title(key, size="xx-large")
            axs.tick_params(labelsize=18, pad=10)
        n += 1

    # Tweak spacing between subplots to prevent labels from overlapping
    plt.subplots_adjust(hspace=0.3)
    plt.show()


def plot_field_histogram(
    name, values_lists, valuename=None, bins=40, colors=None, format="png",
        reference_xline=None, yplotrange=None, xplotrange=None,normalized=True,
        leg_names=None):

    '''Plot a list of histograms from a value list.
    @param name the name of the plot
    @param value_lists the list of list of values eg: [[...],[...],[...]]
    @param valuename the y-label
    @param bins the number of bins
    @param colors If None, will use rainbow. Else will use specific list
    @param format output format
    @param reference_xline plot a reference line parallel to the y-axis
    @param yplotrange the range for the y-axis
    @param normalized whether the histogram is normalized or not
    @param leg_names names for the legend
    '''

    import matplotlib as mpl
    mpl.use('Agg')
    import matplotlib.pyplot as plt
    import matplotlib.cm as cm
    fig = plt.figure(figsize=(18.0, 9.0))

    if colors is None:
        colors = cm.rainbow(np.linspace(0, 1, len(values_lists)))
    for nv,values in enumerate(values_lists):
        col=colors[nv]
        if leg_names is not None:
            label=leg_names[nv]
        else:
            label=str(nv)
        h=plt.hist(
            [float(y) for y in values],
            bins=bins,
            color=col,
            normed=normalized,histtype='step',lw=4,
            label=label)

    # plt.title(name,size="xx-large")
    plt.tick_params(labelsize=12, pad=10)
    if valuename is None:
        plt.xlabel(name, size="xx-large")
    else:
        plt.xlabel(valuename, size="xx-large")
    plt.ylabel("Frequency", size="xx-large")

    if not yplotrange is None:
        plt.ylim()
    if not xplotrange is None:
        plt.xlim(xplotrange)

    plt.legend(loc=2)

    if not reference_xline is None:
        plt.axvline(
            reference_xline,
            color='red',
            linestyle='dashed',
            linewidth=1)

    plt.savefig(name + "." + format, dpi=150, transparent=True)
    plt.show()


def plot_fields_box_plots(name, values, positions, frequencies=None,
                          valuename="None", positionname="None", xlabels=None):
    '''
    Plot time series as boxplots.
    fields is a list of time series, positions are the x-values
    valuename is the y-label, positionname is the x-label
    '''
    import matplotlib as mpl
    mpl.use('Agg')
    import matplotlib.pyplot as plt
    from matplotlib.patches import Polygon

    bps = []
    fig = plt.figure(figsize=(float(len(positions)) / 2, 5.0))
    fig.canvas.set_window_title(name)

    ax1 = fig.add_subplot(111)

    plt.subplots_adjust(left=0.2, right=0.990, top=0.95, bottom=0.4)

    bps.append(plt.boxplot(values, notch=0, sym='', vert=1,
                           whis=1.5, positions=positions))

    plt.setp(bps[-1]['boxes'], color='black', lw=1.5)
    plt.setp(bps[-1]['whiskers'], color='black', ls=":", lw=1.5)

    if frequencies is not None:
        ax1.plot(positions, frequencies, 'k.', alpha=0.5, markersize=20)

    # print ax1.xaxis.get_majorticklocs()
    if not xlabels is None:
        ax1.set_xticklabels(xlabels)
    plt.xticks(rotation=90)
    plt.xlabel(positionname)
    plt.ylabel(valuename)

    plt.savefig(name,dpi=150)
    plt.show()


def plot_xy_data(x,y,title=None,out_fn=None,display=True,set_plot_yaxis_range=None,
                 xlabel=None,ylabel=None):
    import matplotlib as mpl
    mpl.use('Agg')
    import matplotlib.pyplot as plt
    plt.rc('lines', linewidth=2)

    fig, ax  = plt.subplots(nrows=1)
    fig.set_size_inches(8,4.5)
    if title is not None:
        fig.canvas.set_window_title(title)

    #plt.rc('axes', color='r')
    ax.plot(x,y,color='r')
    if set_plot_yaxis_range is not None:
        x1,x2,y1,y2=plt.axis()
        y1=set_plot_yaxis_range[0]
        y2=set_plot_yaxis_range[1]
        plt.axis((x1,x2,y1,y2))
    if title is not None:
        ax.set_title(title)
    if xlabel is not None:
        ax.set_xlabel(xlabel)
    if ylabel is not None:
        ax.set_ylabel(ylabel)
    if out_fn is not None:
        plt.savefig(out_fn+".pdf")
    if display:
        plt.show()
    plt.close(fig)

def plot_scatter_xy_data(x,y,labelx="None",labely="None",
                         xmin=None,xmax=None,ymin=None,ymax=None,
                         savefile=False,filename="None.eps",alpha=0.75):

    import matplotlib as mpl
    mpl.use('Agg')
    import matplotlib.pyplot as plt
    import sys
    from matplotlib import rc
    #rc('font', **{'family':'serif','serif':['Palatino']})
    rc('font',**{'family':'sans-serif','sans-serif':['Helvetica']})
    #rc('text', usetex=True)

    fig, axs = plt.subplots(1)

    axs0 = axs

    axs0.set_xlabel(labelx, size="xx-large")
    axs0.set_ylabel(labely, size="xx-large")
    axs0.tick_params(labelsize=18, pad=10)

    plot2 = []

    plot2.append(axs0.plot(x,  y,   'o', color='k',lw=2, ms=0.1, alpha=alpha,  c="w"))

    axs0.legend(
        loc=0,
        frameon=False,
        scatterpoints=1,
        numpoints=1,
        columnspacing=1)

    fig.set_size_inches(8.0, 8.0)
    fig.subplots_adjust(left=0.161, right=0.850, top=0.95, bottom=0.11)
    if (not ymin is None) and (not ymax is None):
        axs0.set_ylim(ymin,ymax)
    if (not xmin is None) and (not xmax is None):
        axs0.set_xlim(xmin,xmax)

    #plt.show()
    if savefile:
        fig.savefig(filename, dpi=300)


def get_graph_from_hierarchy(hier):
    graph = []
    depth_dict = {}
    depth = 0
    (graph, depth, depth_dict) = recursive_graph(
        hier, graph, depth, depth_dict)

    # filters node labels according to depth_dict
    node_labels_dict = {}
    node_size_dict = {}
    for key in depth_dict:
        node_size_dict = 10 / depth_dict[key]
        if depth_dict[key] < 3:
            node_labels_dict[key] = key
        else:
            node_labels_dict[key] = ""
    draw_graph(graph, labels_dict=node_labels_dict)


def recursive_graph(hier, graph, depth, depth_dict):
    depth = depth + 1
    nameh = IMP.atom.Hierarchy(hier).get_name()
    index = str(hier.get_particle().get_index())
    name1 = nameh + "|#" + index
    depth_dict[name1] = depth

    children = IMP.atom.Hierarchy(hier).get_children()

    if len(children) == 1 or children is None:
        depth = depth - 1
        return (graph, depth, depth_dict)

    else:
        for c in children:
            (graph, depth, depth_dict) = recursive_graph(
                c, graph, depth, depth_dict)
            nameh = IMP.atom.Hierarchy(c).get_name()
            index = str(c.get_particle().get_index())
            namec = nameh + "|#" + index
            graph.append((name1, namec))

        depth = depth - 1
        return (graph, depth, depth_dict)


def draw_graph(graph, labels_dict=None, graph_layout='spring',
               node_size=5, node_color=None, node_alpha=0.3,
               node_text_size=11, fixed=None, pos=None,
               edge_color='blue', edge_alpha=0.3, edge_thickness=1,
               edge_text_pos=0.3,
               validation_edges=None,
               text_font='sans-serif',
               out_filename=None):

    import matplotlib as mpl
    mpl.use('Agg')
    import networkx as nx
    import matplotlib.pyplot as plt
    from math import sqrt, pi

    # create networkx graph
    G = nx.Graph()

    # add edges
    if type(edge_thickness) is list:
        for edge,weight in zip(graph,edge_thickness):
            G.add_edge(edge[0], edge[1], weight=weight)
    else:
        for edge in graph:
            G.add_edge(edge[0], edge[1])

    if node_color==None:
        node_color_rgb=(0,0,0)
        node_color_hex="000000"
    else:
        cc=IMP.pmi.tools.ColorChange()
        tmpcolor_rgb=[]
        tmpcolor_hex=[]
        for node in G.nodes():
            cctuple=cc.rgb(node_color[node])
            tmpcolor_rgb.append((cctuple[0]/255,cctuple[1]/255,cctuple[2]/255))
            tmpcolor_hex.append(node_color[node])
        node_color_rgb=tmpcolor_rgb
        node_color_hex=tmpcolor_hex

    # get node sizes if dictionary
    if type(node_size) is dict:
        tmpsize=[]
        for node in G.nodes():
            size=sqrt(node_size[node])/pi*10.0
            tmpsize.append(size)
        node_size=tmpsize

    for n,node in enumerate(G.nodes()):
        color=node_color_hex[n]
        size=node_size[n]
        nx.set_node_attributes(G, "graphics", {node : {'type': 'ellipse','w': size, 'h': size,'fill': '#'+color, 'label': node}})
        nx.set_node_attributes(G, "LabelGraphics", {node : {'type': 'text','text':node, 'color':'#000000', 'visible':'true'}})

    for edge in G.edges():
        nx.set_edge_attributes(G, "graphics", {edge : {'width': 1,'fill': '#000000'}})

    for ve in validation_edges:
        print(ve)
        if (ve[0],ve[1]) in G.edges():
            print("found forward")
            nx.set_edge_attributes(G, "graphics", {ve : {'width': 1,'fill': '#00FF00'}})
        elif (ve[1],ve[0]) in G.edges():
            print("found backward")
            nx.set_edge_attributes(G, "graphics", {(ve[1],ve[0]) : {'width': 1,'fill': '#00FF00'}})
        else:
            G.add_edge(ve[0], ve[1])
            print("not found")
            nx.set_edge_attributes(G, "graphics", {ve : {'width': 1,'fill': '#FF0000'}})

    # these are different layouts for the network you may try
    # shell seems to work best
    if graph_layout == 'spring':
        print(fixed, pos)
        graph_pos = nx.spring_layout(G,k=1.0/8.0,fixed=fixed,pos=pos)
    elif graph_layout == 'spectral':
        graph_pos = nx.spectral_layout(G)
    elif graph_layout == 'random':
        graph_pos = nx.random_layout(G)
    else:
        graph_pos = nx.shell_layout(G)


    # draw graph
    nx.draw_networkx_nodes(G, graph_pos, node_size=node_size,
                           alpha=node_alpha, node_color=node_color_rgb,
                           linewidths=0)
    nx.draw_networkx_edges(G, graph_pos, width=edge_thickness,
                           alpha=edge_alpha, edge_color=edge_color)
    nx.draw_networkx_labels(
        G, graph_pos, labels=labels_dict, font_size=node_text_size,
        font_family=text_font)
    if out_filename:
        plt.savefig(out_filename)
        nx.write_gml(G,'out.gml')
    plt.show()


def draw_table():

    # still an example!

    from ipyD3 import d3object
    from IPython.display import display

    d3 = d3object(width=800,
                  height=400,
                  style='JFTable',
                  number=1,
                  d3=None,
                  title='Example table with d3js',
                  desc='An example table created created with d3js with data generated with Python.')
    data = [
        [1277.0,
         654.0,
         288.0,
         1976.0,
         3281.0,
         3089.0,
         10336.0,
         4650.0,
         4441.0,
         4670.0,
         944.0,
         110.0],
        [1318.0,
         664.0,
         418.0,
         1952.0,
         3581.0,
         4574.0,
         11457.0,
         6139.0,
         7078.0,
         6561.0,
         2354.0,
         710.0],
        [1783.0,
         774.0,
         564.0,
         1470.0,
         3571.0,
         3103.0,
         9392.0,
         5532.0,
         5661.0,
         4991.0,
         2032.0,
         680.0],
        [1301.0,
         604.0,
         286.0,
         2152.0,
         3282.0,
         3369.0,
         10490.0,
         5406.0,
         4727.0,
         3428.0,
         1559.0,
         620.0],
        [1537.0,
         1714.0,
         724.0,
         4824.0,
         5551.0,
         8096.0,
         16589.0,
         13650.0,
         9552.0,
         13709.0,
         2460.0,
         720.0],
        [5691.0,
         2995.0,
         1680.0,
         11741.0,
         16232.0,
         14731.0,
         43522.0,
         32794.0,
         26634.0,
         31400.0,
         7350.0,
         3010.0],
        [1650.0,
         2096.0,
         60.0,
         50.0,
         1180.0,
         5602.0,
         15728.0,
         6874.0,
         5115.0,
         3510.0,
         1390.0,
         170.0],
        [72.0, 60.0, 60.0, 10.0, 120.0, 172.0, 1092.0, 675.0, 408.0, 360.0, 156.0, 100.0]]
    data = [list(i) for i in zip(*data)]
    sRows = [['January',
              'February',
              'March',
              'April',
              'May',
              'June',
              'July',
              'August',
              'September',
              'October',
              'November',
              'Deecember']]
    sColumns = [['Prod {0}'.format(i) for i in range(1, 9)],
                [None, '', None, None, 'Group 1', None, None, 'Group 2']]
    d3.addSimpleTable(data,
                      fontSizeCells=[12, ],
                      sRows=sRows,
                      sColumns=sColumns,
                      sRowsMargins=[5, 50, 0],
                      sColsMargins=[5, 20, 10],
                      spacing=0,
                      addBorders=1,
                      addOutsideBorders=-1,
                      rectWidth=45,
                      rectHeight=0
                      )
    html = d3.render(mode=['html', 'show'])
    display(html)
