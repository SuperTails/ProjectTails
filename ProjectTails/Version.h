#pragma once
#include <iosfwd>

struct Version {
	int major;
	int minor;
	int patch;
};

const Version currentVersion = { 0, 0, 2 };

bool operator== (Version a, Version b);
bool operator<  (Version a, Version b);
bool operator>  (Version a, Version b);
bool operator>= (Version a, Version b);
bool operator<= (Version a, Version b);
bool operator!= (Version a, Version b);

std::istream& operator>> (std::istream& str, Version& version);
std::ostream& operator<< (std::ostream& str, Version version);
