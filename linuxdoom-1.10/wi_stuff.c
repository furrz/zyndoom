/******************************************************************************

   Copyright (C) 1993-1996 by id Software, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   DESCRIPTION:
        Intermission screens.

******************************************************************************/

#include <stdio.h>

#include "z_zone.h"

#include "m_random.h"
#include "m_swap.h"

#include "i_system.h"

#include "w_wad.h"

#include "g_game.h"

#include "r_local.h"
#include "s_sound.h"

#include "doomstat.h"

/* Data. */
#include "sounds.h"

/* Needs access to LFB. */
#include "v_video.h"

#include "wi_stuff.h"

/* Data needed to add patches to full screen intermission pics. */
/* Patches are statistics messages, and animations. */
/* Loads of by-pixel layout and placement, offsets etc. */


/* Different vetween registered DOOM (1994) and */
/*  Ultimate DOOM - Final edition (retail, 1995?). */
/* This is supposedly ignored for commercial */
/*  release (aka DOOM II), which had 34 maps */
/*  in one episode. So there. */
#define NUMEPISODES     4
#define NUMMAPS         9


/* in tics */
/* U #define PAUSELEN           (TICRATE*2) */
/* U #define SCORESTEP          100 */
/* U #define ANIMPERIOD         32 */
/* pixel distance from "(YOU)" to "PLAYER N" */
/* U #define STARDIST           10 */
/* U #define WK 1 */


/* GLOBAL LOCATIONS */
#define WI_TITLEY               Y_CENTRE(2)
#define WI_SPACINGY             (33*HUD_SCALE)

/* SINGLE-PLAYER STUFF */
#define SP_STATSX               X_CENTRE(50)
#define SP_STATSY               Y_CENTRE(50)

#define SP_TIMEX                X_CENTRE(16)
#define SP_TIMEY                Y_CENTRE(ORIGINAL_SCREEN_HEIGHT-32)


/* NET GAME STUFF */
#define NG_STATSY               Y_CENTRE(50)
#define NG_STATSX               X_CENTRE(32 + SHORT(star->width)/2 + 32*!dofrags)

#define NG_SPACINGX             (64*HUD_SCALE)


/* DEATHMATCH STUFF */
#define DM_MATRIXX              X_CENTRE(42)
#define DM_MATRIXY              Y_CENTRE(68)

#define DM_SPACINGX             (40*HUD_SCALE)

#define DM_TOTALSX              X_CENTRE(269)

#define DM_KILLERSX             X_CENTRE(10)
#define DM_KILLERSY             Y_CENTRE(100)
#define DM_VICTIMSX             X_CENTRE(5)
#define DM_VICTIMSY             Y_CENTRE(50)




typedef enum
{
	ANIM_ALWAYS,
	ANIM_RANDOM,
	ANIM_LEVEL

} animenum_t;

typedef struct
{
	int         x;
	int         y;

} point_t;


/* Animation. */
/* There is another anim_t used in p_spec. */
typedef struct
{
	animenum_t  type;

	/* period in tics between animations */
	int         period;

	/* number of animation frames */
	int         nanims;

	/* location of animation */
	point_t     loc;

	/* ALWAYS: n/a, */
	/* RANDOM: period deviation (<256), */
	/* LEVEL: level */
	int         data1;

	/* ALWAYS: n/a, */
	/* RANDOM: random base period, */
	/* LEVEL: n/a */
	int         data2;

	/* actual graphics for frames of animations */
	patch_t*    p[3];

	/* following must be initialized to zero before use! */

	/* next value of bcnt (used in conjunction with period) */
	int         nexttic;

	/* last drawn animation frame */
	int         lastdrawn;

	/* next frame number to animate */
	int         ctr;

	/* used by RANDOM and LEVEL when animating */
	int         state;

} anim_t;


static point_t lnodes[NUMEPISODES][NUMMAPS] =
{
	/* Episode 0 World Map */
	{
		{ 185, 164 },   /* location of level 0 (CJ) */
		{ 148, 143 },   /* location of level 1 (CJ) */
		{ 69 , 122 },   /* location of level 2 (CJ) */
		{ 209, 102 },   /* location of level 3 (CJ) */
		{ 116, 89  },   /* location of level 4 (CJ) */
		{ 166, 55  },   /* location of level 5 (CJ) */
		{ 71 , 56  },   /* location of level 6 (CJ) */
		{ 135, 29  },   /* location of level 7 (CJ) */
		{ 71 , 24  }    /* location of level 8 (CJ) */
	},

	/* Episode 1 World Map should go here */
	{
		{ 254, 25  },   /* location of level 0 (CJ) */
		{ 97 , 50  },   /* location of level 1 (CJ) */
		{ 188, 64  },   /* location of level 2 (CJ) */
		{ 128, 78  },   /* location of level 3 (CJ) */
		{ 214, 92  },   /* location of level 4 (CJ) */
		{ 133, 130 },   /* location of level 5 (CJ) */
		{ 208, 136 },   /* location of level 6 (CJ) */
		{ 148, 140 },   /* location of level 7 (CJ) */
		{ 235, 158 }    /* location of level 8 (CJ) */
	},

	/* Episode 2 World Map should go here */
	{
		{ 156, 168 },   /* location of level 0 (CJ) */
		{ 48 , 154 },   /* location of level 1 (CJ) */
		{ 174, 95  },   /* location of level 2 (CJ) */
		{ 265, 75  },   /* location of level 3 (CJ) */
		{ 130, 48  },   /* location of level 4 (CJ) */
		{ 279, 23  },   /* location of level 5 (CJ) */
		{ 198, 48  },   /* location of level 6 (CJ) */
		{ 140, 25  },   /* location of level 7 (CJ) */
		{ 281, 136 }    /* location of level 8 (CJ) */
	}

};


/* Animation locations for episode 0 (1). */
/* Using patches saves a lot of space, */
/*  as they replace 320x200 full screen frames. */
static anim_t epsd0animinfo[] =
{
	{ ANIM_ALWAYS, TICRATE/3, 3, { 224, 104 }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_ALWAYS, TICRATE/3, 3, { 184, 160 }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_ALWAYS, TICRATE/3, 3, { 112, 136 }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_ALWAYS, TICRATE/3, 3, { 72 , 112 }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_ALWAYS, TICRATE/3, 3, { 88 , 96  }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_ALWAYS, TICRATE/3, 3, { 64 , 48  }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_ALWAYS, TICRATE/3, 3, { 192, 40  }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_ALWAYS, TICRATE/3, 3, { 136, 16  }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_ALWAYS, TICRATE/3, 3, { 80 , 16  }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_ALWAYS, TICRATE/3, 3, { 64 , 24  }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 }
};

static anim_t epsd1animinfo[] =
{
	{ ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 1, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 2, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 3, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 4, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 5, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 6, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 7, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_LEVEL, TICRATE/3, 3, { 192, 144 }, 8, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 8, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 }
};

static anim_t epsd2animinfo[] =
{
	{ ANIM_ALWAYS, TICRATE/3, 3, { 104, 168 }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_ALWAYS, TICRATE/3, 3, { 40 , 136 }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_ALWAYS, TICRATE/3, 3, { 160, 96  }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_ALWAYS, TICRATE/3, 3, { 104, 80  }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_ALWAYS, TICRATE/3, 3, { 120, 32  }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 },
	{ ANIM_ALWAYS, TICRATE/4, 3, { 40 , 0   }, 0, 0, {NULL, NULL, NULL}, 0, 0, 0, 0 }
};

static int NUMANIMS[NUMEPISODES] =
{
	sizeof(epsd0animinfo)/sizeof(anim_t),
	sizeof(epsd1animinfo)/sizeof(anim_t),
	sizeof(epsd2animinfo)/sizeof(anim_t)
};

static anim_t *anims[NUMEPISODES] =
{
	epsd0animinfo,
	epsd1animinfo,
	epsd2animinfo
};


/* GENERAL DATA */

/* States for single-player */
#define SP_KILLS                0
#define SP_ITEMS                2
#define SP_SECRET               4
#define SP_FRAGS                6
#define SP_TIME                 8
#define SP_PAR                  ST_TIME

#define SP_PAUSE                1

/* in seconds */
#define SHOWNEXTLOCDELAY        4
/* #define SHOWLASTLOCDELAY     SHOWNEXTLOCDELAY */


/* used to accelerate or skip a stage */
static int              acceleratestage;

/* wbs->pnum */
static int              me;

 /* specifies current state */
static stateenum_t      state;

/* contains information passed into intermission */
static wbstartstruct_t* wbs;

static wbplayerstruct_t* plrs;  /* wbs->plyr[] */

/* used for general timing */
static int              cnt;

/* used for timing of background animation */
static int              bcnt;

/* signals to refresh everything for one frame */
static int              firstrefresh;

static int              cnt_kills[MAXPLAYERS];
static int              cnt_items[MAXPLAYERS];
static int              cnt_secret[MAXPLAYERS];
static int              cnt_time;
static int              cnt_par;
static int              cnt_pause;

/* # of commercial levels */
static int              NUMCMAPS;


/*      GRAPHICS */

/* background (map of levels). */
static patch_t*         bg;

/* You Are Here graphic */
static patch_t*         yah[2];

/* splat */
static patch_t*         splat;

/* %, : graphics */
static patch_t*         percent;
static patch_t*         colon;

/* 0-9 graphic */
static patch_t*         num[10];

/* minus sign */
static patch_t*         wiminus;

/* "Finished!" graphics */
static patch_t*         finished;

/* "Entering" graphic */
static patch_t*         entering;

/* "secret" */
static patch_t*         sp_secret;

 /* "Kills", "Scrt", "Items", "Frags" */
static patch_t*         kills;
static patch_t*         secret;
static patch_t*         items;
static patch_t*         frags;

/* Time sucks. */
static patch_t*         time;
static patch_t*         par;
static patch_t*         sucks;

/* "killers", "victims" */
static patch_t*         killers;
static patch_t*         victims;

/* "Total", your face, your dead face */
static patch_t*         total;
static patch_t*         star;
static patch_t*         bstar;

/* "red P[1..MAXPLAYERS]" */
static patch_t*         p[MAXPLAYERS];

/* "gray P[1..MAXPLAYERS]" */
static patch_t*         bp[MAXPLAYERS];

 /* Name graphics of each level (centered) */
static patch_t**        lnames;

static int kill_percent[MAXPLAYERS], item_percent[MAXPLAYERS], secret_percent[MAXPLAYERS];

/* CODE */

/* slam background */
/* UNUSED static unsigned char *background=0; */


void WI_slamBackground(void)
{
	memcpy(screens[SCREEN_FRAMEBUFFER], screens[SCREEN_BACK], SCREENWIDTH * SCREENHEIGHT * sizeof(colourindex_t));
}

/* The ticker is used to detect keys */
/*  because of timing issues in netgames. */
d_bool WI_Responder(event_t* ev)
{
	(void)ev;

	return d_false;
}


/* Draws "<Levelname> Finished!" */
void WI_drawLF(void)
{
	int y = WI_TITLEY;

	/* draw <LevelName> */
	V_DrawPatch((SCREENWIDTH - SHORT(lnames[wbs->last]->width)*HUD_SCALE)/2,
				y, SCREEN_FRAMEBUFFER, lnames[wbs->last]);

	/* draw "Finished!" */
	y += (5*SHORT(lnames[wbs->last]->height)*HUD_SCALE)/4;

	V_DrawPatch((SCREENWIDTH - SHORT(finished->width)*HUD_SCALE)/2,
				y, SCREEN_FRAMEBUFFER, finished);
}



/* Draws "Entering <LevelName>" */
void WI_drawEL(void)
{
	int y = WI_TITLEY;

	/* draw "Entering" */
	V_DrawPatch((SCREENWIDTH - SHORT(entering->width)*HUD_SCALE)/2,
				y, SCREEN_FRAMEBUFFER, entering);

	/* draw level */
	y += (5*SHORT(lnames[wbs->next]->height)*HUD_SCALE)/4;

	V_DrawPatch((SCREENWIDTH - SHORT(lnames[wbs->next]->width)*HUD_SCALE)/2,
				y, SCREEN_FRAMEBUFFER, lnames[wbs->next]);

}

void
WI_drawOnLnode
( int           n,
  patch_t*      c[] )
{

	int         i;
	int         left;
	int         top;
	int         right;
	int         bottom;
	d_bool      fits = d_false;

	i = 0;
	do
	{
		left = X_CENTRE(lnodes[wbs->epsd][n].x) - SHORT(c[i]->leftoffset)*HUD_SCALE;
		top = Y_CENTRE(lnodes[wbs->epsd][n].y) - SHORT(c[i]->topoffset)*HUD_SCALE;
		right = left + SHORT(c[i]->width)*HUD_SCALE;
		bottom = top + SHORT(c[i]->height)*HUD_SCALE;

		if (left >= 0
			&& right < SCREENWIDTH
			&& top >= 0
			&& bottom < SCREENHEIGHT)
		{
			fits = d_true;
		}
		else
		{
			i++;
		}
	} while (!fits && i!=2);

	if (fits && i<2)
	{
		V_DrawPatch(X_CENTRE(lnodes[wbs->epsd][n].x), Y_CENTRE(lnodes[wbs->epsd][n].y),
					SCREEN_FRAMEBUFFER, c[i]);
	}
	else
	{
		/* DEBUG */
		I_Info("Could not place patch on level %d", n+1);
	}
}



void WI_initAnimatedBack(void)
{
	int         i;
	anim_t*     a;

	if (gamemode == commercial)
		return;

	if (wbs->epsd > 2)
		return;

	for (i=0;i<NUMANIMS[wbs->epsd];i++)
	{
		a = &anims[wbs->epsd][i];

		/* init variables */
		a->ctr = -1;

		/* specify the next time to draw it */
		if (a->type == ANIM_ALWAYS)
			a->nexttic = bcnt + 1 + (M_Random()%a->period);
		else if (a->type == ANIM_RANDOM)
			a->nexttic = bcnt + 1 + a->data2+(M_Random()%a->data1);
		else if (a->type == ANIM_LEVEL)
			a->nexttic = bcnt + 1;
	}

}

void WI_updateAnimatedBack(void)
{
	int         i;
	anim_t*     a;

	if (gamemode == commercial)
		return;

	if (wbs->epsd > 2)
		return;

	for (i=0;i<NUMANIMS[wbs->epsd];i++)
	{
		a = &anims[wbs->epsd][i];

		if (bcnt == a->nexttic)
		{
			switch (a->type)
			{
			  case ANIM_ALWAYS:
				if (++a->ctr >= a->nanims) a->ctr = 0;
				a->nexttic = bcnt + a->period;
				break;

			  case ANIM_RANDOM:
				a->ctr++;
				if (a->ctr == a->nanims)
				{
					a->ctr = -1;
					a->nexttic = bcnt+a->data2+(M_Random()%a->data1);
				}
				else a->nexttic = bcnt + a->period;
				break;

			  case ANIM_LEVEL:
				/* gawd-awful hack for level anims */
				if (!(state == StatCount && i == 7)
					&& wbs->next == a->data1)
				{
					a->ctr++;
					if (a->ctr == a->nanims) a->ctr--;
					a->nexttic = bcnt + a->period;
				}
				break;
			}
		}

	}

}

void WI_drawAnimatedBack(void)
{
	int                 i;
	anim_t*             a;

	if (gamemode == commercial)
		return;

	if (wbs->epsd > 2)
		return;

	for (i=0 ; i<NUMANIMS[wbs->epsd] ; i++)
	{
		a = &anims[wbs->epsd][i];

		if (a->ctr >= 0)
			V_DrawPatch(X_CENTRE(a->loc.x), Y_CENTRE(a->loc.y), SCREEN_FRAMEBUFFER, a->p[a->ctr]);
	}

}

/* Draws a number. */
/* If digits > 0, then use that many digits minimum, */
/*  otherwise only use as many as necessary. */
/* Returns new x position. */

int
WI_drawNum
( int           x,
  int           y,
  int           n,
  int           digits )
{

	int         fontwidth = SHORT(num[0]->width)*HUD_SCALE;
	int         neg;
	int         temp;

	if (digits < 0)
	{
		if (!n)
		{
			/* make variable-length zeros 1 digit long */
			digits = 1;
		}
		else
		{
			/* figure out # of digits in # */
			digits = 0;
			temp = n;

			while (temp)
			{
				temp /= 10;
				digits++;
			}
		}
	}

	neg = n < 0;
	if (neg)
		n = -n;

	/* if non-number, do not draw it */
	if (n == 1994)
		return 0;

	/* draw the new number */
	while (digits--)
	{
		x -= fontwidth;
		V_DrawPatch(x, y, SCREEN_FRAMEBUFFER, num[ n % 10 ]);
		n /= 10;
	}

	/* draw a minus sign if necessary */
	if (neg)
		V_DrawPatch(x-=8*HUD_SCALE, y, SCREEN_FRAMEBUFFER, wiminus);

	return x;

}

void
WI_drawPercent
( int           x,
  int           y,
  int           p )
{
	if (p < 0)
		return;

	V_DrawPatch(x, y, SCREEN_FRAMEBUFFER, percent);
	WI_drawNum(x, y, p, -1);
}



/* Display level completion time and par, */
/*  or "sucks" message if overflow. */
void
WI_drawTime
( int           x,
  int           y,
  int           t )
{

	int         div;
	int         n;

	if (t<0)
		return;

	if (t <= 61*59)
	{
		div = 1;

		do
		{
			n = (t / div) % 60;
			x = WI_drawNum(x, y, n, 2) - SHORT(colon->width)*HUD_SCALE;
			div *= 60;

			/* draw */
			if (div==60 || t / div)
				V_DrawPatch(x, y, SCREEN_FRAMEBUFFER, colon);

		} while (t / div);
	}
	else
	{
		/* "sucks" */
		V_DrawPatch(x - SHORT(sucks->width)*HUD_SCALE, y, SCREEN_FRAMEBUFFER, sucks);
	}
}


void WI_End(void)
{
	void WI_unloadData(void);
	WI_unloadData();
}

void WI_initNoState(void)
{
	state = NoState;
	acceleratestage = 0;
	cnt = 10;
}

void WI_updateNoState(void) {

	WI_updateAnimatedBack();

	if (!--cnt)
	{
		WI_End();
		G_WorldDone();
	}

}

static d_bool           snl_pointeron = d_false;


void WI_initShowNextLoc(void)
{
	state = ShowNextLoc;
	acceleratestage = 0;
	cnt = SHOWNEXTLOCDELAY * TICRATE;

	WI_initAnimatedBack();
}

void WI_updateShowNextLoc(void)
{
	WI_updateAnimatedBack();

	if (!--cnt || acceleratestage)
		WI_initNoState();
	else
		snl_pointeron = (cnt & 31) < 20;
}

void WI_drawShowNextLoc(void)
{

	int         i;
	int         last;

	WI_slamBackground();

	/* draw animated background */
	WI_drawAnimatedBack();

	if ( gamemode != commercial)
	{
		if (wbs->epsd > 2)
		{
			WI_drawEL();
			return;
		}

		last = (wbs->last == 8) ? wbs->next - 1 : wbs->last;

		/* draw a splat on taken cities. */
		for (i=0 ; i<=last ; i++)
			WI_drawOnLnode(i, &splat);

		/* splat the secret level? */
		if (wbs->didsecret)
			WI_drawOnLnode(8, &splat);

		/* draw flashing ptr */
		if (snl_pointeron)
			WI_drawOnLnode(wbs->next, yah);
	}

	/* draws which level you are entering.. */
	if ( (gamemode != commercial)
		 || wbs->next != 30)
		WI_drawEL();

}

void WI_drawNoState(void)
{
	snl_pointeron = d_true;
	WI_drawShowNextLoc();
}

int WI_fragSum(int playernum)
{
	int         i;
	int         frags = 0;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (playeringame[i]
			&& i!=playernum)
		{
			frags += plrs[playernum].frags[i];
		}
	}


	/* JDC hack - negative frags. */
	frags -= plrs[playernum].frags[playernum];
	/* UNUSED if (frags < 0) */
	/*  frags = 0; */

	return frags;
}



static int              dm_state;
static int              dm_frags[MAXPLAYERS][MAXPLAYERS];
static int              dm_totals[MAXPLAYERS];



void WI_initDeathmatchStats(void)
{

	int         i;
	int         j;

	state = StatCount;
	acceleratestage = 0;
	dm_state = 1;

	cnt_pause = TICRATE;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (playeringame[i])
		{
			for (j=0 ; j<MAXPLAYERS ; j++)
				if (playeringame[j])
					dm_frags[i][j] = 0;

			dm_totals[i] = 0;
		}
	}

	WI_initAnimatedBack();
}



void WI_updateDeathmatchStats(void)
{

	int         i;
	int         j;

	d_bool      stillticking;

	WI_updateAnimatedBack();

	if (acceleratestage && dm_state != 4)
	{
		acceleratestage = 0;

		for (i=0 ; i<MAXPLAYERS ; i++)
		{
			if (playeringame[i])
			{
				for (j=0 ; j<MAXPLAYERS ; j++)
					if (playeringame[j])
						dm_frags[i][j] = plrs[i].frags[j];

				dm_totals[i] = WI_fragSum(i);
			}
		}


		S_StartSound(NULL, sfx_barexp);
		dm_state = 4;
	}


	if (dm_state == 2)
	{
		if (!(bcnt&3))
			S_StartSound(NULL, sfx_pistol);

		stillticking = d_false;

		for (i=0 ; i<MAXPLAYERS ; i++)
		{
			if (playeringame[i])
			{
				for (j=0 ; j<MAXPLAYERS ; j++)
				{
					if (playeringame[j]
						&& dm_frags[i][j] != plrs[i].frags[j])
					{
						if (plrs[i].frags[j] < 0)
							dm_frags[i][j]--;
						else
							dm_frags[i][j]++;

						if (dm_frags[i][j] > 99)
							dm_frags[i][j] = 99;

						if (dm_frags[i][j] < -99)
							dm_frags[i][j] = -99;

						stillticking = d_true;
					}
				}
				dm_totals[i] = WI_fragSum(i);

				if (dm_totals[i] > 99)
					dm_totals[i] = 99;

				if (dm_totals[i] < -99)
					dm_totals[i] = -99;
			}

		}
		if (!stillticking)
		{
			S_StartSound(NULL, sfx_barexp);
			dm_state++;
		}

	}
	else if (dm_state == 4)
	{
		if (acceleratestage)
		{
			S_StartSound(NULL, sfx_slop);

			if ( gamemode == commercial)
				WI_initNoState();
			else
				WI_initShowNextLoc();
		}
	}
	else if (dm_state & 1)
	{
		if (!--cnt_pause)
		{
			dm_state++;
			cnt_pause = TICRATE;
		}
	}
}



void WI_drawDeathmatchStats(void)
{

	int         i;
	int         j;
	int         x;
	int         y;
	int         w;

	WI_slamBackground();

	/* draw animated background */
	WI_drawAnimatedBack();
	WI_drawLF();

	/* draw stat titles (top line) */
	V_DrawPatch(DM_TOTALSX-SHORT(total->width)*HUD_SCALE/2,
	                  DM_MATRIXY-WI_SPACINGY+10*HUD_SCALE,
	                  SCREEN_FRAMEBUFFER,
	                  total);

	V_DrawPatch(DM_KILLERSX, DM_KILLERSY, SCREEN_FRAMEBUFFER, killers);
	V_DrawPatch(DM_VICTIMSX, DM_VICTIMSY, SCREEN_FRAMEBUFFER, victims);

	/* draw P? */
	x = DM_MATRIXX + DM_SPACINGX;
	y = DM_MATRIXY;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (playeringame[i])
		{
			V_DrawPatch(x-SHORT(p[i]->width)*HUD_SCALE/2,
			                  DM_MATRIXY - WI_SPACINGY,
			                  SCREEN_FRAMEBUFFER,
			                  p[i]);

			V_DrawPatch(DM_MATRIXX-SHORT(p[i]->width)*HUD_SCALE/2,
			                  y,
			                  SCREEN_FRAMEBUFFER,
			                  p[i]);

			if (i == me)
			{
				V_DrawPatch(x-SHORT(p[i]->width)*HUD_SCALE/2,
				                  DM_MATRIXY - WI_SPACINGY,
				                  SCREEN_FRAMEBUFFER,
				                  bstar);

				V_DrawPatch(DM_MATRIXX-SHORT(p[i]->width)*HUD_SCALE/2,
				                  y,
				                  SCREEN_FRAMEBUFFER,
				                  star);
			}
		}
		else
		{
			/* V_DrawPatch(x-SHORT(bp[i]->width)*SCREEN_MUL/2, */
			/*   DM_MATRIXY - WI_SPACINGY, FB, bp[i]); */
			/* V_DrawPatch(DM_MATRIXX-SHORT(bp[i]->width)*SCREEN_MUL/2, */
			/*   y, FB, bp[i]); */
		}
		x += DM_SPACINGX;
		y += WI_SPACINGY;
	}

	/* draw stats */
	y = DM_MATRIXY+10*HUD_SCALE;
	w = SHORT(num[0]->width)*HUD_SCALE;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		x = DM_MATRIXX + DM_SPACINGX;

		if (playeringame[i])
		{
			for (j=0 ; j<MAXPLAYERS ; j++)
			{
				if (playeringame[j])
					WI_drawNum(x+w, y, dm_frags[i][j], 2);

				x += DM_SPACINGX;
			}
			WI_drawNum(DM_TOTALSX+w, y, dm_totals[i], 2);
		}
		y += WI_SPACINGY;
	}
}

static int      cnt_frags[MAXPLAYERS];
static int      dofrags;
static int      ng_state;

void WI_initNetgameStats(void)
{

	int i;

	state = StatCount;
	acceleratestage = 0;
	ng_state = 1;

	cnt_pause = TICRATE;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (!playeringame[i])
			continue;

		cnt_kills[i] = cnt_items[i] = cnt_secret[i] = cnt_frags[i] = 0;

		dofrags += WI_fragSum(i);
	}

	dofrags = !!dofrags;

	WI_initAnimatedBack();
}



void WI_updateNetgameStats(void)
{

	int         i;
	int         fsum;

	d_bool      stillticking;

	WI_updateAnimatedBack();

	if (acceleratestage && ng_state != 10)
	{
		acceleratestage = 0;

		for (i=0 ; i<MAXPLAYERS ; i++)
		{
			if (!playeringame[i])
				continue;

			cnt_kills[i] = kill_percent[i];
			cnt_items[i] = item_percent[i];
			cnt_secret[i] = secret_percent[i];

			if (dofrags)
				cnt_frags[i] = WI_fragSum(i);
		}
		S_StartSound(NULL, sfx_barexp);
		ng_state = 10;
	}

	if (ng_state == 2)
	{
		if (!(bcnt&3))
			S_StartSound(NULL, sfx_pistol);

		stillticking = d_false;

		for (i=0 ; i<MAXPLAYERS ; i++)
		{
			if (!playeringame[i])
				continue;

			cnt_kills[i] += 2;

			if (cnt_kills[i] >= kill_percent[i])
				cnt_kills[i] = kill_percent[i];
			else
				stillticking = d_true;
		}

		if (!stillticking)
		{
			S_StartSound(NULL, sfx_barexp);
			ng_state++;
		}
	}
	else if (ng_state == 4)
	{
		if (!(bcnt&3))
			S_StartSound(NULL, sfx_pistol);

		stillticking = d_false;

		for (i=0 ; i<MAXPLAYERS ; i++)
		{
			if (!playeringame[i])
				continue;

			cnt_items[i] += 2;
			if (cnt_items[i] >= item_percent[i])
				cnt_items[i] = item_percent[i];
			else
				stillticking = d_true;
		}
		if (!stillticking)
		{
			S_StartSound(NULL, sfx_barexp);
			ng_state++;
		}
	}
	else if (ng_state == 6)
	{
		if (!(bcnt&3))
			S_StartSound(NULL, sfx_pistol);

		stillticking = d_false;

		for (i=0 ; i<MAXPLAYERS ; i++)
		{
			if (!playeringame[i])
				continue;

			cnt_secret[i] += 2;

			if (cnt_secret[i] >= secret_percent[i])
				cnt_secret[i] = secret_percent[i];
			else
				stillticking = d_true;
		}

		if (!stillticking)
		{
			S_StartSound(NULL, sfx_barexp);
			ng_state += 1 + 2*!dofrags;
		}
	}
	else if (ng_state == 8)
	{
		if (!(bcnt&3))
			S_StartSound(NULL, sfx_pistol);

		stillticking = d_false;

		for (i=0 ; i<MAXPLAYERS ; i++)
		{
			if (!playeringame[i])
				continue;

			cnt_frags[i] += 1;

			if (cnt_frags[i] >= (fsum = WI_fragSum(i)))
				cnt_frags[i] = fsum;
			else
				stillticking = d_true;
		}

		if (!stillticking)
		{
			S_StartSound(NULL, sfx_pldeth);
			ng_state++;
		}
	}
	else if (ng_state == 10)
	{
		if (acceleratestage)
		{
			S_StartSound(NULL, sfx_sgcock);
			if ( gamemode == commercial )
				WI_initNoState();
			else
				WI_initShowNextLoc();
		}
	}
	else if (ng_state & 1)
	{
		if (!--cnt_pause)
		{
			ng_state++;
			cnt_pause = TICRATE;
		}
	}
}



void WI_drawNetgameStats(void)
{
	int         i;
	int         x;
	int         y;
	int         pwidth = SHORT(percent->width)*HUD_SCALE;

	WI_slamBackground();

	/* draw animated background */
	WI_drawAnimatedBack();

	WI_drawLF();

	/* draw stat titles (top line) */
	V_DrawPatch(NG_STATSX+NG_SPACINGX-SHORT(kills->width)*HUD_SCALE,
				NG_STATSY, SCREEN_FRAMEBUFFER, kills);

	V_DrawPatch(NG_STATSX+2*NG_SPACINGX-SHORT(items->width)*HUD_SCALE,
				NG_STATSY, SCREEN_FRAMEBUFFER, items);

	V_DrawPatch(NG_STATSX+3*NG_SPACINGX-SHORT(secret->width)*HUD_SCALE,
				NG_STATSY, SCREEN_FRAMEBUFFER, secret);

	if (dofrags)
		V_DrawPatch(NG_STATSX+4*NG_SPACINGX-SHORT(frags->width)*HUD_SCALE,
					NG_STATSY, SCREEN_FRAMEBUFFER, frags);

	/* draw stats */
	y = NG_STATSY + SHORT(kills->height)*HUD_SCALE;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (!playeringame[i])
			continue;

		x = NG_STATSX;
		V_DrawPatch(x-SHORT(p[i]->width)*HUD_SCALE, y, SCREEN_FRAMEBUFFER, p[i]);

		if (i == me)
			V_DrawPatch(x-SHORT(p[i]->width)*HUD_SCALE, y, SCREEN_FRAMEBUFFER, star);

		x += NG_SPACINGX;
		WI_drawPercent(x-pwidth, y+10*HUD_SCALE, cnt_kills[i]);   x += NG_SPACINGX;
		WI_drawPercent(x-pwidth, y+10*HUD_SCALE, cnt_items[i]);   x += NG_SPACINGX;
		WI_drawPercent(x-pwidth, y+10*HUD_SCALE, cnt_secret[i]);  x += NG_SPACINGX;

		if (dofrags)
			WI_drawNum(x, y+10*HUD_SCALE, cnt_frags[i], -1);

		y += WI_SPACINGY;
	}

}

static int      sp_state;

void WI_initStats(void)
{
	state = StatCount;
	acceleratestage = 0;
	sp_state = 1;
	cnt_kills[0] = cnt_items[0] = cnt_secret[0] = -1;
	cnt_time = cnt_par = -1;
	cnt_pause = TICRATE;

	WI_initAnimatedBack();
}

void WI_updateStats(void)
{

	WI_updateAnimatedBack();

	if (acceleratestage && sp_state != 10)
	{
		acceleratestage = 0;
		cnt_kills[0] = kill_percent[me];
		cnt_items[0] = item_percent[me];
		cnt_secret[0] = secret_percent[me];
		cnt_time = plrs[me].stime / TICRATE;
		cnt_par = wbs->partime / TICRATE;
		S_StartSound(NULL, sfx_barexp);
		sp_state = 10;
	}

	if (sp_state == 2)
	{
		cnt_kills[0] += 2;

		if (!(bcnt&3))
			S_StartSound(NULL, sfx_pistol);

		if (cnt_kills[0] >= kill_percent[me])
		{
			cnt_kills[0] = kill_percent[me];
			S_StartSound(NULL, sfx_barexp);
			sp_state++;
		}
	}
	else if (sp_state == 4)
	{
		cnt_items[0] += 2;

		if (!(bcnt&3))
			S_StartSound(NULL, sfx_pistol);

		if (cnt_items[0] >= item_percent[me])
		{
			cnt_items[0] = item_percent[me];
			S_StartSound(NULL, sfx_barexp);
			sp_state++;
		}
	}
	else if (sp_state == 6)
	{
		cnt_secret[0] += 2;

		if (!(bcnt&3))
			S_StartSound(NULL, sfx_pistol);

		if (cnt_secret[0] >= secret_percent[me])
		{
			cnt_secret[0] = secret_percent[me];
			S_StartSound(NULL, sfx_barexp);
			sp_state++;
		}
	}

	else if (sp_state == 8)
	{
		if (!(bcnt&3))
			S_StartSound(NULL, sfx_pistol);

		cnt_time += 3;

		if (cnt_time >= plrs[me].stime / TICRATE)
			cnt_time = plrs[me].stime / TICRATE;

		cnt_par += 3;

		if (cnt_par >= wbs->partime / TICRATE)
		{
			cnt_par = wbs->partime / TICRATE;

			if (cnt_time >= plrs[me].stime / TICRATE)
			{
				S_StartSound(NULL, sfx_barexp);
				sp_state++;
			}
		}
	}
	else if (sp_state == 10)
	{
		if (acceleratestage)
		{
			S_StartSound(NULL, sfx_sgcock);

			if (gamemode == commercial)
				WI_initNoState();
			else
				WI_initShowNextLoc();
		}
	}
	else if (sp_state & 1)
	{
		if (!--cnt_pause)
		{
			sp_state++;
			cnt_pause = TICRATE;
		}
	}

}

void WI_drawStats(void)
{
	/* line height */
	int lh;

	lh = (3*SHORT(num[0]->height)*HUD_SCALE)/2;

	WI_slamBackground();

	/* draw animated background */
	WI_drawAnimatedBack();

	WI_drawLF();

	V_DrawPatch(SP_STATSX, SP_STATSY, SCREEN_FRAMEBUFFER, kills);
	WI_drawPercent(SCREENWIDTH - SP_STATSX, SP_STATSY, cnt_kills[0]);

	V_DrawPatch(SP_STATSX, SP_STATSY+lh, SCREEN_FRAMEBUFFER, items);
	WI_drawPercent(SCREENWIDTH - SP_STATSX, SP_STATSY+lh, cnt_items[0]);

	V_DrawPatch(SP_STATSX, SP_STATSY+2*lh, SCREEN_FRAMEBUFFER, sp_secret);
	WI_drawPercent(SCREENWIDTH - SP_STATSX, SP_STATSY+2*lh, cnt_secret[0]);

	V_DrawPatch(SP_TIMEX, SP_TIMEY, SCREEN_FRAMEBUFFER, time);
	WI_drawTime(SCREENWIDTH - SP_TIMEX - ORIGINAL_SCREEN_WIDTH*HUD_SCALE/2, SP_TIMEY, cnt_time);

	V_DrawPatch(SP_TIMEX + ORIGINAL_SCREEN_WIDTH*HUD_SCALE/2, SP_TIMEY, SCREEN_FRAMEBUFFER, par);
	WI_drawTime(SCREENWIDTH - SP_TIMEX, SP_TIMEY, cnt_par);
}

void WI_checkForAccelerate(void)
{
	int   i;
	player_t  *player;

	/* check for button presses to skip delays */
	for (i=0, player = players ; i<MAXPLAYERS ; i++, player++)
	{
		if (playeringame[i])
		{
			if (player->cmd.buttons & BT_ATTACK)
			{
				if (!player->attackdown)
					acceleratestage = 1;
				player->attackdown = d_true;
			}
			else
				player->attackdown = d_false;
			if (player->cmd.buttons & BT_USE)
			{
				if (!player->usedown)
					acceleratestage = 1;
				player->usedown = d_true;
			}
			else
				player->usedown = d_false;
		}
	}
}



/* Updates stuff each tick */
void WI_Ticker(void)
{
	/* counter for general background animation */
	bcnt++;

	if (bcnt == 1)
	{
		/* intermission music */
		if ( gamemode == commercial )
		  S_ChangeMusic(mus_dm2int, d_true);
		else
		  S_ChangeMusic(mus_inter, d_true);
	}

	WI_checkForAccelerate();

	switch (state)
	{
	  case StatCount:
		if (deathmatch != DM_OFF) WI_updateDeathmatchStats();
		else if (netgame) WI_updateNetgameStats();
		else WI_updateStats();
		break;

	  case ShowNextLoc:
		WI_updateShowNextLoc();
		break;

	  case NoState:
		WI_updateNoState();
		break;
	}

}

void WI_loadData(void)
{
	int         i;
	int         j;
	char        name[9];
	anim_t*     a;

	if (gamemode == commercial)
		strcpy(name, "INTERPIC");
	else
		sprintf(name, "WIMAP%d", wbs->epsd);

	if ( gamemode == retail )
	{
	  if (wbs->epsd == 3)
		strcpy(name,"INTERPIC");
	}

	/* background */
	bg = (patch_t*)W_CacheLumpName(name, PU_CACHE);
	V_DrawPatch(SCREENWIDTH/2-160*HUD_SCALE, SCREENHEIGHT/2-100*HUD_SCALE, SCREEN_BACK, bg);


	/* UNUSED unsigned char *pic = screens[SCREEN_BACK]; */
	/* if (gamemode == commercial) */
	/* { */
	/* darken the background image */
	/* while (pic != screens[SCREEN_BACK] + SCREENHEIGHT*SCREENWIDTH) */
	/* { */
	/*   *pic = colormaps[25][*pic]; */
	/*   pic++; */
	/* } */
   /* } */

	if (gamemode == commercial)
	{
		NUMCMAPS = 32;
		lnames = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMCMAPS,
									   PU_STATIC, NULL);
		for (i=0 ; i<NUMCMAPS ; i++)
		{
			name[0] = 'C';
			name[1] = 'W';
			name[2] = 'I';
			name[3] = 'L';
			name[4] = 'V';
			name[5] = '0' + (i / 10) % 10;
			name[6] = '0' + i % 10;
			name[7] = '\0';
			lnames[i] = (patch_t*)W_CacheLumpName(name, PU_STATIC);
		}
	}
	else
	{
		lnames = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMMAPS,
									   PU_STATIC, NULL);
		for (i=0 ; i<NUMMAPS ; i++)
		{
			sprintf(name, "WILV%d%d", wbs->epsd, i);
			lnames[i] = (patch_t*)W_CacheLumpName(name, PU_STATIC);
		}

		/* you are here */
		yah[0] = (patch_t*)W_CacheLumpName("WIURH0", PU_STATIC);

		/* you are here (alt.) */
		yah[1] = (patch_t*)W_CacheLumpName("WIURH1", PU_STATIC);

		/* splat */
		splat = (patch_t*)W_CacheLumpName("WISPLAT", PU_STATIC);

		if (wbs->epsd < 3)
		{
			for (j=0;j<NUMANIMS[wbs->epsd];j++)
			{
				a = &anims[wbs->epsd][j];
				for (i=0;i<a->nanims;i++)
				{
					/* MONDO HACK! */
					if (wbs->epsd != 1 || j != 8)
					{
						/* animations */
						name[0] = 'W';
						name[1] = 'I';
						name[2] = 'A';
						name[3] = '0' + wbs->epsd;
						name[4] = '0' + (j / 10) % 10;
						name[5] = '0' + j % 10;
						name[6] = '0' + (i / 10) % 10;
						name[7] = '0' + i % 10;
						name[8] = '\0';
						a->p[i] = (patch_t*)W_CacheLumpName(name, PU_STATIC);
					}
					else
					{
						/* HACK ALERT! */
						a->p[i] = anims[1][4].p[i];
					}
				}
			}
		}
	}

	/* More hacks on minus sign. */
	wiminus = (patch_t*)W_CacheLumpName("WIMINUS", PU_STATIC);

	for (i=0;i<10;i++)
	{
		 /* numbers 0-9 */
		sprintf(name, "WINUM%d", i);
		num[i] = (patch_t*)W_CacheLumpName(name, PU_STATIC);
	}

	/* percent sign */
	percent = (patch_t*)W_CacheLumpName("WIPCNT", PU_STATIC);

	/* "finished" */
	finished = (patch_t*)W_CacheLumpName("WIF", PU_STATIC);

	/* "entering" */
	entering = (patch_t*)W_CacheLumpName("WIENTER", PU_STATIC);

	/* "kills" */
	kills = (patch_t*)W_CacheLumpName("WIOSTK", PU_STATIC);

	/* "scrt" */
	secret = (patch_t*)W_CacheLumpName("WIOSTS", PU_STATIC);

	 /* "secret" */
	sp_secret = (patch_t*)W_CacheLumpName("WISCRT2", PU_STATIC);

	/* Yuck. */
	if (language == french)
	{
		/* "items" */
		if (netgame && deathmatch == DM_OFF)
			items = (patch_t*)W_CacheLumpName("WIOBJ", PU_STATIC);
		else
			items = (patch_t*)W_CacheLumpName("WIOSTI", PU_STATIC);
	} else
		items = (patch_t*)W_CacheLumpName("WIOSTI", PU_STATIC);

	/* "frgs" */
	frags = (patch_t*)W_CacheLumpName("WIFRGS", PU_STATIC);

	/* ":" */
	colon = (patch_t*)W_CacheLumpName("WICOLON", PU_STATIC);

	/* "time" */
	time = (patch_t*)W_CacheLumpName("WITIME", PU_STATIC);

	/* "sucks" */
	sucks = (patch_t*)W_CacheLumpName("WISUCKS", PU_STATIC);

	/* "par" */
	par = (patch_t*)W_CacheLumpName("WIPAR", PU_STATIC);

	/* "killers" (vertical) */
	killers = (patch_t*)W_CacheLumpName("WIKILRS", PU_STATIC);

	/* "victims" (horiz) */
	victims = (patch_t*)W_CacheLumpName("WIVCTMS", PU_STATIC);

	/* "total" */
	total = (patch_t*)W_CacheLumpName("WIMSTT", PU_STATIC);

	/* your face */
	star = (patch_t*)W_CacheLumpName("STFST01", PU_STATIC);

	/* dead face */
	bstar = (patch_t*)W_CacheLumpName("STFDEAD0", PU_STATIC);

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		/* "1,2,3,4" */
		sprintf(name, "STPB%d", i);
		p[i] = (patch_t*)W_CacheLumpName(name, PU_STATIC);

		/* "1,2,3,4" */
		sprintf(name, "WIBP%d", i+1);
		bp[i] = (patch_t*)W_CacheLumpName(name, PU_STATIC);
	}

}

void WI_unloadData(void)
{
	int         i;
	int         j;

	Z_ChangeTag(wiminus, PU_CACHE);

	for (i=0 ; i<10 ; i++)
		Z_ChangeTag(num[i], PU_CACHE);

	if (gamemode == commercial)
	{
		for (i=0 ; i<NUMCMAPS ; i++)
			Z_ChangeTag(lnames[i], PU_CACHE);
	}
	else
	{
		Z_ChangeTag(yah[0], PU_CACHE);
		Z_ChangeTag(yah[1], PU_CACHE);

		Z_ChangeTag(splat, PU_CACHE);

		for (i=0 ; i<NUMMAPS ; i++)
			Z_ChangeTag(lnames[i], PU_CACHE);

		if (wbs->epsd < 3)
		{
			for (j=0;j<NUMANIMS[wbs->epsd];j++)
			{
				if (wbs->epsd != 1 || j != 8)
					for (i=0;i<anims[wbs->epsd][j].nanims;i++)
						Z_ChangeTag(anims[wbs->epsd][j].p[i], PU_CACHE);
			}
		}
	}

	Z_Free(lnames);

	Z_ChangeTag(percent, PU_CACHE);
	Z_ChangeTag(colon, PU_CACHE);
	Z_ChangeTag(finished, PU_CACHE);
	Z_ChangeTag(entering, PU_CACHE);
	Z_ChangeTag(kills, PU_CACHE);
	Z_ChangeTag(secret, PU_CACHE);
	Z_ChangeTag(sp_secret, PU_CACHE);
	Z_ChangeTag(items, PU_CACHE);
	Z_ChangeTag(frags, PU_CACHE);
	Z_ChangeTag(time, PU_CACHE);
	Z_ChangeTag(sucks, PU_CACHE);
	Z_ChangeTag(par, PU_CACHE);

	Z_ChangeTag(victims, PU_CACHE);
	Z_ChangeTag(killers, PU_CACHE);
	Z_ChangeTag(total, PU_CACHE);
	/* Don't change the tag for these because they're also allocated by the status bar code. */
	/*  Z_ChangeTag(star, PU_CACHE); */
	/*  Z_ChangeTag(bstar, PU_CACHE); */

	for (i=0 ; i<MAXPLAYERS ; i++)
		Z_ChangeTag(p[i], PU_CACHE);

	for (i=0 ; i<MAXPLAYERS ; i++)
		Z_ChangeTag(bp[i], PU_CACHE);
}

void WI_Drawer (void)
{
	switch (state)
	{
	  case StatCount:
		if (deathmatch != DM_OFF)
			WI_drawDeathmatchStats();
		else if (netgame)
			WI_drawNetgameStats();
		else
			WI_drawStats();
		break;

	  case ShowNextLoc:
		WI_drawShowNextLoc();
		break;

	  case NoState:
		WI_drawNoState();
		break;
	}
}


void WI_initVariables(wbstartstruct_t* wbstartstruct)
{
	int i;

	wbs = wbstartstruct;

	/* TODO: Figure out what to do with this... */
#ifdef RANGECHECKING
	if (gamemode != commercial)
	{
	  if ( gamemode == retail )
		RNGCHECK(wbs->epsd, 0, 3);
	  else
		RNGCHECK(wbs->epsd, 0, 2);
	}
	else
	{
		RNGCHECK(wbs->last, 0, 8);
		RNGCHECK(wbs->next, 0, 8);
	}
	RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
	RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
#endif

	acceleratestage = 0;
	cnt = bcnt = 0;
	firstrefresh = 1;
	me = wbs->pnum;
	plrs = wbs->plyr;

	for (i = 0; i < MAXPLAYERS; ++i)
	{
		/* The original would treat 0/0 as 0%. */
		kill_percent[i] = wbs->maxkills == 0 ? 100 : plrs[i].skills * 100 / wbs->maxkills;
		item_percent[i] = wbs->maxitems == 0 ? 100 : plrs[i].sitems * 100 / wbs->maxitems;
		secret_percent[i] = wbs->maxsecret == 0 ? 100 : plrs[i].ssecret * 100 / wbs->maxsecret;
	}

	if ( gamemode != retail )
	  if (wbs->epsd > 2)
		wbs->epsd -= 3;
}

void WI_Start(wbstartstruct_t* wbstartstruct)
{

	WI_initVariables(wbstartstruct);
	WI_loadData();

	if (deathmatch != DM_OFF)
		WI_initDeathmatchStats();
	else if (netgame)
		WI_initNetgameStats();
	else
		WI_initStats();
}
