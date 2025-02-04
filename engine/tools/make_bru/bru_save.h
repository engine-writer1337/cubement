#ifndef _BRU_SAVE_H_
#define _BRU_SAVE_H_

extern int gnum_texinfos;
extern bru_texinfo_s gtexinfos[MAX_MAP_TEXINFOS];

extern int gnum_textures;
extern bru_texture_s gtextures[MAX_MAP_TEXTURES];

extern void bru_save(const char* filename);

#endif