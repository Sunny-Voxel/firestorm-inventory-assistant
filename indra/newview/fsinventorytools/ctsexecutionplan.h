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

struct CTSPlannedFolderRename
{
    LLUUID folderId;
    std::string oldName;
    std::string newName;
};

struct CTSPlannedDuplicateRemoval
{
    LLUUID itemId;
    LLUUID sourceFolderId;
    std::string itemName;
    std::string sourceFolderName;
    std::string destinationFolderName;
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

    std::vector<CTSPlannedFolderRename> foldersToRename;
    std::vector<CTSPlannedFolderCreate> foldersToCreate;
    std::vector<CTSPlannedMove> moves;
    std::vector<CTSPlannedDuplicateRemoval> duplicatesToRemove;

    std::size_t alreadyCorrect = 0;

    bool empty() const
    {
        return foldersToRename.empty() &&
               foldersToCreate.empty() &&
               moves.empty() &&
               duplicatesToRemove.empty();
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
