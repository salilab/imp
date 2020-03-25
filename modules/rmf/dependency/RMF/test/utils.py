import unittest

# If we're using Python 2.6, add in more modern unittest convenience methods
if not hasattr(unittest.TestCase, 'assertIn'):
    def assertIn(self, member, container, msg=None):
        return self.assertTrue(member in container,
                        msg or '%s not found in %s' % (member, container))
    def assertNotIn(self, member, container, msg=None):
        return self.assertTrue(member not in container,
                        msg or '%s unexpectedly found in %s'
                        % (member, container))
    def assertIs(self, a, b, msg=None):
        return self.assertTrue(a is b, msg or '%s is not %s' % (a, b))
    def assertIsInstance(self, obj, cls, msg=None):
        return self.assertTrue(isinstance(obj, cls),
                        msg or '%s is not an instance of %s' % (obj, cls))
    def assertLess(self, a, b, msg=None):
        return self.assertTrue(a < b, msg or '%s not less than %s' % (a, b))
    def assertGreater(self, a, b, msg=None):
        return self.assertTrue(a > b, msg or '%s not greater than %s' % (a, b))
    def assertLessEqual(self, a, b, msg=None):
        return self.assertTrue(a <= b,
                        msg or '%s not less than or equal to %s' % (a, b))
    def assertGreaterEqual(self, a, b, msg=None):
        return self.assertTrue(a >= b,
                        msg or '%s not greater than or equal to %s' % (a, b))
    def assertIsNone(self, obj, msg=None):
        return self.assertTrue(obj is None, msg or '%s is not None' % obj)
    def assertIsNotNone(self, obj, msg=None):
        return self.assertTrue(obj is not None, msg or 'unexpectedly None')
    def assertAlmostEqual(self, first, second, places=None, msg=None,
                          delta=None):
        if first == second:
            return
        if delta is not None and places is not None:
            raise TypeError("specify delta or places not both")
        diff = abs(first - second)
        if delta is not None:
            if diff <= delta:
                return
            standard_msg = ("%s != %s within %s delta (%s difference)"
                            % (first, second, delta, diff))
        else:
            if places is None:
                places = 7
            if round(diff, places) == 0:
                return
            standard_msg = ("%s != %s within %r places (%s difference)"
                            % (first, second, places, diff))
        raise self.failureException(msg or standard_msg)
    unittest.TestCase.assertIn = assertIn
    unittest.TestCase.assertNotIn = assertNotIn
    unittest.TestCase.assertIs = assertIs
    unittest.TestCase.assertIsInstance = assertIsInstance
    unittest.TestCase.assertLess = assertLess
    unittest.TestCase.assertGreater = assertGreater
    unittest.TestCase.assertLessEqual = assertLessEqual
    unittest.TestCase.assertGreaterEqual = assertGreaterEqual
    unittest.TestCase.assertIsNone = assertIsNone
    unittest.TestCase.assertIsNotNone = assertIsNotNone
    unittest.TestCase.assertAlmostEqual = assertAlmostEqual
