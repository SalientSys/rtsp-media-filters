#pragma once

#include <sstream>
#include <string>

const unsigned MajorVersion = 0;
const unsigned MinorVersion = 0;
const unsigned BuildVersion = 0;

/// 0.0.0: - Initial release of filter with version control

struct VersionInfo
{
    static std::string toString()
    {
        std::ostringstream ostr;
        ostr << MajorVersion << "." << MinorVersion << "." << BuildVersion;
        return ostr.str();
    }

};
