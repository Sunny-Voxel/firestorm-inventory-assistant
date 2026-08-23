#ifndef CTS_ITEM_CLASSIFIER_H
#define CTS_ITEM_CLASSIFIER_H

#include <string>
#include <vector>

enum class CTSConfidence
{
    Low,
    Medium,
    High
};

struct CTSItemClassification
{
    std::string body;
    std::string head;
    std::string otherTarget;
    std::string type;
    std::string placement;
    std::string side;
    std::string variant;
    std::string version;
    std::string color;
    CTSConfidence confidence = CTSConfidence::Low;
    int score = 0;
    std::vector<std::string> signals;
};

class CTSItemClassifier
{
public:
    static CTSItemClassification classify(const std::string& itemName);
    static const char* confidenceName(CTSConfidence confidence);
};

#endif
