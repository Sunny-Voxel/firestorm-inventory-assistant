#ifndef CTS_EXECUTION_PLAN_H
#define CTS_EXECUTION_PLAN_H

#include "ctsdestinationplanner.h"

#include <cstddef>
#include <string>
#include <vector>

enum class CTSExecutionMode
{
    Repair,
    Organize
};

struct CTSPlannedFolderCreate
{
    std::string folderName;
};

struct CTSPlannedMove
{
    LLUUID itemId;
    LLUUID sourceFolderId;
    std::string itemName;
    std::string sourceFolderName;
    std::string destinationFolderName;
};

struct CTSExecutionPlan
{
    CTSExecutionMode mode = CTSExecutionMode::Repair;
    std::string productName;
    std::vector<CTSPlannedFolderCreate> foldersToCreate;
    std::vector<CTSPlannedMove> moves;
    std::size_t alreadyCorrect = 0;

    bool empty() const
    {
        return foldersToCreate.empty() && moves.empty();
    }
};

class CTSExecutionPlanBuilder
{
public:
    static CTSExecutionPlan build(
        const FSInventoryScanResult& scan,
        const CTSDestinationPlan& destinationPlan,
        CTSExecutionMode mode);
};

#endif
