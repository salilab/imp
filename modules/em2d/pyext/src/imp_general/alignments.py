

import IMP
import IMP.domino as domino
import IMP.core as core
import IMP.display as display
import IMP.atom as atom
import IMP.algebra as alg
import IMP.em2d as em2d

import IMP.em2d.imp_general.io as io
import IMP.em2d.imp_general.representation as representation

import sys
import logging


log = logging.getLogger("alignments")


def get_reference_frames_from_chain_alignment(reference_rbs, reference_index,
                                              rbs_to_align, index_to_align):
    """
        Align the rigid bodies rbs_to_align to the the reference frames of
        reference_rbs. The rb with index_to_align is aligned to the reference rb
        with reference_index. The function returns the reference frames to
        apply to the rbs_to_align.
    """
    ref_coords = \
        [m.get_coordinates()
         for m in reference_rbs[reference_index].get_members()]
    coords = [m.get_coordinates()
              for m in rbs_to_align[index_to_align].get_members()]
    if(len(coords) != len(ref_coords)):
        raise ValueError(
            "Mismatch in the number of members. Reference %d Aligned %d " % (
                len(ref_coords), len(coords)))
    T = alg.get_transformation_aligning_first_to_second(coords, ref_coords)
    new_refs = []
    for rb in rbs_to_align:
#        log.debug("aligning ... %s",rb)
        t = rb.get_reference_frame().get_transformation_to()
        new_t = alg.compose(T, t)
        new_refs.append(alg.ReferenceFrame3D(new_t))
    return new_refs


def align_centroids_using_pca(ref_frames, ref_frames_reference):
    """
        Align the centroids of 2 sets of rigid bodyes using PCA using their
        reference frames. returns the best rmsd and the ref_frames to get it.
    """
    if(len(ref_frames) != len(ref_frames_reference)):
        raise ValueError("The number of reference frames must be the same")
    Ts1 = [r.get_transformation_to() for r in ref_frames]
    vs1 = [T.get_translation() for T in Ts1]
    Ts2 = [r.get_transformation_to() for r in ref_frames_reference]
    vs2 = [T.get_translation() for T in Ts2]

    # align with PCA
    pc1 = alg.get_principal_components(vs1)
    pc2 = alg.get_principal_components(vs2)
    pcTs = alg.get_alignments_from_first_to_second(pc1, pc2)
    best_refs = []
    best_rmsd = 1e5
    for j, pcT in enumerate(pcTs):
        new_Ts1 = [alg.compose(pcT, T) for T in Ts1]
        new_vs1 = [T.get_translation() for T in new_Ts1]
        r = atom.get_rmsd(new_vs1, vs2)
        if(r < best_rmsd):
            best_rmsd = r
            best_refs = [alg.ReferenceFrame3D(T) for T in new_Ts1]
    return best_rmsd, best_refs


def get_reference_frames_aligning_rbs(rbs, reference_rbs):
    """ rbs = rigid bodies """
    refs = [rb.get_reference_frame() for rb in rbs]
    ref_refs = [rb.get_reference_frame() for rb in reference_rbs]
    best_rmsd, best_refs = align_centroids_using_pca(refs, ref_refs)
