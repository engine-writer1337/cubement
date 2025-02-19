#include "game.h"

typedef struct { int state, down[2]; } kbutton_s;

static kbutton_s in_left;
static kbutton_s in_right;
static kbutton_s in_forward;
static kbutton_s in_back;
static kbutton_s in_use;

static void plr_key_down(kbutton_s* b, const char* arg1)
{
	int k;

	if (arg1[0])
		k = atoi(arg1);
	else
		k = -1;

	if (k == b->down[0] || k == b->down[1])
		return;

	if (!b->down[0])
		b->down[0] = k;
	else if (!b->down[1])
		b->down[1] = k;
	else
		return;

	if (b->state & 1)
		return;

	b->state |= 3;
}

static void plr_key_up(kbutton_s* b, const char* arg1)
{
	int k;

	if (arg1[0])
		k = atoi(arg1);
	else
	{
		b->down[0] = b->down[1] = 0;
		b->state = 4;
		return;
	}

	if (b->down[0] == k)
		b->down[0] = 0;
	else if (b->down[1] == k)
		b->down[1] = 0;
	else
		return;

	if (b->down[0] || b->down[1] || !(b->state & 1))
		return;

	b->state &= ~1;
	b->state |= 4;
}

static int plr_key_bits()
{
	int bits = 0;

	if (in_forward.state & 3)
		bits |= IN_FORWARD;
	if (in_back.state & 3)
		bits |= IN_BACK;
	if (in_left.state & 3)
		bits |= IN_LEFT;
	if (in_right.state & 3)
		bits |= IN_RIGHT;
	if (in_use.state & 3)
		bits |= IN_USE;

	in_forward.state &= ~2;
	in_back.state &= ~2;
	in_left.state &= ~2;
	in_right.state &= ~2;
	in_use.state &= ~2;
	return bits;
}

#define cmd_key_up(name, kbtn)		void name(const char* arg1, const char* arg2) { plr_key_up(kbtn, arg1); }
#define cmd_key_down(name, kbtn)	void name(const char* arg1, const char* arg2) { plr_key_down(kbtn, arg1); }

cmd_key_up(forward_up, &in_forward);
cmd_key_down(forward_down, &in_forward);
cmd_key_up(back_up, &in_back);
cmd_key_down(back_down, &in_back);
cmd_key_up(left_up, &in_left);
cmd_key_down(left_down, &in_left);
cmd_key_up(right_up, &in_right);
cmd_key_down(right_down, &in_right);
cmd_key_up(use_up, &in_use);
cmd_key_down(use_down, &in_use);

void plr_inputs(player_s* pev)
{
	int mx, my;

	if (cment->console_active)
	{
		pev->buttons = 0;
		return;
	}

	cment->cursor_get_pos(&mx, &my, FALSE);
	mx -= cment->centr_x;
	my -= cment->centr_y;
	if (mx || my)
	{
		pev->base.angles[YAW] -= 0.01f * glob.sens->value * mx;
		pev->base.angles[YAW] = anglemod(pev->base.angles[YAW]);
		pev->base.angles[PITCH] += 0.01f * glob.sens->value * my;
		pev->base.angles[PITCH] = clamp(-89, pev->base.angles[PITCH], 89);
		cment->cursor_reset_pos();
	}

	pev->buttons = plr_key_bits();
}

void plr_input_cvar()
{
	cment->con_create_cmd("+forward", forward_down);
	cment->con_create_cmd("-forward", forward_up);
	cment->con_create_cmd("+back", back_down);
	cment->con_create_cmd("-back", back_up);
	cment->con_create_cmd("+left", left_down);
	cment->con_create_cmd("-left", left_up);
	cment->con_create_cmd("+right", right_down);
	cment->con_create_cmd("-right", right_up);
	cment->con_create_cmd("+use", use_down);
	cment->con_create_cmd("-use", use_up);

	glob.sens = cment->con_create_cvar("m_sens", 3, TRUE);
}