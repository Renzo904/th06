#pragma once

#include "AnmVm.hpp"
#include "Chain.hpp"
#include "ZunResult.hpp"
#include "inttypes.hpp"

namespace th06
{
struct TrackDescriptor
{
    TrackDescriptor()
    {
        memset(this, 0, sizeof(TrackDescriptor));
    }

    char path[64];
    char title[34];
    char description[8][66];
};
C_ASSERT(sizeof(TrackDescriptor) == 0x272);

struct MusicRoom
{
    MusicRoom()
    {
        i32 unused[12];

        memset(this, 0, sizeof(MusicRoom));
    }

    static ZunResult AddedCallback(MusicRoom *musicRoom);
    static ZunResult DeletedCallback(MusicRoom *musicRoom);
    u32 DrawMusicList();
    ZunResult FUN_00424e8f();
    static ChainCallbackResult OnDraw(MusicRoom *musicRoom);
    static ChainCallbackResult OnUpdate(MusicRoom *musicRoom);
    static ZunResult RegisterChain();

    ChainElem *calc_chain;
    ChainElem *draw_chain;
    i32 unk_0x8;
    i32 shouldDrawMusicList;
    i32 cursor;
    i32 selectedSongIndex;
    i32 listingOffset;
    i32 numDescriptors;
    TrackDescriptor *trackDescriptors;
    AnmVm mainVm[1];
    AnmVm titleSprites[32];
    AnmVm descriptionSprites[16];
};
C_ASSERT(sizeof(MusicRoom) == 0x3434);
}; // namespace th06
