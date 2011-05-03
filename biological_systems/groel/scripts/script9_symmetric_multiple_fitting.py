import sys,os
commands=[]
model_fn="data/templates/1iokA.pdb"
dens_fn=["output/groel-11.5A.bottom.mrc","output/groel-11.5A.top.mrc"]
resolution=11.5
spacing=2.7
threshold=1.3
map_origin_x=[-135,-135]
map_origin_y=[-135,-135]
map_origin_z=[2.7,-135]
param_fn=["output/multifit.bottom.param","output/multifit.top.param"]
output_fn=["output/multifit.bottom.output","output/multifit.top.output"]
sol_model_fn=["output/model.bottom","output/model.top"]
symm_deg=7
#1. generate a surface file of the model
command1 = "perl $IMP/modules/cn_multifit/bin/runMSPoints.pl "+model_fn
os.system(command1)
#generate parameter file for the two halfs
#generate a parameter file
for i in range(2):
    command="$IMP/tools/imppy.sh python $IMP/modules/cn_multifit/bin/build_cn_multifit_params.py " +" -o " + output_fn[i]+ " -p " + param_fn[i] + " -m " + sol_model_fn[i] +" -- 7 "+ model_fn + " " + dens_fn[i] + " " + str(resolution) + " " + str(spacing)+ " " + str(threshold) + " " + str(map_origin_x[i]) + " " + str(map_origin_y[i]) + " " + str(map_origin_z[i])
    print command
    os.system(command)
#run the modeling procedure
for i in range(2):
    command="$IMP/tools/imppy.sh $IMP/modules/cn_multifit/bin/symmetric_multifit " + param_fn[i]
    print command
    os.system(command)
