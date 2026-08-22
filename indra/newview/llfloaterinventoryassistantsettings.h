#ifndef LL_LLFLOATERINVENTORYASSISTANTSETTINGS_H
#define LL_LLFLOATERINVENTORYASSISTANTSETTINGS_H

#include "llfloater.h"

class LLFloaterInventoryAssistantSettings final : public LLFloater
{
public:
    explicit LLFloaterInventoryAssistantSettings(const LLSD& key);
    ~LLFloaterInventoryAssistantSettings() override = default;

    bool postBuild() override;
    void onOpen(const LLSD& key) override;

private:
    void loadSettings();
    void saveSettings();
    void onSaveClicked();
    void onCancelClicked();
};

#endif
