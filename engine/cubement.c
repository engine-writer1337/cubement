#include "cubement.h"

game_s ggame;
engine_s gengine;

SAVEFUNC void Cubement(engine_s** e, game_s* g)
{
	*e = &gengine;
	memcpy(&ggame, g, sizeof(ggame));
}