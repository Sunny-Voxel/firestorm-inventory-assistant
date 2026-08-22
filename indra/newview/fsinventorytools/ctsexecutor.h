#ifndef CTS_EXECUTOR_H
#define CTS_EXECUTOR_H

#include "ctsexecutionplan.h"
#include "ctsundomanager.h"

#include "lluuid.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

struct CTSExecutionResult
{
    bool success = false;
    std::size_t foldersCreated = 0;
    std::size_t itemsMoved = 0;
    std::string message;
};

class CTSExecutor
{
public:
    using completion_func_t = std::function<void(const CTSExecutionResult&)>;

    static void execute(
        const LLUUID& productFolderId,
        const FSInventoryScanResult& scan,
        const CTSExecutionPlan& plan,
        completion_func_t completion);

private:
    struct Context;

    static bool preflight(
        const LLUUID& productFolderId,
        const FSInventoryScanResult& scan,
        const CTSExecutionPlan& plan,
        std::map<std::string, LLUUID>& destinationFolders,
        std::string& error);

    static void createNextFolder(const std::shared_ptr<Context>& context);
    static void moveItems(const std::shared_ptr<Context>& context);
    static void rollbackMovedItems(const std::shared_ptr<Context>& context);

    static void finish(
        const std::shared_ptr<Context>& context,
        bool success,
        const std::string& message);
};

#endif
