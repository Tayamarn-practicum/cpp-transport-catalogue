// #pragma once

// #include <cmath>

// struct Coordinates {
//     double lat;
//     double lng;
//     bool operator==(const Coordinates& other) const;
//     bool operator!=(const Coordinates& other) const;
// };

// inline double ComputeDistance(Coordinates from, Coordinates to) {
//     using namespace std;
//     if (from == to) {
//         return 0;
//     }
//     static const double dr = 3.1415926535 / 180.;
//     static const int earth_radius = 6371000;
//     return acos(sin(from.lat * dr) * sin(to.lat * dr)
//                 + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
//         * earth_radius;
// }

#pragma once

namespace geo {

struct Coordinates {
    double lat; // Широта
    double lng; // Долгота
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo
