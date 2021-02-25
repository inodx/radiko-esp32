#ifndef _INCLUDED_RADIKO_H
#define _INCLUDED_RADIKO_H

char _auth_token[128];
char _playlist_url[128];
char _audio_url[128];
char _region_id[16];

typedef struct
{
    char id[16];
    char name[64];
    char ascii_name[32];
    char logo_xsmall[128];
} station_t;

station_t * stations;
int station_count;

void auth(void);
void get_station_list();
void generate_playlist_url(station_t * st);
#endif