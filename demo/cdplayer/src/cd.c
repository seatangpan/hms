#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "cd.h"

typedef struct
{
    uint32_t trackcnt;
    process_track_t* tracks;
}process_cd_t;

static const process_track_t JackChou[] =
{
    {5, "QingHuaCi","JackChou"},
    {6, "RainBow","JackChou"},
    {7, "YeQu","JackChou"},
    {8, "ShuangJieGun","JackChou"},
};

static const process_track_t Jain[] =
{
    {5, "Jain1","Jain"},
    {6, "Jain2","Jain"},
    {7, "Jain3","Jain"},
    {8, "Jain4","Jain"},
    {9, "Jain5","Jain"},
};

static const process_cd_t cd_items[] =
{
    {4, &JackChou},
    {5, &Jain},
};

static process_track_t current_track;
static uint32_t cd_index;
static uint32_t track_index;

void cd_init_status()
{
    cd_index = 0;
    track_index = 0;
    current_track = cd_items[cd_index].tracks[track_index];
}

void cd_set_status(uint32_t cd , uint32_t track)
{
    const uint32_t cnt = (sizeof(cd_items) / sizeof(process_cd_t));

    if (cd >= cnt)
    {
        cd_init_status();
        return;
    }

    if (track >= cd_items[cd_index].trackcnt)
    {
        cd_init_status();
        return;
    }
    
    cd_index = cd;
    track_index = track;
    current_track = cd_items[cd_index].tracks[track_index];
}

process_track_t* cd_get_status()
{
    return &current_track;
}

process_track_t* cd_playing_next()
{
    const uint32_t cnt = (sizeof(cd_items) / sizeof(process_cd_t));
    uint32_t cd = cd_index;
    uint32_t track = track_index + 1;
    if (track >= cd_items[cd].trackcnt)
    {
        track = 0;
        cd++;
        cd %= cnt;
    }
    return cd_load(cd , track);
}

process_track_t* cd_playing_pre()
{
    const uint32_t cnt = (sizeof(cd_items) / sizeof(process_cd_t));
    uint32_t cd = cd_index;
    uint32_t track = track_index - 1;
    if (track >= cd_items[cd].trackcnt)
    {
        cd--;
        cd %= cnt;
        track = cd_items[cd].trackcnt - 1;
    }
    return cd_load(cd , track);
}

process_track_t* cd_load(uint32_t cd , uint32_t track)
{
    cd_set_status(cd , track);
    return &current_track;
}

bool cd_eject()
{
    cd_init_status();
    return true;
}
