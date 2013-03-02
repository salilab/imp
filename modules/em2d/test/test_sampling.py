import IMP
import IMP.test
import IMP.algebra as alg
import sys
import os

import IMP.em2d.sampling as sampling
import IMP.em2d.Database as Database
import IMP.em2d.imp_general.io as io

class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)


    def test_sampling_schema(self):
        """
            Test
        """
        subunits = [ "subunitA", "subunitB", "subunitC", "subunitD"]
        anchored = [False, False, False, False]
        fixed = [False, False, False, False]

        n_transformations = 50
        db = Database.Database2()
        fn = 'temp.db'
        db.create(fn, overwrite=True)
        db.connect(fn)

        transformations = []
        table_name = "results"
        db.create_table(table_name, ["reference_frames"], [str])
        for i in range(n_transformations):
            Ts = []
            for j in range(len(subunits)):
                center = alg.Vector3D(0, 0, 0)
                T = alg.Transformation3D(alg.get_random_rotation_3d(),
                        alg.get_random_vector_in(alg.Sphere3D(center,34)))
                Ts.append(T)
            transformations.append(Ts)

        data = []
        for Ts in transformations:
            text = [io.Transformation3DToText(T).get_text() for T in Ts]
            text = "/".join(text)
            data.append([text,])
        db.store_data(table_name,data)
        db.close()

        sch = sampling.SamplingSchema(4, fixed, anchored)
        sch.read_from_database(fn)

        for i in range(len(transformations)):
            for j in range(len(subunits)):
                T = transformations[i][j]
                t = T.get_translation()
                q = T.get_rotation().get_quaternion()

                pos = sch.transformations[j][i].get_translation()
                ori = sch.transformations[j][i].get_rotation().get_quaternion()
                for k in range(3):
                    self.assertAlmostEqual(pos[k], t[k])
                for k in range(4):
                    self.assertAlmostEqual(q[k], ori[k])

        os.remove(fn)


if __name__ == '__main__':
    IMP.test.main()
