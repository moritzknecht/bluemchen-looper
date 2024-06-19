#pragma once
// control menu items
FullScreenItemMenu mainMenu;
FullScreenItemMenu controlEditMenu;
FullScreenItemMenu fx1EditMenu;
FullScreenItemMenu fx2EditMenu;
FullScreenItemMenu reverbEditMenu;

LoopView    loopView;
const int   loopLengths[] = {16, 32, 64, 128, 256};
const char *barValues[]   = {"1 Bar", "2 Bars", "4 Bars", "8 Bars", "16 Bars"};
MappedStringListValue barListValues(barValues, 5, 2);

MappedFloatValue fx1ResoValue
    = MappedFloatValue(0, 1, 0, MappedFloatValue::Mapping::lin, "", 2);
MappedFloatValue fx1RevAmountValue
    = MappedFloatValue(0, 1, 0, MappedFloatValue::Mapping::lin, "", 2);
MappedFloatValue volumeValue
    = MappedFloatValue(0, 1, 1, MappedFloatValue::Mapping::lin, "", 2);
MappedFloatValue midsideValue
    = MappedFloatValue(0, 2, 1, MappedFloatValue::Mapping::lin, "", 2);
MappedIntValue crossFaderValue
    = MappedIntValue(0, CROSSFADER_RESOLUTION, 0, 1, 1);

const int                kNumMainMenuItems = 7;
AbstractMenu::ItemConfig mainMenuItems[kNumMainMenuItems];
const int                kNumControlEditMenuItems = 2;
AbstractMenu::ItemConfig controlEditMenuItems[kNumControlEditMenuItems];
const int                kNumfx1EditMenuItems = 3;
AbstractMenu::ItemConfig fx1EditMenuItems[kNumfx1EditMenuItems];
const int                kNumfx2EditMenuItems = 3;
AbstractMenu::ItemConfig fx2EditMenuItems[kNumfx2EditMenuItems];
const int                kNumReverbEditMenuItems = 5;
AbstractMenu::ItemConfig reverbEditMenuItems[kNumReverbEditMenuItems];

void InitUiPages()
{
    // ====================================================================
    // The main menu
    // ====================================================================

    mainMenuItems[0].type = daisy::AbstractMenu::ItemType::customItem;
    mainMenuItems[0].text = "Loop";
    mainMenuItems[0].asCustomItem.itemObject = &loopView;

    mainMenuItems[1].type = daisy::AbstractMenu::ItemType::valueItem;
    mainMenuItems[1].text = "Length";
    mainMenuItems[1].asMappedValueItem.valueToModify = &barListValues;

    mainMenuItems[2].type = daisy::AbstractMenu::ItemType::openUiPageItem;
    mainMenuItems[2].text = "Knob 1";
    mainMenuItems[2].asOpenUiPageItem.pageToOpen = &fx1EditMenu;

    mainMenuItems[3].type = daisy::AbstractMenu::ItemType::openUiPageItem;
    mainMenuItems[3].text = "Knob 2";
    mainMenuItems[3].asOpenUiPageItem.pageToOpen = &fx2EditMenu;

    mainMenuItems[4].type = daisy::AbstractMenu::ItemType::openUiPageItem;
    mainMenuItems[4].text = "Reverb";
    mainMenuItems[4].asOpenUiPageItem.pageToOpen = &reverbEditMenu;

    mainMenuItems[5].type = daisy::AbstractMenu::ItemType::valueItem;
    mainMenuItems[5].text = "Mid/Side";
    mainMenuItems[5].asMappedValueItem.valueToModify = &midsideValue;

    mainMenuItems[6].type = daisy::AbstractMenu::ItemType::valueItem;
    mainMenuItems[6].text = "Volume";
    mainMenuItems[6].asMappedValueItem.valueToModify = &volumeValue;


    /*
    mainMenuItems[3].type = daisy::AbstractMenu::ItemType::valueItem;
    mainMenuItems[4].text = "FX 2";
    mainMenuItems[5].asOpenUiPageItem.pageToOpen = &fx2EditMenu;

    mainMenuItems[3].type = daisy::AbstractMenu::ItemType::valueItem;
    mainMenuItems[3].text = "Reverb";
    mainMenuItems[3].asOpenUiPageItem.pageToOpen = &reverbEditMenu;
*/
    // mainMenuItems[1].asMappedValueItem.valueToModify = &barListValues;

    mainMenu.Init(mainMenuItems, kNumMainMenuItems);
    // mainMenu.OnOkayButton(1, false);

    fx1EditMenuItems[0].type = daisy::AbstractMenu::ItemType::valueItem;
    fx1EditMenuItems[0].text = "Filter Res";
    fx1EditMenuItems[0].asMappedValueItem.valueToModify = &fx1ResoValue;

    fx1EditMenuItems[1].type = daisy::AbstractMenu::ItemType::valueItem;
    fx1EditMenuItems[1].text = "Rev Amount";
    fx1EditMenuItems[1].asMappedValueItem.valueToModify = &fx1RevAmountValue;

    fx1EditMenuItems[2].type = daisy::AbstractMenu::ItemType::closeMenuItem;
    fx1EditMenuItems[2].text = "Back";

    fx1EditMenu.Init(fx1EditMenuItems, kNumfx1EditMenuItems);

    fx2EditMenuItems[0].type = daisy::AbstractMenu::ItemType::valueItem;
    fx2EditMenuItems[0].text = "Filter Res";
    fx2EditMenuItems[0].asMappedValueItem.valueToModify = &fx1ResoValue;

    fx2EditMenuItems[1].type = daisy::AbstractMenu::ItemType::valueItem;
    fx2EditMenuItems[1].text = "Rev Amount";
    fx2EditMenuItems[1].asMappedValueItem.valueToModify = &fx1RevAmountValue;

    fx2EditMenuItems[2].type = daisy::AbstractMenu::ItemType::closeMenuItem;
    fx2EditMenuItems[2].text = "Back";

    fx2EditMenu.Init(fx2EditMenuItems, kNumfx2EditMenuItems);

    reverbEditMenuItems[0].type = daisy::AbstractMenu::ItemType::valueItem;
    reverbEditMenuItems[0].text = "Amount";
    reverbEditMenuItems[0].asMappedValueItem.valueToModify = &fx1ResoValue;

    reverbEditMenuItems[1].type = daisy::AbstractMenu::ItemType::valueItem;
    reverbEditMenuItems[1].text = "Width";
    reverbEditMenuItems[1].asMappedValueItem.valueToModify = &fx1RevAmountValue;

    reverbEditMenuItems[2].type = daisy::AbstractMenu::ItemType::valueItem;
    reverbEditMenuItems[2].text = "Dampening";
    reverbEditMenuItems[2].asMappedValueItem.valueToModify = &fx1RevAmountValue;

    reverbEditMenuItems[3].type = daisy::AbstractMenu::ItemType::valueItem;
    reverbEditMenuItems[3].text = "LPF";
    reverbEditMenuItems[3].asMappedValueItem.valueToModify = &fx1RevAmountValue;

    reverbEditMenuItems[4].type = daisy::AbstractMenu::ItemType::closeMenuItem;
    reverbEditMenuItems[4].text = "Back";

    reverbEditMenu.Init(reverbEditMenuItems, kNumReverbEditMenuItems);
    // controlEditMenuItems[1].type = daisy::AbstractMenu::ItemType::closeMenuItem;
    // controlEditMenuItems[1].text = "Back";

    // controlEditMenu.Init(controlEditMenuItems, kNumControlEditMenuItems);
}