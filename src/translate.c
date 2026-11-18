#include "common/common.h"
#include "vars.h"

#if defined(ENABLE_TRANSLATION)

#include "translate.h"

#include <ctype.h>
#include <string.h>

char* TRS_GetString(u32 addr, const char* originalText)
{
    // TODO: Test code
    if (originalText == 0)
        return 0;

    int len = strlen(originalText);
    if (len == 0)
    {
        return "";
    }

    char* newText = malloc(len + 1);

    for (int i = 0; i < len; i++)
    {
        newText[i] = toupper(originalText[i]);
    }

    newText[len] = 0;

    return newText;
}

static void TRS_PatchTable(char** table, int count)
{
    //debug("Count: %d", count);
    for (int i = 0; i < count; i++)
    {
        if (table[i] == 0)
            continue;

        char* text = table[i];

        //debug("- %s", text);

        char* end;
        int addr = strtol(text, &end, 16);
        ASSERT((int)(end - text) == 6);

        char* originalText = end;
        if (*originalText == '|')
            originalText++;

        table[i] = TRS_GetString(addr, originalText);
    }
}

void TRS_Initialize(void)
{
    TRS_PatchTable(D_17f6_equipNames, ARRAYSIZE(D_17f6_equipNames));
    TRS_PatchTable(D_1856_enemyNames, ARRAYSIZE(D_1856_enemyNames));
    TRS_PatchTable(D_18b6_enemyNamesPlural, ARRAYSIZE(D_18b6_enemyNamesPlural));
    TRS_PatchTable(D_1916_itemNames, ARRAYSIZE(D_1916_itemNames));
    TRS_PatchTable(D_1962_equipShortNames, ARRAYSIZE(D_1962_equipShortNames));
    TRS_PatchTable(D_19d2_reagentNames, ARRAYSIZE(D_19d2_reagentNames));
    TRS_PatchTable(D_19e2_spellNames, ARRAYSIZE(D_19e2_spellNames));
    TRS_PatchTable(D_1a44_classNames, ARRAYSIZE(D_1a44_classNames));
    TRS_PatchTable(D_1a6a_statusNames, ARRAYSIZE(D_1a6a_statusNames));
    TRS_PatchTable(D_1bfc_mantraWords, ARRAYSIZE(D_1bfc_mantraWords));
    TRS_PatchTable(D_1c30_spellCodes, ARRAYSIZE(D_1c30_spellCodes));
    TRS_PatchTable(D_1e3a_mapNames, ARRAYSIZE(D_1e3a_mapNames));
    TRS_PatchTable(D_1f4e_virtueNames, ARRAYSIZE(D_1f4e_virtueNames));
    TRS_PatchTable(D_1f5e_mantras, ARRAYSIZE(D_1f5e_mantras));
    TRS_PatchTable((char**)D_21ca_shopNames, 8 * 16);
    TRS_PatchTable((char**)D_22ca_shopOwners, 8 * 16);
    TRS_PatchTable(D_24ea_wordTable, ARRAYSIZE(D_24ea_wordTable));
    //TRS_PatchTable(D_25ea_fileNames, ARRAYSIZE(D_25ea_fileNames));
    //TRS_PatchTable(D_2652_townFileNames, ARRAYSIZE(D_2652_townFileNames));
    TRS_PatchTable(D_27dc_shadowlordTraits, ARRAYSIZE(D_27dc_shadowlordTraits));
    TRS_PatchTable(D_2e10_dungeonPlaceNames, ARRAYSIZE(D_2e10_dungeonPlaceNames));
    TRS_PatchTable(D_3666_fieldLabels, ARRAYSIZE(D_3666_fieldLabels));
    TRS_PatchTable(D_3810_lookPictures, ARRAYSIZE(D_3810_lookPictures));
    TRS_PatchTable(D_3c20_reagentNames, ARRAYSIZE(D_3c20_reagentNames));
    TRS_PatchTable(D_3ca6_buyOffers, ARRAYSIZE(D_3ca6_buyOffers));
    TRS_PatchTable(D_3cae_cantPayInsults, ARRAYSIZE(D_3cae_cantPayInsults));
    TRS_PatchTable(D_3cb6_showPrompts, ARRAYSIZE(D_3cb6_showPrompts));
    TRS_PatchTable(D_3cce_shopItemNames, ARRAYSIZE(D_3cce_shopItemNames));
    TRS_PatchTable(D_3d2e_sellPrompts, ARRAYSIZE(D_3d2e_sellPrompts));
    TRS_PatchTable(D_3d36_farewells, ARRAYSIZE(D_3d36_farewells));
    TRS_PatchTable(D_3d3e_sellMorePrompts, ARRAYSIZE(D_3d3e_sellMorePrompts));
    TRS_PatchTable(D_3d46_buyOrSellGreetings, ARRAYSIZE(D_3d46_buyOrSellGreetings));
    TRS_PatchTable(D_3d4a_goodReplies, ARRAYSIZE(D_3d4a_goodReplies));
    TRS_PatchTable(D_3d52_stockIntros, ARRAYSIZE(D_3d52_stockIntros));

    D_3d5a_noNeedOfArt = TRS_GetString(0x3d5a, D_3d5a_noNeedOfArt);

    TRS_PatchTable(D_3e0c_numberNames, ARRAYSIZE(D_3e0c_numberNames));
    TRS_PatchTable(D_3e32_tensNames, ARRAYSIZE(D_3e32_tensNames));
    TRS_PatchTable(D_3e42_ordinalNames, ARRAYSIZE(D_3e42_ordinalNames));
    TRS_PatchTable(D_3e72_harvestNames, ARRAYSIZE(D_3e72_harvestNames));
    TRS_PatchTable(D_419c_potionColors, ARRAYSIZE(D_419c_potionColors));
    TRS_PatchTable(D_41ac_scrollRunes, ARRAYSIZE(D_41ac_scrollRunes));
    TRS_PatchTable(D_444a_shadowlordNames, ARRAYSIZE(D_444a_shadowlordNames));
    TRS_PatchTable(D_4502_vileWords, ARRAYSIZE(D_4502_vileWords));
    //TRS_PatchTable(D_4aa0_talkFileNames, ARRAYSIZE(D_4aa0_talkFileNames));
    TRS_PatchTable(D_4aa8_standardWords, ARRAYSIZE(D_4aa8_standardWords));
    TRS_PatchTable(D_4b3e_virtueKeywords, ARRAYSIZE(D_4b3e_virtueKeywords));
    TRS_PatchTable(D_4c74_pubTopicWords, ARRAYSIZE(D_4c74_pubTopicWords));
    TRS_PatchTable(D_4ca8_pubPersons, ARRAYSIZE(D_4ca8_pubPersons));
    TRS_PatchTable(D_4cf6_pubPlaces, ARRAYSIZE(D_4cf6_pubPlaces));
    TRS_PatchTable(D_515c_sceneCaptions, ARRAYSIZE(D_515c_sceneCaptions));
}

#endif
