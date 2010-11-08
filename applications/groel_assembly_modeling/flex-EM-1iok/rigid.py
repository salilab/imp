from modeller import *

def load_rigid(path,mdl,sel_rigid,rand_rigid,rigid_filename):
    f = open(path + '/' + rigid_filename,"r")
    line = f.readline()
    flag = 0
    while(line!=''):
        c = line[0]
        line = line[:-1] # remove endline character
        if(not c in "hbld #\n"):
            resnum = [w for w in line.strip().split(' ') if len(w)]
            if resnum[-1] == 'nr':
                # Do NOT randomize
                randflag = 0
                length = len(resnum) - 1
            else:
                # Randomize
                randflag = 1
                length = len(resnum)
            i=0
            sel_residues=selection()
            while i < length:
                sel_residues.add(mdl.residue_range(resnum[i], resnum[i+1]))
                i=i+2
            sel_rigid.append(sel_residues)
            rand_rigid.append(randflag)
        line = f.readline()
    f.close()


# Reads selections of residues from a file and puts them into the file
# \param[in] path path of the input file
# \param[in] fn filename of the input file
# \paran[out] sel_list the list of selections
# \param[in] mdl the model where the selections are made
# \note The format is the same as that for rigid in the function above
def read_selection_list(path,mdl,sel_list,fn):
    f = open(path + '/' + fn,"r")
    line = f.readline()
    while(line!=''):
        c=line[0]
        # h from helix, b from beta, l from loop, d from domain
        if(not c in "hbld #\n"):
            line = line[:-1] # remove endline character
            resnum = [w for w in line.strip().split(' ') if len(w)]
            if resnum[-1] == 'nr':
                length = len(resnum) - 1
            else:
                length = len(resnum)
            i=0
            sel=selection()
            while i < length:
                sel.add(mdl.residue_range(resnum[i], resnum[i+1]))
                i=i+2
            sel_list.append(sel)
        line = f.readline()
    f.close()
