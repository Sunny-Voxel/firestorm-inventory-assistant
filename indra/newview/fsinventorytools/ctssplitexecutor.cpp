#include "../llviewerprecompiledheaders.h"

#include "ctssplitexecutor.h"
#include "ctsbodysettings.h"
#include "ctsitemclassifier.h"
#include "ctsrules.h"
#include "ctsundomanager.h"

#include "../llagent.h"
#include "../llinventorymodel.h"
#include "../llviewerinventory.h"

#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace
{
struct SplitItem
{
    LLUUID itemId;
    std::string itemName;
    std::string targetKey;
};

struct SplitPackage
{
    std::string body;
    std::string outputFolderName;
    std::string rootName;
    std::vector<SplitItem> items;

    LLUUID rootId;
    LLUUID bodyId;
    LLUUID addOnId;
    LLUUID hudId;
    LLUUID alphaId;
};

struct SplitContext
{
    LLUUID sourceFolderId;
    LLUUID sourceParentId;
    FSInventoryScanResult scan;
    CTSExecutor::completion_func_t completion;

    std::vector<SplitPackage> packages;
    std::size_t packageIndex = 0;
    std::size_t itemIndex = 0;

    CTSUndoTransaction transaction;
    CTSExecutionResult result;
};

void finish(const std::shared_ptr<SplitContext>& context,
            bool success,
            const std::string& message);

void createNextPackage(const std::shared_ptr<SplitContext>& context);
void createBodyFolder(const std::shared_ptr<SplitContext>& context);
void createAddOnFolder(const std::shared_ptr<SplitContext>& context);
void createHudFolder(const std::shared_ptr<SplitContext>& context);
void createAlphaFolder(const std::shared_ptr<SplitContext>& context);
void copyNextItem(const std::shared_ptr<SplitContext>& context);

void cleanupCreatedRoots(const std::shared_ptr<SplitContext>& context)
{
    for (auto it = context->transaction.createdRootFolderIds.rbegin();
         it != context->transaction.createdRootFolderIds.rend();
         ++it)
    {
        if (gInventory.getCategory(*it))
            gInventory.removeCategory(*it);
    }
}

bool rootNameExists(const LLUUID& parentId, const std::string& wanted)
{
    LLInventoryModel::cat_array_t* categories = nullptr;
    LLInventoryModel::item_array_t* items = nullptr;
    gInventory.getDirectDescendentsOf(parentId, categories, items);

    if (!categories)
        return false;

    for (const auto& category : *categories)
    {
        if (category &&
            LLStringUtil::compareInsensitive(category->getName(), wanted) == 0)
            return true;
    }
    return false;
}

bool buildPlan(const std::shared_ptr<SplitContext>& context,
               std::string& error)
{
    LLViewerInventoryCategory* source =
        gInventory.getCategory(context->sourceFolderId);

    if (!source)
    {
        error = "The selected product folder no longer exists.";
        return false;
    }

    context->sourceParentId = source->getParentUUID();

    const std::vector<std::string> bodies =
        CTSSplitExecutor::applicableTargets(context->scan);

    if (bodies.empty())
    {
        error =
            "No selected body was detected in this product. "
            "No per-body packages were created.";
        return false;
    }

    std::set<LLUUID> itemsThatNeedCopy;

    for (const std::string& body : bodies)
    {
        SplitPackage package;
        package.body = body;
        package.outputFolderName = CTSBodySettings::outputTargetFolderName(body);
        package.rootName = context->scan.productName + " - " + package.outputFolderName;

        if (rootNameExists(context->sourceParentId, package.rootName))
        {
            error = "A package folder already exists: " + package.rootName;
            return false;
        }

        for (const FSInventoryScannedItem& scanned : context->scan.items)
        {
            const CTSItemClassification classification =
                CTSItemClassifier::classify(scanned.itemName);
            const std::string special =
                CTSRules::classifyItem(scanned.itemName);

            SplitItem planned;
            planned.itemId = scanned.itemId;
            planned.itemName = scanned.itemName;

            if (special == "HUD")
            {
                planned.targetKey =
                    CTSBodySettings::splitHUDFolder() ? "hud" : "body";
            }
            else if (special == "Alpha")
            {
                planned.targetKey =
                    CTSBodySettings::splitAlphaFolder() ? "alpha" : "body";
            }
            else if (!classification.body.empty() ||
                     !classification.head.empty())
            {
                const std::string target =
                    !classification.body.empty()
                        ? classification.body
                        : classification.head;
                if (target != body)
                    continue;
                planned.targetKey = "body";
            }
            else
            {
                planned.targetKey = "addon";
            }

            package.items.push_back(planned);
            itemsThatNeedCopy.insert(scanned.itemId);
        }

        context->packages.push_back(package);
    }

    for (const LLUUID& itemId : itemsThatNeedCopy)
    {
        LLViewerInventoryItem* item = gInventory.getItem(itemId);
        if (!item)
        {
            error = "An item disappeared before execution.";
            return false;
        }

        if (!item->getPermissions().allowCopyBy(gAgent.getID()))
        {
            error = "Per-body package mode cannot copy a no-copy item: " +
                    item->getName();
            return false;
        }
    }

    return true;
}

void createNextPackage(const std::shared_ptr<SplitContext>& context)
{
    if (context->packageIndex >= context->packages.size())
    {
        context->transaction.productFolderId = context->sourceFolderId;
        context->transaction.productName = context->scan.productName;
        CTSUndoManager::instance().setLastTransaction(context->transaction);

        std::ostringstream msg;
        msg << context->packages.size()
            << " body package"
            << (context->packages.size() == 1 ? "" : "s")
            << " created, "
            << context->result.itemsCopied
            << " items copied. Original product kept unchanged.";

        finish(context, true, msg.str());
        return;
    }

    SplitPackage& package = context->packages[context->packageIndex];
    const std::string rootName = package.rootName;

    gInventory.createNewCategory(
        context->sourceParentId,
        LLFolderType::FT_NONE,
        rootName,
        [context](const LLUUID& rootId)
        {
            if (rootId.isNull())
            {
                cleanupCreatedRoots(context);
                finish(context, false, "Could not create per-body main folder.");
                return;
            }

            SplitPackage& package =
                context->packages[context->packageIndex];
            package.rootId = rootId;
            context->transaction.createdRootFolderIds.push_back(rootId);
            context->transaction.createdFolderIds.push_back(rootId);
            ++context->result.foldersCreated;
            createBodyFolder(context);
        });
}

void createBodyFolder(const std::shared_ptr<SplitContext>& context)
{
    SplitPackage& package = context->packages[context->packageIndex];

    gInventory.createNewCategory(
        package.rootId,
        LLFolderType::FT_NONE,
        package.outputFolderName,
        [context](const LLUUID& id)
        {
            if (id.isNull())
            {
                cleanupCreatedRoots(context);
                finish(context, false, "Could not create body folder.");
                return;
            }

            SplitPackage& package =
                context->packages[context->packageIndex];
            package.bodyId = id;
            context->transaction.createdFolderIds.push_back(id);
            ++context->result.foldersCreated;
            createAddOnFolder(context);
        });
}

void createAddOnFolder(const std::shared_ptr<SplitContext>& context)
{
    SplitPackage& package = context->packages[context->packageIndex];

    gInventory.createNewCategory(
        package.rootId,
        LLFolderType::FT_NONE,
        CTSBodySettings::addOnFolderName(),
        [context](const LLUUID& id)
        {
            if (id.isNull())
            {
                cleanupCreatedRoots(context);
                finish(context, false, "Could not create Add On folder.");
                return;
            }

            SplitPackage& package =
                context->packages[context->packageIndex];
            package.addOnId = id;
            context->transaction.createdFolderIds.push_back(id);
            ++context->result.foldersCreated;
            createHudFolder(context);
        });
}

void createHudFolder(const std::shared_ptr<SplitContext>& context)
{
    if (!CTSBodySettings::splitHUDFolder())
    {
        createAlphaFolder(context);
        return;
    }

    SplitPackage& package = context->packages[context->packageIndex];

    gInventory.createNewCategory(
        package.rootId,
        LLFolderType::FT_NONE,
        CTSBodySettings::hudFolderName(),
        [context](const LLUUID& id)
        {
            if (id.isNull())
            {
                cleanupCreatedRoots(context);
                finish(context, false, "Could not create HUD folder.");
                return;
            }

            SplitPackage& package =
                context->packages[context->packageIndex];
            package.hudId = id;
            context->transaction.createdFolderIds.push_back(id);
            ++context->result.foldersCreated;
            createAlphaFolder(context);
        });
}

void createAlphaFolder(const std::shared_ptr<SplitContext>& context)
{
    if (!CTSBodySettings::splitAlphaFolder())
    {
        context->itemIndex = 0;
        copyNextItem(context);
        return;
    }

    SplitPackage& package = context->packages[context->packageIndex];

    gInventory.createNewCategory(
        package.rootId,
        LLFolderType::FT_NONE,
        CTSBodySettings::alphaFolderName(),
        [context](const LLUUID& id)
        {
            if (id.isNull())
            {
                cleanupCreatedRoots(context);
                finish(context, false, "Could not create Alpha folder.");
                return;
            }

            SplitPackage& package =
                context->packages[context->packageIndex];
            package.alphaId = id;
            context->transaction.createdFolderIds.push_back(id);
            ++context->result.foldersCreated;
            context->itemIndex = 0;
            copyNextItem(context);
        });
}

void copyNextItem(const std::shared_ptr<SplitContext>& context)
{
    SplitPackage& package = context->packages[context->packageIndex];

    if (context->itemIndex >= package.items.size())
    {
        ++context->packageIndex;
        context->itemIndex = 0;
        createNextPackage(context);
        return;
    }

    const SplitItem planned = package.items[context->itemIndex];

    LLUUID target;
    if (planned.targetKey == "body")
        target = package.bodyId;
    else if (planned.targetKey == "hud")
        target = package.hudId;
    else if (planned.targetKey == "alpha")
        target = package.alphaId;
    else
        target = package.addOnId;

    LLViewerInventoryItem* item = gInventory.getItem(planned.itemId);
    if (!item || target.isNull())
    {
        cleanupCreatedRoots(context);
        finish(context, false,
               "Inventory changed while creating per-body packages.");
        return;
    }

    LLPointer<LLInventoryCallback> callback =
        new LLBoostFuncInventoryCallback(
            [context](const LLUUID& newItemId)
            {
                if (newItemId.isNull())
                {
                    cleanupCreatedRoots(context);
                    finish(context, false,
                           "An inventory item could not be copied.");
                    return;
                }

                ++context->result.itemsCopied;
                ++context->itemIndex;
                copyNextItem(context);
            });

    copy_inventory_item(
        gAgent.getID(),
        item->getPermissions().getOwner(),
        item->getUUID(),
        target,
        item->getName(),
        callback);
}

void finish(const std::shared_ptr<SplitContext>& context,
            bool success,
            const std::string& message)
{
    context->result.success = success;
    context->result.message = message;
    if (context->completion)
        context->completion(context->result);
}
}


std::vector<std::string> CTSSplitExecutor::applicableBodies(
    const FSInventoryScanResult& scan)
{
    const std::vector<std::string> enabled =
        CTSBodySettings::enabledBodies();

    std::set<std::string> enabledSet(
        enabled.begin(), enabled.end());
    std::set<std::string> found;

    for (const FSInventoryScannedItem& scanned : scan.items)
    {
        const CTSItemClassification classification =
            CTSItemClassifier::classify(scanned.itemName);

        if (!classification.body.empty() &&
            enabledSet.count(classification.body))
        {
            found.insert(classification.body);
        }
    }

    std::vector<std::string> result;
    for (const std::string& body : enabled)
    {
        if (found.count(body))
            result.push_back(body);
    }

    return result;
}


std::vector<std::string> CTSSplitExecutor::applicableTargets(
    const FSInventoryScanResult& scan)
{
    std::vector<std::string> enabled = CTSBodySettings::enabledBodies();
    const std::vector<std::string> heads = CTSBodySettings::enabledHeads();
    enabled.insert(enabled.end(), heads.begin(), heads.end());

    std::set<std::string> enabledSet(enabled.begin(), enabled.end());
    std::set<std::string> found;

    for (const FSInventoryScannedItem& scanned : scan.items)
    {
        const CTSItemClassification c =
            CTSItemClassifier::classify(scanned.itemName);
        if (!c.body.empty() && enabledSet.count(c.body))
            found.insert(c.body);
        if (!c.head.empty() && enabledSet.count(c.head))
            found.insert(c.head);
    }

    std::vector<std::string> result;
    for (const std::string& target : enabled)
        if (found.count(target))
            result.push_back(target);
    return result;
}

void CTSSplitExecutor::execute(
    const LLUUID& productFolderId,
    const FSInventoryScanResult& scan,
    CTSExecutor::completion_func_t completion)
{
    auto context = std::make_shared<SplitContext>();
    context->sourceFolderId = productFolderId;
    context->scan = scan;
    context->completion = completion;

    std::string error;
    if (!buildPlan(context, error))
    {
        finish(context, false, error);
        return;
    }

    createNextPackage(context);
}
