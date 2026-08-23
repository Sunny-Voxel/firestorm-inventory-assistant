#ifndef CTS_SPLIT_EXECUTOR_H
#define CTS_SPLIT_EXECUTOR_H

#include "ctsexecutor.h"
#include "fsinventoryscan.h"

#include <vector>
#include <string>

class CTSSplitExecutor
{
public:
    static std::vector<std::string> applicableBodies(
        const FSInventoryScanResult& scan);
    static std::vector<std::string> applicableTargets(
        const FSInventoryScanResult& scan);

    static void execute(
        const LLUUID& productFolderId,
        const FSInventoryScanResult& scan,
        CTSExecutor::completion_func_t completion);
};

#endif
