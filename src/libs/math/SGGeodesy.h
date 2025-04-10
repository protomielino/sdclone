// Copyright (C) 2006  Mathias Froehlich - Mathias.Froehlich@web.de
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef SGGeodesy_H
#define SGGeodesy_H

class SGGeodesy {
public:
  // Hard numbers from the WGS84 standard.
  static const double EQURAD;
  static const double iFLATTENING;
  static const double SQUASH;
  static const double STRETCH;
  static const double POLRAD;

  /// Takes a cartesian coordinate data and returns the geodetic
  /// coordinates.
  static void SGCartToGeod(const SGVec3<double>& cart, SGGeod& geod);

  /// Takes a geodetic coordinate data and returns the cartesian
  /// coordinates.
  static void SGGeodToCart(const SGGeod& geod, SGVec3<double>& cart);

  /// Takes a geodetic coordinate data and returns the sea level radius.
  static double SGGeodToSeaLevelRadius(const SGGeod& geod);

  /// Takes a cartesian coordinate data and returns the geocentric
  /// coordinates.
  static void SGCartToGeoc(const SGVec3<double>& cart, SGGeoc& geoc);

  /// Takes a geocentric coordinate data and returns the cartesian
  /// coordinates.
  static void SGGeocToCart(const SGGeoc& geoc, SGVec3<double>& cart);
};

#endif
