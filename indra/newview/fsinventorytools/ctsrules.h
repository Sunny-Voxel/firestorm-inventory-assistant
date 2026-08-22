#ifndef CTS_RULES_H
#define CTS_RULES_H

#include <string>

#include "ctsclassification.h"

class CTSRules
{
public:
    static std::string classifyItem(const std::string& itemName);
    static CTSClassification analyzeItem(const std::string& itemName);
};

#endif
