"""Classes for handling geometry.

   Geometric objects (see :class:`GeometricObject`) are usually
   used in :class:`~ihm.restraint.GeometricRestraint` objects.
"""

class Center(object):
    """Define the center of a geometric object in Cartesian space.

       :param float x: x coordinate
       :param float y: y coordinate
       :param float z: z coordinate
    """
    def __init__(self, x, y, z):
        self.x, self.y, self.z = x, y, z


class Transformation(object):
    """Rotation and translation applied to a geometric object.

       :param rot_matrix: Rotation matrix (as a 3x3 array of floats) that
              places the object in its final position.
       :param tr_vector: Translation vector (as a 3-element float list) that
              places the object in its final position.
    """
    def __init__(self, rot_matrix, tr_vector):
        self.rot_matrix, self.tr_vector = rot_matrix, tr_vector

    """Return the identity transformation.

       :return: A new identity Transformation.
       :rtype: :class:`Transformation`
    """
    @classmethod
    def identity(cls):
        return cls([[1., 0., 0.], [0., 1., 0.], [0., 0., 1.]], [0.,0.,0.])


class GeometricObject(object):
    """A generic geometric object. See also :class:`Sphere`, :class:`Torus`,
       :class:`Axis`, :class:`Plane`.

       Geometric objects are typically assigned to one or more
       :class:`~ihm.restraint.GeometricRestraint` objects.

       :param str name: A short user-provided name.
       :param str description: A brief description of the object.
       :param str details: Additional details about the object (particularly
              useful for generic objects).
    """
    type = 'other'
    def __init__(self, name=None, description=None, details=None):
        self.name, self.description = name, description
        self.details = details


class Sphere(GeometricObject):
    """A sphere in Cartesian space.

       :param center: Coordinates of the center of the sphere.
       :type center: :class:`Center`
       :param radius: Radius of the sphere.
       :param transformation: Rotation and translation that moves the sphere
              from the original center to its final location, if any.
       :type transformation: :class:`Transformation`
       :param str name: A short user-provided name.
       :param str description: A brief description of the object.
       :param str details: Additional details about the object.
    """

    type = 'sphere'

    def __init__(self, center, radius, transformation=None,
                 name=None, description=None, details=None):
        super(Sphere, self).__init__(name, description, details)
        self.center, self.transformation = center, transformation
        self.radius = radius


class Torus(GeometricObject):
    """A torus in Cartesian space.

       :param center: Coordinates of the center of the torus.
       :type center: :class:`Center`
       :param major_radius: The major radius - the distance from the center of
              the tube to the center of the torus.
       :param minor_radius: The minor radius - the radius of the tube.
       :param transformation: Rotation and translation that moves the torus
              (which by default lies in the xy plane) from the original
              center to its final location, if any.
       :type transformation: :class:`Transformation`
       :param str name: A short user-provided name.
       :param str description: A brief description of the object.
       :param str details: Additional details about the object.
    """
    type = 'torus'

    def __init__(self, center, major_radius, minor_radius, transformation=None,
                 name=None, description=None, details=None):
        super(Torus, self).__init__(name, description, details)
        self.center, self.transformation = center, transformation
        self.major_radius, self.minor_radius = major_radius, minor_radius


class HalfTorus(GeometricObject):
    """A section of a :class:`Torus`. This is defined as a surface over part
       of the torus with a given thickness, and is often used to represent
       a membrane.

       :param thickness: The thickness of the surface.
       :param inner: True if the surface is the 'inner' half of the torus
              (i.e. closer to the center), False for the outer surface, or
              None for some other section (described in `details`).

       See :class:`Torus` for a description of the other parameters.
    """
    type = 'half-torus'

    def __init__(self, center, major_radius, minor_radius, thickness,
                 transformation=None, inner=None, name=None, description=None,
                 details=None):
        super(HalfTorus, self).__init__(name, description, details)
        self.center, self.transformation = center, transformation
        self.major_radius, self.minor_radius = major_radius, minor_radius
        self.thickness, self.inner = thickness, inner


class Axis(GeometricObject):
    """One of the three Cartesian axes - see :class:`XAxis`, :class:`YAxis`,
       :class:`ZAxis`.

       :param transformation: Rotation and translation that moves the axis
              from the original Cartesian axis to its final location, if any.
       :type transformation: :class:`Transformation`
       :param str name: A short user-provided name.
       :param str description: A brief description of the object.
       :param str details: Additional details about the object.
    """
    type = 'axis'

    def __init__(self, transformation=None, name=None, description=None,
                 details=None):
        super(Axis, self).__init__(name, description, details)
        self.transformation = transformation


class XAxis(Axis):
    """The x Cartesian axis.

       See :class:`GeometricObject` for a description of the parameters.
    """
    axis_type = 'x-axis'


class YAxis(Axis):
    """The y Cartesian axis.

       See :class:`GeometricObject` for a description of the parameters.
    """
    axis_type = 'y-axis'


class ZAxis(Axis):
    """The z Cartesian axis.

       See :class:`GeometricObject` for a description of the parameters.
    """
    axis_type = 'z-axis'


class Plane(GeometricObject):
    """A plane in Cartesian space - see :class:`XYPlane`, :class:`YZPlane`,
       :class:`XZPlane`.

       :param transformation: Rotation and translation that moves the plane
              from the original position to its final location, if any.
       :type transformation: :class:`Transformation`
       :param str name: A short user-provided name.
       :param str description: A brief description of the object.
       :param str details: Additional details about the object.
    """
    type = 'plane'

    def __init__(self, transformation=None, name=None, description=None,
                 details=None):
        super(Plane, self).__init__(name, description, details)
        self.transformation = transformation


class XYPlane(Plane):
    """The xy plane in Cartesian space.

       See :class:`GeometricObject` for a description of the parameters.
    """
    plane_type = 'xy-plane'


class YZPlane(Plane):
    """The yz plane in Cartesian space.

       See :class:`GeometricObject` for a description of the parameters.
    """
    plane_type = 'yz-plane'


class XZPlane(Plane):
    """The xz plane in Cartesian space.

       See :class:`GeometricObject` for a description of the parameters.
    """
    plane_type = 'xz-plane'
