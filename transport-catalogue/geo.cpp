// This file was not supported by yandex.practicum,
// so methods from here should be returned to .h file in case of new assignments.

#include "geo.h"

bool Coordinates::operator==(const Coordinates& other) const {
    return lat == other.lat && lng == other.lng;
}

bool Coordinates::operator!=(const Coordinates& other) const {
    return !(*this == other);
}
