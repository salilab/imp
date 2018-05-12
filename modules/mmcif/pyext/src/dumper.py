"""@namespace IMP.mmcif.dumper
   @brief Utility classes to dump out information in mmCIF format.
"""

from __future__ import print_function
import ihm.location
import IMP.atom
import IMP.mmcif.data
import IMP.mmcif.restraint
import operator
import os

class _Dumper(object):
    """Base class for helpers to dump output to mmCIF"""
    def __init__(self):
        pass
    def finalize_metadata(self, system):
        pass
    def finalize(self, system):
        pass


class _EM3DDumper(_Dumper):
    def dump(self, system, writer):
        rs = []
        # todo: restraints should really be per-system (same restraint might
        # be used by multiple states)
        for state in system._states:
            rs.extend(r for r in state._wrapped_restraints
                   if isinstance(r, IMP.mmcif.restraint._GaussianEMRestraint))
        ordinal = 1
        with writer.loop("_ihm_3dem_restraint",
                         ["ordinal_id", "dataset_list_id", "fitting_method",
                          "struct_assembly_id",
                          "number_of_gaussians", "model_id",
                          "cross_correlation_coefficient"]) as l:
            for r in rs:
                for frame, info in r._all_frame_info():
                    ccc = info['cross correlation']
                    l.write(ordinal_id=ordinal, dataset_list_id=r.dataset._id,
                            fitting_method=r.fitting_method,
                            number_of_gaussians=r.number_of_gaussians
                                       if r.number_of_gaussians is not None
                                       else writer.unknown,
                            model_id=frame.id,
                            cross_correlation_coefficient=ccc)
                    ordinal += 1
