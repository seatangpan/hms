#ifndef CD_H
#define CD_H

typedef struct
{
    uint32_t time;
    const char* name;
    const char* singer;
}process_track_t;

void cd_init_status();

void cd_set_status(uint32_t cd, uint32_t track);

process_track_t* cd_get_status();

process_track_t* cd_playing_next();

process_track_t* cd_playing_pre();

process_track_t* cd_load(uint32_t cd , uint32_t track);

bool cd_eject();

#endif // CD_H
