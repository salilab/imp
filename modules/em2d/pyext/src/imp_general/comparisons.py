

import IMP
import IMP.algebra as alg
import IMP.core as core
import IMP.em as em
import IMP.atom as atom
import IMP.em2d.imp_general.alignments as alignments
import IMP.em2d.imp_general.representation as representation

import random
import itertools
import math
import csv
import itertools
import logging
log = logging.getLogger("comparisons")


def get_coordinates(hierarchy):
    xyz = [core.XYZ(l) for l in atom.get_leaves(hierarchy)]
    coords = [x.get_coordinates() for x in xyz]
    return coords


def get_assembly_placement_score(assembly, native_assembly, align=False):
    """
        Computes the placement score of an assembly respect to the native_assembly.
        @param assembly.  An atom.Molecule object
        @param native_assembly  An atom.Molecule object
        @param align If True, assembly is aligned to native_assembly before
                    calculating the placement score
    """

    distances, angles = get_components_placement_scores(assembly,
                                                        native_assembly, align)
    n = 1. * len(distances)
    return sum(distances) / n, sum(angles) / n


def get_components_placement_scores(assembly, native_assembly, align=False):
    """
        Compute the placement score of each of the children of an assembly.
        The function does not do any time of alignment of the coordinates
        @param assembly An atom.Molecule object
        @param native_assembly An atom.Molecule object with the native conformation
                                Obviously the atoms in assembly and native assembly
                                must be the same
        @return The function returns 2 lists. The first list contains the
                placement distances of the children. The second list contains the
                placnement angles
    """
    model_coords_per_child = [get_coordinates(c)
                              for c in assembly.get_children()]
    native_coords_per_child = [get_coordinates(c)
                               for c in native_assembly.get_children()]
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
    for model_coords, native_coords in zip(
            model_components_coords, native_components_coords):
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
    native_centroid = alg.get_centroid(native_coords)
    model_centroid = alg.get_centroid(model_coords)
    translation_vector = native_centroid - model_centroid
    distance = translation_vector.get_magnitude()
    if(len(model_coords) != len(native_coords)):
        raise ValueError(
            "Mismatch in the number of members %d %d " % (
                len(model_coords),
                len(native_coords)))
    TT = alg.get_transformation_aligning_first_to_second(model_coords,
                                                         native_coords)
    P = alg.get_axis_and_angle(TT.get_rotation())
    angle = P.second
    return distance, angle


def get_rmsd(hierarchy1, hierarchy2):
    xyz1 = [core.XYZ(l) for l in atom.get_leaves(hierarchy1)]
    xyz2 = [core.XYZ(l) for l in atom.get_leaves(hierarchy2)]
    return atom.get_rmsd(xyz1, xyz2)


def get_ccc(native_assembly, assembly, resolution, voxel_size,
            threshold, write_maps=False):
    """
        Threshold - threshold used for the map of the native assembly. Pixels
        with values above this threshold in the native map are used for the
        calculation of the cross_correlation_coefficient
    """
    import IMP.em as em
    particles_native = atom.get_leaves(native_assembly)
    particles_solution = atom.get_leaves(assembly)
    bb_native = core.get_bounding_box(core.XYZs(particles_native))
    bb_solution = core.get_bounding_box(core.XYZs(particles_solution))
    # bounding box enclosing both the particles of the native assembly
    #  and the particles of the model
    bb_union = alg.get_union(bb_native, bb_solution)
    # add border of 4 voxels
    border = 4 * voxel_size
    bottom = bb_union.get_corner(0)
    bottom += alg.Vector3D(-border, -border, -border)
    top = bb_union.get_corner(1)
    top += alg.Vector3D(border, border, border)
    bb_union = alg.BoundingBox3D(bottom, top)

    mrw = em.MRCReaderWriter()
    header = em.create_density_header(bb_union, voxel_size)
    header.set_resolution(resolution)

    map_native = em.SampledDensityMap(header)
    map_native.set_particles(particles_native)
    map_native.resample()

    map_solution = em.SampledDensityMap(header)
    map_solution.set_particles(particles_solution)
    map_solution.resample()

    if(write_maps):
        em.write_map(map_solution, "map_solution.mrc", mrw)
        em.write_map(map_native, "map_native.mrc", mrw)
    map_native.calcRMS()
    map_solution.calcRMS()
    coarse_cc = em.CoarseCC()
    # base the calculation of the cross_correlation coefficient on the threshold]
    # for the native map, because the threshold for the map of the model changes
    # with each model
    threshold = 0.25  # threshold AFTER normalization using calcRMS()
    ccc = coarse_cc.cross_correlation_coefficient(map_solution,
                                                  map_native, threshold)
    log.debug("cross_correlation_coefficient (based on native_map "
              "treshold %s) %s", threshold, ccc)
    return ccc


def get_drms_for_backbone(assembly, native_assembly):
    """
        Measure the DRMS ob the backbone between two assemblies.
        @param assembly The DRMS is computed for this assembly
        @param native_assembly The assembly that acts as a reference

       Notes:
          1) The components of the assembly can be proteins or nucleic acids
          2) If a protein, the c-alphas are used for calculating the drms
          3) If a nucleic acid, the backbone of C4' atoms is used
          4) The chains are treated as rigid bodies to speed the calculation.

        WARNING: if the function fails with a segmentation fault, one of the
        possible problems is that IMP reads some HETATM as calphas. Check that
        the chain does not have heteroatoms.
    """
    log.debug("Measuring DRMS of the backbone")
    begin_range = 0
    ranges = []
    backbone = []
    h_chains = atom.get_by_type(assembly, atom.CHAIN_TYPE)
    for h in h_chains:
        atoms = representation.get_backbone(h)
        """"
        for a in atoms:
            print "atom ===> ",
            at = atom.Atom(a)
            hr = at.get_parent()
            res = atom.Residue(hr)
            ch = atom.Chain(h)
            ch.show()
            print " - ",
            res.show()
            print " - ",
            at.show()
            print ""
        """
        backbone.extend(atoms)
        end_range = begin_range + len(atoms)
        ranges.append((begin_range, end_range))
        begin_range = end_range
    log.debug("Ranges %s number of atoms %s", ranges, len(backbone))
    xyzs = [core.XYZ(l) for l in backbone]
    native_chains = atom.get_by_type(native_assembly, atom.CHAIN_TYPE)
    names = [atom.Chain(ch).get_id() for ch in native_chains]
    native_backbone = []
    for h in native_chains:
        native_backbone.extend(representation.get_backbone(h))
    native_xyzs = [core.XYZ(l) for l in native_backbone]
    if len(xyzs) != len(native_xyzs):
        raise ValueError(
            "Cannot compute DRMS for sets of atoms of different size")
    log.debug("Getting rigid bodies rmsd")
    drms = atom.get_rigid_bodies_drms(xyzs, native_xyzs, ranges)
    if drms < 0 or math.isnan(drms):  # or drms > 100:
        log.debug(
            "len(xyzs) = %s. len(native_xyzs) = %s",
            len(xyzs),
            len(native_xyzs))
        log.debug("drms = %s", drms)
        atom.write_pdb(assembly, "drms_model_calphas.pdb")
        atom.write_pdb(native_assembly, "drms_native_calphas.pdb")
        raise ValueError("There is a problem with the drms. I wrote the pdbs "
                         "for you: drms_model_calphas.pdb drms_native_calphas.pdb")
    return drms
