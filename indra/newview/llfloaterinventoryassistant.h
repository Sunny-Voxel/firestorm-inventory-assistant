#ifndef LL_LLFLOATERINVENTORYASSISTANT_H
#define LL_LLFLOATERINVENTORYASSISTANT_H

#include "llfloater.h"
#include "lluuid.h"
#include "fsinventorytools/ctsexecutionplan.h"

class LLScrollListCtrl;
class LLTextBox;
class LLTextEditor;
struct CTSExecutionResult;
struct CTSUndoResult;

class LLFloaterInventoryAssistant final : public LLFloater
{
public:
    explicit LLFloaterInventoryAssistant(const LLSD& key);
    ~LLFloaterInventoryAssistant() override = default;
    bool postBuild() override;
    void onOpen(const LLSD& key) override;

private:
    enum class PendingAction { None, Repair, Organize };

    void analyzeFolder();
    void prepareAction(PendingAction action);
    void executePendingAction();
    void resetWorkflowButtons();

    void onPlanSelectionChanged();
    void onRefreshClicked();
    void onRepairClicked();
    void onOrganizeClicked();
    void onUndoClicked();
    void onBodiesClicked();
    void onCloseClicked();

    void onExecutionComplete(const CTSExecutionResult& result);
    void onUndoComplete(const CTSUndoResult& result);

    LLUUID mFolderId;
    PendingAction mPendingAction = PendingAction::None;

    LLScrollListCtrl* mPlanList = nullptr;
    LLTextEditor* mDetails = nullptr;
    LLTextBox* mProductText = nullptr;
    LLTextBox* mStatusText = nullptr;
    LLTextBox* mItemsText = nullptr;
    LLTextBox* mCorrectText = nullptr;
    LLTextBox* mMovesText = nullptr;
    LLTextBox* mFoldersText = nullptr;
};

#endif
