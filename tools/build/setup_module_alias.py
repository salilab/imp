#!/usr/bin/env python

"""
   Make aliases for the kernel and base module headers and Python code.
"""

import tools
import os
import glob

def get_header_contents(incdir, fname):
    return """#include <IMP/kernel_config.h>

IMPKERNEL_DEPRECATED_HEADER(2.5, "Use top-level IMP namespace directly");
#include <%(incdir)s/%(fname)s>
""" % locals()

def alias_headers(fromdir, kerneldir, basedir, incdir,
                  kernel_headers, base_renames):
    kernel_headers = dict.fromkeys(kernel_headers)
    for g in glob.glob(os.path.join(fromdir, '*.h')):
        if "Include all non-deprecated headers" not in open(g).read():
            fname = os.path.basename(g)
            contents = get_header_contents(incdir, fname)
            if fname in kernel_headers:
                tools.rewrite(os.path.join(kerneldir, fname), contents)
            else:
                to_name = base_renames.get(fname, fname)
                tools.rewrite(os.path.join(basedir, to_name), contents)

def main():
    alias_headers(os.path.join('include', 'IMP'),
                  os.path.join('include', 'IMP', 'kernel'),
                  os.path.join('include', 'IMP', 'base'),
                  'IMP', [ 'AttributeOptimizer.h', 'base_types.h',
         'Configuration.h', 'ConfigurationSet.h', 'constants.h',
         'Constraint.h', 'container_base.h', 'container_macros.h',
         'Decorator.h', 'decorator_macros.h', 'dependency_graph.h',
         'DerivativeAccumulator.h', 'doxygen.h', 'FloatIndex.h',
         'functor.h', 'generic.h', 'input_output.h', 'io.h', 'Key.h',
         'macros.h', 'Model.h', 'ModelObject.h', 'model_object_helpers.h',
         'Optimizer.h', 'OptimizerState.h', 'Particle.h', 'particle_index.h',
         'ParticleTuple.h', 'python_only.h', 'Refiner.h', 'Restraint.h',
         'RestraintSet.h', 'Sampler.h', 'scoped.h', 'ScoreAccumulator.h',
         'ScoreState.h', 'ScoringFunction.h', 'UnaryFunction.h',
         'Undecorator.h', 'utility.h' ],
         {'base_utility.h': 'utility.h' })
    alias_headers(os.path.join('include', 'IMP', 'internal'),
                  os.path.join('include', 'IMP', 'kernel', 'internal'),
                  os.path.join('include', 'IMP', 'base', 'internal'),
                 'IMP/internal', [ 'AccumulatorScoreModifier.h',
         'AttributeTable.h', 'attribute_tables.h', 'constants.h',
         'ContainerConstraint.h', 'container_helpers.h', 'ContainerRestraint.h',
         'ContainerScoreState.h', 'create_decomposition.h',
         'DynamicListContainer.h', 'evaluate_utility.h', 'ExponentialNumber.h',
         'functors.h', 'graph_utility.h', 'IndexingIterator.h',
         'input_output_exception.h', 'key_helpers.h', 'ListLikeContainer.h',
         'NestedIterator.h', 'pdb.h', 'PrefixStream.h',
         'restraint_evaluation.h', 'RestraintsScoringFunction.h',
         'scoring_functions.h', 'static.h', 'StaticListContainer.h', 'swig.h',
         'swig_helpers.h', 'TupleConstraint.h', 'TupleRestraint.h',
         'Unit.h', 'units.h', 'utility.h' ],
         {'base_graph_utility.h': 'graph_utility.h',
          'base_static.h': 'static.h',
          'swig_base.h': 'swig.h',
          'swig_helpers_base.h': 'swig_helpers.h'})
    tools.rewrite(os.path.join('include', 'IMP', 'base', 'base_config.h'),
                  get_header_contents('IMP', 'kernel_config.h'))
    tools.link(os.path.join('include', 'IMP.h'),
               os.path.join('include', 'IMP', 'kernel.h'))
    tools.link(os.path.join('include', 'IMP.h'),
               os.path.join('include', 'IMP', 'base.h'))
    for mod in ('base', 'kernel'):
        subdir = os.path.join('lib', 'IMP', mod)
        if not os.path.exists(subdir):
            os.mkdir(subdir)
        pymod = os.path.join(subdir, '__init__.py')
        with open(pymod, 'w') as fh:
            fh.write("""import sys
sys.stderr.write('IMP.%s is deprecated - use "import IMP" instead\\n')
from IMP import *
""" % mod)

if __name__ == '__main__':
    main()
