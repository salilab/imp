import IMP
import IMP.test

plural_exceptions=['Decorator', 'WarningContext', "SetLogTarget", "RefCounted",
            "ScopedFailureHandler", "RandomNumberGenerator", "FloatKeyBase",
            "IntKeyBase", "StringKeyBase","ParticleKeyBase", "ObjectKeyBase",
            "ParticlesList", "FailureHandlerBase", "IncreaseIndent",
            "ScoreStatePointer", "SingletonFunctor", "PairFunctor",
            "TripletFunctor", "QuadFunctor", 'IntPair', 'FloatRange', 'Object',
                   "DependencyGraph", "ScopedRestraint", "ScopedScoreState"]

show_exceptions=["IntRange", "DerivativeAccumulator", "TextInput", "TextOutput",'Decorator', 'IncreaseIndent', 'PairFunctor', 'ParticlesList', 'QuadFunctor', 'RandomNumberGenerator', 'RefCounted', 'ScopedFailureHandler', 'ScoreStatePointer', 'SetLogTarget', 'SingletonFunctor', 'TripletFunctor', 'WarningContext', 'DerivativePair', 'FloatPair', 'FloatRange', 'IntPair', 'ParticlePair', 'ParticleQuad', 'ParticleTriplet', "DependencyGraph"]

class DirectoriesTests(IMP.test.TestCase):

    def test_data_directory(self):
        """Test all kernel objects have plural version"""
        return self.assertPlural(IMP, plural_exceptions)

    def test_2(self):
        """Test all objects have show"""
        return self.assertShow(IMP, show_exceptions)

if __name__ == '__main__':
    IMP.test.main()
