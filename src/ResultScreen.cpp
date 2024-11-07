#include "ResultScreen.hpp"
#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "BulletManager.hpp"
#include "Chain.hpp"
#include "ChainPriorities.hpp"
#include "FileSystem.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "ReplayManager.hpp"
#include "SoundPlayer.hpp"
#include "Stage.hpp"
#include "i18n.hpp"
#include "utils.hpp"
#include <cstdio>
#include <cstring>
#include <direct.h>
#include <time.h>

namespace th06
{

DIFFABLE_STATIC_ARRAY_ASSIGN(f32, 5, g_DifficultyWeightsList) = {-30.0f, -10.0f, 20.0f, 30.0f, 30.0f};

DIFFABLE_STATIC_ASSIGN(u32, g_DefaultMagic) = 'DMYS';

DIFFABLE_STATIC_ASSIGN(char *, g_AlphabetList) =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ.,:;･@abcdefghijklmnopqrstuvwxyz+-/*=%0123456789(){}[]<>#!?'\"$      --";

DIFFABLE_STATIC_ARRAY_ASSIGN(char *, 6, g_CharacterList) = {TH_HAKUREI_REIMU_SPIRIT,  TH_HAKUREI_REIMU_DREAM,
                                                            TH_KIRISAME_MARISA_DEVIL, TH_KIRISAME_MARISA_LOVE,
                                                            TH_SATSUKI_RIN_FLOWER,    TH_SATSUKI_RIN_WIND};

DIFFABLE_STATIC_ARRAY_ASSIGN(f32, 5, g_SpellcardsWeightsList) = {1.0f, 1.5f, 1.5f, 2.0f, 2.5f};

DIFFABLE_STATIC_ARRAY_ASSIGN(char *, 5, g_RightAlignedDifficultyList) = {"     Easy", "   Normal", "     Hard",
                                                                         "  Lunatic", "    Extra"};

DIFFABLE_STATIC_ARRAY_ASSIGN(char *, 4, g_ShortCharacterList2) = {"ReimuA ", "ReimuB ", "MarisaA", "MarisaB"};

#define DEFAULT_HIGH_SCORE_NAME "Nanashi "

#pragma function(memset)
#pragma optimize("s", on)
ResultScreen::ResultScreen()
{
    i32 unused[12];
    memset(this, 0, sizeof(ResultScreen));
    this->cursor = 1;
}
#pragma optimize("", on)

#pragma optimize("s", on)
#pragma var_order(resultScreen, unused)
ZunResult ResultScreen::RegisterChain(i32 unk)
{

    i32 unused[16];
    ResultScreen *resultScreen;
    resultScreen = new ResultScreen();

    utils::DebugPrint(TH_DBG_RESULTSCREEN_COUNAT, g_GameManager.counat);

    resultScreen->calcChain = g_Chain.CreateElem((ChainCallback)ResultScreen::OnUpdate);
    resultScreen->calcChain->addedCallback = (ChainAddedCallback)ResultScreen::AddedCallback;
    resultScreen->calcChain->deletedCallback = (ChainDeletedCallback)ResultScreen::DeletedCallback;
    resultScreen->calcChain->arg = resultScreen;

    if (unk != 0)
    {
        if (!g_GameManager.isInPracticeMode)
        {
            resultScreen->resultScreenState = RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME;
        }
        else
        {
            resultScreen->resultScreenState = RESULT_SCREEN_STATE_EXIT;
        }
    }

    if (g_Chain.AddToCalcChain(resultScreen->calcChain, TH_CHAIN_PRIO_CALC_RESULTSCREEN))
    {
        return ZUN_ERROR;
    }

    resultScreen->drawChain = g_Chain.CreateElem((ChainCallback)ResultScreen::OnDraw);
    resultScreen->drawChain->arg = resultScreen;
    g_Chain.AddToDrawChain(resultScreen->drawChain, TH_CHAIN_PRIO_DRAW_RESULTSCREEN);

    return ZUN_SUCCESS;
}
#pragma optimize("", on)

#pragma function("strcpy")
#pragma optimize("s", on)
#pragma var_order(i, sprite, character, slot)
ZunResult ResultScreen::AddedCallback(ResultScreen *resultScreen)
{

    i32 slot;
    i32 characterShot;
    AnmVm *sprite;
    i32 i;

    if (resultScreen->resultScreenState != RESULT_SCREEN_STATE_EXIT)
    {

        if (g_AnmManager->LoadSurface(0, "data/result/result.jpg") != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }

        if (g_AnmManager->LoadAnm(ANM_FILE_RESULT00, "data/result00.anm", ANM_OFFSET_RESULT00) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }

        if (g_AnmManager->LoadAnm(ANM_FILE_RESULT01, "data/result01.anm", ANM_OFFSET_RESULT01) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }

        if (g_AnmManager->LoadAnm(ANM_FILE_RESULT02, "data/result02.anm", ANM_OFFSET_RESULT02) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }

        if (g_AnmManager->LoadAnm(ANM_FILE_RESULT03, "data/result03.anm", ANM_OFFSET_RESULT03) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }

        sprite = &resultScreen->unk_40[0];
        for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, sprite++)
        {

            sprite->pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            sprite->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

            // Execute all the scripts from the start of result00 to the end of result02
            g_AnmManager->SetAndExecuteScriptIdx(sprite, ANM_SCRIPT_RESULT00_START + i);
        }

        sprite = &resultScreen->unk_28a0[0];
        for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_28a0); i++, sprite++)
        {
            g_AnmManager->InitializeAndSetSprite(sprite, ANM_SCRIPT_TEXT_RESULTSCREEN_CHARACTER_NAME + i);

            sprite->pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

            sprite->flags.anchor = AnmVmAnchor_TopLeft;

            sprite->fontWidth = 15;
            sprite->fontHeight = 15;
        }
    }

    for (i = 0; i < HSCR_NUM_DIFFICULTIES; i++)
    {
        for (characterShot = 0; characterShot < HSCR_NUM_CHARS_SHOTTYPES; characterShot++)
        {
            for (slot = 0; slot < HSCR_NUM_SCORES_SLOTS; slot++)
            {
                resultScreen->defaultScore[i][characterShot][slot].score = 1000000 - slot * 100000;
                resultScreen->defaultScore[i][characterShot][slot].base.magic = g_DefaultMagic;
                resultScreen->defaultScore[i][characterShot][slot].difficulty = i;
                resultScreen->defaultScore[i][characterShot][slot].base.version = TH6K_VERSION;
                resultScreen->defaultScore[i][characterShot][slot].base.unkLen = 28;
                resultScreen->defaultScore[i][characterShot][slot].base.th6kLen = 28;
                resultScreen->defaultScore[i][characterShot][slot].stage = 1;
                resultScreen->defaultScore[i][characterShot][slot].base.unk_9 = 0;

                resultScreen->LinkScoreEx(resultScreen->defaultScore[i][characterShot] + slot, i, characterShot);

                strcpy(resultScreen->defaultScore[i][characterShot][slot].name, DEFAULT_HIGH_SCORE_NAME);
            }
        }
    }

    resultScreen->unk_14 = 0;
    resultScreen->scoreDat = ResultScreen::OpenScore("score.dat");

    for (i = 0; i < HSCR_NUM_DIFFICULTIES; i++)
    {
        for (characterShot = 0; characterShot < HSCR_NUM_CHARS_SHOTTYPES; characterShot++)
        {
            ResultScreen::GetHighScore(resultScreen->scoreDat, &resultScreen->scores[i][characterShot], characterShot,
                                       i);
        }
    }

    if (resultScreen->resultScreenState != RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME &&
        resultScreen->resultScreenState != RESULT_SCREEN_STATE_EXIT)
    {
        ParseCatk(resultScreen->scoreDat, g_GameManager.catk);
        ParseClrd(resultScreen->scoreDat, g_GameManager.clrd);
        ParsePscr(resultScreen->scoreDat, (Pscr *)g_GameManager.pscr);
    }

    if (resultScreen->resultScreenState == RESULT_SCREEN_STATE_EXIT &&
        g_GameManager.pscr[g_GameManager.CharacterShotType()][g_GameManager.currentStage - 1][g_GameManager.difficulty]
                .score < g_GameManager.score)
    {
        g_GameManager.pscr[g_GameManager.CharacterShotType()][g_GameManager.currentStage - 1][g_GameManager.difficulty]
            .score = g_GameManager.score;
    }

    resultScreen->unk_39a0.activeSpriteIndex = -1;

    return ZUN_SUCCESS;
}
#pragma optimize("", on)
#pragma intrinsic("strcpy")

#pragma optimize("s", on)
void ResultScreen::MoveCursor(ResultScreen *resultScreen, i32 length)
{
    if (WAS_PRESSED_WEIRD(TH_BUTTON_UP))
    {
        resultScreen->cursor--;
        if (resultScreen->cursor < 0)
        {
            resultScreen->cursor += length;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
    }
    if (WAS_PRESSED_WEIRD(TH_BUTTON_DOWN))
    {
        resultScreen->cursor++;
        if (resultScreen->cursor >= length)
        {
            resultScreen->cursor -= length;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
    }
}
#pragma optimize("", on)

#pragma optimize("s", on)
ZunBool ResultScreen::MoveCursorHorizontally(ResultScreen *resultScreen, i32 length)
{
    if (WAS_PRESSED_WEIRD(TH_BUTTON_LEFT))
    {
        resultScreen->cursor--;
        if (resultScreen->cursor < 0)
        {
            resultScreen->cursor += length;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
        return true;
    }
    else if (WAS_PRESSED_WEIRD(TH_BUTTON_RIGHT))
    {
        resultScreen->cursor++;
        if (resultScreen->cursor >= length)
        {
            resultScreen->cursor -= length;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
        return true;
    }
    else
    {
        return false;
    }
}
#pragma optimize("", on)

#pragma optimize("s", on)
ZunResult ResultScreen::CheckConfirmButton()
{
    AnmVm *viewport;

    switch (this->resultScreenState)
    {
    case RESULT_SCREEN_STATE_STATS_SCREEN:
        if (this->frameTimer <= 30)
        {
            viewport = &this->unk_40[37];
            viewport->pendingInterrupt = 16;
        }
        if (this->frameTimer >= 90 && WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            viewport = &this->unk_40[37];
            viewport->pendingInterrupt = 2;
            this->frameTimer = 0;
            this->resultScreenState = RESULT_SCREEN_STATE_STATS_TO_SAVE_TRANSITION;
        }
        break;

    case RESULT_SCREEN_STATE_STATS_TO_SAVE_TRANSITION:
        if (this->frameTimer >= 30)
        {
            this->frameTimer = 59;
            this->resultScreenState = RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION;
        }
        break;
    }
    return ZUN_SUCCESS;
}
#pragma optimize("", on)

#pragma optimize("s", on)
#pragma var_order(viewport, strPos, unknownFloat, completion, slowdownRate, color)
u32 ResultScreen::DrawFinalStats()
{
    f32 completion;
    f32 unknownFloat;
    D3DXVECTOR3 strPos;
    AnmVm *viewport;
    i32 color;
    f32 slowdownRate;

    switch (this->resultScreenState)
    {
    case RESULT_SCREEN_STATE_STATS_SCREEN:
    case RESULT_SCREEN_STATE_STATS_TO_SAVE_TRANSITION:

        viewport = &this->unk_40[37];
        color = viewport->color;
        g_AsciiManager.color = color;
        unknownFloat = 0.0;

        completion = g_GameManager.difficulty < 4 ? g_GameManager.counat / 39600.0f : g_GameManager.counat / 89500.0f;
        strPos = viewport->pos;
        strPos.x += 224.0f;
        strPos.y += 32.0f;
        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.score);

        if (g_GameManager.guiScore < 2000000)
        {
            unknownFloat -= 20.0f;
        }
        else if (g_GameManager.guiScore < 200000000)
        {
            unknownFloat += (g_GameManager.guiScore - 2000000) / 198000000.0f * 60.0f - 20.0f;
        }
        else
        {
            unknownFloat += 40.0f;
        }

        strPos.y += 22.0f;
        g_AsciiManager.AddString(&strPos, g_RightAlignedDifficultyList[g_GameManager.difficulty]);

        unknownFloat += g_DifficultyWeightsList[g_GameManager.difficulty];
        strPos.y += 22.0f;
        if (g_GameManager.difficulty == EASY || !g_GameManager.isGameCompleted)
        {
            g_AsciiManager.AddFormatText(&strPos, "    %3.2f%%", completion * 100.0f);
            unknownFloat += completion * 70.0f;
        }
        else
        {
            g_AsciiManager.AddFormatText(&strPos, "      100%%");
            unknownFloat += 70.0f;
        }
        strPos.y += 22.0f;
        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.numRetries);

        unknownFloat -= g_GameManager.numRetries * 10.0f;
        strPos.y += 22.0f;

        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.deaths);

        unknownFloat -= g_GameManager.deaths * 5.0f - 10.0f;

        strPos.y += 22.0f;

        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.bombsUsed);

        unknownFloat -= g_GameManager.bombsUsed * 2.0f - 10.0f;
        strPos.y += 22.0f;

        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.spellcardsCaptured);

        unknownFloat += g_GameManager.spellcardsCaptured * g_SpellcardsWeightsList[g_GameManager.difficulty];

        slowdownRate = (g_Supervisor.unk1b4 / g_Supervisor.unk1b8 - 0.5f) * 2;

        if (slowdownRate < 0.0f)
        {
            slowdownRate = 0.0f;
        }
        else if (slowdownRate >= 1.0f)
        {
            slowdownRate = 1.0f;
        }

        slowdownRate = (1 - slowdownRate) * 100.0f;

        strPos.y += 22.0f;
        g_AsciiManager.AddFormatText(&strPos, "    %3.2f%%", slowdownRate);

        if (slowdownRate < 50.0f)
        {
            unknownFloat -= 70.0f * slowdownRate / 100.0f;
        }
        else
        {
            unknownFloat = -999.0f;
        }
        // Useless calculations, maybe in earlier versions it showed the point items and graze, but it was later
        // removed? unknowFloat is also unused, maybe it was some kind of grading system
        if (g_GameManager.pointItemsCollected < 800)
        {
            unknownFloat += 0.01f * g_GameManager.pointItemsCollected;
        }
        else
        {
            unknownFloat += 8.0f;
        }

        if (g_GameManager.grazeInTotal < 5000)
        {
            unknownFloat += 0.0025f * g_GameManager.grazeInTotal;
        }
        else
        {
            unknownFloat += 12.5f;
        }

        g_AsciiManager.color = COLOR_WHITE;
    }
    return 0;
}
#pragma optimize("", on)

#pragma function("strcpy")
#pragma optimize("s", on)
#pragma var_order(idx, sprite, replayNameIdx, replayNameIdx2)
i32 ResultScreen::HandleResultKeyboard()
{
    i32 idx;
    AnmVm *sprite;
    i32 replayNameIdx;
    i32 replayNameIdx2;

    if (this->frameTimer == 0)
    {
        this->charUsed = g_GameManager.character;
        this->diffSelected = g_GameManager.difficulty;

        sprite = &this->unk_40[0];
        for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
        {
            sprite->pendingInterrupt = this->diffSelected + 3;
        }

        AnmManager::DrawStringFormat2(g_AnmManager, this->unk_28a0, COLOR_RGB(COLOR_WHITE), COLOR_RGB(COLOR_BLACK),
                                      g_CharacterList[this->charUsed * 2]);
        if (g_GameManager.shotType != SHOT_TYPE_A)
        {
            this->unk_28a0[0].color = COLOR_TRANSPARENT_WHITE;
        }

        AnmManager::DrawStringFormat2(g_AnmManager, &this->unk_28a0[1], COLOR_RGB(COLOR_WHITE), COLOR_RGB(COLOR_BLACK),
                                      g_CharacterList[this->charUsed * 2]);
        if (g_GameManager.shotType != SHOT_TYPE_B)
        {
            this->unk_28a0[1].color = COLOR_TRANSPARENT_WHITE;
        }

        this->hscr.character = this->charUsed * 2 + g_GameManager.shotType;
        this->hscr.difficulty = this->diffSelected;
        this->hscr.score = g_GameManager.score;
        this->hscr.base.version = 16;
        this->hscr.base.magic = *(i32 *)"HSCR";

        if (g_GameManager.isGameCompleted == 0)
        {
            this->hscr.stage = g_GameManager.currentStage;
        }
        else
        {
            this->hscr.stage = 99;
        }

        this->hscr.base.unk_9 = 1;
        strcpy(this->hscr.name, "        ");

        if (this->LinkScoreEx(&this->hscr, this->diffSelected, this->charUsed * 2 + g_GameManager.shotType) >= 10)
            goto RETURN_TO_STATS_SCREEN_WITHOUT_SOUND;

        this->cursor = 0;
        strcpy(this->replayName, "");
    }
    if (this->frameTimer < 30)
    {
        return 0;
    }
    if (WAS_PRESSED_WEIRD(TH_BUTTON_UP))
    {
        for (;;)
        {
            this->selectedCharacter -= RESULT_KEYBOARD_COLUMNS;

            if (this->selectedCharacter < 0)
            {
                this->selectedCharacter += RESULT_KEYBOARD_CHARACTERS;
            }

            if (g_AlphabetList[this->selectedCharacter] == ' ')
            {
                continue;
            }
            break;
        };
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
    }
    if (WAS_PRESSED_WEIRD(TH_BUTTON_DOWN))
    {
        for (;;)
        {
            this->selectedCharacter += RESULT_KEYBOARD_COLUMNS;

            if (this->selectedCharacter >= RESULT_KEYBOARD_CHARACTERS)
            {
                this->selectedCharacter -= RESULT_KEYBOARD_CHARACTERS;
            }

            if (g_AlphabetList[this->selectedCharacter] == ' ')
            {
                continue;
            }
            break;
        };
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
    }
    if (WAS_PRESSED_WEIRD(TH_BUTTON_LEFT))
    {
        for (;;)
        {
            this->selectedCharacter--;
            if (this->selectedCharacter % RESULT_KEYBOARD_COLUMNS == RESULT_KEYBOARD_COLUMNS - 1)
            {
                this->selectedCharacter += RESULT_KEYBOARD_COLUMNS;
            }

            if (this->selectedCharacter < 0)
            {
                this->selectedCharacter = RESULT_KEYBOARD_COLUMNS - 1;
            }

            if (g_AlphabetList[this->selectedCharacter] == ' ')
            {
                continue;
            }
            break;
        };
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
    }
    if (WAS_PRESSED_WEIRD(TH_BUTTON_RIGHT))
    {
        for (;;)
        {
            this->selectedCharacter++;

            if (this->selectedCharacter % RESULT_KEYBOARD_COLUMNS == 0)
            {
                this->selectedCharacter -= RESULT_KEYBOARD_COLUMNS;
            }

            if (g_AlphabetList[this->selectedCharacter] == ' ')
            {
                continue;
            }
            break;
        };
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
    }
    if (WAS_PRESSED_WEIRD(TH_BUTTON_SELECTMENU))
    {
        replayNameIdx = this->cursor >= 8 ? 7 : this->cursor;

        if (this->selectedCharacter < RESULT_KEYBOARD_SPACE)
        {
            this->hscr.name[replayNameIdx] = g_AlphabetList[this->selectedCharacter];
        }
        else if (this->selectedCharacter == RESULT_KEYBOARD_SPACE)
        {
            this->hscr.name[replayNameIdx] = ' ';
        }
        else
        {
            goto RETURN_TO_STATS_SCREEN;
        }

        if (this->cursor < 8)
        {
            this->cursor++;
            if (this->cursor == 8)
            {
                this->selectedCharacter = RESULT_KEYBOARD_END;
            }
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
    }

    if (WAS_PRESSED_WEIRD(TH_BUTTON_RETURNMENU))
    {
        replayNameIdx2 = this->cursor >= 8 ? 7 : this->cursor;

        if (this->cursor > 0)
        {
            this->cursor--;
            this->hscr.name[replayNameIdx2] = ' ';
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
    }
    if (WAS_PRESSED(TH_BUTTON_MENU))
    {
    RETURN_TO_STATS_SCREEN:
        g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);

    RETURN_TO_STATS_SCREEN_WITHOUT_SOUND:

        this->resultScreenState = RESULT_SCREEN_STATE_STATS_SCREEN;
        this->frameTimer = 0;

        sprite = &this->unk_40[0];
        for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
        {
            sprite->pendingInterrupt = 2;
        }
        strcpy(this->replayName, this->hscr.name);
    }
    return 0;
}
#pragma optimize("", on)
#pragma intrinsic("strcpy")

#pragma optimize("s", on)
#pragma var_order(sprite, saveInterrupt, idx, replayLoaded, replayToReadPath, replayNameCharacter, replayPath,         \
                  replayNameCharacter2)
i32 ResultScreen::HandleReplaySaveKeyboard()
{
    AnmVm *sprite;
    i32 replayNameCharacter2;
    char replayPath[64];
    i32 replayNameCharacter;
    char replayToReadPath[64];
    ReplayData *replayLoaded;
    i32 idx;
    i32 saveInterrupt;

    switch (this->resultScreenState)
    {
    case RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION:
        if (this->frameTimer == 60)
        {
            if (g_GameManager.numRetries != 0)
            {
                saveInterrupt = 0xc;
            }
            else
            {
                if (g_Supervisor.framerateMultiplier < 0.99f)
                {
                    saveInterrupt = 0xd;
                }
                else
                {
                    saveInterrupt = 9;
                }
            }
            sprite = &this->unk_40[1];
            for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
            {
                sprite->pendingInterrupt = saveInterrupt;
            }
            if (saveInterrupt != 9)
            {
                this->resultScreenState = RESULT_SCREEN_STATE_CANT_SAVE_REPLAY;
            }
            this->cursor = 0;
        }
        sprite = &this->unk_40[16];
        if (this->cursor == 0)
        {
            sprite[0].color = COLOR_COMBINE_ALPHA(COLOR_PASTEL_RED, sprite[0].color);
            sprite[1].color = COLOR_COMBINE_ALPHA(COLOR_ASHEN_GREY, sprite[1].color);
        }
        else
        {
            sprite[0].color = COLOR_COMBINE_ALPHA(COLOR_ASHEN_GREY, sprite[0].color);
            sprite[1].color = COLOR_COMBINE_ALPHA(COLOR_PASTEL_RED, sprite[1].color);
        }
        if (this->frameTimer < 80)
        {
            return 0;
        }
        ResultScreen::MoveCursorHorizontally(this, 2);
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU) || WAS_PRESSED(TH_BUTTON_MENU))
        {
            goto asd;
        }
        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {

            if (this->cursor == 0)
            {
            YOLO:

                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
                this->resultScreenState = RESULT_SCREEN_STATE_CHOOSING_REPLAY_FILE;

                sprite = &this->unk_40[0];
                for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
                {
                    sprite->pendingInterrupt = 0xa;
                }

                this->frameTimer = 0;
                goto CHOOSE_REPLAY_FILE;
            }

        asd:

            this->frameTimer = 0;
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            this->resultScreenState = RESULT_SCREEN_STATE_EXITING;
            sprite = &this->unk_40[0];
            for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
            {
                sprite->pendingInterrupt = 2;
            }
        }
        break;
    case RESULT_SCREEN_STATE_CANT_SAVE_REPLAY:

        if (this->frameTimer < 0x14)
        {
            return 0;
        }

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU) || WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {

            this->frameTimer = 0;
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            this->resultScreenState = RESULT_SCREEN_STATE_EXITING;
            sprite = &this->unk_40[0];
            for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
            {
                sprite->pendingInterrupt = 2;
            }
        }
        break;

    case RESULT_SCREEN_STATE_CHOOSING_REPLAY_FILE:

    CHOOSE_REPLAY_FILE:

        if (this->frameTimer == 0)
        {
            _mkdir("replay");
            for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->replays); idx++)
            {
                sprintf(replayToReadPath, "./replay/th6_%.2d.rpy", idx + 1);
                replayLoaded = (ReplayData *)FileSystem::OpenPath(replayToReadPath, 1);
                if (replayLoaded == NULL)
                {
                    continue;
                }

                if (ReplayManager::ValidateReplayData(replayLoaded, g_LastFileSize) == ZUN_SUCCESS)
                {
                    this->replays[idx] = *replayLoaded;
                }
                free(replayLoaded);
            }
        }

        if (this->frameTimer < 20)
        {
            return 0;
        }

        MoveCursor(this, 15);
        this->replayNumber = this->cursor;
        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
            this->replayNumber = this->cursor;
            this->frameTimer = 0;
            _strdate(this->defaultReplayMaybe.date);
            (this->defaultReplayMaybe).score = g_GameManager.score;
            if (*(i32 *)&this->replays[this->cursor].magic != *(i32 *)&"PR6T" ||
                this->replays[this->cursor].version != 0x102)
            {
                sprite = &this->unk_40[0];
                for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
                {
                    sprite->pendingInterrupt = 0xf;
                }
                sprite = &this->unk_40[this->replayNumber + 0x16];
                sprite->pendingInterrupt = 0xe;
                this->resultScreenState = RESULT_SCREEN_STATE_WRITING_REPLAY_NAME;
            }
            else
            {
                sprite = &this->unk_40[0];
                for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
                {
                    sprite->pendingInterrupt = 0xb;
                }
                sprite = &this->unk_40[this->replayNumber + 0x16];
                sprite->pendingInterrupt = 0xe;
                this->resultScreenState = RESULT_SCREEN_STATE_OVERWRITE_REPLAY_FILE;
            }
            this->cursor = 0;
            this->selectedCharacter = 0;
        }
        if (WAS_PRESSED(10))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            this->resultScreenState = RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION;
            sprite = &this->unk_40[0];
            for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
            {
                sprite->pendingInterrupt = 2;
            }
            this->frameTimer = 0;
        }
        break;
    case RESULT_SCREEN_STATE_WRITING_REPLAY_NAME:
        if (this->frameTimer < 30)
        {
            return 0;
        }
        if (WAS_PRESSED_WEIRD(TH_BUTTON_UP))
        {
            for (;;)
            {
                this->selectedCharacter -= RESULT_KEYBOARD_COLUMNS;

                if (this->selectedCharacter < 0)
                {
                    this->selectedCharacter += RESULT_KEYBOARD_CHARACTERS;
                }

                if (g_AlphabetList[this->selectedCharacter] == ' ')
                {
                    continue;
                }
                break;
            };
            g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
        }
        if (WAS_PRESSED_WEIRD(TH_BUTTON_DOWN))
        {
            for (;;)
            {
                this->selectedCharacter += RESULT_KEYBOARD_COLUMNS;

                if (this->selectedCharacter >= RESULT_KEYBOARD_CHARACTERS)
                {
                    this->selectedCharacter -= RESULT_KEYBOARD_CHARACTERS;
                }

                if (g_AlphabetList[this->selectedCharacter] == ' ')
                {
                    continue;
                }
                break;
            };
            g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
        }
        if (WAS_PRESSED_WEIRD(TH_BUTTON_LEFT))
        {
            for (;;)
            {
                this->selectedCharacter--;
                if (this->selectedCharacter % RESULT_KEYBOARD_COLUMNS == RESULT_KEYBOARD_COLUMNS - 1)
                {
                    this->selectedCharacter += RESULT_KEYBOARD_COLUMNS;
                }

                if (this->selectedCharacter < 0)
                {
                    this->selectedCharacter = RESULT_KEYBOARD_COLUMNS - 1;
                }

                if (g_AlphabetList[this->selectedCharacter] == ' ')
                {
                    continue;
                }
                break;
            };
            g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
        }
        if (WAS_PRESSED_WEIRD(TH_BUTTON_RIGHT))
        {
            for (;;)
            {
                this->selectedCharacter++;
                if (this->selectedCharacter % RESULT_KEYBOARD_COLUMNS == 0)
                {
                    this->selectedCharacter -= RESULT_KEYBOARD_COLUMNS;
                }

                if (g_AlphabetList[this->selectedCharacter] == ' ')
                {
                    continue;
                }
                break;
            };
            g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
        }
        if (WAS_PRESSED_WEIRD(TH_BUTTON_SELECTMENU))
        {

            replayNameCharacter = this->cursor >= 8 ? 7 : this->cursor;

            if (this->selectedCharacter < RESULT_KEYBOARD_SPACE)
            {
                this->replayName[replayNameCharacter] = g_AlphabetList[this->selectedCharacter];
            }
            else if (this->selectedCharacter == RESULT_KEYBOARD_SPACE)
            {
                this->replayName[replayNameCharacter] = ' ';
            }
            else
            {
                sprintf(replayPath, "./replay/th6_%.2d.rpy", this->replayNumber + 1);
                ReplayManager::SaveReplay(replayPath, this->replayName);
                this->frameTimer = 0;
                this->resultScreenState = RESULT_SCREEN_STATE_EXITING;
                sprite = &this->unk_40[0];
                for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
                {
                    sprite->pendingInterrupt = 2;
                }
            }
            if (this->cursor < 8)
            {
                this->cursor++;
                if (this->cursor == 8)
                {
                    this->selectedCharacter = RESULT_KEYBOARD_END;
                }
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
        }

        if (WAS_PRESSED_WEIRD(TH_BUTTON_RETURNMENU))
        {
            replayNameCharacter2 = this->cursor >= 8 ? 7 : this->cursor;

            if (this->cursor > 0)
            {
                this->cursor--;
                this->replayName[replayNameCharacter2] = ' ';
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
        }
        if (WAS_PRESSED(TH_BUTTON_MENU))
        {
            goto YOLO;
        }
        break;

    case RESULT_SCREEN_STATE_OVERWRITE_REPLAY_FILE:
        sprite = &this->unk_40[16];
        if (this->cursor == 0)
        {
            sprite[0].color = COLOR_COMBINE_ALPHA(COLOR_PASTEL_RED, sprite[0].color);
            sprite[1].color = COLOR_COMBINE_ALPHA(COLOR_ASHEN_GREY, sprite[1].color);
        }
        else
        {
            sprite[0].color = COLOR_COMBINE_ALPHA(COLOR_ASHEN_GREY, sprite[0].color);
            sprite[1].color = COLOR_COMBINE_ALPHA(COLOR_PASTEL_RED, sprite[1].color);
        }

        if (this->frameTimer < 20)
        {
            return 0;
        }
        MoveCursorHorizontally(this, 2);

        if (WAS_PRESSED(TH_BUTTON_RETURNMENU) || WAS_PRESSED(TH_BUTTON_MENU))
        {
            goto YOLO;
        }

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {

            this->frameTimer = 0;
            if (this->cursor == 0)
            {
                sprite = &this->unk_40[0];
                for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
                {
                    sprite->pendingInterrupt = 15;
                }
                sprite = &this->unk_40[this->replayNumber + 22];
                sprite->pendingInterrupt = 14;
                this->resultScreenState = RESULT_SCREEN_STATE_WRITING_REPLAY_NAME;
                break;
            }
            goto YOLO;
        }
    }

LAB_0042d095:

    return 0;
}

#pragma optimize("s", on)

#pragma optimize("s", on)
#pragma var_order(highScore, remainingSize, scoreData, dataScore, score)
u32 ResultScreen::GetHighScore(ScoreDat *scoreDat, ScoreListNode *node, u32 character, u32 difficulty)
{
    u32 score;
    u32 dataScore;
    i32 remainingSize;
    Hscr *highScore;
    ScoreDat *scoreData;

    scoreData = scoreDat;

    if (node == NULL)
    {
        ResultScreen::ZunFreeArray(scoreData->scores);
        scoreData->scores->next = NULL;
        scoreData->scores->data = NULL;
        scoreData->scores->prev = NULL;
    }

    remainingSize = scoreData->fileLen;
    highScore = (Hscr *)scoreData->ShiftBytes(scoreData->dataOffset);
    remainingSize -= scoreData->dataOffset;

    while (remainingSize > 0)
    {
        if (highScore->base.magic == 'RCSH' && highScore->base.version == TH6K_VERSION &&
            highScore->character == character && highScore->difficulty == difficulty)
        {
            if (node != NULL)
            {
                ResultScreen::LinkScore(node, highScore);
            }
            else
            {
                ResultScreen::LinkScore(scoreData->scores, highScore);
            }
        }

        remainingSize -= highScore->base.th6kLen;
        highScore = highScore->ShiftBytes(highScore->base.th6kLen);
    }
    if (scoreData->scores->next != NULL)
    {
        if (scoreData->scores->next->data->score > 1000000)
        {
            dataScore = scoreData->scores->next->data->score;
        }
        else
        {
            dataScore = 1000000;
        }
        score = dataScore;
    }
    else
    {
        score = 1000000;
    }
    return score;
}
#pragma optimize("", on)

#pragma optimize("s", on)
#pragma var_order(scoreData, bytesShifted, xorValue, checksum, bytes, remainingData, decryptedFilePointer, fileLen,    \
                  scoreDatSize, scoreListNodeSize)
ScoreDat *ResultScreen::OpenScore(char *path)
{
    u8 *bytes;
    i32 bytesShifted;
    i32 fileLen;
    Th6k *decryptedFilePointer;
    i32 remainingData;
    i32 scoreListNodeSize;
    u16 checksum;
    u8 xorValue;
    i32 scoreDatSize;
    ScoreDat *scoreData;

    scoreData = (ScoreDat *)FileSystem::OpenPath(path, true);
    if (scoreData == NULL)
    {
    FAILED_TO_READ:
        scoreDatSize = sizeof(ScoreDat);
        scoreData = (ScoreDat *)malloc(scoreDatSize);
        scoreData->dataOffset = sizeof(ScoreDat);
        scoreData->fileLen = sizeof(ScoreDat);
    }
    else
    {
        if (g_LastFileSize < sizeof(ScoreDat))
        {
            free(scoreData);
            goto FAILED_TO_READ;
        }

        remainingData = g_LastFileSize - 2;
        checksum = 0;
        xorValue = 0;
        bytesShifted = 0;
        bytes = &scoreData->xorseed[1];

        while (0 < remainingData)
        {

            xorValue += bytes[0];
            // Invert top 3 bits and bottom 5 bits
            xorValue = (xorValue & 0xe0) >> 5 | (xorValue & 0x1f) << 3;
            // xor one byte later with the resulting inverted bits
            bytes[1] ^= xorValue;
            if (bytesShifted >= 2)
            {
                checksum += bytes[1];
            }
            bytes++;
            remainingData--;
            bytesShifted++;
        }
        if (scoreData->csum != checksum)
        {
            free(scoreData);
            goto FAILED_TO_READ;
        }
        fileLen = scoreData->fileLen;
        decryptedFilePointer = scoreData->ShiftBytes(scoreData->dataOffset);
        fileLen -= scoreData->dataOffset;
        while (fileLen > 0)
        {
            if (decryptedFilePointer->magic == 'K6HT')
                break;

            decryptedFilePointer = decryptedFilePointer->ShiftBytes(decryptedFilePointer->th6kLen);
            fileLen = fileLen - decryptedFilePointer->th6kLen;
        }
        if (fileLen <= 0)
        {
            free(scoreData);
            goto FAILED_TO_READ;
        };
    }
    scoreListNodeSize = sizeof(ScoreListNode);
    scoreData->scores = (ScoreListNode *)malloc(scoreListNodeSize);
    scoreData->scores->next = NULL;
    scoreData->scores->data = NULL;
    scoreData->scores->prev = NULL;
    return scoreData;
}
#pragma optimize("", on)

#pragma optimize("s", on)
#pragma var_order(parsedCatk, cursor, sd)
ZunResult ResultScreen::ParseCatk(ScoreDat *scoreDat, Catk *outCatk)
{

    i32 cursor;
    Catk *parsedCatk;
    ScoreDat *sd;
    sd = scoreDat;

    if (outCatk == NULL)
    {
        return ZUN_ERROR;
    }

    parsedCatk = (Catk *)sd->ShiftBytes(sd->dataOffset);
    cursor = sd->fileLen - sd->dataOffset;
    while (cursor > 0)
    {
        if (parsedCatk->base.magic == 'KTAC' && parsedCatk->base.version == TH6K_VERSION)
        {
            if (parsedCatk->idx >= CATK_NUM_CAPTURES)
                break;

            outCatk[parsedCatk->idx] = *parsedCatk;
        }
        cursor -= parsedCatk->base.th6kLen;
        parsedCatk = (Catk *)&parsedCatk->name[parsedCatk->base.th6kLen - 0x18];
    }
    return ZUN_SUCCESS;
}
#pragma optimize("", on)

#pragma optimize("s", on)
#pragma var_order(parsedClrd, characterShotType, cursor, difficulty, sd)
ZunResult ResultScreen::ParseClrd(ScoreDat *scoreDat, Clrd *outClrd)
{
    i32 cursor;
    Clrd *parsedClrd;
    ScoreDat *sd;
    i32 characterShotType;
    i32 difficulty;
    sd = scoreDat;

    if (outClrd == NULL)
    {
        return ZUN_ERROR;
    }

    for (characterShotType = 0; characterShotType < CLRD_NUM_CHARACTERS; characterShotType++)
    {
        memset(&outClrd[characterShotType], 0, sizeof(Clrd));

        outClrd[characterShotType].base.magic = 'DRLC';
        outClrd[characterShotType].base.unkLen = sizeof(Clrd);
        outClrd[characterShotType].base.th6kLen = sizeof(Clrd);
        outClrd[characterShotType].base.version = TH6K_VERSION;
        outClrd[characterShotType].characterShotType = characterShotType;

        for (difficulty = 0; difficulty < ARRAY_SIZE_SIGNED(outClrd[0].difficultyClearedWithoutRetries); difficulty++)
        {
            outClrd[characterShotType].difficultyClearedWithRetries[difficulty] = 1;
            outClrd[characterShotType].difficultyClearedWithoutRetries[difficulty] = 1;
        }
    }

    parsedClrd = (Clrd *)sd->ShiftBytes(sd->dataOffset);
    cursor = sd->fileLen - sd->dataOffset;
    while (cursor > 0)
    {
        if (parsedClrd->base.magic == 'DRLC' && parsedClrd->base.version == TH6K_VERSION)
        {
            if (parsedClrd->characterShotType >= CLRD_NUM_CHARACTERS)
                break;

            outClrd[parsedClrd->characterShotType] = *parsedClrd;
        }
        cursor -= parsedClrd->base.th6kLen;
        parsedClrd = (Clrd *)((i32)&parsedClrd->base + parsedClrd->base.th6kLen);
    }
    return ZUN_SUCCESS;
}
#pragma optimize("", on)

#pragma optimize("s", on)
#pragma var_order(pscr, parsedPscr, character, stage, cursor, difficulty, sd)
ZunResult ResultScreen::ParsePscr(ScoreDat *scoreDat, Pscr *outClrd)
{
    i32 cursor;
    Pscr *parsedPscr;
    ScoreDat *sd;
    i32 stage;
    i32 character;
    i32 difficulty;
    sd = scoreDat;
    Pscr *pscr;

    if (outClrd == NULL)
    {
        return ZUN_ERROR;
    }

    for (pscr = outClrd, character = 0; character < PSCR_NUM_CHARS_SHOTTYPES; character++)
    {
        for (stage = 0; stage < PSCR_NUM_STAGES; stage++)
        {
            for (difficulty = 0; difficulty < PSCR_NUM_DIFFICULTIES; difficulty++, pscr++)
            {

                memset(pscr, 0, sizeof(Pscr));

                pscr->base.magic = 'RCSP';
                pscr->base.unkLen = sizeof(Pscr);
                pscr->base.th6kLen = sizeof(Pscr);
                pscr->base.version = 16;
                pscr->character = character;
                pscr->difficulty = difficulty;
                pscr->stage = stage;
            }
        }
    }

    parsedPscr = (Pscr *)sd->ShiftBytes(sd->dataOffset);
    cursor = sd->fileLen - sd->dataOffset;

    while (cursor > 0)
    {
        if (parsedPscr->base.magic == 'RCSP' && parsedPscr->base.version == TH6K_VERSION)
        {
            pscr = parsedPscr;
            if (pscr->character >= PSCR_NUM_CHARS_SHOTTYPES || pscr->difficulty >= PSCR_NUM_DIFFICULTIES + 1 ||
                pscr->stage >= PSCR_NUM_STAGES + 1)
                break;

            outClrd[pscr->character * 6 * 4 + pscr->stage * 4 + pscr->difficulty] = *pscr;
        }
        cursor -= parsedPscr->base.th6kLen;
        parsedPscr = parsedPscr->ShiftBytes(parsedPscr->base.th6kLen);
    }
    return ZUN_SUCCESS;
}
#pragma optimize("", on)

#pragma optimize("s", on)
#pragma var_order(i, vm, characterShotType, difficulty)
ChainCallbackResult ResultScreen::OnUpdate(ResultScreen *resultScreen)
{
    i32 difficulty;
    i32 characterShotType;
    AnmVm *vm;
    i32 i;
    switch (resultScreen->resultScreenState)
    {

    case RESULT_SCREEN_STATE_EXIT:
        g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;

    case RESULT_SCREEN_STATE_INIT:

        if (resultScreen->frameTimer == 0)
        {

            vm = &resultScreen->unk_40[0];
            for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
            {
                vm->pendingInterrupt = 1;
                vm->flags.colorOp = 1;
                if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                {
                    vm->color &= COLOR_BLACK;
                }
                else
                {
                    vm->color &= COLOR_WHITE;
                }
            }

            vm = &resultScreen->unk_40[1];
            for (i = 0; i <= 6; i++, vm++)
            {
                if (i == resultScreen->cursor)
                {
                    if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                    {
                        vm->color = COLOR_DARK_GREY;
                    }
                    else
                    {
                        vm->color = COLOR_WHITE;
                    }

                    vm->posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
                }
                else
                {
                    if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                    {
                        vm->color = COLOR_SET_ALPHA(COLOR_BLACK, 176);
                    }
                    else
                    {
                        vm->color = COLOR_SET_ALPHA(COLOR_WHITE, 176);
                    }
                    vm->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                }
            }
        }

        if (resultScreen->frameTimer < 20)
        {
            break;
        }

        resultScreen->resultScreenState++;
        resultScreen->frameTimer = 0;

    case RESULT_SCREEN_STATE_CHOOSING_DIFFICULTY:

        ResultScreen::MoveCursor(resultScreen, 7);

        vm = &resultScreen->unk_40[1];
        for (i = 0; i <= 6; i++, vm++)
        {
            if (i == resultScreen->cursor)
            {
                if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                {
                    vm->color = COLOR_DARK_GREY;
                }
                else
                {
                    vm->color = COLOR_WHITE;
                }
                vm->posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
            }
            else
            {
                if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                {
                    vm->color = COLOR_SET_ALPHA(COLOR_BLACK, 176);
                }
                else
                {
                    vm->color = COLOR_SET_ALPHA(COLOR_WHITE, 176);
                }
                vm->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            }
        }

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            vm = &resultScreen->unk_40[0];
            switch (resultScreen->cursor)
            {
            case RESULT_SCREEN_CURSOR_EASY:
            case RESULT_SCREEN_CURSOR_NORMAL:
            case RESULT_SCREEN_CURSOR_HARD:
            case RESULT_SCREEN_CURSOR_LUNATIC:
            case RESULT_SCREEN_CURSOR_EXTRA:
                for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
                {
                    vm->pendingInterrupt = resultScreen->cursor + 3;
                }
                resultScreen->diffSelected = resultScreen->cursor;

                resultScreen->resultScreenState = resultScreen->cursor + RESULT_SCREEN_STATE_BEST_SCORES_EASY;
                resultScreen->unk_c = resultScreen->resultScreenState;
                resultScreen->frameTimer = 0;
                resultScreen->cursor = resultScreen->unk_14;
                resultScreen->charUsed = -1;
                resultScreen->lastSpellcardSelected = -1;
                break;

            case RESULT_SCREEN_CURSOR_SPELLCARDS:
                for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
                {
                    vm->pendingInterrupt = resultScreen->cursor + 3;
                }
                resultScreen->diffSelected = resultScreen->cursor;
                resultScreen->resultScreenState = RESULT_SCREEN_STATE_SPELLCARDS;
                resultScreen->unk_c = resultScreen->resultScreenState;
                resultScreen->frameTimer = 0;
                resultScreen->charUsed = -1;
                resultScreen->cursor = resultScreen->previousCursor;
                resultScreen->lastSpellcardSelected = -1;
                break;

            case RESULT_SCREEN_CURSOR_EXIT:
                for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
                {
                    vm->pendingInterrupt = 2;
                }
                resultScreen->resultScreenState = RESULT_SCREEN_STATE_EXITING;
                g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            }
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            resultScreen->cursor = RESULT_SCREEN_CURSOR_EXIT;
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
        }
        break;

    case RESULT_SCREEN_STATE_EXITING:

        if (resultScreen->frameTimer < 60)
        {
            break;
        }
        else
        {
            g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }

    case RESULT_SCREEN_STATE_BEST_SCORES_EXTRA:

        if (IS_PRESSED(TH_BUTTON_FOCUS) || IS_PRESSED(TH_BUTTON_SKIP))
        {

            if (resultScreen->cheatCodeStep < 5)
            {
                if (WAS_PRESSED(TH_BUTTON_HOME))
                {
                    resultScreen->cheatCodeStep++;
                }
                else if (WAS_PRESSED(TH_BUTTON_WRONG_CHEATCODE))
                {
                    resultScreen->cheatCodeStep = 0;
                }
            }
            else if (resultScreen->cheatCodeStep < 7)
            {
                if (WAS_PRESSED(TH_BUTTON_Q))
                {

                    resultScreen->cheatCodeStep++;
                }
                else if (WAS_PRESSED(TH_BUTTON_WRONG_CHEATCODE))
                {
                    resultScreen->cheatCodeStep = 0;
                }
            }
            else if (resultScreen->cheatCodeStep < 10)
            {
                if (WAS_PRESSED(TH_BUTTON_S))
                {
                    resultScreen->cheatCodeStep++;
                }
                else if (WAS_PRESSED(TH_BUTTON_WRONG_CHEATCODE))
                {
                    resultScreen->cheatCodeStep = 0;
                }
            }
            else
            {
                for (characterShotType = 0; characterShotType < HSCR_NUM_CHARS_SHOTTYPES; characterShotType++)
                {
                    for (difficulty = 0; difficulty < HSCR_NUM_DIFFICULTIES; difficulty++)
                    {
                        g_GameManager.clrd[characterShotType].difficultyClearedWithRetries[difficulty] = 99;
                        g_GameManager.clrd[characterShotType].difficultyClearedWithoutRetries[difficulty] = 99;
                    }
                }
                resultScreen->cheatCodeStep = 0;
                g_SoundPlayer.PlaySoundByIdx(SOUND_1UP, 0);
            }
        }
        else
        {
            resultScreen->cheatCodeStep = 0;
        }
    case RESULT_SCREEN_STATE_BEST_SCORES_EASY:
    case RESULT_SCREEN_STATE_BEST_SCORES_NORMAL:
    case RESULT_SCREEN_STATE_BEST_SCORES_HARD:
    case RESULT_SCREEN_STATE_BEST_SCORES_LUNATIC:

        if (resultScreen->charUsed != resultScreen->cursor && resultScreen->frameTimer == 20)
        {
            resultScreen->charUsed = resultScreen->cursor;
            AnmManager::DrawStringFormat2(g_AnmManager, &resultScreen->unk_28a0[0], COLOR_RGB(COLOR_WHITE),
                                          COLOR_RGB(COLOR_BLACK), g_CharacterList[resultScreen->charUsed * 2]);
            AnmManager::DrawStringFormat2(g_AnmManager, &resultScreen->unk_28a0[1], COLOR_RGB(COLOR_WHITE),
                                          COLOR_RGB(COLOR_BLACK), g_CharacterList[resultScreen->charUsed * 2 + 1]);
        }
        if (resultScreen->frameTimer < 30)
        {
            break;
        }
        if (ResultScreen::MoveCursorHorizontally(resultScreen, 2))
        {
            resultScreen->frameTimer = 0;
            vm = &resultScreen->unk_40[0];
            for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
            {
                vm->pendingInterrupt = resultScreen->diffSelected + 3;
            }
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            resultScreen->resultScreenState = RESULT_SCREEN_STATE_INIT;
            resultScreen->frameTimer = 1;
            vm = &resultScreen->unk_40[0];
            for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
            {
                vm->pendingInterrupt = 1;
            }
            resultScreen->unk_14 = resultScreen->cursor;
            resultScreen->cursor = resultScreen->diffSelected;
        }

        break;

    case RESULT_SCREEN_STATE_SPELLCARDS:

        if (resultScreen->lastSpellcardSelected != resultScreen->cursor && resultScreen->frameTimer == 20)
        {

            resultScreen->lastSpellcardSelected = resultScreen->cursor;
            for (i = resultScreen->lastSpellcardSelected * 10; i < resultScreen->lastSpellcardSelected * 10 + 10; i++)
            {
                if (i >= ARRAY_SIZE_SIGNED(g_GameManager.catk))
                {
                    break;
                }
                if (g_GameManager.catk[i].numAttempts == 0)
                {
                    AnmManager::DrawVmTextFmt(g_AnmManager, &resultScreen->unk_28a0[i % 10], COLOR_RGB(COLOR_WHITE),
                                              COLOR_RGB(COLOR_BLACK), TH_UNKNOWN_SPELLCARD);
                }
                else
                {
                    AnmManager::DrawVmTextFmt(g_AnmManager, &resultScreen->unk_28a0[i % 10], COLOR_RGB(COLOR_WHITE),
                                              COLOR_RGB(COLOR_BLACK), g_GameManager.catk[i].name);
                }
            }
        }
        if (resultScreen->frameTimer < 30)
        {
            break;
        }
        if (ResultScreen::MoveCursorHorizontally(resultScreen, 7))
        {
            resultScreen->frameTimer = 0;
            vm = &resultScreen->unk_40[0];
            for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
            {
                vm->pendingInterrupt = resultScreen->diffSelected + 3;
            }
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            resultScreen->resultScreenState = RESULT_SCREEN_STATE_INIT;
            resultScreen->frameTimer = 1;
            vm = &resultScreen->unk_40[0];
            for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
            {
                vm->pendingInterrupt = 1;
            }
            resultScreen->previousCursor = resultScreen->cursor;
            resultScreen->cursor = resultScreen->diffSelected;
        }
        break;

    case RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME:
        resultScreen->HandleResultKeyboard();
        break;

    case RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION:
    case RESULT_SCREEN_STATE_CANT_SAVE_REPLAY:
    case RESULT_SCREEN_STATE_CHOOSING_REPLAY_FILE:
    case RESULT_SCREEN_STATE_WRITING_REPLAY_NAME:
    case RESULT_SCREEN_STATE_OVERWRITE_REPLAY_FILE:
        resultScreen->HandleReplaySaveKeyboard();
        break;

    case RESULT_SCREEN_STATE_STATS_SCREEN:
    case RESULT_SCREEN_STATE_STATS_TO_SAVE_TRANSITION:
        resultScreen->CheckConfirmButton();
        break;
    };

    vm = &resultScreen->unk_40[0];
    for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
    {
        g_AnmManager->ExecuteScript(vm);
    }
    resultScreen->frameTimer++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}
#pragma optimize("", on)

#pragma optimize("s", on)
#pragma var_order(strPos, row, local_20, local_24, ShootScoreListNodeA, column, ShootScoreListNodeB, local_40,         \
                  local_44, local_48, local_4c, unused, unused2, unused3, unk, keyboardCharacter)
ChainCallbackResult th06::ResultScreen::OnDraw(ResultScreen *resultScreen)
{
    u8 unused[12];
    u8 unused2;
    u8 unused3;

    AnmVm *local_24;
    char keyboardCharacter;
    u8 unk;
    f32 local_4c;
    f32 local_48;
    i32 local_44;
    ZunVec3 local_40;
    ScoreListNode *ShootScoreListNodeB;
    i32 column;
    ScoreListNode *ShootScoreListNodeA;

    char local_20[9];

    i32 row;
    D3DXVECTOR3 strPos;

    local_24 = &resultScreen->unk_40[0];
    g_Supervisor.viewport.X = 0;
    g_Supervisor.viewport.Y = 0;
    g_Supervisor.viewport.Width = 640;
    g_Supervisor.viewport.Height = 480;
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    g_AnmManager->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
    for (row = 0; row < 0x26; row++, local_24++)
    {
        *local_40.AsD3dXVec() = local_24->pos;
        local_24->pos += local_24->posOffset;
        g_AnmManager->DrawNoRotation(local_24);
        local_24->pos = *local_40.AsD3dXVec();
    }
    local_24 = &resultScreen->unk_40[14];
    if (local_24->pos.x < 640.0f)
    {
        if (resultScreen->unk_c != 8)
        {
            *local_40.AsD3dXVec() = local_24->pos;
            resultScreen->unk_28a0->pos = *local_40.AsD3dXVec();
            g_AnmManager->DrawNoRotation(&resultScreen->unk_28a0[0]);

            local_40.AsD3dXVec()->x += 320.0f;

            resultScreen->unk_28a0[1].pos = *local_40.AsD3dXVec();
            g_AnmManager->DrawNoRotation(&resultScreen->unk_28a0[1]);

            local_40.AsD3dXVec()->x -= -320.0f;
            local_40.AsD3dXVec()->y += 18.0f;
            local_40.AsD3dXVec()->y += 320.0f;

            ShootScoreListNodeA = resultScreen->scores[resultScreen->diffSelected][resultScreen->charUsed * 2].next;
            ShootScoreListNodeB = resultScreen->scores[resultScreen->diffSelected][resultScreen->charUsed * 2 + 1].next;
            for (row = 0; row < 10; row++)
            {
                if (resultScreen->resultScreenState == RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME)
                {
                    if (g_GameManager.shotType == SHOT_TYPE_A)
                    {
                        if (ShootScoreListNodeA->data->base.unk_9 != '\0')
                        {
                            g_AsciiManager.color = 0xfff0f0ff;

                            strcpy(local_20, "       ");
                            local_20[8] = 0;

                            local_20[resultScreen->cursor >= 8 ? 7 : resultScreen->cursor] = '_';
                            g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "   %8s", &local_20);
                        }
                        else
                        {
                            g_AsciiManager.color = 0x80ffffc0;
                        }
                    }
                    else
                    {
                        g_AsciiManager.color = 0x80ffc0c0;
                    }
                }
                else
                {
                    g_AsciiManager.color = 0xffffc0c0;
                }
                g_AsciiManager.AddFormatText(
                    local_40.AsD3dXVec(), (char *)&g_BulletManager.bullets[421].sprites.spriteBullet.posInterpFinal.x,
                    row + 1);

                local_40.x += 36.0f;
                if (ShootScoreListNodeA->data->stage <= 6)
                {
                    g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "%8s %9d(%d)", ShootScoreListNodeA->data->name,
                                                 ShootScoreListNodeA->data->score, ShootScoreListNodeA->data->stage);
                }
                else if (ShootScoreListNodeA->data->stage == 7)
                {
                    g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "%8s %9d(1)", ShootScoreListNodeA->data->name,
                                                 ShootScoreListNodeA->data->score);
                }
                else
                {
                    g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "%8s %9d(C)", ShootScoreListNodeA->data->name,
                                                 ShootScoreListNodeA->data->score);
                }
                local_40.AsD3dXVec()->x += 300.0f;
                if (resultScreen->resultScreenState == RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME)
                {
                    if (g_GameManager.shotType == SHOT_TYPE_B)
                    {
                        if (ShootScoreListNodeB->data->base.unk_9 != '\0')
                        {
                            g_AsciiManager.color = 0xfffff0f0;

                            strcpy(local_20, "       ");
                            local_20[8] = 0;

                            local_20[resultScreen->cursor >= 8 ? 7 : resultScreen->cursor] = '_';
                            g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "%8s", &local_20);
                        }
                        else
                        {
                            g_AsciiManager.color = 0xc0c0c0ff;
                        }
                    }
                    else
                    {
                        g_AsciiManager.color = 0x80c0c0ff;
                    }
                }
                else
                {
                    g_AsciiManager.color = 0xffc0c0ff;
                }
                if (ShootScoreListNodeB->data->stage <= 6)
                {
                    g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "%8s %9d(%d)", ShootScoreListNodeB->data->name,
                                                 ShootScoreListNodeB->data->score, ShootScoreListNodeB->data->stage);
                }
                else if (ShootScoreListNodeB->data->stage == 7)
                {
                    g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "%8s %9d(1)", ShootScoreListNodeB->data->name,
                                                 ShootScoreListNodeB->data->score);
                }
                else
                {
                    g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "%8s %9d(C)", ShootScoreListNodeB->data->name,
                                                 ShootScoreListNodeB->data->score);
                }
                local_40.AsD3dXVec()->x -= 336.0f;
                local_40.AsD3dXVec()->y += 336.0f;
                ShootScoreListNodeA = ShootScoreListNodeA->next;
                ShootScoreListNodeB = ShootScoreListNodeB->next;
            }
        }
        else
        {

            *local_40.AsD3dXVec() = local_24->pos;
            local_40.AsD3dXVec()->y += 16.0f;

            for (row = 0; row < 10; row++)
            {
                local_44 = resultScreen->lastSpellcardSelected * 10 + row;
                if (local_44 >= 0x40)
                {
                    break;
                }

                resultScreen->unk_28a0[row].pos = *local_40.AsD3dXVec();
                if (g_GameManager.catk[local_44].numAttempts == 0)
                {
                    g_AsciiManager.color = 0x80c0c0ff;
                }
                else if (g_GameManager.catk[local_44].numSuccess == 0)
                {
                    g_AsciiManager.color = 0xffc0a0a0;
                }
                else
                {
                    g_AsciiManager.color = 0xfff0f0ff - row * 0x80800;
                }
                g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "No.%.2d", local_44 + 1);

                // TODO: This is really cursed, there has to be a better way
                (*(ZunVec3 *)&resultScreen->unk_28a0[row].pos).AsD3dXVec()->x += 96.0f;

                g_AnmManager->DrawNoRotation(&resultScreen->unk_28a0[row]);

                local_40.AsD3dXVec()->x += 368.0f;

                g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "%3d/%3d", g_GameManager.catk[local_44].numSuccess,
                                             g_GameManager.catk[local_44].numAttempts);
                local_40.AsD3dXVec()->x -= 368.0f;
                local_40.AsD3dXVec()->y += 30.0f;
            }
        }
    }
    if (resultScreen->resultScreenState == RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME ||
        resultScreen->resultScreenState == RESULT_SCREEN_STATE_WRITING_REPLAY_NAME)
    {
        *local_40.AsD3dXVec() = D3DXVECTOR3(160.0f, 356.0f, 0.0f);

        for (row = 0; row < RESULT_KEYBOARD_ROWS; row++)
        {
            for (column = 0; column < RESULT_KEYBOARD_COLUMNS; column++)
            {
                local_48 = 0.0f;
                local_4c = 0.0f;
                if (resultScreen->selectedCharacter == row * RESULT_KEYBOARD_COLUMNS + column)
                {
                    g_AsciiManager.color = 0xffffffc0;
                    if (resultScreen->frameTimer % 0x40 < 0x20)
                    {
                        local_48 = 1.2f + 0.8f * (resultScreen->frameTimer % 0x20) / 32.0f;
                    }
                    else
                    {
                        local_48 = 2.0f - 0.8f * (resultScreen->frameTimer % 0x20) / 32.0f;
                    }
                    g_AsciiManager.scale.x = local_48;
                    g_AsciiManager.scale.y = local_48;
                    local_48 = -(local_48 - 1.0f) * 8.0f;
                    local_4c = local_48;
                }
                else
                {
                    g_AsciiManager.color = 0x60c0c0c0;
                    g_AsciiManager.scale.x = 1.0f;
                    g_AsciiManager.scale.y = 1.0f;
                }
                strPos = *local_40.AsD3dXVec();
                strPos.x += local_48;
                strPos.y += local_4c;
                keyboardCharacter = g_AlphabetList[row * 0x10 + column];
                unk = 0;

                if (row == 5)
                {
                    if (column == 0xe)
                    {
                        keyboardCharacter = 0x80; // SP
                    }
                    else if (column == 0xf)
                    {
                        keyboardCharacter = 0x81; // END
                    }
                }

                g_AsciiManager.AddString(&strPos, &keyboardCharacter);

                local_40.AsD3dXVec()->x += 20.0f;
            }
            local_40.AsD3dXVec()->x -= (column * 20);
            local_40.AsD3dXVec()->y += 18.0f;
        }
    }
    g_AsciiManager.scale.x = 1.0;
    g_AsciiManager.scale.y = 1.0;
    if ((resultScreen->resultScreenState >= RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION) &&
        (resultScreen->resultScreenState <= RESULT_SCREEN_STATE_OVERWRITE_REPLAY_FILE))
    {
        local_24 = &resultScreen->unk_40[15];
        for (row = 0; row < 6; row++, local_24++)
        {
            g_AnmManager->DrawNoRotation(local_24);
        }
        local_24 = &resultScreen->unk_40[21];
        *local_40.AsD3dXVec() = local_24->pos;
        local_24++;
        g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "No.   Name     Date     Player Score");
        for (row = 0; row < ARRAY_SIZE_SIGNED(resultScreen->replays); row++)
        {
            *local_40.AsD3dXVec() = local_24->pos;
            local_24++;
            if (row == resultScreen->replayNumber)
            {
                g_AsciiManager.color = COLOR_LIGHT_RED;
            }
            else
            {
                g_AsciiManager.color = COLOR_GREY;
            }
            if (resultScreen->resultScreenState == RESULT_SCREEN_STATE_WRITING_REPLAY_NAME)
            {
                g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "No.%.2d %8s %8s %7s %9d", row + 1,
                                             &resultScreen->replayName, resultScreen->defaultReplayMaybe.date,
                                             g_ShortCharacterList2[g_GameManager.CharacterShotType()],
                                             resultScreen->defaultReplayMaybe.score);
                g_AsciiManager.color = 0xfff0f0ff;

                strcpy(local_20, "       ");

                local_20[8] = 0;

                local_20[resultScreen->cursor >= 8 ? 7 : resultScreen->cursor] = '_';
                g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "      %8s", &local_20);
            }
            else if (*(i32 *)&resultScreen->replays[row].magic != *(i32 *)"T6RP" ||
                     resultScreen->replays[row].version != 0x102)
            {
                g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "No.%.2d -------- --/--/-- -------         0",
                                             row + 1);
            }
            else
            {
                g_AsciiManager.AddFormatText(local_40.AsD3dXVec(), "No.%.2d %8s %8s %7s %9d", row + 1,
                                             resultScreen->replays[row].name, resultScreen->replays[row].date,
                                             g_ShortCharacterList2[resultScreen->replays[row].shottypeChara],
                                             resultScreen->replays[row].score);
            }
        }
    }
    g_AsciiManager.color = COLOR_WHITE;
    resultScreen->DrawFinalStats();

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}
#pragma optimize("", on)

#pragma optimize("s", on)
#pragma var_order(difficulty, character)
ZunResult ResultScreen::DeletedCallback(ResultScreen *resultScreen)
{
    i32 character;
    i32 difficulty;

    if (resultScreen->scoreDat != NULL)
    {
        ResultScreen::WriteScore(resultScreen);
        ResultScreen::ReleaseScoreDat(resultScreen->scoreDat);
    }

    resultScreen->scoreDat = NULL;
    for (difficulty = 0; difficulty < HSCR_NUM_DIFFICULTIES; difficulty++)
    {
        for (character = 0; character < HSCR_NUM_CHARS_SHOTTYPES; character++)
        {
            resultScreen->FreeScore(difficulty, character);
        }
    }
    g_AnmManager->ReleaseAnm(ANM_FILE_RESULT00);
    g_AnmManager->ReleaseAnm(ANM_FILE_RESULT01);
    g_AnmManager->ReleaseAnm(ANM_FILE_RESULT02);
    g_AnmManager->ReleaseAnm(ANM_FILE_RESULT03);
    g_AnmManager->ReleaseSurface(0);

    g_Chain.Cut(resultScreen->drawChain);

    resultScreen->drawChain = NULL;

    delete resultScreen;
    resultScreen = NULL;

    return ZUN_SUCCESS;
}
#pragma optimize("", on)

}; // namespace th06