/**
 *  \file SlabWithSphericalIndentGeometry.cpp
 *  \brief A geometry for displaying a wireframe model of a surface with a spherical indent.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/npc/SlabWithSphericalIndentGeometry.h"
#include <IMP/display/geometry.h>
#include <IMP/constants.h>
#include <IMP/algebra/Plane3D.h>
#include <math.h>

IMPNPC_BEGIN_NAMESPACE

// length is the x-y bounds on displaying the edges of the plane
SlabWithSphericalIndentGeometry
::SlabWithSphericalIndentGeometry(double radius, double length, double depth) : Geometry("SlabWithSphericalIndentSurfaceMesh"),
                                                                                radius_(radius),
                                                                                length_(length),
                                                                                depth_(depth)
{}

display::Geometries SlabWithSphericalIndentGeometry::get_components() const
{
    //  initialize return geometries
    display::Geometries ret;

    // compute the base circe radius (at the intersection of the plane and the sphere)
    double a_sqrd = depth_*(2.0*radius_-depth_);

    // build the spherical cap segement

    // number of segments in the radial and semicircle directions
    const int nrad = 10;
    const int ncc = 5;

    // put line segments on plane outer boundary
    //ret.push_back(new display::SegmentGeometry(algebra::Segment3D(v00, v10)));
    double hlen = length_ / 2.0;

    // corners
    algebra::Vector3D c1(-hlen, hlen, 0.0);
    algebra::Vector3D c2(hlen, hlen, 0.0);
    algebra::Vector3D c3(hlen, -hlen, 0.0);
    algebra::Vector3D c4(-hlen, -hlen, 0.0);

    // top edge
    ret.push_back(new display::SegmentGeometry(algebra::Segment3D(c1, c2)));
    // right edge
    ret.push_back(new display::SegmentGeometry(algebra::Segment3D(c2, c3)));
    // bottom edge
    ret.push_back(new display::SegmentGeometry(algebra::Segment3D(c3, c4)));
    // left edge
    ret.push_back(new display::SegmentGeometry(algebra::Segment3D(c1, c4)));




    // build semicircles
    const double dz = depth_ / ncc;  // spacing between semiscircles
    const double nrseg = 360;  // number of segments to produce

    double z;
    double x_1,y_1;
    double x_2,y_2;
    double alpha;


    for (int i = 0; i < ncc; i++) {
        z = dz * i;

        // again compute radius of semicircle as alpha of spherical cap of z-height
        alpha =sqrt((depth_ - z) * (2.0 * radius_- (depth_ - z)));

        for (int s = 0; s < nrseg; s++) {
            // again compute radius of semicircle as alpha of spherical cap of z-height
            x_1 = alpha * cos(2.0 * IMP::PI * s / 360);
            y_1 = alpha * sin(2.0 * IMP::PI * s / 360);
            x_2 = alpha * cos(2.0 * IMP::PI * (s+1) / 360);
            y_2 = alpha * sin(2.0 * IMP::PI * (s+1) / 360);
            algebra::Segment3D seg(algebra::Vector3D(x_1, y_1, z),
                                   algebra::Vector3D(x_2, y_2, z));
            ret.push_back(new display::SegmentGeometry(seg));

        }
    }

    // build longitudinal arcs and segments extending to boundary box
    const double nlseg = 12;  // number of longitudinal arcs to draw
    double theta, phi;
    double z_1, z_2;
    double m;


    phi = asin(sqrt(depth_ * (2.0 * radius_ - depth_)) / radius_);
    // draw longitudinal arcs
    for (int i = 0; i < nlseg; i++) {
        theta = 2.0 * IMP::PI * i / nlseg;
        //printf("%f theta \n", theta);
        for (int s = 0; s < 90; s++) {
            x_1 = radius_ * sin(phi * s / 90) * cos(theta);
            y_1 = radius_ * sin(phi * s / 90) * sin(theta);
            x_2 = radius_ * sin(phi * (s+1) / 90) * cos(theta);
            y_2 = radius_ * sin(phi * (s+1) / 90) * sin(theta);
            z_1 = radius_ * cos(phi * s / 90) - (radius_ - depth_);
            z_2 = radius_ * cos(phi * (s+1) / 90) - (radius_ - depth_);

            algebra::Segment3D seg(algebra::Vector3D(x_1, y_1, z_1),
                                   algebra::Vector3D(x_2, y_2, z_2));
            ret.push_back(new display::SegmentGeometry(seg));

        }

        // build segments extending to boundary
        x_2 = radius_ * sin(phi) * cos(theta);  // last iteration from inner for
        y_2 = radius_ * sin(phi) * sin(theta);

        m = y_2 / x_2;
        // compute intersection of the line with the boundary
        if (theta <= (IMP::PI / 4.0)) {
            algebra::Segment3D seg(algebra::Vector3D(x_2, y_2, 0.0),
                                   algebra::Vector3D(hlen, m*hlen, 0.0));

            ret.push_back(new display::SegmentGeometry(seg));
        }
        else if (theta <= (3.0 * IMP::PI / 4.0)) {
            algebra::Segment3D seg(algebra::Vector3D(x_2, y_2, 0.0),
                                   algebra::Vector3D(hlen / m, hlen, 0.0));

            ret.push_back(new display::SegmentGeometry(seg));
        }
        else if (theta <= (5.0*IMP::PI / 4.0)) {
            algebra::Segment3D seg(algebra::Vector3D(x_2, y_2, 0.0),
                                   algebra::Vector3D(-hlen,-m * hlen, 0.0));

            ret.push_back(new display::SegmentGeometry(seg));
        }
        else if (theta <= (7.0 * IMP::PI / 4.0)) {
            algebra::Segment3D seg(algebra::Vector3D(x_2, y_2, 0.0),
                                   algebra::Vector3D(-hlen / m, -hlen, 0.0));

            ret.push_back(new display::SegmentGeometry(seg));
        }
        else if (theta <= (2.0 * IMP::PI)) {
            algebra::Segment3D seg(algebra::Vector3D(x_2, y_2, 0.0),
                                   algebra::Vector3D(hlen, m*hlen, 0.0));

            ret.push_back(new display::SegmentGeometry(seg));
        }

    }




    return ret;

}

IMPNPC_END_NAMESPACE
