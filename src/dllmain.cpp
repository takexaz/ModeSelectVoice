#include <Mebius.hpp>

#include <Windows.h>
#include <iostream>
#include <vector>

static auto GameLoadSystem = reinterpret_cast<BOOL(*)(void)>(0x47f8b0);
static auto ModeSelect = reinterpret_cast<void (*)(void)>(0x42f0c0);
static auto SndReadFile = reinterpret_cast<uint32_t (*)(const char* path)>(0x44d400);
static auto GameSndPlay = reinterpret_cast<void(*)(uint32_t PSOUND, int grp, int no, int undef)>(0x44e2d0);

static std::vector<std::pair<int, int>> sounds_select;
static std::vector<std::pair<int, int>> sounds_confirm;

static uint32_t PSND = 0;
static std::string snd_path;

using namespace mebius::util;
using namespace mebius::config;

void ModeSelect_InHook_Select(mebius::inline_hook::PMBCONTEXT context) {
    int selected_item = context->Esi;
    if (sounds_select.size() >= selected_item) {
        int idx = sounds_select[selected_item].first;
        int snd = sounds_select[selected_item].second;
        if (idx != -1 && snd != -1 && PSND != 0) {
            GameSndPlay(PSND, idx, snd, 0);
        }
    }
}

void ModeSelect_InHook_Confirm(mebius::inline_hook::PMBCONTEXT context) {
    int selected_item = context->Esi;
    if (sounds_confirm.size() >= selected_item) {
        int idx = sounds_confirm[selected_item].first;
        int snd = sounds_confirm[selected_item].second;
        if (idx != -1 && snd != -1 && PSND != 0) {
            GameSndPlay(PSND, idx, snd, 0);
        }
    }
}


void GameLoadSystem_Hook(void) {
    // sndを読み込み
    PSND = SndReadFile(snd_path.c_str());
    if (PSND == 0) {
        PLOGE << std::format("Could not load {}", snd_path);
        return;
    }
    // サウンド再生フック(Up)
    mebius::inline_hook::HookInline(ModeSelect, 0x44F, ModeSelect_InHook_Select);
    PLOGD << "Inline Hooked ModeSelect+0x44F";
    // サウンド再生フック(Down)
    mebius::inline_hook::HookInline(ModeSelect, 0x48A, ModeSelect_InHook_Select);
    PLOGD << "Inline Hooked ModeSelect+0x48A";

    // サウンド再生フック(Confirm)
    mebius::inline_hook::HookInline(ModeSelect, 0x5AB, ModeSelect_InHook_Confirm);
    PLOGD << "Inline Hooked ModeSelect+0x5AB";
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: { 
        std::string path = get_module_directory(GetCurrentModule());
        std::string config_path = path + "ModeSelectVoice.toml";
        Config msv_cf(config_path.c_str());
        if (!msv_cf.is_loaded()) {
            PLOGE << std::format("Could not load {}", "ModeSelectVoice.toml");
            return TRUE;
        }
        if (!msv_cf.get_bool("Options.Enable").value_or(false)) {
            return TRUE;
        }

        // SNDのパスを保存
        if (!msv_cf.get_string("Options.Snd").has_value()) {
            PLOGE << "Could not find Options.Snd";
            return TRUE;
        }
        snd_path = path + msv_cf.get_string("Options.Snd").value();

        uint32_t idx, snd;
        // 各選択時サウンドのindex, numberを保存
        {
            idx = (uint32_t)msv_cf.get_int("Select.Menu.Arcade[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Select.Menu.Arcade[1]").value_or(0xFFFFFFFF);
            sounds_select.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Select.Menu.Versus[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Select.Menu.Versus[1]").value_or(0xFFFFFFFF);
            sounds_select.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Select.Menu.TeamArcade[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Select.Menu.TeamArcade[1]").value_or(0xFFFFFFFF);
            sounds_select.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Select.Menu.TeamVersus[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Select.Menu.TeamVersus[1]").value_or(0xFFFFFFFF);
            sounds_select.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Select.Menu.TeamCoop[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Select.Menu.TeamCoop[1]").value_or(0xFFFFFFFF);
            sounds_select.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Select.Menu.Survival[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Select.Menu.Survival[1]").value_or(0xFFFFFFFF);
            sounds_select.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Select.Menu.SurvivalCoop[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Select.Menu.SurvivalCoop[1]").value_or(0xFFFFFFFF);
            sounds_select.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Select.Menu.Training[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Select.Menu.Training[1]").value_or(0xFFFFFFFF);
            sounds_select.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Select.Menu.Watch[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Select.Menu.Watch[1]").value_or(0xFFFFFFFF);
            sounds_select.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Select.Menu.Options[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Select.Menu.Options[1]").value_or(0xFFFFFFFF);
            sounds_select.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Select.Menu.Exit[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Select.Menu.Exit[1]").value_or(0xFFFFFFFF);
            sounds_select.push_back({ idx, snd });
        }

        // 各決定時サウンドのindex, numberを保存
        {
            idx = (uint32_t)msv_cf.get_int("Confirm.Menu.Arcade[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Confirm.Menu.Arcade[1]").value_or(0xFFFFFFFF);
            sounds_confirm.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Confirm.Menu.Versus[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Confirm.Menu.Versus[1]").value_or(0xFFFFFFFF);
            sounds_confirm.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Confirm.Menu.TeamArcade[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Confirm.Menu.TeamArcade[1]").value_or(0xFFFFFFFF);
            sounds_confirm.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Confirm.Menu.TeamVersus[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Confirm.Menu.TeamVersus[1]").value_or(0xFFFFFFFF);
            sounds_confirm.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Confirm.Menu.TeamCoop[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Confirm.Menu.TeamCoop[1]").value_or(0xFFFFFFFF);
            sounds_confirm.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Confirm.Menu.Survival[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Confirm.Menu.Survival[1]").value_or(0xFFFFFFFF);
            sounds_confirm.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Confirm.Menu.SurvivalCoop[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Confirm.Menu.SurvivalCoop[1]").value_or(0xFFFFFFFF);
            sounds_confirm.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Confirm.Menu.Training[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Confirm.Menu.Training[1]").value_or(0xFFFFFFFF);
            sounds_confirm.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Confirm.Menu.Watch[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Confirm.Menu.Watch[1]").value_or(0xFFFFFFFF);
            sounds_confirm.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Confirm.Menu.Options[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Confirm.Menu.Options[1]").value_or(0xFFFFFFFF);
            sounds_confirm.push_back({ idx, snd });
            idx = (uint32_t)msv_cf.get_int("Confirm.Menu.Exit[0]").value_or(0xFFFFFFFF);
            snd = (uint32_t)msv_cf.get_int("Confirm.Menu.Exit[1]").value_or(0xFFFFFFFF);
            sounds_confirm.push_back({ idx, snd });
        }

        // sndファイル読み込み用hook
        mebius::hook::HookOnHead(GameLoadSystem, GameLoadSystem_Hook);
        break;
    }
    case DLL_THREAD_ATTACH: {
        break;
    }
    case DLL_THREAD_DETACH: {
        break;
    }
    case DLL_PROCESS_DETACH: {
        break;
    }
    }
    return TRUE;
}
