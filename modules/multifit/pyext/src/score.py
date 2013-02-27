#!/usr/bin/env python

__doc__ = "Score each of a set of combinations."

#analyse the ensemble, first we will do the rmsd stuff
import IMP.multifit
from optparse import OptionParser

def get_color_map():
    colors={}
    colors["Rpt1"]=[0.78,0.78,0.73]
    colors["Rpt2"]=[0.78,0.66,0.58]
    colors["Rpt3"]=[0.77,0.43,0.5]
    colors["Rpt4"]=[0.76,0.29,0.67]
    colors["Rpt5"]=[0.51,0.14,0.75]
    colors["Rpt6"]=[0.0,0.,0.75]
    colors["Rpn1"]=[0.34,0.36,0.27]
    colors["Rpn2"]=[0.42,0.43,0.36]
    colors["Rpn3"]=[0.49,0.5,0.44]
    colors["Rpn5"]=[0.56,0.57,0.51]
    colors["Rpn6"]=[0.64,0.64,0.59]
    colors["Rpn7"]=[0.71,0.71,0.66]
    colors["Rpn8"]=[0.78,0.78,0.74]
    colors["Rpn9"]=[1,0,0]
    colors["Rpn10"]=[0,1,0]
    colors["Rpn11"]=[0,0,1]
    colors["Rpn12"]=[0.5,0.2,0.4]
    colors["a1"]=[0.78,0.78,0.73]
    colors["a2"]=[0.78,0.66,0.58]
    colors["a3"]=[0.77,0.43,0.5]
    colors["a4"]=[0.76,0.29,0.67]
    colors["a5"]=[0.51,0.14,0.75]
    colors["a6"]=[0.0,0.,0.75]
    colors["a7"]=[0.34,0.36,0.27]
    colors["a8"]=[0.42,0.43,0.36]
    colors["a9"]=[0.49,0.5,0.44]
    colors["a10"]=[0.56,0.57,0.51]

    colors["a11"]=[0.78,0.78,0.73]
    colors["a12"]=[0.78,0.66,0.58]
    colors["a13"]=[0.77,0.43,0.5]
    colors["a14"]=[0.76,0.29,0.67]
    colors["a15"]=[0.51,0.14,0.75]
    colors["a16"]=[0.0,0.,0.75]
    colors["a17"]=[0.34,0.36,0.27]
    colors["a18"]=[0.42,0.43,0.36]
    colors["a19"]=[0.49,0.5,0.44]
    colors["a20"]=[0.56,0.57,0.51]


    colors["a21"]=[0.78,0.78,0.73]
    colors["a22"]=[0.78,0.66,0.58]
    colors["a23"]=[0.77,0.43,0.5]
    colors["a24"]=[0.76,0.29,0.67]
    colors["a25"]=[0.51,0.14,0.75]
    colors["a26"]=[0.0,0.,0.75]
    colors["a27"]=[0.34,0.36,0.27]
    colors["a28"]=[0.42,0.43,0.36]
    colors["a29"]=[0.49,0.5,0.44]
    colors["a30"]=[0.56,0.57,0.51]
    return colors


def decompose(dmap,mhs):
    full_sampled_map=IMP.em.SampledDensityMap(dmap.get_header())
    all_ps=[]
    for mh in mhs:
        all_ps+=IMP.core.get_leaves(mh)
    full_sampled_map.set_particles(all_ps)
    full_sampled_map.resample()
    full_sampled_map.calcRMS()
    upper=(dmap.get_number_of_voxels()*dmap.get_header().dmean*full_sampled_map.get_header().dmean)/len(mhs)
    lower=dmap.get_number_of_voxels()*dmap.get_header().rms*full_sampled_map.get_header().rms
    norm_factors=[upper,lower]
    print "===============my norm factors:",upper,lower
    return norm_factors


def score_each_protein(dmap,mhs,sd):
    norm_factors=decompose(dmap,mhs)
    scores=[]
    cc=IMP.em.CoarseCC()
    mdl=mhs[0].get_model()
    for i in range(len(mhs)):
        leaves=IMP.core.get_leaves(mhs[i])
        rb=IMP.core.RigidMember(leaves[0]).get_rigid_body()
        mh_dmap=IMP.em.SampledDensityMap(dmap.get_header())
        mh_dmap.set_particles(leaves)
        mh_dmap.resample()
        mh_dmap.calcRMS()
        fits=IMP.multifit.read_fitting_solutions(sd.get_component_header(i).get_transformations_fn())
        mh_scores=[]
        for fit in fits[:15]:
            IMP.core.transform(rb,fit.get_fit_transformation())
            mh_dmap.resample()
            mh_scores.append(cc.cross_correlation_coefficient(dmap,mh_dmap,0.,False,norm_factors))
            IMP.core.transform(rb,fit.get_fit_transformation().get_inverse())
        scores.append(mh_scores)
        print "=====mol",i,mh_scores
    return scores

def usage():
    usage = """%prog [options] <asmb> <asmb.proteomics> <asmb.mapping>
           <alignment.params> <combinatins> <score combinations [output]>

Score each of a set of combinations.
"""
    parser = OptionParser(usage)
    parser.add_option("-m", "--max", dest="max",default=999999999,
                      help="maximum number of fits considered")
    (options, args) = parser.parse_args()
    if len(args) != 6:
        parser.error("incorrect number of arguments")
    return [options,args]

def run(asmb_fn,proteomics_fn,mapping_fn,params_fn,combs_fn,
        scored_comb_output_fn,max_comb):
    asmb=IMP.multifit.read_settings(asmb_fn)
    dmap=IMP.em.read_map(asmb.get_assembly_header().get_dens_fn())
    dmap.get_header().set_resolution(asmb.get_assembly_header().get_resolution())
    dmap.update_voxel_size(asmb.get_assembly_header().get_spacing())
    dmap.set_origin(asmb.get_assembly_header().get_origin())
    threshold=asmb.get_assembly_header().get_threshold()
    combs=IMP.multifit.read_paths(combs_fn)
    #get rmsd for subunits
    colors=get_color_map()
    names=colors.keys()
    print params_fn
    alignment_params = IMP.multifit.AlignmentParams(params_fn)
    alignment_params.show()

    IMP.base.set_log_level(IMP.base.TERSE)
    print "=========",combs_fn
    combs=IMP.multifit.read_paths(combs_fn)
    print "=========1"
    #sd=IMP.multifit.read_settings(asmb_fn)
    print "=========2"
    prot_data=IMP.multifit.read_proteomics_data(proteomics_fn)
    print "=========3"
    mapping_data=IMP.multifit.read_protein_anchors_mapping(prot_data,mapping_fn)
    print "=========4"
    em_anchors =  mapping_data.get_anchors()
    print "=========5"
    ensmb=IMP.multifit.Ensemble(asmb,mapping_data)
    print "=========6"
    #load all proteomics restraints
    align=IMP.multifit.ProteomicsEMAlignmentAtomic(mapping_data,asmb,alignment_params)
    align.set_fast_scoring(False)
    print "align"
    mdl=align.get_model()
    mhs=align.get_molecules()
    align.add_states_and_filters()
    rbs=align.get_rigid_bodies()
    print IMP.core.RigidMember(IMP.core.get_leaves(mhs[0])[0]).get_rigid_body()
    align.set_density_map(dmap,threshold)
    gs=[]
    for i,mh in enumerate(mhs):
        ensmb.add_component_and_fits(mh,
                                      IMP.multifit.read_fitting_solutions(asmb.get_component_header(i).get_transformations_fn()))
        try:
            rgb=colors[mh.get_name()]
        except:
            rgb=colors[names[i]]
        color=IMP.display.Color(rgb[0],rgb[1],rgb[2])
        '''
        for p in IMP.core.get_leaves(mh):
            g= IMP.display.XYZRGeometry(p)
            g.set_color(color)
            gs.append(g)
        '''
    all_leaves=[]
    for mh in mhs:
        all_leaves+=IMP.core.XYZs(IMP.core.get_leaves(mh))

    align.add_all_restraints()
    print "====1"
    print "Get number of restraints:",len(mdl.get_restraints())
    rs=mdl.get_restraints()
    for r in mdl.get_restraints():
        rr=IMP.RestraintSet.get_from(r)
        for i in range(rr.get_number_of_restraints()):
            print rr.get_restraint(i).get_name()
    output = open(scored_comb_output_fn,"w")
    #load ref structure
    ref_mhs=[]
    all_ref_leaves=[]
    for i in range(asmb.get_number_of_component_headers()):
        c=asmb.get_component_header(i)
        fn=c.get_reference_fn()
        if fn:
            ref_mhs.append(IMP.atom.read_pdb(fn,mdl))
            all_ref_leaves+=IMP.core.get_leaves(ref_mhs[-1])
    for r in rs:
        rr=IMP.RestraintSet.get_from(r)
        for i in range(rr.get_number_of_restraints()):
            output.write(rr.get_restraint(i).get_name()+"|")
    output.write("\n")
    #add fit restraint
    fitr=IMP.em.FitRestraint(all_leaves,dmap)
    mdl.add_restraint(fitr)
    print "Number of combinations:",len(combs[:max_comb])

    print "native score"
    num_violated=0
    for r in rs:
        rr=IMP.RestraintSet.get_from(r)
        for j in range(rr.get_number_of_restraints()):
            print rr.get_restraint(j).get_name(),rr.evaluate(False)

    prev_name=''
    for i,comb in enumerate(combs[:max_comb]):
        print "Scoring combination:",comb
        ensmb.load_combination(comb)
        num_violated=0
        for r in rs:
            rr=IMP.RestraintSet.get_from(r)
            for j in range(rr.get_number_of_restraints()):
                print rr.get_restraint(j).get_name()
                rscore=rr.evaluate(False)
                if rscore>5:
                    num_violated=num_violated+1
        IMP.atom.write_pdb(mhs,"model.%d.pdb"%(i))
        print str(all_leaves[0])+" :: " + str(all_leaves[-1])
        score=mdl.evaluate(None)
        num_violated=0
        msg="COMB"+str(i)+"|"
        for r in rs:
            rr=IMP.RestraintSet.get_from(r)
            for j in range(rr.get_number_of_restraints()):
                current_name=rr.get_restraint(j).get_name()
                if current_name!=prev_name:
                    msg+=' '+current_name+' '
                    prev_name=current_name
                rscore=rr.get_restraint(j).evaluate(False)
                msg+=str(rscore)+"|"
                if rscore>5:
                    num_violated=num_violated+1
        #msg+="|"+str(score)+"|"+str(num_violated)+"|\n"
        msg+="|"+str(score)+"|"+str(num_violated)+"||||"+str(fitr.evaluate(None))+"||:"
        if all_ref_leaves:
            msg+=str(IMP.atom.get_rmsd(IMP.core.XYZs(all_leaves),
                                       IMP.core.XYZs(all_ref_leaves)))
        output.write(msg+"\n")
        print msg
        ensmb.unload_combination(comb)
    output.close()

def main():
    (options,args) = usage()
    run(args[0],args[1],args[2],args[3],args[4],args[5],int(options.max))

if __name__=="__main__":
    main()
