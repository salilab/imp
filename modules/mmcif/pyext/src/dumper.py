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


class _SiteDumper(_Dumper):
    """Dump model coordinates (atom_site, sphere_obj_site, ...)"""
    def dump(self, system, writer):
        self.dump_spheres(system, writer)
        # todo: support atoms (atom_site) too

    def all_frames(self, system):
        """Yield all frames in the system"""
        for state in system._states:
            for frame in state._frames:
                state._load_frame(frame)
                yield state, frame

    def _get_structure_particles(self, system, state):
        """Yield all particles in the state"""
        for h in state.hiers:
            chains = [IMP.atom.Chain(c)
                      for c in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)]
            for chain in chains:
                comp = system.components[chain]
                for p in system._get_structure_particles(chain):
                    yield comp, p

    def dump_spheres(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_sphere_obj_site",
                         ["ordinal_id", "entity_id", "seq_id_begin",
                          "seq_id_end", "asym_id", "Cartn_x",
                          "Cartn_y", "Cartn_z", "object_radius", "rmsf",
                          "model_id"]) as l:
            for state, frame in self.all_frames(system):
                for comp, p in self._get_structure_particles(system, state):
                    if isinstance(p, IMP.atom.Fragment):
                        resinds = p.get_residue_indexes()
                        # todo: handle non-contiguous fragments
                        sbegin = resinds[0]
                        send = resinds[-1]
                    else: # residue
                        sbegin = send = p.get_index()
                    xyzr = IMP.core.XYZR(p)
                    xyz = xyzr.get_coordinates()
                    l.write(ordinal_id=ordinal, entity_id=comp.entity._id,
                            seq_id_begin=sbegin,
                            seq_id_end=send, asym_id=comp.asym_id,
                            Cartn_x=xyz[0], Cartn_y=xyz[1],
                            Cartn_z=xyz[2], object_radius = xyzr.get_radius(),
                            model_id=frame.id)
                    ordinal += 1
