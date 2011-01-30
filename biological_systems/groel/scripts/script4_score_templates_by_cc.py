import IMP.em
import IMP.atom
import IMP.multifit
IMP.set_log_level(IMP.NONE)
#--- parse the templates file
templates_file="build_profile.prf"
templates=[]
seq_ids=[]
for line in open(templates_file):
    if line[0]=="#":
        continue
    s=line.split()
    if int(s[0])==1:
        continue
    if not(int(s[4])-int(s[3])>480):
        print "Not including:",s[0],s[1]
    templates.append([s[1][:4],s[1][-1]])
    seq_ids.append(s[10])
templates_dir="data/templates/"
#--- load the target density map
dmap=IMP.em.read_map("groel_subunit_8.mrc",IMP.em.MRCReaderWriter())
dmap.get_header_writable().set_resolution(10)
#--- load IMP model
mdl=IMP.Model()
rb_refiner=IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
template_fit_sols=[]
best_fit=1.
best_temp=[]
dens_threshold=0.02
#--- iterate over the templates and fit each of them
for i,t in enumerate(templates):
    print "fitting template "+t[0]+t[1]
    #load the template
    mh=IMP.atom.read_pdb(templates_dir+t[0]+".pdb",mdl)
    #get the right chain
    mh_chain=IMP.atom.get_by_type(mh,IMP.atom.CHAIN_TYPE)[ord(t[1])-ord('A')]
    rb=IMP.atom.setup_as_rigid_body(mh_chain)
    #fit the template to the density map
    dmap.show()
    sols=IMP.multifit.pca_based_rigid_fitting(rb,rb_refiner,dmap,dens_threshold)
    IMP.core.transform(rb,sols.get_transformation(0))
    #refine the best scoring fit
    mhs=IMP.atom.Hierarchies()
    mhs.append(mh_chain)
    pdb_opt_state=None #IMP.atom.WritePDBOptimizerState(mhs,"refined_temp_%03d.pdb")
    #todo - return refinement
    refined_sols = IMP.em.local_rigid_fitting(
        rb,rb_refiner,
        IMP.atom.Mass.get_mass_key(),dmap,[],1,3,100)
    #refined_sols = sols
    IMP.core.transform(rb,refined_sols.get_transformation(0))
    IMP.atom.write_pdb(mh_chain,t[0]+t[1]+"_fitted.pdb")
    template_fit_sols.append([
        i,refined_sols.get_transformation(0)*sols.get_transformation(0),1.-refined_sols.get_score(0),t])
    IMP.core.transform(rb,refined_sols.get_transformation(0).get_inverse())
    IMP.core.transform(rb,sols.get_transformation(0).get_inverse())
#-- sort the results by the cross correlation scores
template_fit_sols=sorted(template_fit_sols,key=lambda fit: fit[2],reverse=True)
#--- write the best fitting score for each template
output=open("score_templates_by_cc.log","w")
output.write('{0:<12}{1:<12}{2:<12} {3:<}\n'.format('name','seq id','cc score','transformation'))
print len(template_fit_sols[0])
print template_fit_sols[0]
for i,trans,score,t in template_fit_sols:
    rot=trans.get_rotation().get_quaternion()
    v=trans.get_translation()
    pretty_trans='{0:3.6f} {1:3.6f} {2:3.6f} {3:3.6f} {4:3.6f} {5:3.6f} {6:3.6f}\n'.format(
        rot[0],rot[1],rot[2],rot[3],v[0],v[1],v[2])
    output.write('{0:<12}{1:<12}{2:<12} {3:<}\n'.format(
        t[0]+t[1],
        seq_ids[i],
        score,
        pretty_trans))
output.close()
