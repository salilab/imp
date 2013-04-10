#!/usr/bin/env python

__doc__ = "Compare output models to a reference structure."

#analyse the ensemble, first we will do the rmsd stuff
import IMP.multifit
from optparse import OptionParser


def get_placement_scores_from_coordinates(model_components_coords,
                                         native_components_coords):
    """
        Computes the placement score for each of the components
        @param model_components_coords A list with the coordinates for each
                    component
        @param native_components_coords A list with the coordinates for each
                    component in the native assembly
    """
    distances = []
    angles = []
    for model_coords, native_coords  in zip(
                        model_components_coords,native_components_coords):
        distance, angle = get_placement_score_from_coordinates(model_coords,
                                                                native_coords)
        distances.append(distance)
        angles.append(angle)
    return distances, angles


def get_placement_score_from_coordinates(model_coords, native_coords):
    """
        Computes the position error (placement distance) and the orientation
        error (placement angle) of the coordinates in model_coords respect to
        the coordinates in native_coords.
        placement distance - translation between the centroids of the
                            coordinates
        placement angle - Angle in the axis-angle formulation of the rotation
        aligning the two rigid bodies.
    """
    native_centroid = IMP.algebra.get_centroid(native_coords)
    model_centroid = IMP.algebra.get_centroid(model_coords)
    translation_vector = native_centroid - model_centroid
    distance = translation_vector.get_magnitude()
    if(len(model_coords) != len(native_coords) ):
        raise ValueError(
          "Mismatch in the number of members %d %d " % (
                                                len(model_coords),
                                                len(native_coords)) )
    TT = IMP.algebra.get_transformation_aligning_first_to_second(model_coords,
                                                                 native_coords)
    P = IMP.algebra.get_axis_and_angle( TT.get_rotation() )
    angle = P.second
    return distance, angle


def get_rmsd(hierarchy1, hierarchy2):
    xyz1 = [IMP.core.XYZ(l) for l in IMP.atom.get_leaves(hierarchy1)]
    xyz2 = [IMP.core.XYZ(l) for l in IMP.atom.get_leaves(hierarchy2)]
    return IMP.atom.get_rmsd(xyz1, xyz2)



def get_components_placement_scores(assembly, native_assembly, align=False):
    """
        Compute the placement score of each of the children of an assembly.
        @param assembly An atom.Molecule object
        @param native_assembly An atom.Molecule object with the native
                        conformation. Obviously the atoms in assembly and
                        native_assembly must be the same.
        @param if True, the coordinates are aligned before the score is
               calculated.
        @return The function returns 2 lists. The first list contains the
                placement distances of the children. The second list contains
                the placement angles
    """
    model_coords_per_child = [get_coordinates(c) for c in assembly.get_children()]
    native_coords_per_child = [get_coordinates(c) for c in native_assembly.get_children()]
    if align:
        model_coords = []
        nil = [model_coords.extend(x) for x in model_coords_per_child]
        native_coords = []
        nil = [native_coords.extend(x) for x in native_coords_per_child]
        T = alg.get_transformation_aligning_first_to_second(model_coords,
                                                            native_coords)
        # get aligned coordinates
        new_model_coords_per_child = []
        for c in model_coords_per_child:
            coords = [T.get_transformed(x) for x in c]
            new_model_coords_per_child.append(coords)
        model_coords_per_child = new_model_coords_per_child
    distances, angles = get_placement_scores_from_coordinates(
                            native_coords_per_child, model_coords_per_child)
    return distances, angles


def parse_args():
    usage = """%prog [options] <asmb.input> <proteomics.input>
           <mapping.input> <combinations>

Compare output models to a reference structure.
The reference structure for each subunit is read from the rightmost column
of the asmb.input file.
"""
    parser = OptionParser(usage)
    parser.add_option("-m", "--max", type="int", dest="max", default=None,
                      help="maximum number of models to compare")
    (options, args) = parser.parse_args()
    if len(args) != 4:
        parser.error("incorrect number of arguments")
    return options,args

def run(asmb_fn,proteomics_fn,mapping_fn,combs_fn,max_comb):
    #get rmsd for subunits
    mdl=IMP.Model()
    combs=IMP.multifit.read_paths(combs_fn)
    sd=IMP.multifit.read_settings(asmb_fn)
    sd.set_was_used(True)
    prot_data=IMP.multifit.read_proteomics_data(proteomics_fn)
    mapping_data=IMP.multifit.read_protein_anchors_mapping(prot_data,
                                                           mapping_fn)
    ensmb=IMP.multifit.load_ensemble(sd,mdl,mapping_data)
    mhs=ensmb.get_molecules()
    mhs_ref=[]
    for j,mh in enumerate(mhs):
        mhs_ref.append(IMP.atom.read_pdb(sd.get_component_header(j).get_reference_fn(),mdl))
    print "number of combinations:",len(combs),max_comb
    results = []
    for i,comb in enumerate(combs[:max_comb]):
        if i%500==0:
            print i
        ensmb.load_combination(comb)
        scores=[]
        for j,mh in enumerate(mhs):
            mh_ref=mhs_ref[j]
            coords1=[]
            for xyz in IMP.core.XYZs(IMP.core.get_leaves(mh)):
                coords1.append(xyz.get_coordinates())
            coords2=[]
            for xyz in IMP.core.XYZs(IMP.core.get_leaves(mh_ref)):
                coords2.append(xyz.get_coordinates())
            scores.append(get_placement_score_from_coordinates(coords1,coords2))
            #scores=get_placement_score_from_coordinates(IMP.core.XYZs(IMP.atom.get_leaves(mh)),
            #                                             IMP.core.XYZs(IMP.atom.get_leaves(mh_ref)))

        rmsd=get_rmsd(mhs,mhs_ref)
        print i,rmsd,scores
        results.append((rmsd, scores))
        ensmb.unload_combination(comb)
    return results

def main():
    options,args = parse_args()
    return run(args[0],args[1],args[2],args[3],options.max)

if __name__ == "__main__":
    main()
