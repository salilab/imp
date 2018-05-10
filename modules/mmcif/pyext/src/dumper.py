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
                            starting_model_id=r.starting_model.id
                                                  if r.starting_model
                                                  else writer.omitted,
                            model_mode='rigid' if r.rigid_body else 'flexible',
                            model_granularity=self._get_granularity(r),
                            model_object_count=len(r.particles))
                    ordinal_id += 1
                    segment_id += 1

class _MSESeqDif(object):
    """Track an MSE -> MET mutation in the starting model sequence"""
    comp_id = 'MET'
    db_comp_id = 'MSE'
    details = 'Conversion of modified residue MSE to MET'
    def __init__(self, res, comp, source, model):
        self.res, self.comp, self.source, self.model = res, comp, source, model
        self.model = model


class _StartingModelDumper(_Dumper):
    def finalize(self, system):
        for comp in system.components.get_all_modeled():
            for n, starting_model in enumerate(self._all_models(system, comp)):
                starting_model.id = '%s-m%d' % (comp.name, n + 1)

    def _all_models(self, system, comp):
        """Get the set of all starting models for a component"""
        for state in system._states.keys():
            seen_models = {}
            for rep in state.representation.get(comp, []):
                sm = rep.starting_model
                if sm and sm not in seen_models:
                    seen_models[sm] = None
                    yield sm

    def dump(self, system, writer):
        self.dump_details(system, writer)
        self.dump_comparative(system, writer)
        seq_dif = self.dump_coords(system, writer)
        self.dump_seq_dif(writer, seq_dif)

    def dump_details(self, system, writer):
        with writer.loop("_ihm_starting_model_details",
                     ["starting_model_id", "entity_id", "entity_description",
                      "asym_id", "seq_id_begin",
                      "seq_id_end", "starting_model_source",
                      "starting_model_auth_asym_id",
                      "starting_model_sequence_offset",
                      "dataset_list_id"]) as l:
            for comp in system.components.get_all_modeled():
                for sm in self._all_models(system, comp):
                    seq_id_begin, seq_id_end = sm.get_seq_id_range_all_sources()
                    l.write(starting_model_id=sm.id, entity_id=comp.entity._id,
                            entity_description=comp.entity.description,
                            asym_id=comp.asym_id,
                            seq_id_begin=seq_id_begin,
                            seq_id_end=seq_id_end,
                            starting_model_source=sm.sources[0].source,
                            starting_model_auth_asym_id=sm.chain_id,
                            dataset_list_id=sm.dataset._id,
                            starting_model_sequence_offset=sm.offset)

    def dump_comparative(self, system, writer):
        """Dump details on comparative models."""
        with writer.loop("_ihm_starting_comparative_models",
                     ["ordinal_id", "starting_model_id",
                      "starting_model_auth_asym_id",
                      "starting_model_seq_id_begin",
                      "starting_model_seq_id_end",
                      "template_auth_asym_id", "template_seq_id_begin",
                      "template_seq_id_end", "template_sequence_identity",
                      "template_sequence_identity_denominator",
                      "template_dataset_list_id",
                      "alignment_file_id"]) as l:
            ordinal = 1
            for comp in system.components.get_all_modeled():
                for sm in self._all_models(system, comp):
                    for template in [s for s in sm.sources
                          if isinstance(s, IMP.mmcif.metadata._TemplateSource)]:
                        seq_id_begin, seq_id_end = template.get_seq_id_range(sm)
                        l.write(ordinal_id=ordinal,
                          starting_model_id=sm.id,
                          starting_model_auth_asym_id=template.chain_id,
                          starting_model_seq_id_begin=seq_id_begin,
                          starting_model_seq_id_end=seq_id_end,
                          template_auth_asym_id=template.tm_chain_id,
                          template_seq_id_begin=template.tm_seq_id_begin,
                          template_seq_id_end=template.tm_seq_id_end,
                          template_sequence_identity=template.sequence_identity,
                          # Assume Modeller-style sequence identity for now
                          template_sequence_identity_denominator=1,
                          template_dataset_list_id=template.tm_dataset._id
                                                   if template.tm_dataset
                                                   else writer.unknown,
                          alignment_file_id=sm.alignment_file._id
                                            if hasattr(sm, 'alignment_file')
                                            else writer.unknown)
                        ordinal += 1

    def dump_coords(self, system, writer):
        seq_dif = []
        ordinal = 1
        with writer.loop("_ihm_starting_model_coord",
                     ["starting_model_id", "group_PDB", "id", "type_symbol",
                      "atom_id", "comp_id", "entity_id", "asym_id",
                      "seq_id", "Cartn_x",
                      "Cartn_y", "Cartn_z", "B_iso_or_equiv",
                      "ordinal_id"]) as l:
            for comp in system.components.get_all_modeled():
                for sm in self._all_models(system, comp):
                    m, sel = self._read_coords(sm)
                    last_res_index = None
                    for a in sel.get_selected_particles():
                        coord = IMP.core.XYZ(a).get_coordinates()
                        atom = IMP.atom.Atom(a)
                        element = atom.get_element()
                        element = IMP.atom.get_element_table().get_name(element)
                        atom_name = atom.get_atom_type().get_string().strip()
                        group_pdb = 'ATOM'
                        if atom_name.startswith('HET:'):
                            group_pdb = 'HETATM'
                            atom_name = atom_name[4:]
                        res = IMP.atom.get_residue(atom)
                        res_name = res.get_residue_type().get_string()
                        # MSE in the original PDB is automatically mutated
                        # by IMP to MET, so reflect that in the output,
                        # and pass back to populate the seq_dif category.
                        if res_name == 'MSE':
                            res_name = 'MET'
                            # Only add one seq_dif record per residue
                            ind = res.get_index()
                            if ind != last_res_index:
                                last_res_index = ind
                                # This should only happen when we're using
                                # a crystal structure as the source (a
                                # comparative model would use MET in
                                # the sequence)
                                assert(len(sm.sources) == 1)
                                seq_dif.append(_MSESeqDif(res, comp,
                                                          sm.sources[0], sm))
                        l.write(starting_model_id=sm.id,
                                group_PDB=group_pdb,
                                id=atom.get_input_index(), type_symbol=element,
                                atom_id=atom_name, comp_id=res_name,
                                entity_id=comp.entity._id,
                                asym_id=sm.chain_id,
                                seq_id=res.get_index() + sm.offset,
                                Cartn_x=coord[0],
                                Cartn_y=coord[1], Cartn_z=coord[2],
                                B_iso_or_equiv=atom.get_temperature_factor(),
                                ordinal_id=ordinal)
                        ordinal += 1
        return seq_dif

    def dump_seq_dif(self, writer, seq_dif):
        ordinal = 1
        with writer.loop("_ihm_starting_model_seq_dif",
                     ["ordinal_id", "entity_id", "asym_id",
                      "seq_id", "comp_id", "starting_model_id",
                      "db_asym_id", "db_seq_id", "db_comp_id",
                      "details"]) as l:
            for sd in seq_dif:
                l.write(ordinal_id=ordinal, entity_id=sd.comp.entity.id,
                        asym_id=sd.model.chain_id,
                        seq_id=sd.res.get_index(),
                        comp_id=sd.comp_id,
                        db_asym_id=sd.source.chain_id,
                        db_seq_id=sd.res.get_index() - sd.model.offset,
                        db_comp_id=sd.db_comp_id,
                        starting_model_id=sd.model.name,
                        details=sd.details)
                ordinal += 1

    def _read_coords(self, sm):
        """Read the coordinates for a starting model"""
        m = IMP.Model()
        # todo: support reading other subsets of the atoms (e.g. CA/CB)
        slt = IMP.atom.ChainPDBSelector(sm.chain_id) \
                   & IMP.atom.NonWaterNonHydrogenPDBSelector()
        hier = IMP.atom.read_pdb(sm.filename, m, slt)
        sel = IMP.atom.Selection(hier,
                     residue_indexes=list(range(sm.seq_id_begin - sm.offset,
                                                sm.seq_id_end + 1 - sm.offset)))
        return m, sel


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
