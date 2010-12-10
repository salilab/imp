import math
import IMP.algebra
import IMP.test

# square Euclidean distance
def sq_dist(p1, p2):
    return (p1[0] - p2[0])**2 + (p1[1] - p2[1])**2


# find the points within the distance of radius from the center - the slow way
def slow_nearest(points, center, radius):
    radius *= radius
    r = list()
    for p in points:
        if sq_dist(p, center) <= radius:
            r.append(p)
    return r


# find the points within the distance of radius from the center - the "fast" way
def fast_nearest(points, center, radius):
    H = IMP.algebra.GeometricHash2()
    for i, p in enumerate(points):
        H.add(p, i)
    v = H.neighbors(H.EUCLIDEAN, center, radius)
    return [points[x] for x in v]


# generate some points
def gen_points(N):
    points = list()
    for r in xrange(N):
        for i in xrange(N):
            angle = 2.0*i*math.pi/N
            p = [0.1*r*math.cos(angle), 0.1*r*math.sin(angle)]
            points.append(p)
    return points


# compare the points generated using the slow and fast algorithm
def compare(slow, fast):
    if len(slow) != len(fast):
        return False
    slow.sort(key = lambda x : x[1])
    slow.sort(key = lambda x : x[0])
    fast.sort(key = lambda x : x[1])
    fast.sort(key = lambda x : x[0])
    for s, f in zip(slow, fast):
        if s[0] != f[0] or s[1] != f[1]:
            return False
    return True



class GeometricHashTest(IMP.test.TestCase):
    def test_neighbors(self):
        points = gen_points(200)
        cnt = [1, 1]
        rd = 0.45
        slow = slow_nearest(points, cnt, rd)
        fast = fast_nearest(points, cnt, rd)
        status = compare(slow, fast)
        self.assertEqual(status, True)


if __name__ == '__main__':
    IMP.test.main()
