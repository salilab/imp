/**
 * \file IMP/em/EnvelopeScore.h
 * \brief class for envelope based scoring using MapDistanceTransform
 *
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_ENVELOPE_SCORE_H
#define IMPEM_ENVELOPE_SCORE_H

#include <IMP/em/em_config.h>
#include <IMP/em/MapDistanceTransform.h>
#include <IMP/Object.h>

#include <vector>

IMPEM_BEGIN_NAMESPACE

//! class for envelope based scoring using MapDistanceTransform
class IMPEMEXPORT EnvelopeScore : public IMP::Object {
 public:
  // init with MapDistanceTransform
  EnvelopeScore(const MapDistanceTransform* mdt);

  /** check for penetration with respect to the envelope as defined
      by the distance transform
      \param[in] points check this set of points for penetration
      \param[in] penetration_thr threshold that defines penetration.
      Use penetration_thr=0 to require for all points inside the envelope
      to allow some penetration set the threshold to the -distance of
      the allowed penetration distance. Recommended value is map
      resolution.
  */
  bool is_penetrating(const IMP::algebra::Vector3Ds& points,
                      float penetration_thr) const;

  /** same as above
      \param[in] points
      \param[in] trans apply this transformation on points before
      checking for penetrations
      \param[in] penetration_thr
  */
  bool is_penetrating(const IMP::algebra::Vector3Ds& points,
                      const IMP::algebra::Transformation3D& trans,
                      float penetration_thr) const;

  /**
      score points with respect to the distance transform
      the score awards points that fall into the envelope
      and penalizes for points outside the envelope
  */
  double score(const IMP::algebra::Vector3Ds& points) const;

  /** same as above
      \param[in] points
      \param[in] trans apply this transformation on points before scoring
  */
  double score(const IMP::algebra::Vector3Ds& points,
               const IMP::algebra::Transformation3D& trans) const;

  // methods required by Object
  IMP::VersionInfo get_version_info() const IMP_OVERRIDE {
    return IMP::VersionInfo(get_module_name(), get_module_version());
  }

  ~EnvelopeScore() {}

 private:
  unsigned int find_range(float distance) const {
    for (unsigned int i = 0; i < ranges_.size(); i++)
      // Add a small delta to handle floating point 'equals'
      if (distance <= ranges_[i] + 1e-5) return i;
    return ranges_.size();
  }

 private:
  const MapDistanceTransform* mdt_;
  std::vector<float> ranges_;
  std::vector<int> weights_;
  std::vector<long> inner_voxels_;
  std::vector<long> neighbors_;
};

IMPEM_END_NAMESPACE

#endif /* IMPEM_ENVELOPE_SCORE_H */
