#ifndef CTS_BODY_SETTINGS_H
#define CTS_BODY_SETTINGS_H

#include <string>
#include <vector>

struct CTSBodySetting
{
    std::string bodyName;
    std::string settingName;
};

class CTSBodySettings
{
public:
    static const std::vector<CTSBodySetting>& knownBodies();
    static bool isEnabled(const std::string& bodyName);
};

#endif
