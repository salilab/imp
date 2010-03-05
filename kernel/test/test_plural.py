import unittest
import IMP
import IMP.test

plural_exceptions=['Decorator', 'WarningContext', "SetLogTarget", "RefCounted",
            "ScopedFailureHandler", "RandomNumberGenerator", "FloatKeyBase",
            "IntKeyBase", "StringKeyBase","ParticleKeyBase", "ObjectKeyBase",
            "ParticlesList", "FailureHandlerBase", "IncreaseIndent",
            "ScoreStatePointer", "SingletonFunctor", "PairFunctor",
            "TripletFunctor", "QuadFunctor"]

show_exceptions=["IntRange", "DerivativeAccumulator", "TextInput", "TextOutput"]

class DirectoriesTests(IMP.test.TestCase):

    def test_data_directory(self):
        """Test all objects have plural version"""
        all= dir(IMP)
        not_found=[]
        for f in dir(IMP):
            if f[0].upper()== f[0] and f[1].upper() != f[1]\
                    and  "_" not in f and not f.endswith("_swigregister")\
                    and f not in plural_exceptions and not f.endswith("s")\
                    and not f.endswith("Temp") and not f.endswith("Iterator")\
                    and not f.endswith("Exception"):
                print f
                print f+"s"
                if f+"s" not in dir(IMP):
                    not_found.append(f)
        print "not found:"
        print not_found
        self.assert_(len(not_found) == 0)

    def test_2(self):
        """Test all objects have show"""
        all= dir(IMP)
        not_found=[]
        for f in dir(IMP):
            if f[0].upper()== f[0] and f[1].upper() != f[1]\
                    and  "_" not in f and not f.endswith("_swigregister")\
                    and f not in plural_exceptions and not f.endswith("s")\
                    and not f.endswith("Temp") and not f.endswith("Iterator")\
                    and not f.endswith("Exception"):
                print f
                if "show" not in eval("dir(IMP."+f+")"):
                    print "bad"
                    not_found.append(f)
        print "not found:"
        print not_found
        self.assert_(len(not_found) == 0)

if __name__ == '__main__':
    unittest.main()
