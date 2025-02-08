#ifndef _ENT_SHARED_H_
#define _ENT_SHARED_H_

#define CONTENTS_EMPTY		0
#define CONTENTS_WORLD		(1 << 0)
#define CONTENTS_SOLID		(1 << 1)
#define CONTENTS_TRIGGER	(1 << 2)
#define CONTENTS_WATER		(1 << 3)
#define CONTENTS_ALL		(CONTENTS_WORLD | CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_WATER)

typedef enum
{
	ENTID_REMOVE = -1,

	ENTID_WORLDSPAWN,
	ENTID_PLAYER,
	ENTID_WALL,

	MAX_ENTMAP = 256
}entid_e;

typedef struct
{
	struct entity_t* owner;
}entbase_s;

#endif