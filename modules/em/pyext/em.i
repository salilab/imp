%module(directors="1") "IMP.em"

%{
#include "IMP.h"
#include "IMP/em.h"
#include "IMP/algebra.h"
#include "IMP/core.h"
#include "IMP/atom.h"
%}

%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"
%include "kernel/pyext/IMP_streams.i"

/* Ignore things to prevent SWIG warning about them */
namespace IMP::em {
  %ignore DensityMap::operator =;
}
%ignore operator<<(std::ostream&, const DensityHeader &);
%ignore operator<<(std::ostream&, const EMHeader &);
%ignore operator<<(std::ostream&, const MRCHeader &);

%include "em_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Make selected classes extensible in Python */
%import "kernel/pyext/IMP_directors.i"
IMP_DIRECTOR_MODULE_CLASS(em, ParticlesAccessPoint);
IMP_DIRECTOR_MODULE_CLASS(em, IMPParticlesAccessPoint);

/* Get definitions of kernel and module base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "modules/algebra/pyext/algebra.i"
%import "modules/core/pyext/core.i"
%import "modules/atom/pyext/atom.i"

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

/* Allow runtime casting of Restraint* objects to FitRestraint* */
namespace IMP {
  namespace em {
    %extend FitRestraint {
      static FitRestraint* cast(Restraint *r) {
        return dynamic_cast<IMP::em::FitRestraint *>(r);
      }
    }

    %template(project) ::IMP::em::project<float>;
    %template(_ImageReaderWriter) ::IMP::em::ImageReaderWriter<float>;
    %template(_SpiderImageReaderWriter)
                          ::IMP::em::SpiderImageReaderWriter<float>;
    %template(_Image) ::IMP::em::Image<float>;
    %template(_Volume) ::IMP::em::Volume<float>;
    %template(floats) ::std::vector<float>;
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
