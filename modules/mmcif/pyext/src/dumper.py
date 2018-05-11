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


class _ModelRepresentationDumper(_Dumper):
    def _get_granularity(self, r):
        if isinstance(r.particles[0], IMP.atom.Residue):
            return 'by-residue'
        else:
            return 'by-feature'

    def dump(self, system, writer):
        ordinal_id = 1
        segment_id = 1
        with writer.loop("_ihm_model_representation",
                         ["ordinal_id", "representation_id",
                          "segment_id", "entity_id", "entity_description",
                          "entity_asym_id",
                          "seq_id_begin", "seq_id_end",
                          "model_object_primitive", "starting_model_id",
                          "model_mode", "model_granularity",
                          "model_object_count"]) as l:
            for comp in system.components.get_all_modeled():
                # For now, assume that representation of the same-named
                # component is the same in all states, so just take the first
                state = list(system._states.keys())[0]
                for r in state.representation[comp]:
                    # todo: handle multiple representations
                    l.write(ordinal_id=ordinal_id, representation_id=1,
                            segment_id=segment_id, entity_id=comp.entity._id,
                            entity_description=comp.entity.description,
                            entity_asym_id=comp.asym_id,
                            seq_id_begin=r.residue_range[0],
                            seq_id_end=r.residue_range[1],
                            model_object_primitive=r.primitive,
                            starting_model_id=r.starting_model._id
                                                  if r.starting_model
                                                  else writer.omitted,
                            model_mode='rigid' if r.rigid_body else 'flexible',
                            model_granularity=self._get_granularity(r),
                            model_object_count=len(r.particles))
                    ordinal_id += 1
                    segment_id += 1


class _ProtocolDumper(_Dumper):
    class _Step(object):
        name = 'Sampling' # todo: support 'refinement' too

        def __init__(self, s):
            self.orig_s = s
        def __get_method(self):
            first_prov = self.orig_s._prov[0]
            meth = first_prov.get_method()
            if first_prov.get_number_of_replicas() > 1:
                meth = "Replica exchange " + meth
            return meth

        num_models_begin = property(lambda self: self.orig_s.num_models_begin)
        num_models_end = property(lambda self: self.orig_s.num_models_end)
        id = property(lambda self: self.orig_s.id)
        method = property(__get_method)

    def dump(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_modeling_protocol",
                         ["ordinal_id", "protocol_id", "step_id",
                          "struct_assembly_id", "dataset_group_id",
                          "struct_assembly_description", "protocol_name",
                          "step_name", "step_method", "num_models_begin",
                          "num_models_end", "multi_scale_flag",
                          "multi_state_flag", "time_ordered_flag"]) as l:
            for p in system.protocols.get_all():
                for s in p._steps:
                    step = self._Step(s)
                    l.write(ordinal_id=ordinal, protocol_id=p.id,
                            step_id=step.id, step_method=step.method,
                            step_name=step.name,
                            struct_assembly_id=p.modeled_assembly._id,
                            num_models_begin=step.num_models_begin,
                            num_models_end=step.num_models_end,
                            # todo: support multiple states, time ordered
                            multi_state_flag=False, time_ordered_flag=False,
                            # todo: revisit assumption all models are multiscale
                            multi_scale_flag=True)
                    ordinal += 1


class _PostProcessDumper(_Dumper):
    class _PostProc(object):
        def __init__(self, pp):
            self._pp = pp
        def __get_type(self):
            type_map = {IMP.core.ClusterProvenance: 'cluster',
                        IMP.core.FilterProvenance: 'filter'}
            return type_map[type(self._pp._prov)]
        def __get_feature(self):
            type_map = {IMP.core.ClusterProvenance: 'RMSD',
                        IMP.core.FilterProvenance: 'energy/score'}
            return type_map[type(self._pp._prov)]

        num_models_begin = property(lambda self: self._pp.num_models_begin)
        num_models_end = property(lambda self: self._pp.num_models_end)
        id = property(lambda self: self._pp.id)
        type = property(__get_type)
        feature = property(__get_feature)

    def dump(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_modeling_post_process",
                         ["id", "protocol_id", "analysis_id", "step_id",
                          "type", "feature", "num_models_begin",
                          "num_models_end"]) as l:
            # todo: handle multiple analyses
            for prot in system.protocols.get_all():
                for pp in prot._postprocs:
                    p = self._PostProc(pp)
                    l.write(id=ordinal, protocol_id=prot.id, analysis_id=1,
                            step_id=p.id, type=p.type, feature=p.feature,
                            num_models_begin=p.num_models_begin,
                            num_models_end=p.num_models_end)
                    ordinal += 1


class _EnsembleDumper(_Dumper):
    def dump(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_ensemble_info",
                         ["ensemble_id", "ensemble_name", "post_process_id",
                          "model_group_id", "ensemble_clustering_method",
                          "ensemble_clustering_feature", "num_ensemble_models",
                          "num_ensemble_models_deposited",
                          "ensemble_precision_value",
                          "ensemble_file_id"]) as l:
            for ens in system._ensembles:
                l.write(ensemble_id=ens.id, ensemble_name=ens.name,
                        model_group_id=ens.id,
                        num_ensemble_models=len(ens._frames))


class _ModelListDumper(_Dumper):
    def dump(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_model_list",
                         ["ordinal_id", "model_id", "model_group_id",
                          "model_name", "model_group_name", "assembly_id",
                          "protocol_id", "representation_id"]) as l:
            for ens in system._ensembles:
                for frame in ens._frames:
                    # todo: handle multiple representations
                    l.write(ordinal_id=ordinal, model_id=frame.id,
                            model_group_id=ens.id, model_name=frame.name,
                            model_group_name=ens.name,
                            representation_id=1)
                    ordinal += 1


class _EM3DDumper(_Dumper):
    def dump(self, system, writer):
        rs = []
        # todo: restraints should really be per-system (same restraint might
        # be used by multiple states)
        for state in system._states.keys():
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
        for state in system._states.keys():
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
