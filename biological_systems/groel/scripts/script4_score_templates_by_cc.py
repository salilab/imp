import IMP.em
import IMP.atom
import IMP.multifit
IMP.base.set_log_level(IMP.base.NONE)
#--- parse the templates file
templates_file="output/build_profile.prf"
templates=[]
seq_ids=[]
for line in open(templates_file):
    if line[0]=="#":
        continue
    s=line.split()
    if int(s[0])==1:
        continue
    if not(int(s[4])-int(s[3])>400):
        print "Not including:",s[0],s[1]
        continue
    templates.append([s[1][:4],s[1][-1]])
    seq_ids.append(s[10])
print "number of seq:",len(seq_ids)
templates_dir="data/templates/"
#--- load the target density map
dmap=IMP.em.read_map("output/groel_subunit_11.mrc",IMP.em.MRCReaderWriter())
dmap.get_header_writable().set_resolution(10)
#--- load IMP model
mdl=IMP.Model()
rb_refiner=IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
template_fit_sols=[]
best_fit=1.
best_temp=[]
dens_threshold=0.02
#--- iterate over the templates and fit each of them
dmap.show()
for i,t in enumerate(templates):
    print "fitting template "+t[0]+t[1]
    #load the template
    mh=IMP.atom.read_pdb(templates_dir+t[0]+".pdb",mdl)
    #get the right chain
    mh_chain=IMP.atom.get_by_type(mh,IMP.atom.CHAIN_TYPE)[ord(t[1])-ord('A')]
    rb=IMP.atom.setup_as_rigid_body(mh_chain)
    #fit the template to the density map
    sols=IMP.multifit.pca_based_rigid_fitting(rb,rb_refiner,dmap,dens_threshold)
    IMP.core.transform(rb,sols.get_transformation(0))
    #refine the best scoring fit
    mhs=IMP.atom.Hierarchies()
    mhs.append(mh_chain)
    pdb_opt_state=None #IMP.atom.WritePDBOptimizerState(mhs,"refined_temp_%03d.pdb")
    #todo - return refinement
    #refined_sols = IMP.em.local_rigid_fitting(
    #    rb,rb_refiner,
    #    IMP.atom.Mass.get_mass_key(),dmap,[],1,3,100)
    refined_sols = IMP.em.FittingSolutions()
    refined_sols.add_solution(IMP.algebra.get_identity_transformation_3d(),
                              sols.get_score(0))
    IMP.core.transform(rb,refined_sols.get_transformation(0))
    IMP.atom.write_pdb(mh_chain,"output/"+t[0]+t[1]+"_fitted.pdb")
    template_fit_sols.append([
        i,refined_sols.get_transformation(0)*sols.get_transformation(0),1.-refined_sols.get_score(0),t])
    IMP.core.transform(rb,refined_sols.get_transformation(0).get_inverse())
    IMP.core.transform(rb,sols.get_transformation(0).get_inverse())
#-- sort the results by the cross correlation scores
template_fit_sols=sorted(template_fit_sols,key=lambda fit: fit[2],reverse=True)
#--- write the best fitting score for each template
output=open("output/score_templates_by_cc.log","w")
output.write('%(a)-12s%(b)-12s%(c)-12s%(d)-30s\n'%{'a':'name','b':'seq id','c':'cc score','d':'transformation'})
print len(template_fit_sols[0])
print template_fit_sols[0]
for i,trans,score,t in template_fit_sols:
    rot=trans.get_rotation().get_quaternion()
    v=trans.get_translation()
    pretty_trans='%(a)6.3f %(b)6.3f %(c)6.3f %(d)6.3f %(e)6.3f %(f)6.3f %(g)6.3f\n'%{
        'a':rot[0],'b':rot[1],'c':rot[2],'d':rot[3],'e':v[0],'f':v[1],'g':v[2]}
    output.write('%(a)-12s%(b)-12s%(c)-12.3f%(d)-30s\n'%{
        'a':t[0]+t[1], \
        'b':seq_ids[i], \
        'c':score, \
        'd':pretty_trans})
output.close()
