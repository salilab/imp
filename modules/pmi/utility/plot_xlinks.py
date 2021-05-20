from __future__ import print_function
import RMF
import IMP
import IMP.rmf
import IMP.pmi
import IMP.pmi.tools
import IMP.pmi.analysis
import argparse
import os
from collections import defaultdict


def parse_args():
    parser = argparse.ArgumentParser(
        description = "Draw the cross-links from an RMF file. You can specify two colors, one for"
        "under the threshold and one for over. Also you can change the threshold."
        "Outputs a CMM file, which is readable in chimera."
        "Flag -h for more options")
    parser.add_argument("-f","--rmf_fn",dest="rmf_fn",required=True,
                        help="RMF file for extracting coordinates / restraints")
    parser.add_argument("-n","--frame number",dest="frame_num",
                        type=int,
                        default=0,
                        help="RMF frame number. Default is 0")
    parser.add_argument("-o","--cmm_fn",dest="cmm_fn",
                        help="Output CMM file. Default is the rmf_fn.cmm")
    parser.add_argument("-t","--threshold",dest="threshold",
                        default=35.0,
                        type=float,
                        help="Above this threshold, use a different color. "
                        "Default is 35A")
    parser.add_argument("-r","--radius",dest="radius",
                        default=2.0,
                        type=float,
                        help="Radius for the XL. Default is 2.0")
    parser.add_argument("-c","--color",dest="color",
                        default='93,238,93',
                        help="RGB colors for non-violated XL"
                        "format is R,G,B where each value is out of 255"
                        "Default is green")
    parser.add_argument("-v","--color_viol",dest="color_viol",
                        default='250,77,63',
                        help="RGB colors for violated XL (above options.threshold)"
                        "format is R,G,B where each value is out of 255"
                        "Default is red")
    parser.add_argument("-m","--model_rs",dest="model_rs",
                        help="Kluge to let you read the restraints from a different RMF file")
    parser.add_argument("-l","--limit_to",dest="limit_to",
                        nargs="+",
                        help="Only write XL's with at least one end in this list of subunits."
                        "e.g. -l med2 med3 med5 1,100,med14"
                        "WARNING: this only works on canonical RMF files")
    parser.add_argument("-s","--restrict",dest="restrict",
                        choices=['inter','intra','all'],
                        default='all',
                        help="Optionally restrict to only inter- or intra-module XLs."
                        "Modules defaulted to molecule name. Alternatively provide a topology file with -t"
                        "WARNING: this only works on canonical RMF files")
    parser.add_argument("-g","--topology_fn",dest="topology_fn",
                        help="For use with restricting to inter/intra flag, -s"
                        "It's a text file, each line is <modulename> <tuple1> <tuple2> ..."
                        "WARNING: this only works on canonical RMF files")
    result = parser.parse_args()
    return result

def get_unique_particle_name(p):
    """Return molname:low-high"""
    n = get_molecule_name(p)
    r = get_particle_indices_as_set(p)
    return '%s:%i-%i'%(n,min(r),max(r))

def get_molecule_name(p):
    """Get the name of the molecule for some particle.
    Kind of a kluge until we properly use the Molecule decorator
    """
    mname = IMP.atom.Hierarchy(p).get_parent().get_parent().get_name()
    if '_Res' in mname:
        mname = IMP.atom.Hierarchy(p).get_parent().get_parent().get_parent().get_name()
    #print IMP.atom.Hierarchy(p).get_parent().get_parent(),mname
    return mname

def get_module_name(p,topology_dict):
    """Get the name of a MODULE for a particle.
    If no modules are specified, just returns the molecule name."""
    mname = get_molecule_name(p)
    if topology_dict is None:
        return mname
    else:
        if mname not in topology_dict.keys():
            raise Exception("Alert! The molecule name %s is not in the topology file" %mname)
        for res,module_name in topology_dict[mname]:
            if res==-1:
                return module_name
            else:
                s = get_particle_indices_as_set(p)
                if s<=res:
                    return module_name
        raise Exception("Could not find particle %s in the topology file!"%p.get_name())

def get_particle_indices_as_set(p):
    """Return a set whether it's a residue or a fragment."""
    if IMP.atom.Fragment.get_is_setup(p):
        s = set(IMP.atom.Fragment(p).get_residue_indexes())
    elif IMP.atom.Residue.get_is_setup(p):
        s = set([IMP.atom.Residue(p).get_index()])
    else:
        print('ALERT!',p,'is neither fragment nor residue!')
        exit()
    return s


def check_is_selected(limit_dict,p):
    """ Check if the particle is in the limit dictionary"""
    mname = get_molecule_name(p)
    if mname in limit_dict.keys():
        if limit_dict[mname]==-1:
            return True
        else:
            s = get_particle_indices_as_set(p)
            if s <= limit_dict[mname]:
                return True
    return False

def check_status(restrict_flag,p1,p2,topology_dict):
    """Check if p1,p2 are inter or intra (depending on the flag)"""
    n1 = get_module_name(p1,topology_dict)
    n2 = get_module_name(p2,topology_dict)
    if restrict_flag=='inter' and n1!=n2:
        return True
    elif restrict_flag=='intra' and n1==n2:
        return True
    return False

def run():
    ### process input
    args = parse_args()
    try:
        color = map(lambda x: float(x)/255,args.color.split(','))
        vcolor = map(lambda x: float(x)/255,args.color_viol.split(','))
        t = color[2]
        t = vcolor[2]
    except:
        raise InputError("Wrong color format")
    threshold = float(args.threshold)
    radius = float(args.radius)
    if args.cmm_fn:
        out_fn = args.cmm_fn
    else:
        out_fn = os.path.splitext(args.rmf_fn)[0]+'.cmm'
    limit_dict=None
    if args.limit_to:
        limit_dict=defaultdict(set)
        for lim in args.limit_to:
            fields=lim.split(',')
            if len(fields)==1:
                limit_dict[lim]=-1
            elif len(fields)==3:
                limit_dict[fields[2]]|=set(range(int(fields[0]),int(fields[1])+1))
            else:
                raise Exception('The list of limits must be subunit names or '
                                'start,stop,name tuples')
    topology_dict=None
    if args.restrict and args.topology_fn:
        # kind of a kluge. we need to standardize "topology" files!
        topology_dict=defaultdict(list) # keys are moleculename, values are [(set or -1(=all),module name)]
        inf = open(args.topology_fn)
        for l in inf:
            if l[0]!='#':
                fields = l.split()
                module_name = fields[0]
                for sel_tuple in fields[1:]:
                    tsplit = sel_tuple.split(',')
                    if len(tsplit)==1:
                        topology_dict[sel_tuple].append((-1,module_name))
                    elif len(tsplit)==3:
                        topology_dict[tsplit[2]].append((set(range(int(tsplit[0]),int(tsplit[1])+1)),module_name))
                    else:
                        raise Exception('The list of limits must be subunit names or '
                                'start,stop,name tuples')

    ### collect particles based on the restraints
    mdl = IMP.Model()
    rh = RMF.open_rmf_file_read_only(args.rmf_fn)
    prots = IMP.rmf.create_hierarchies(rh, mdl)
    prot=prots[0]
    rs = IMP.rmf.create_restraints(rh, mdl)
    IMP.rmf.load_frame(rh, RMF.FrameID(int(args.frame_num)))
    pairs=[]
    if args.model_rs:
        ps_dict={}
        for p in IMP.core.get_leaves(prot):
            if IMP.atom.Residue.get_is_setup(p) or IMP.atom.Fragment.get_is_setup(p):
                ps_dict[get_unique_particle_name(p)]=p
        rh2 = RMF.open_rmf_file_read_only(args.model_rs)
        prots2 = IMP.rmf.create_hierarchies(rh2, mdl)
        rs2 = IMP.rmf.create_restraints(rh2, mdl)
        IMP.rmf.load_frame(rh2, RMF.FrameID(0))
        for r in rs2:
            ps2 = r.get_inputs()
            try:
                pp = [ps_dict[get_unique_particle_name(IMP.Particle.get_from(p))] for p in ps2]
            except:
                print('the restraint particles',ps2,'could not be found in the new rmf')
                exit()
            #n1 = get_molecule_name(pp[0])
            #n2 = get_molecule_name(pp[1])
            #print r,n1,n2

            pairs.append(pp+[True]) #flag for good/bad
    else:
        for r in rs:
            ps = r.get_inputs()
            pp = [IMP.Particle.get_from(p) for p in ps]
            pairs.append(pp+[True]) #flag for good/bad

    ### filter the particles as requested
    name='xlinks'
    if limit_dict:
        for np,(p1,p2,flag) in enumerate(pairs):
            if not (check_is_selected(limit_dict,p1) or check_is_selected(limit_dict,p2)):
                pairs[np][2]=False
    if args.restrict!='all':
        name+='_'+args.restrict
        for np,(p1,p2,flag) in enumerate(pairs):
            if not check_status(args.restrict,p1,p2,topology_dict):
                pairs[np][2]=False

    ### prepare output
    outf=open(out_fn,'w')
    outf.write('<marker_set name="%s">\n'%name)


    ### draw the coordinates
    nv=0
    for p1,p2,flag in pairs:
        if not flag:
            continue
        if args.topology_fn:
            print('writing XL between %s(%s) and %s(%s)'%(get_molecule_name(p1),get_module_name(p1,topology_dict),
                                                          get_molecule_name(p2),get_module_name(p2,topology_dict)))
        else:
            print('writing XL between %s and %s'%(get_molecule_name(p1),get_molecule_name(p2)))
        c1 = IMP.core.XYZ(p1).get_coordinates()
        c2 = IMP.core.XYZ(p2).get_coordinates()
        dist = IMP.algebra.get_distance(c1,c2)
        if dist<threshold:
            r,g,b = color
        else:
            r,g,b = vcolor
        outf.write('<marker id= "%d" x="%.3f" y="%.3f" z="%.3f" radius="%.2f" '
                   'r="%.2f" g="%.2f" b="%.2f"/> \n' % (nv,c1[0],c1[1],c1[2],radius,r,g,b))
        outf.write('<marker id= "%d" x="%.3f" y="%.3f" z="%.3f" radius="%.2f" '
                   'r="%.2f" g="%.2f" b="%.2f"/> \n' % (nv+1,c2[0],c2[1],c2[2],radius,r,g,b))
        outf.write('<link id1= "%d" id2="%d" radius="%.2f" '
                   'r="%.2f" g="%.2f" b="%.2f"/> \n' % (nv,nv+1,radius,r,g,b))
        nv+=2
    outf.write('</marker_set>\n')
    outf.close()
    print('wrote',nv/2,'XLs to',out_fn)

if __name__=="__main__":
    run()
