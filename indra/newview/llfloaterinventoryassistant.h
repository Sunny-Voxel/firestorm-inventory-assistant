#ifndef LL_LLFLOATERINVENTORYASSISTANT_H
#define LL_LLFLOATERINVENTORYASSISTANT_H

#include "llfloater.h"
#include "lluuid.h"

class LLScrollListCtrl;
class LLTextBox;
class LLTextEditor;

class LLFloaterInventoryAssistant final : public LLFloater
{
public:
    explicit LLFloaterInventoryAssistant(const LLSD& key);
    ~LLFloaterInventoryAssistant() override = default;

    bool postBuild() override;
    void onOpen(const LLSD& key) override;

private:
    void analyzeFolder();
    void showDryRun(bool repairMode);
    void onPlanSelectionChanged();

    void onAnalyzeClicked();
    void onRepairClicked();
    void onOrganizeClicked();
    void onCloseClicked();

    LLUUID mFolderId;
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
