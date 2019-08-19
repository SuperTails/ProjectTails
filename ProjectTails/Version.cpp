#include "Version.h"
#include <iostream>
#include <tuple>

std::istream& operator>> (std::istream& stream, Version& version) {
	stream >> version.major;
	if (!stream || stream.get() != '.') {
		stream.setstate(std::istream::failbit);
		return stream;
	}
	stream >> version.minor;
	if (!stream || stream.get() != '.') {
		stream.setstate(std::istream::failbit);
		return stream;
	}
	stream >> version.patch;

	return stream;
}

std::ostream& operator<< (std::ostream& stream, Version version) {
	stream << version.major << '.' << version.minor << '.' << version.patch;
	return stream;
}

bool operator== (Version a, Version b) {
	return std::make_tuple(a.major, a.minor, a.patch) == std::make_tuple(b.major, b.minor, b.patch);
}

bool operator<  (Version a, Version b) {
	return std::make_tuple(a.major, a.minor, a.patch) < std::make_tuple(b.major, b.minor, b.patch);
}

bool operator>  (Version a, Version b) {
	return !(a <= b);
}

bool operator<= (Version a, Version b) {
	return (a < b) || (a == b);
}

bool operator>= (Version a, Version b) {
	return !(a < b);
}
