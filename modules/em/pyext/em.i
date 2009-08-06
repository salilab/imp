%module(directors="1") "IMP.em"

%{
#include "IMP.h"
#include "IMP/algebra.h"
#include "IMP/core.h"
#include "IMP/em.h"
#include "IMP/algebra.h"
#include "IMP/core.h"
#include "IMP/atom.h"
%}

%include "IMP_macros.i"
%include "IMP_exceptions.i"
%include "IMP_streams.i"
%include "IMP_refcount.i"

/* Ignore things to prevent SWIG warning about them */
namespace IMP::em {
  %ignore DensityMap::operator =;
}
%ignore operator<<(std::ostream&, const DensityHeader &);
%ignore operator<<(std::ostream&, const EMHeader &);
%ignore operator<<(std::ostream&, const MRCHeader &);

%include "modules/em/pyext/em_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Make selected classes extensible in Python */
%import "IMP_directors.i"
IMP_DIRECTOR_MODULE_CLASS(em, ParticlesAccessPoint);
IMP_DIRECTOR_MODULE_CLASS(em, IMPParticlesAccessPoint);

IMP_REFCOUNT_RETURN_SINGLE(IMP::em::DensityMap)

/* Get definitions of kernel and module base classes (but do not wrap) */
%import "kernel.i"
/* Get definitions of algebra base classes (but do not wrap) */
%import "algebra.i"
%import "core.i"
%import "atom.i"

/* Wrap our own classes */
%include "IMP/em/def.h"
%include "IMP/em/DensityHeader.h"
%include "IMP/em/MapReaderWriter.h"
%include "IMP/em/DensityMap.h"
%include "IMP/em/EMReaderWriter.h"
%include "IMP/em/XplorReaderWriter.h"
%include "IMP/em/MRCReaderWriter.h"
%include "IMP/em/KernelParameters.h"
%include "IMP/em/ParticlesAccessPoint.h"
%include "IMP/em/SampledDensityMap.h"
%include "IMP/em/CoarseCC.h"
%include "IMP/em/CoarseConvolution.h"
%include "IMP/em/CoarseCCatIntervals.h"
%include "IMP/em/SurfaceShellDensityMap.h"
%include "IMP/em/IMPParticlesAccessPoint.h"
%include "IMP/em/FitRestraint.h"
%include "IMP/em/converters.h"
%include "IMP/em/project.h"
%include "IMP/em/SpiderHeader.h"
%include "IMP/em/ImageHeader.h"
%include "IMP/em/ImageReaderWriter.h"
%include "IMP/em/SpiderReaderWriter.h"
%include "IMP/em/Volume.h"
%include "IMP/em/Image.h"
%include "IMP/em/noise.h"
%include "IMP/em/filters.h"
%include "IMP/em/rigid_fitting.h"

/* Allow runtime casting of Restraint* objects to FitRestraint* */
namespace IMP {
  namespace em {
    %extend FitRestraint {
      static FitRestraint* cast(Restraint *r) {
        return dynamic_cast<IMP::em::FitRestraint *>(r);
      }
    }
    %template(project_given_rotation1) 
                  ::IMP::em::project_given_rotation1<double>;
    %template(project_given_direction1) 
                  ::IMP::em::project_given_direction1<double>;
    %template(_ImageReaderWriter) ::IMP::em::ImageReaderWriter<double>;
    %template(_SpiderImageReaderWriter)
                          ::IMP::em::SpiderImageReaderWriter<double>;
    %template(_Image) ::IMP::em::Image<double>;
    %template(_Volume) ::IMP::em::Volume<double>;
    %template(floats) ::std::vector<float>;

    %template(add_noise) ::IMP::em::add_noise<::IMP::algebra::Matrix2D<double> >;
    %template(add_noise_d) 
              ::IMP::em::add_noise<::IMP::algebra::Matrix2D<double> >;

    %template(FilterByThreshold2D) ::IMP::em::FilterByThreshold<double,2>;
    %template(FilterByThreshold3D) ::IMP::em::FilterByThreshold<double,3>;
  }
}

// Cannot instantiate template classes directly, since SWIG complains about
// attempts to redefine the template classes; so rename them in the Python layer
%pythoncode {
  ImageReaderWriter = _ImageReaderWriter
  SpiderImageReaderWriter = _SpiderImageReaderWriter
  Image = _Image
  Volume = _Volume
}
