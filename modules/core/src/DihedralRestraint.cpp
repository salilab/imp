/**
 *  \file DihedralRestraint.cpp \brief Dihedral restraint between four
 *                                     particles.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/DihedralRestraint.h>
#include <IMP/core/DihedralQuadScore.h>

IMPCORE_BEGIN_NAMESPACE

DihedralRestraint::DihedralRestraint(Model *m, UnaryFunction* score_func,
                    ParticleIndexAdaptor p1,
                    ParticleIndexAdaptor p2,
                    ParticleIndexAdaptor p3,
                    ParticleIndexAdaptor p4)
    : QuadRestraint(m, new DihedralQuadScore(score_func),
                    ParticleIndexQuad(p1, p2, p3, p4),
                    "DihedralRestraint%1%") {}

IMP_OBJECT_SERIALIZE_IMPL(IMP::core::DihedralRestraint);

IMPCORE_END_NAMESPACE
