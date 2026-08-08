#ifndef CTS_UNDO_MANAGER_H
#define CTS_UNDO_MANAGER_H

#include "lluuid.h"
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

struct CTSUndoMove
{
    LLUUID itemId;
    LLUUID originalParentId;
    LLUUID newParentId;
    std::string itemName;
};

struct CTSRemovedSourceFolder
{
    LLUUID folderId;
    LLUUID originalParentId;
    std::string folderName;
};

struct CTSUndoTransaction
{
    LLUUID productFolderId;
    std::string productName;
    std::vector<CTSUndoMove> moves;
    std::vector<LLUUID> createdFolderIds;
    std::vector<CTSRemovedSourceFolder> removedSourceFolders;
};

struct CTSUndoResult
{
    bool success = false;
    std::size_t itemsRestored = 0;
    std::size_t itemsSkipped = 0;
    std::size_t foldersRemoved = 0;
    std::size_t foldersRestored = 0;
    std::size_t foldersKept = 0;
    std::string message;
};

class CTSUndoManager
{
public:
    using completion_func_t = std::function<void(const CTSUndoResult&)>;
    static CTSUndoManager& instance();
    void setLastTransaction(const CTSUndoTransaction& transaction);
    bool canUndo() const;
    void clear();
    const CTSUndoTransaction& lastTransaction() const;
    void undo(completion_func_t completion);

private:
    CTSUndoManager() = default;
    bool mHasTransaction = false;
    CTSUndoTransaction mLastTransaction;
};

#endif
