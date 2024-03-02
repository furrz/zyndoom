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
        DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
        plus functions to determine game mode (shareware, registered),
        parse command line parameters, configure game parameters (turbo),
        and call the startup functions.

******************************************************************************/


#define BGCOLOR         7
#define FGCOLOR         8


#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef __unix__
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif


#include "doomdef.h"
#include "doomstat.h"

#include "dstrings.h"
#include "sounds.h"


#include "z_zone.h"
#include "w_wad.h"
#include "s_sound.h"
#include "v_video.h"

#include "f_finale.h"
#include "f_wipe.h"

#include "m_argv.h"
#include "m_misc.h"
#include "m_menu.h"

#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"

#include "g_game.h"

#include "hu_stuff.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "am_map.h"

#include "p_setup.h"
#include "r_local.h"


#include "d_main.h"

/* D-DoomLoop() */
/* Not a globally visible function, */
/*  just included for source reference, */
/*  called by D_DoomMain, never exits. */
/* Manages timing and IO, */
/*  calls all ?_Responder, ?_Ticker, and ?_Drawer, */
/*  calls I_GetTime, I_StartFrame, and I_StartTic */
void D_DoomLoop (void);


const char*     wadfiles[MAXWADFILES];


d_bool          devparm;        /* started game with -devparm */
d_bool          nomonsters;     /* checkparm of -nomonsters */
d_bool          respawnparm;    /* checkparm of -respawn */
d_bool          fastparm;       /* checkparm of -fast */

d_bool          drone;

d_bool          singletics = d_false; /* debug flag to cancel adaptiveness */



/* extern int soundVolume; */
/* extern  int  sfxVolume; */
/* extern  int  musicVolume; */

extern  d_bool inhelpscreens;

skill_t         startskill;
int             startepisode;
int             startmap;
d_bool          autostart;

FILE*           debugfile;

d_bool          advancedemo;




char            wadfile[1024];          /* primary wad file */
char            mapdir[1024];           /* directory of development maps */
char            basedefault[1024];      /* default file */


void D_CheckNetGame (void);
void D_ProcessEvents (void);
void G_BuildTiccmd (ticcmd_t* cmd);
void D_DoAdvanceDemo (void);


/* EVENT HANDLING */
/* Events are asynchronous inputs generally generated by the game user. */
/* Events can be discarded if no responder claims them */
event_t         events[MAXEVENTS];
int             eventhead;
int             eventtail;


/* D_PostEvent */
/* Called by the I/O functions when input is detected */
void D_PostEvent (const event_t* ev)
{
	events[eventhead] = *ev;
	eventhead = (eventhead+1)&(MAXEVENTS-1);
}


/* D_ProcessEvents */
/* Send all the events of the given timestamp down the responder chain */
void D_ProcessEvents (void)
{
	event_t*    ev;

	/* IF STORE DEMO, DO NOT ACCEPT INPUT */
	if ( ( gamemode == commercial )
		 && (W_CheckNumForName("map01")<0) )
	  return;

	for ( ; eventtail != eventhead ; eventtail = (eventtail+1)&(MAXEVENTS-1) )
	{
		static d_bool altheld;

		ev = &events[eventtail];

		switch (ev->type)
		{
			case ev_keydown:
				switch (ev->data1)
				{
					case KEY_LALT:
						altheld = d_true;
						break;

					case KEY_ENTER:
						if (altheld)
						{
							I_ToggleFullscreen();
							continue; /* Eat the event. */
						}

						break;

					default:
						break;
				}

				break;

			case ev_keyup:

				switch (ev->data1)
				{
					case KEY_LALT:
						altheld = d_false;
						break;

					default:
						break;
				}

				break;

			default:
				break;
		}

		if (M_Responder (ev))
			continue;               /* menu ate the event */
		G_Responder (ev);
	}
}




/* D_Display */
/*  draw current display, possibly wiping it from the previous */

/* wipegamestate can be set to -1 to force a wipe on the next draw */
gamestate_t     wipegamestate = GS_DEMOSCREEN;
extern  d_bool setsizeneeded; /* TODO: This is such spaghetti code bullshit */
extern  int             showMessages;
void R_ExecuteSetViewSize (void);

void D_Display (void)
{
	static  d_bool              viewactivestate = d_false;
	static  d_bool              menuactivestate = d_false;
	static  d_bool              inhelpscreensstate = d_false;
	static  d_bool              fullscreen = d_false;
	static  gamestate_t         oldgamestate = GS_FORCEWIPE;
	static  int                 borderdrawcount;
	int                         nowtime;
	int                         tics;
	int                         wipestart;
	int                         y;
	d_bool                      done;
	d_bool                      wipe;
	d_bool                      redrawsbar;

	if (nodrawers)
		return;                    /* for comparative timing / profiling */

	redrawsbar = d_false;

	/* change the view size if needed */
	if (setsizeneeded)
	{
		R_ExecuteSetViewSize ();
		oldgamestate = GS_FORCEWIPE;                      /* force background redraw */
		borderdrawcount = 3;
	}

	/* save the current screen if about to wipe */
	if (gamestate != wipegamestate)
	{
		wipe = d_true;
		wipe_StartScreen();
	}
	else
		wipe = d_false;

	if (gamestate == GS_LEVEL && gametic)
		HU_Erase();

	/* control mouse cursor grab */
	I_GrabMouse(!paused
		 && !menuactive
		 && !demoplayback
		 && players[consoleplayer].viewz != 1);

	/* do buffered drawing */
	switch (gamestate)
	{
	  case GS_LEVEL:
		if (!gametic)
			break;
		if (automapactive)
			AM_Drawer ();
		if (wipe || (viewheight != SCREENHEIGHT && fullscreen) )
			redrawsbar = d_true;
		if (inhelpscreensstate && !inhelpscreens)
			redrawsbar = d_true;              /* just put away the help screen */
		ST_Drawer (viewheight == SCREENHEIGHT, redrawsbar );
		fullscreen = viewheight == SCREENHEIGHT;
		break;

	  case GS_INTERMISSION:
		WI_Drawer ();
		break;

	  case GS_FINALE:
		F_Drawer ();
		break;

	  case GS_DEMOSCREEN:
		D_PageDrawer ();
		break;

	  case GS_FORCEWIPE:
		break;
	}

	/* draw buffered stuff to screen */
	I_UpdateNoBlit ();

	/* draw the view directly */
	if (gamestate == GS_LEVEL && !automapactive && gametic)
		R_RenderPlayerView (&players[displayplayer]);

	if (gamestate == GS_LEVEL && gametic)
		HU_Drawer ();

	/* clean up border stuff */
	if (gamestate != oldgamestate && gamestate != GS_LEVEL)
		V_SetPalette(0);

	/* see if the border needs to be initially drawn */
	if (gamestate == GS_LEVEL && oldgamestate != GS_LEVEL)
	{
		viewactivestate = d_false;        /* view was not active */
		R_FillBackScreen ();    /* draw the pattern into the back screen */
	}

	/* see if the border needs to be updated to the screen */
	if (gamestate == GS_LEVEL && !automapactive && scaledviewwidth != SCREENWIDTH)
	{
		if (menuactive || menuactivestate || !viewactivestate)
			borderdrawcount = 3;
		if (borderdrawcount)
		{
			R_DrawViewBorder ();    /* erase old menu stuff */
			borderdrawcount--;
		}

	}

	menuactivestate = menuactive;
	viewactivestate = viewactive;
	inhelpscreensstate = inhelpscreens;
	oldgamestate = wipegamestate = gamestate;

	/* draw pause pic */
	if (paused)
	{
		if (automapactive)
			y = 4*HUD_SCALE;
		else
			y = viewwindowy+4*HUD_SCALE;
		V_DrawPatch(viewwindowx+(scaledviewwidth-68*HUD_SCALE)/2,
						  y,SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName ("M_PAUSE", PU_CACHE));
	}


	/* menus go directly to the screen */
	M_Drawer ();          /* menu is drawn even on top of everything */
	NetUpdate ();         /* send out any new accumulation */


	/* normal update */
	if (!wipe)
	{
		I_FinishUpdate ();              /* page flip or blit buffer */
		return;
	}

	/* wipe update */
	wipe_EndScreen();

	wipestart = I_GetTime () - 1;

	do
	{
		do
		{
			nowtime = I_GetTime ();
			tics = nowtime - wipestart;
			I_Sleep();
		} while (!tics);
		wipestart = nowtime;
		done = wipe_ScreenWipe(wipe_Melt, tics);
		I_UpdateNoBlit ();
		M_Drawer ();                            /* menu is drawn even on top of wipes */
		I_FinishUpdate ();                      /* page flip or blit buffer */
	} while (!done);
}



/*  D_DoomLoop */
extern  d_bool          demorecording;

void D_DoomLoop (void)
{
	if (demorecording)
		G_BeginRecording ();

	if (M_CheckParm ("-debugfile"))
	{
		char    filename[20];
		sprintf (filename,"debug%i.txt",consoleplayer);
		I_Info ("debug output to: %s\n",filename);
		debugfile = fopen (filename,"w");
	}

	while (1)
	{
		/* frame syncronous IO operations */
		I_StartFrame ();

		/* process one or more tics */
		if (singletics)
		{
			I_StartTic ();
			D_ProcessEvents ();
			G_BuildTiccmd (&netcmds[consoleplayer][maketic%BACKUPTICS]);
			if (advancedemo)
				D_DoAdvanceDemo ();
			M_Ticker ();
			G_Ticker ();
			gametic++;
			maketic++;
		}
		else
		{
			TryRunTics (); /* will run at least one tic */
		}

		S_UpdateSounds (players[consoleplayer].mo);/* move positional sounds */

		/* Update display, next frame, with current state. */
		D_Display ();
	}
}



/*  DEMO LOOP */
int             demosequence;
int             pagetic;
const char                    *pagename;


/* D_PageTicker */
/* Handles timing for warped projection */
void D_PageTicker (void)
{
	if (--pagetic < 0)
		D_AdvanceDemo ();
}



/* D_PageDrawer */
void D_PageDrawer (void)
{
	V_ClearScreen(SCREEN_FRAMEBUFFER);
	V_DrawPatch (X_CENTRE(0), Y_CENTRE(0), SCREEN_FRAMEBUFFER, (patch_t*)W_CacheLumpName(pagename, PU_CACHE));
}


/* D_AdvanceDemo */
/* Called after each demo or intro demosequence finishes */
void D_AdvanceDemo (void)
{
	advancedemo = d_true;
}


/* This cycles through the demo sequences. */
/* FIXME - version dependend demo numbers? */
 void D_DoAdvanceDemo (void)
{
	players[consoleplayer].playerstate = PST_LIVE;  /* not reborn */
	advancedemo = d_false;
	usergame = d_false;               /* no save / end game here */
	paused = d_false;
	gameaction = ga_nothing;

	if ( gamemode == retail )
	  demosequence = (demosequence+1)%8;
	else
	  demosequence = (demosequence+1)%6;

	switch (demosequence)
	{
	  case 0:
		if ( gamemode == commercial )
			pagetic = TICRATE * 11;
		else
			pagetic = TICRATE * 34 / 7;
		gamestate = GS_DEMOSCREEN;
		pagename = "TITLEPIC";
		if ( gamemode == commercial )
		  S_StartMusic(mus_dm2ttl);
		else
		  S_StartMusic (mus_intro);
		break;
	  case 1:
		G_DeferedPlayDemo ("demo1");
		break;
	  case 2:
		pagetic = TICRATE * 40 / 7;
		gamestate = GS_DEMOSCREEN;
		pagename = "CREDIT";
		break;
	  case 3:
		G_DeferedPlayDemo ("demo2");
		break;
	  case 4:
		gamestate = GS_DEMOSCREEN;
		if ( gamemode == commercial)
		{
			pagetic = TICRATE * 11;
			pagename = "TITLEPIC";
			S_StartMusic(mus_dm2ttl);
		}
		else
		{
			pagetic = TICRATE * 40 / 7;

			if ( gamemode == retail )
			  pagename = "CREDIT";
			else
			  pagename = "HELP2";
		}
		break;
	  case 5:
		G_DeferedPlayDemo ("demo3");
		break;
	  case 6:
		  pagetic = TICRATE * 40 / 7;
		  gamestate = GS_DEMOSCREEN;
		  pagename = "CREDIT";
		  break;
		/* THE DEFINITIVE DOOM Special Edition demo */
	  case 7:
		G_DeferedPlayDemo ("demo4");
		break;
	}
}



/* D_StartTitle */
void D_StartTitle (void)
{
	gameaction = ga_nothing;
	demosequence = -1;
	D_AdvanceDemo ();
}




/*      print title for every printed line */
char            title[128];



/* D_AddFile */
void D_AddFile (const char *file)
{
	size_t i;

	for (i = 0; i < D_COUNT_OF(wadfiles); ++i)
	{
		if (wadfiles[i] == NULL)
		{
			wadfiles[i] = M_strdup(file);
			return;
		}
	}

	I_Info("D_AddFile: Too many wads!\n");
}

/* IdentifyVersion */
/* Checks availability of IWAD files by name, */
/* to determine whether registered/commercial features */
/* should be executed (notably loading PWAD's). */
void IdentifyVersion (void)
{
#ifdef __unix__
	const char *configdir;
#endif
	const char *doomwaddir;
#ifdef __unix__
	doomwaddir = getenv("DOOMWADDIR");
	if (!doomwaddir)
#endif
		doomwaddir = ".";

#ifdef __unix__
	configdir = getenv("XDG_CONFIG_HOME");
	if (configdir != NULL)
	{
		sprintf(basedefault, "%s/", configdir);
	}
	else
	{
		configdir = getenv("HOME");
		if (configdir != NULL)
			sprintf(basedefault, "%s/.config/", configdir);
	}
	strcat(basedefault, "clowndoomrc");
#else
	strcpy(basedefault, "default.cfg");
#endif

	if (M_CheckParm ("-shdev"))
	{
		gamemode = shareware;
		gamemission = doom;
		devparm = d_true;
		D_AddFile (DEVDATA"doom1.wad");
		D_AddFile (DEVMAPS"data_se/texture1.lmp");
		D_AddFile (DEVMAPS"data_se/pnames.lmp");
		strcpy (basedefault,DEVDATA"default.cfg");
	}
	else if (M_CheckParm ("-regdev"))
	{
		gamemode = registered;
		gamemission = doom;
		devparm = d_true;
		D_AddFile (DEVDATA"doom.wad");
		D_AddFile (DEVMAPS"data_se/texture1.lmp");
		D_AddFile (DEVMAPS"data_se/texture2.lmp");
		D_AddFile (DEVMAPS"data_se/pnames.lmp");
		strcpy (basedefault,DEVDATA"default.cfg");
	}
	else if (M_CheckParm ("-comdev"))
	{
		gamemode = commercial;
		gamemission = doom2;
		devparm = d_true;
		/* I don't bother
		if(plutonia)
			D_AddFile (DEVDATA"plutonia.wad");
		else if(tnt)
			D_AddFile (DEVDATA"tnt.wad");
		else*/
			D_AddFile (DEVDATA"doom2.wad");

		D_AddFile (DEVMAPS"cdata/texture1.lmp");
		D_AddFile (DEVMAPS"cdata/pnames.lmp");
		strcpy (basedefault,DEVDATA"default.cfg");
	}
	else
	{
		static const struct
		{
			char filename[8 + 1];
			GameMode_t gamemode;
			GameMission_t gamemission;
			Language_t language;
		} wads[] = {
			{"doom2f",   commercial, doom2,     french },
			{"doom2",    commercial, doom2,     english},
			{"plutonia", commercial, pack_plut, english},
			{"tnt",      commercial, pack_tnt,  english},
			{"doomu",    retail,     doom,      english},
			{"doom",     registered, doom,      english},
			{"doom1",    shareware,  doom,      english},
		};

		const size_t wad_directory_length = strlen(doomwaddir);
		char* const path = (char*)malloc(wad_directory_length + 1 + sizeof(wads[0].filename) - 1 + 4 + 1);

		gamemode = indetermined;
		gamemission = none;
		language = english;

		if (path != NULL)
		{
			size_t i;

			memcpy(&path[0], doomwaddir, wad_directory_length);
			path[wad_directory_length] = '/';

			for (i = 0; i < D_COUNT_OF(wads); ++i)
			{
				sprintf(&path[wad_directory_length + 1], "%.*s.wad", (int)sizeof(wads[i].filename) - 1, wads[i].filename);

				if (M_FileExists(path))
				{
					gamemode = wads[i].gamemode;
					gamemission = wads[i].gamemission;
					language = wads[i].language;
					D_AddFile(path);
					break;
				}
			}

			free(path);

			if (language == french)
				I_Info("French version\n");
		}

		if (gamemode == indetermined)
		{
			I_Info("Game mode indeterminate.\n");

			/* We don't abort. Let's see what the PWAD contains. */
			/* exit(1); */
			/* I_Error ("Game mode indeterminate\n"); */
		}
	}
}

/* Find a Response File */
void FindResponseFile (void)
{
	int             i;
#define MAXARGVS        100

	for (i = 1;i < myargc;i++)
		if (myargv[i][0] == '@')
		{
			FILE *          handle;
			int             size;
			int             k;
			int             index;
			int             indexinfile;
			char    *infile;
			char    *file;
			char    *moreargs[20];
			char    *firstargv;

			/* READ THE RESPONSE FILE INTO MEMORY */
			handle = fopen (&myargv[i][1],"rb");
			if (!handle)
			{
				I_Info ("\nNo such response file!");
				exit(1);
			}
			I_Info("Found response file %s!\n",&myargv[i][1]);
			fseek (handle,0,SEEK_END);
			size = ftell(handle);
			fseek (handle,0,SEEK_SET);
			file = (char*)malloc (size);
			fread (file,size,1,handle);
			fclose (handle);

			/* KEEP ALL CMDLINE ARGS FOLLOWING @RESPONSEFILE ARG */
			for (index = 0,k = i+1; k < myargc; k++)
				moreargs[index++] = myargv[k];

			firstargv = myargv[0];
			myargv = (char**)malloc(sizeof(char *)*MAXARGVS);
			memset(myargv,0,sizeof(char *)*MAXARGVS);
			myargv[0] = firstargv;

			infile = file;
			indexinfile = k = 0;
			indexinfile++;  /* SKIP PAST ARGV[0] (KEEP IT) */
			do
			{
				myargv[indexinfile++] = infile+k;
				while(k < size &&
					  ((*(infile+k)>= ' '+1) && (*(infile+k)<='z')))
					k++;
				*(infile+k) = 0;
				while(k < size &&
					  ((*(infile+k)<= ' ') || (*(infile+k)>'z')))
					k++;
			} while(k < size);

			for (k = 0;k < index;k++)
				myargv[indexinfile++] = moreargs[k];
			myargc = indexinfile;

			/* DISPLAY ARGS */
			I_Info("%d command-line args:\n",myargc);
			for (k=1;k<myargc;k++)
				I_Info("%s\n",myargv[k]);

			break;
		}
}


/* D_DoomMain */
void D_DoomMain (void)
{
	int             p;
	char                    file[256];

	FindResponseFile ();

	IdentifyVersion ();

	setbuf (stdout, NULL);
	modifiedgame = d_false;

	nomonsters = M_CheckParm ("-nomonsters");
	respawnparm = M_CheckParm ("-respawn");
	fastparm = M_CheckParm ("-fast");
	devparm = M_CheckParm ("-devparm");
	if (M_CheckParm ("-altdeath"))
		deathmatch = DM_ALTERNATE;
	else if (M_CheckParm ("-deathmatch"))
		deathmatch = DM_STANDARD;

	switch ( gamemode )
	{
	  case retail:
		sprintf (title,
				 "                         "
				 "The Ultimate DOOM Startup v%i.%i"
				 "                           ",
				 VERSION/100,VERSION%100);
		break;
	  case shareware:
		sprintf (title,
				 "                            "
				 "DOOM Shareware Startup v%i.%i"
				 "                           ",
				 VERSION/100,VERSION%100);
		break;
	  case registered:
		sprintf (title,
				 "                            "
				 "DOOM Registered Startup v%i.%i"
				 "                           ",
				 VERSION/100,VERSION%100);
		break;
	  case commercial:
		switch (gamemission)
		{
		  default:
		  case doom2:
			sprintf (title,
					 "                         "
					 "DOOM 2: Hell on Earth v%i.%i"
					 "                           ",
					 VERSION/100,VERSION%100);
			break;
		  case pack_plut:
			sprintf (title,
					 "                   "
					 "DOOM 2: Plutonia Experiment v%i.%i"
					 "                           ",
					 VERSION/100,VERSION%100);
			break;
		  case pack_tnt:
			sprintf (title,
					 "                     "
					 "DOOM 2: TNT - Evilution v%i.%i"
					 "                           ",
					 VERSION/100,VERSION%100);
			break;
		}
		break;
	  default:
		sprintf (title,
				 "                     "
				 "Public DOOM - v%i.%i"
				 "                           ",
				 VERSION/100,VERSION%100);
		break;
	}

	I_Info ("%s\n",title);

	if (devparm)
		I_Info(D_DEVSTR);

#if 0
	/* Strange old junk. */
	if (M_CheckParm("-cdrom"))
	{
		I_Info(D_CDROM);
		mkdir("c:\\doomdata",0);
		strcpy (basedefault,"c:/doomdata/default.cfg");
	}
#endif

	/* turbo option */
	if ( (p=M_CheckParm ("-turbo")) )
	{
		int     scale = 200;
		extern int forwardmove[2];
		extern int sidemove[2];

		if (p<myargc-1)
			scale = atoi (myargv[p+1]);
		if (scale < 10)
			scale = 10;
		if (scale > 400)
			scale = 400;
		I_Info ("turbo scale: %i%%\n",scale);
		forwardmove[0] = forwardmove[0]*scale/100;
		forwardmove[1] = forwardmove[1]*scale/100;
		sidemove[0] = sidemove[0]*scale/100;
		sidemove[1] = sidemove[1]*scale/100;
	}

	/* add any files specified on the command line with -file wadfile */
	/* to the wad list */
	/* convenience hack to allow -wart e m to add a wad file */
	/* prepend a tilde to the filename so wadfile will be reloadable */
	p = M_CheckParm ("-wart");
	if (p)
	{
		myargv[p][4] = 'p';     /* big hack, change to -warp */

		/* Map name handling. */
		switch (gamemode )
		{
		  case shareware:
		  case retail:
		  case registered:
			sprintf (file,"~"DEVMAPS"E%cM%c.wad",
					 myargv[p+1][0], myargv[p+2][0]);
			I_Info("Warping to Episode %s, Map %s.\n",
				   myargv[p+1],myargv[p+2]);
			break;

		  case commercial:
		  default:
			p = atoi (myargv[p+1]);
			if (p<10)
			  sprintf (file,"~"DEVMAPS"cdata/map0%i.wad", p);
			else
			  sprintf (file,"~"DEVMAPS"cdata/map%i.wad", p);
			break;
		}
		D_AddFile (file);
	}

	p = M_CheckParm ("-file");
	if (p)
	{
		/* the parms after p are wadfile/lump names, */
		/* until end of parms or another - preceded parm */
		modifiedgame = d_true;            /* homebrew levels */
		while (++p != myargc && myargv[p][0] != '-')
			D_AddFile (myargv[p]);
	}

	p = M_CheckParm ("-playdemo");

	if (!p)
		p = M_CheckParm ("-timedemo");

	if (p && p < myargc-1)
	{
		sprintf (file,"%s.lmp", myargv[p+1]);
		D_AddFile (file);
		I_Info("Playing demo %s.lmp.\n",myargv[p+1]);
	}

	/* get skill / episode / map from parms */
	startskill = sk_medium;
	startepisode = 1;
	startmap = 1;
	autostart = d_false;


	p = M_CheckParm ("-skill");
	if (p && p < myargc-1)
	{
		startskill = (skill_t)(myargv[p+1][0]-'1');
		autostart = d_true;
	}

	p = M_CheckParm ("-episode");
	if (p && p < myargc-1)
	{
		startepisode = myargv[p+1][0]-'0';
		startmap = 1;
		autostart = d_true;
	}

	p = M_CheckParm ("-timer");
	if (p && p < myargc-1 && deathmatch != DM_OFF)
	{
		int     time;
		time = atoi(myargv[p+1]);
		I_Info("Levels will end after %d minute",time);
		if (time>1)
			I_Info("s");
		I_Info(".\n");
	}

	p = M_CheckParm ("-avg");
	if (p && p < myargc-1 && deathmatch != DM_OFF)
		I_Info("Austin Virtual Gaming: Levels will end after 20 minutes\n");

	p = M_CheckParm ("-warp");
	if (p && p < myargc-1)
	{
		if (gamemode == commercial)
			startmap = atoi (myargv[p+1]);
		else
		{
			startepisode = myargv[p+1][0]-'0';
			startmap = myargv[p+2][0]-'0';
		}
		autostart = d_true;
	}

	/* init subsystems */
	I_Info ("V_Init: allocate screens.\n");
	V_Init ();

	I_Info ("M_LoadDefaults: Load system defaults.\n");
	M_LoadDefaults ();              /* load before initing other systems */

	I_Info ("Z_Init: Init zone memory allocation daemon. \n");
	Z_Init ();

	I_Info ("W_Init: Init WADfiles.\n");
	W_InitMultipleFiles (wadfiles);


	/* Check for -file in shareware */
	if (modifiedgame)
	{
		/* These are the lumps that will be checked in IWAD, */
		/* if any one is not present, execution will be aborted. */
		char name[23][9]=
		{
			"e2m1","e2m2","e2m3","e2m4","e2m5","e2m6","e2m7","e2m8","e2m9",
			"e3m1","e3m3","e3m3","e3m4","e3m5","e3m6","e3m7","e3m8","e3m9",
			"dphoof","bfgga0","heada1","cybra1","spida1d1"
		};
		int i;

		if ( gamemode == shareware)
			I_Error("\nYou cannot -file with the shareware "
					"version. Register!");

		/* Check for fake IWAD with right name, */
		/* but w/o all the lumps of the registered version. */
		if (gamemode == registered)
			for (i = 0;i < 23; i++)
				if (W_CheckNumForName(name[i])<0)
					I_Error("\nThis is not the registered version.");
	}

	/* Iff additonal PWAD files are used, print modified banner */
	if (modifiedgame)
	{
		/*m*/I_Info (
			"===========================================================================\n"
			"ATTENTION:  This version of DOOM has been modified.  If you would like to\n"
			"get a copy of the original game, call 1-800-IDGAMES or see the readme file.\n"
			"        You will not receive technical support for modified games.\n"
			"                      press enter to continue\n"
			"===========================================================================\n"
			);
		getchar ();
	}


	/* Check and print which version is executed. */
	switch ( gamemode )
	{
	  case shareware:
	  case indetermined:
		I_Info (
			"===========================================================================\n"
			"                                Shareware!\n"
			"===========================================================================\n"
		);
		break;
	  case registered:
	  case retail:
	  case commercial:
		I_Info (
			"===========================================================================\n"
			"                 Commercial product - do not distribute!\n"
			"         Please report software piracy to the SPA: 1-800-388-PIR8\n"
			"===========================================================================\n"
		);
		break;

	  default:
		/* Ouch. */
		break;
	}

	I_Info ("M_Init: Init miscellaneous info.\n");
	M_Init ();

	I_Info ("R_Init: Init DOOM refresh daemon - ");
	R_Init ();

	I_Info ("\nP_Init: Init Playloop state.\n");
	P_Init ();

	I_Info ("I_Init: Setting up machine state.\n");
	I_Init ();

	I_Info ("D_CheckNetGame: Checking network game status.\n");
	D_CheckNetGame ();

	I_Info ("S_Init: Setting up sound.\n");
	S_Init (sfxVolume, musicVolume);

	I_Info ("HU_Init: Setting up heads up display.\n");
	HU_Init ();

	I_Info ("ST_Init: Init status bar.\n");
	ST_Init ();
#if 0
	/* check for a driver that wants intermission stats */
	p = M_CheckParm ("-statcopy");
	if (p && p<myargc-1)
	{
		/* for statistics driver */
		extern  void*   statcopy;

		statcopy = (void*)atoi(myargv[p+1]);
		I_Info ("External statistics registered.\n");
	}
#endif
	/* start the apropriate game based on parms */
	p = M_CheckParm ("-record");

	if (p && p < myargc-1)
	{
		G_RecordDemo (myargv[p+1]);
		autostart = d_true;
	}

	p = M_CheckParm ("-playdemo");
	if (p && p < myargc-1)
	{
		singledemo = d_true;              /* quit after one demo */
		G_DeferedPlayDemo (myargv[p+1]);
		D_DoomLoop ();  /* never returns */
	}

	p = M_CheckParm ("-timedemo");
	if (p && p < myargc-1)
	{
		G_TimeDemo (myargv[p+1]);
		D_DoomLoop ();  /* never returns */
	}

	p = M_CheckParm ("-loadgame");
	if (p && p < myargc-1)
	{
		if (M_CheckParm("-cdrom"))
			sprintf(file, "c:\\doomdata\\"SAVEGAMENAME"%c.dsg",myargv[p+1][0]);
		else
			sprintf(file, SAVEGAMENAME"%c.dsg",myargv[p+1][0]);
		G_LoadGame (file);
	}


	if ( gameaction != ga_loadgame )
	{
		if (autostart || netgame)
			G_InitNew (startskill, startepisode, startmap);
		else
			D_StartTitle ();                /* start up intro loop */

	}

	D_DoomLoop ();  /* never returns */
}
