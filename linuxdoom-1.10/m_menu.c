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
        DOOM selection menu, options, episode etc.
        Sliders and icons. Kinda widget stuff.

******************************************************************************/

#include <stdlib.h>
#include <ctype.h>


#include "doomdef.h"
#include "dstrings.h"

#include "d_main.h"

#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"

#include "r_local.h"


#include "hu_stuff.h"

#include "g_game.h"

#include "m_argv.h"
#include "m_swap.h"

#include "s_sound.h"

#include "doomstat.h"

/* Data. */
#include "sounds.h"

#include "m_menu.h"



extern patch_t*         hu_font[HU_FONTSIZE];
extern d_bool           message_dontfuckwithme;

extern d_bool           chat_on;                /* in heads-up code */

extern int				mouse_ungrab_on_pause;

/* defaulted values */
int                     mouseSensitivity;       /* has default */

/* Show messages has default, 0 = off, 1 = on */
int                     showMessages;


/* Blocky mode, has default, 0 = high, 1 = normal */
int                     detailLevel;
int                     screenblocks;           /* has default */

int						mouse_menu_pointing;	/* has default */


/* temp for screenblocks (0-9) */
int                     screenSize;

/* Maximum volume of a sound effect. */
/* Internal default is max out of 0-15. */
int                     sfxVolume = 15;

/* Maximum volume of music. Useless so far. */
int                     musicVolume = 15;

/* -1 = no quicksave slot picked! */
int                     quickSaveSlot;

 /* 1 = message to be printed */
int                     messageToPrint;
/* ...and here is the message string! */
const char*             messageString;

/* message x & y */
int                     messx;
int                     messy;
int                     messageLastMenuActive;

/* timed message = no input from user */
d_bool                  messageNeedsInput;

void    (*messageRoutine)(int response);

#define SAVESTRINGSIZE  24

char gammamsg[5][26] =
{
	GAMMALVL0,
	GAMMALVL1,
	GAMMALVL2,
	GAMMALVL3,
	GAMMALVL4
};

/* we are going to be entering a savegame string */
int                     saveStringEnter;
int                     saveSlot;       /* which slot to save in */
int                     saveCharIndex;  /* which char we're editing */
/* old save description before edit */
char                    saveOldString[SAVESTRINGSIZE];

d_bool                  inhelpscreens;
d_bool                  menuactive;

#define SKULLXOFF               (-32*HUD_SCALE)
#define LINEHEIGHT              (16*HUD_SCALE)

extern d_bool           sendpause;
char                    savegamestrings[10][SAVESTRINGSIZE];

char    endstring[160];


/* MENU TYPEDEFS */
typedef struct
{
	/* 0 = no cursor here, 1 = ok, 2 = arrows ok */
	short       status;

	char        name[10];

	/* choice = menu item #. */
	/* if status = 2, */
	/*   choice=0:leftarrow,1:rightarrow */
	void        (*routine)(int choice);

	/* hotkey in menu */
	char        alphaKey;
} menuitem_t;



typedef struct menu_s
{
	short               numitems;       /* # of menu items */
	struct menu_s*      prevMenu;       /* previous menu */
	menuitem_t*         menuitems;      /* menu items */
	void                (*routine)();   /* draw routine */
	short               xRaw;
	short               yRaw;              /* x,y of menu */
	short               lastOn;         /* last item user was on in menu */
} menu_t;

short           itemOn;                 /* menu item skull is on */
short           skullAnimCounter;       /* skull animation counter */
short           whichSkull;             /* which skull to draw */

/* graphic name of skulls */
/* warning: initializer-string for array of chars is too long */
char    skullName[2][/*8*/9] = {"M_SKULL1","M_SKULL2"};

/* current menudef */
menu_t* currentMenu;

/* PROTOTYPES */
void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_QuitDOOM(int choice);

void M_ChangeMessages(int choice);
void M_ChangeSensitivity(int choice);
void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_ChangeDetail(int choice);
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Sound(int choice);

void M_FinishReadThis(int choice);
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawOptions(void);
void M_DrawSound(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
void M_WriteText(int x, int y, const char *string);
int  M_StringWidth(const char *string);
int  M_StringHeight(const char *string);
void M_StartControlPanel(void);
void M_StartMessage(const char *string,void (*routine)(int),d_bool input);
void M_StopMessage(void);
void M_ClearMenus (void);




/* DOOM MENU */
typedef enum
{
	newgame = 0,
	options,
	loadgame,
	savegame,
	readthis,
	quitdoom,
	main_end
} main_e;

menuitem_t MainMenu[]=
{
	{1,"M_NGAME",M_NewGame,'n'},
	{1,"M_OPTION",M_Options,'o'},
	{1,"M_LOADG",M_LoadGame,'l'},
	{1,"M_SAVEG",M_SaveGame,'s'},
	/* Another hickup with Special edition. */
	{1,"M_RDTHIS",M_ReadThis,'r'},
	{1,"M_QUITG",M_QuitDOOM,'q'}
};

menu_t  MainDef =
{
	main_end,
	NULL,
	MainMenu,
	M_DrawMainMenu,
	97,64,
	0
};


/* EPISODE SELECT */
typedef enum
{
	ep1,
	ep2,
	ep3,
	ep4,
	ep_end
} episodes_e;

menuitem_t EpisodeMenu[]=
{
	{1,"M_EPI1", M_Episode,'k'},
	{1,"M_EPI2", M_Episode,'t'},
	{1,"M_EPI3", M_Episode,'i'},
	{1,"M_EPI4", M_Episode,'t'}
};

menu_t  EpiDef =
{
	ep_end,                    /* # of menu items */
	&MainDef,                  /* previous menu */
	EpisodeMenu,               /* menuitem_t -> */
	M_DrawEpisode,             /* drawing routine -> */
	48,63, /* x,y */
	ep1                        /* lastOn */
};

/* NEW GAME */
typedef enum
{
	killthings,
	toorough,
	hurtme,
	violence,
	nightmare,
	newg_end
} newgame_e;

menuitem_t NewGameMenu[]=
{
	{1,"M_JKILL",       M_ChooseSkill, 'i'},
	{1,"M_ROUGH",       M_ChooseSkill, 'h'},
	{1,"M_HURT",        M_ChooseSkill, 'h'},
	{1,"M_ULTRA",       M_ChooseSkill, 'u'},
	{1,"M_NMARE",       M_ChooseSkill, 'n'}
};

menu_t  NewDef =
{
	newg_end,                   /* # of menu items */
	&EpiDef,                    /* previous menu */
	NewGameMenu,                /* menuitem_t -> */
	M_DrawNewGame,              /* drawing routine -> */
	48,63,  /* x,y */
	hurtme                      /* lastOn */
};



/* OPTIONS MENU */
typedef enum
{
	endgame,
	messages,
	detail,
	scrnsize,
	option_empty1,
	mousesens,
	option_empty2,
	soundvol,
	opt_end
} options_e;

menuitem_t OptionsMenu[]=
{
	{1,"M_ENDGAM",      M_EndGame,'e'},
	{1,"M_MESSG",       M_ChangeMessages,'m'},
	{1,"M_DETAIL",      M_ChangeDetail,'g'},
	{2,"M_SCRNSZ",      M_SizeDisplay,'s'},
	{-1,"",NULL,0},
	{2,"M_MSENS",       M_ChangeSensitivity,'m'},
	{-1,"",NULL,0},
	{1,"M_SVOL",        M_Sound,'s'}
};

menu_t  OptionsDef =
{
	opt_end,
	&MainDef,
	OptionsMenu,
	M_DrawOptions,
	60,37,
	0
};

/* Read This! MENU 1 & 2 */
typedef enum
{
	rdthsempty1,
	read1_end
} read_e;

menuitem_t ReadMenu1[] =
{
	{1,"",M_ReadThis2,0}
};

menu_t  ReadDef1 =
{
	read1_end,
	&MainDef,
	ReadMenu1,
	M_DrawReadThis1,
	330,175,
	0
};

typedef enum
{
	rdthsempty2,
	read2_end
} read_e2;

menuitem_t ReadMenu2[]=
{
	{1,"",M_FinishReadThis,0}
};

menu_t  ReadDef2 =
{
	read2_end,
	&ReadDef1,
	ReadMenu2,
	M_DrawReadThis2,
	280,185,
	0
};

/* SOUND VOLUME MENU */
typedef enum
{
	sfx_vol,
	sfx_empty1,
	music_vol,
	sfx_empty2,
	sound_end
} sound_e;

menuitem_t SoundMenu[]=
{
	{2,"M_SFXVOL",M_SfxVol,'s'},
	{-1,"",NULL,0},
	{2,"M_MUSVOL",M_MusicVol,'m'},
	{-1,"",NULL,0}
};

menu_t  SoundDef =
{
	sound_end,
	&OptionsDef,
	SoundMenu,
	M_DrawSound,
	80,64,
	0
};

/* LOAD GAME MENU */
typedef enum
{
	load1,
	load2,
	load3,
	load4,
	load5,
	load6,
	load_end
} load_e;

menuitem_t LoadMenu[]=
{
	{1,"", M_LoadSelect,'1'},
	{1,"", M_LoadSelect,'2'},
	{1,"", M_LoadSelect,'3'},
	{1,"", M_LoadSelect,'4'},
	{1,"", M_LoadSelect,'5'},
	{1,"", M_LoadSelect,'6'}
};

menu_t  LoadDef =
{
	load_end,
	&MainDef,
	LoadMenu,
	M_DrawLoad,
	80,54,
	0
};

/* SAVE GAME MENU */
menuitem_t SaveMenu[]=
{
	{1,"", M_SaveSelect,'1'},
	{1,"", M_SaveSelect,'2'},
	{1,"", M_SaveSelect,'3'},
	{1,"", M_SaveSelect,'4'},
	{1,"", M_SaveSelect,'5'},
	{1,"", M_SaveSelect,'6'}
};

menu_t  SaveDef =
{
	load_end,
	&MainDef,
	SaveMenu,
	M_DrawSave,
	80,54,
	0
};


/* M_ReadSaveStrings */
/*  read the strings from the savegame files */
void M_ReadSaveStrings(void)
{
	FILE*           handle;
	int             i;
	char    name[256];

	for (i = 0;i < load_end;i++)
	{
		if (M_CheckParm("-cdrom"))
			sprintf(name,"c:\\doomdata\\"SAVEGAMENAME"%d.dsg",i);
		else
			sprintf(name,SAVEGAMENAME"%d.dsg",i);

		handle = fopen (name, "rb");
		if (handle == NULL)
		{
			strcpy(&savegamestrings[i][0],EMPTYSTRING);
			LoadMenu[i].status = 0;
			continue;
		}
		fread (&savegamestrings[i], 1, SAVESTRINGSIZE, handle);
		fclose (handle);
		LoadMenu[i].status = 1;
	}
}


/* M_LoadGame & Cie. */
void M_DrawLoad(void)
{
	int             i;

	V_DrawPatch (X_CENTRE(72),Y_CENTRE(28),SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_LOADG",PU_CACHE));
	for (i = 0;i < load_end; i++)
	{
		M_DrawSaveLoadBorder(X_CENTRE(LoadDef.xRaw),Y_CENTRE(LoadDef.yRaw)+LINEHEIGHT*i);
		M_WriteText(X_CENTRE(LoadDef.xRaw),Y_CENTRE(LoadDef.yRaw)+LINEHEIGHT*i,savegamestrings[i]);
	}
}



/* Draw border for the savegame description */
void M_DrawSaveLoadBorder(int x,int y)
{
	int             i;

	V_DrawPatch (x-8*HUD_SCALE,y+7*HUD_SCALE,SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_LSLEFT",PU_CACHE));

	for (i = 0;i < 24;i++)
	{
		V_DrawPatch (x,y+7*HUD_SCALE,SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_LSCNTR",PU_CACHE));
		x += 8*HUD_SCALE;
	}

	V_DrawPatch (x,y+7*HUD_SCALE,SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_LSRGHT",PU_CACHE));
}



/* User wants to load this game */
void M_LoadSelect(int choice)
{
	char    name[256];

	if (M_CheckParm("-cdrom"))
		sprintf(name,"c:\\doomdata\\"SAVEGAMENAME"%d.dsg",choice);
	else
		sprintf(name,SAVEGAMENAME"%d.dsg",choice);
	G_LoadGame (name);
	M_ClearMenus ();
}

/* Selected from DOOM menu */
void M_LoadGame (int choice)
{
	(void)choice;

	if (netgame)
	{
		M_StartMessage(LOADNET,NULL,d_false);
		return;
	}

	M_SetupNextMenu(&LoadDef);
	M_ReadSaveStrings();
}


/*  M_SaveGame & Cie. */
void M_DrawSave(void)
{
	int             i;

	V_DrawPatch (X_CENTRE(72),Y_CENTRE(28),SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_SAVEG",PU_CACHE));
	for (i = 0;i < load_end; i++)
	{
		M_DrawSaveLoadBorder(X_CENTRE(LoadDef.xRaw),Y_CENTRE(LoadDef.yRaw)+LINEHEIGHT*i);
		M_WriteText(X_CENTRE(LoadDef.xRaw),Y_CENTRE(LoadDef.yRaw)+LINEHEIGHT*i,savegamestrings[i]);
	}

	if (saveStringEnter)
	{
		i = M_StringWidth(savegamestrings[saveSlot]);
		M_WriteText(X_CENTRE(LoadDef.xRaw) + i,Y_CENTRE(LoadDef.yRaw)+LINEHEIGHT*saveSlot,"_");
	}
}

/* M_Responder calls this when user is finished */
void M_DoSave(int slot)
{
	G_SaveGame (slot,savegamestrings[slot]);
	M_ClearMenus ();

	/* PICK QUICKSAVE SLOT YET? */
	if (quickSaveSlot == -2)
		quickSaveSlot = slot;
}

/* User wants to save. Start string input for M_Responder */
void M_SaveSelect(int choice)
{
	/* we are going to be intercepting all chars */
	saveStringEnter = 1;

	saveSlot = choice;
	strcpy(saveOldString,savegamestrings[choice]);
	if (!strcmp(savegamestrings[choice],EMPTYSTRING))
		savegamestrings[choice][0] = 0;
	saveCharIndex = strlen(savegamestrings[choice]);
}

/* Selected from DOOM menu */
void M_SaveGame (int choice)
{
	(void)choice;

	if (!usergame)
	{
		M_StartMessage(SAVEDEAD,NULL,d_false);
		return;
	}

	if (gamestate != GS_LEVEL)
		return;

	M_SetupNextMenu(&SaveDef);
	M_ReadSaveStrings();
}



/*      M_QuickSave */
char    tempstring[83];

void M_QuickSaveResponse(int ch)
{
	if (ch == 'y')
	{
		M_DoSave(quickSaveSlot);
		S_StartSound(NULL,sfx_swtchx);
	}
}

void M_QuickSave(void)
{
	if (!usergame)
	{
		S_StartSound(NULL,sfx_oof);
		return;
	}

	if (gamestate != GS_LEVEL)
		return;

	if (quickSaveSlot < 0)
	{
		M_StartControlPanel();
		M_ReadSaveStrings();
		M_SetupNextMenu(&SaveDef);
		quickSaveSlot = -2;     /* means to pick a slot now */
		return;
	}
	sprintf(tempstring,QSPROMPT,savegamestrings[quickSaveSlot]);
	M_StartMessage(tempstring,M_QuickSaveResponse,d_true);
}



/* M_QuickLoad */
void M_QuickLoadResponse(int ch)
{
	if (ch == 'y')
	{
		M_LoadSelect(quickSaveSlot);
		S_StartSound(NULL,sfx_swtchx);
	}
}


void M_QuickLoad(void)
{
	if (netgame)
	{
		M_StartMessage(QLOADNET,NULL,d_false);
		return;
	}

	if (quickSaveSlot < 0)
	{
		M_StartMessage(QSAVESPOT,NULL,d_false);
		return;
	}
	sprintf(tempstring,QLPROMPT,savegamestrings[quickSaveSlot]);
	M_StartMessage(tempstring,M_QuickLoadResponse,d_true);
}




/* Read This Menus */
/* Had a "quick hack to fix romero bug" */
void M_DrawReadThis1(void)
{
	inhelpscreens = d_true;
	V_ClearScreen(SCREEN_FRAMEBUFFER);
	switch ( gamemode )
	{
	  case commercial:
		V_DrawPatch (X_CENTRE(0),Y_CENTRE(0),SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("HELP",PU_CACHE));
		break;
	  case shareware:
	  case registered:
	  case retail:
		V_DrawPatch (X_CENTRE(0),Y_CENTRE(0),SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("HELP1",PU_CACHE));
		break;
	  default:
		break;
	}
	return;
}



/* Read This Menus - optional second page. */
void M_DrawReadThis2(void)
{
	inhelpscreens = d_true;
	V_ClearScreen(SCREEN_FRAMEBUFFER);
	switch ( gamemode )
	{
	  case retail:
	  case commercial:
		/* This hack keeps us from having to change menus. */
		V_DrawPatch (X_CENTRE(0),Y_CENTRE(0),SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("CREDIT",PU_CACHE));
		break;
	  case shareware:
	  case registered:
		V_DrawPatch (X_CENTRE(0),Y_CENTRE(0),SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("HELP2",PU_CACHE));
		break;
	  default:
		break;
	}
	return;
}


/* Change Sfx & Music volumes */
void M_DrawSound(void)
{
	V_DrawPatch (X_CENTRE(60),Y_CENTRE(38),SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_SVOL",PU_CACHE));

	M_DrawThermo(X_CENTRE(SoundDef.xRaw),Y_CENTRE(SoundDef.yRaw)+LINEHEIGHT*(sfx_vol+1),
				 16,sfxVolume);

	M_DrawThermo(X_CENTRE(SoundDef.xRaw),Y_CENTRE(SoundDef.yRaw)+LINEHEIGHT*(music_vol+1),
				 16,musicVolume);
}

void M_Sound(int choice)
{
	(void)choice;

	M_SetupNextMenu(&SoundDef);
}

void M_SfxVol(int choice)
{
	switch(choice)
	{
	  case 0:
		if (sfxVolume)
			sfxVolume--;
		break;
	  case 1:
		if (sfxVolume < 15)
			sfxVolume++;
		break;
	}

	S_SetSfxVolume(sfxVolume);
}

void M_MusicVol(int choice)
{
	switch(choice)
	{
	  case 0:
		if (musicVolume)
			musicVolume--;
		break;
	  case 1:
		if (musicVolume < 15)
			musicVolume++;
		break;
	}

	S_SetMusicVolume(musicVolume);
}




/* M_DrawMainMenu */
void M_DrawMainMenu(void)
{
	V_DrawPatch (X_CENTRE(94),Y_CENTRE(2),SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_DOOM",PU_CACHE));
}




/* M_NewGame */
void M_DrawNewGame(void)
{
	V_DrawPatch (X_CENTRE(96),Y_CENTRE(14),SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_NEWG",PU_CACHE));
	V_DrawPatch (X_CENTRE(54),Y_CENTRE(38),SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_SKILL",PU_CACHE));
}

void M_NewGame(int choice)
{
	(void)choice;

	if (netgame && !demoplayback)
	{
		M_StartMessage(NEWGAME,NULL,d_false);
		return;
	}

	if ( gamemode == commercial )
		M_SetupNextMenu(&NewDef);
	else
		M_SetupNextMenu(&EpiDef);
}


/*      M_Episode */
int     epi;

void M_DrawEpisode(void)
{
	V_DrawPatch (X_CENTRE(54),Y_CENTRE(38),SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_EPISOD",PU_CACHE));
}

void M_VerifyNightmare(int ch)
{
	if (ch != 'y')
		return;

	G_DeferedInitNew(sk_nightmare,epi+1,1);
	M_ClearMenus ();
}

void M_ChooseSkill(int choice)
{
	if (choice == nightmare)
	{
		M_StartMessage(NIGHTMARE,M_VerifyNightmare,d_true);
		return;
	}

	G_DeferedInitNew((skill_t)choice,epi+1,1);
	M_ClearMenus ();
}

void M_Episode(int choice)
{
	if ( (gamemode == shareware)
		 && choice)
	{
		M_StartMessage(SWSTRING,NULL,d_false);
		M_SetupNextMenu(&ReadDef1);
		return;
	}

	/* Yet another hack... */
	if ( (gamemode == registered)
		 && (choice > 2))
	{
	  I_Info("M_Episode: 4th episode requires UltimateDOOM\n");
	  choice = 0;
	}

	epi = choice;
	M_SetupNextMenu(&NewDef);
}



/* M_Options */
char    detailNames[2][9]       = {"M_GDHIGH","M_GDLOW"};
char    msgNames[2][9]          = {"M_MSGOFF","M_MSGON"};


void M_DrawOptions(void)
{
	V_DrawPatch (X_CENTRE(108),Y_CENTRE(15),SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_OPTTTL",PU_CACHE));

	V_DrawPatch (X_CENTRE(OptionsDef.xRaw) + 175*HUD_SCALE,Y_CENTRE(OptionsDef.yRaw)+LINEHEIGHT*detail,SCREEN_FRAMEBUFFER,
					   (patch_t*)W_CacheLumpName(detailNames[detailLevel],PU_CACHE));

	V_DrawPatch (X_CENTRE(OptionsDef.xRaw) + 120 * HUD_SCALE,Y_CENTRE(OptionsDef.yRaw)+LINEHEIGHT*messages,SCREEN_FRAMEBUFFER,
					   (patch_t*)W_CacheLumpName(msgNames[showMessages],PU_CACHE));

	M_DrawThermo(X_CENTRE(OptionsDef.xRaw),Y_CENTRE(OptionsDef.yRaw)+LINEHEIGHT*(mousesens+1),
				 10,mouseSensitivity);

	M_DrawThermo(X_CENTRE(OptionsDef.xRaw),Y_CENTRE(OptionsDef.yRaw)+LINEHEIGHT*(scrnsize+1),
				 9,screenSize);
}

void M_Options(int choice)
{
	(void)choice;

	M_SetupNextMenu(&OptionsDef);
}



/*      Toggle messages on/off */
void M_ChangeMessages(int choice)
{
	/* warning: unused parameter `int choice' */
	(void)choice;

	showMessages = !showMessages;

	players[consoleplayer].message = showMessages ? MSGON : MSGOFF;

	message_dontfuckwithme = d_true;
}


/* M_EndGame */
void M_EndGameResponse(int ch)
{
	if (ch != 'y')
		return;

	currentMenu->lastOn = itemOn;
	M_ClearMenus ();
	D_StartTitle ();
}

void M_EndGame(int choice)
{
	(void)choice;

	if (!usergame)
	{
		S_StartSound(NULL,sfx_oof);
		return;
	}

	if (netgame)
	{
		M_StartMessage(NETEND,NULL,d_false);
		return;
	}

	M_StartMessage(ENDGAME,M_EndGameResponse,d_true);
}




/* M_ReadThis */
void M_ReadThis(int choice)
{
	(void)choice;

	M_SetupNextMenu(&ReadDef1);
}

void M_ReadThis2(int choice)
{
	(void)choice;

	M_SetupNextMenu(&ReadDef2);
}

void M_FinishReadThis(int choice)
{
	(void)choice;

	M_SetupNextMenu(&MainDef);
}




/* M_QuitDOOM */
static const int     quitsounds[8] =
{
	sfx_pldeth,
	sfx_dmpain,
	sfx_popain,
	sfx_slop,
	sfx_telept,
	sfx_posit1,
	sfx_posit3,
	sfx_sgtatk
};

static const int     quitsounds2[8] =
{
	sfx_vilact,
	sfx_getpow,
	sfx_boscub,
	sfx_slop,
	sfx_skeswg,
	sfx_kntdth,
	sfx_bspact,
	sfx_sgtatk
};



void M_QuitResponse(int ch)
{
	if (ch != 'y')
		return;
	if (!netgame)
	{
		if (gamemode == commercial)
			S_StartSound(NULL,quitsounds2[(gametic>>2)%D_COUNT_OF(quitsounds2)]);
		else
			S_StartSound(NULL,quitsounds[(gametic>>2)%D_COUNT_OF(quitsounds)]);
		I_WaitVBL(105);
	}
	I_Quit ();
}




void M_QuitDOOM(int choice)
{
	static const char* const endmsg[] =
	{
		QUITMSG,
		QUITMSG1,
		QUITMSGDEV
	};

	static const char* const endmsg2[] =
	{
		QUITMSG,
		QUITMSG2,
		QUITMSGDEV
	};

	const char *message;

	(void)choice;

	/* We pick index 0 which is language sensitive, */
	/*  or one at random, between 1 and maximum number. */
	if (language != english)
	{
		message = QUITMSG;
	}
	else
	{
		if (gamemode == commercial)
			message = endmsg2[gametic % D_COUNT_OF(endmsg2)];
		else
			message = endmsg[gametic % D_COUNT_OF(endmsg)];
	}

	sprintf(endstring,"%s\n\n"DOSY, message);
	M_StartMessage(endstring,M_QuitResponse,d_true);
}




void M_ChangeSensitivity(int choice)
{
	switch(choice)
	{
	  case 0:
		if (mouseSensitivity)
			mouseSensitivity--;
		break;
	  case 1:
		if (mouseSensitivity < 9)
			mouseSensitivity++;
		break;
	}
}




void M_ChangeDetail(int choice)
{
	(void)choice;

	detailLevel = 1 - detailLevel;

	R_SetViewSize (screenblocks, detailLevel, SCREENWIDTH, SCREENHEIGHT, HUD_SCALE);

	if (!detailLevel)
		players[consoleplayer].message = DETAILHI;
	else
		players[consoleplayer].message = DETAILLO;
}




void M_SizeDisplay(int choice)
{
	switch(choice)
	{
	  case 0:
		if (screenSize > 0)
		{
			screenblocks--;
			screenSize--;
		}
		break;
	  case 1:
		if (screenSize < 8)
		{
			screenblocks++;
			screenSize++;
		}
		break;
	}

	R_SetViewSize (screenblocks, detailLevel, SCREENWIDTH, SCREENHEIGHT, HUD_SCALE);
}




/*      Menu Functions */
void
M_DrawThermo
( int   x,
  int   y,
  int   thermWidth,
  int   thermDot )
{
	int         xx;
	int         i;

	xx = x;
	V_DrawPatch (xx,y,SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_THERML",PU_CACHE));
	xx += 8*HUD_SCALE;
	for (i=0;i<thermWidth;i++)
	{
		V_DrawPatch (xx,y,SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_THERMM",PU_CACHE));
		xx += 8*HUD_SCALE;
	}
	V_DrawPatch (xx,y,SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_THERMR",PU_CACHE));

	V_DrawPatch ((x+8*HUD_SCALE) + thermDot*8*HUD_SCALE,y,
					   SCREEN_FRAMEBUFFER,(patch_t*)W_CacheLumpName("M_THERMO",PU_CACHE));
}



void
M_DrawEmptyCell
( menu_t*       menu,
  int           item )
{
	V_DrawPatch (X_CENTRE(menu->xRaw) - 10*HUD_SCALE,        Y_CENTRE(menu->yRaw)+item*LINEHEIGHT - 1*HUD_SCALE, SCREEN_FRAMEBUFFER,
					   (patch_t*)W_CacheLumpName("M_CELL1",PU_CACHE));
}

void
M_DrawSelCell
( menu_t*       menu,
  int           item )
{
	V_DrawPatch (X_CENTRE(menu->xRaw) - 10*HUD_SCALE,        Y_CENTRE(menu->yRaw)+item*LINEHEIGHT - 1*HUD_SCALE, SCREEN_FRAMEBUFFER,
					   (patch_t*)W_CacheLumpName("M_CELL2",PU_CACHE));
}


void
M_StartMessage
( const char*   string,
  void(*routine)(int),
  d_bool        input )
{
	messageLastMenuActive = menuactive;
	messageToPrint = 1;
	messageString = string;
	messageRoutine = routine;
	messageNeedsInput = input;
	menuactive = d_true;
	return;
}



void M_StopMessage(void)
{
	menuactive = messageLastMenuActive;
	messageToPrint = 0;
}



/* Find string width from hu_font chars */
int M_StringWidth(const char* string)
{
	size_t          i;
	int             w = 0;
	int             c;

	for (i = 0;i < strlen(string);i++)
	{
		c = toupper(string[i]) - HU_FONTSTART;
		if (c < 0 || c >= HU_FONTSIZE)
			w += 4*HUD_SCALE;
		else
			w += SHORT (hu_font[c]->width)*HUD_SCALE;
	}

	return w;
}



/*      Find string height from hu_font chars */
int M_StringHeight(const char* string)
{
	size_t          i;
	int             h;
	int             height = SHORT(hu_font[0]->height)*HUD_SCALE;

	h = height;
	for (i = 0;i < strlen(string);i++)
		if (string[i] == '\n')
			h += height;

	return h;
}


/*      Write a string using the hu_font */
void
M_WriteText
( int           x,
  int           y,
  const char*   string)
{
	int         w;
	const char* ch;
	int         c;
	int         cx;
	int         cy;


	ch = string;
	cx = x;
	cy = y;

	while(1)
	{
		c = *ch++;
		if (!c)
			break;
		if (c == '\n')
		{
			cx = x;
			cy += 12*HUD_SCALE;
			continue;
		}

		c = toupper(c) - HU_FONTSTART;
		if (c < 0 || c>= HU_FONTSIZE)
		{
			cx += 4*HUD_SCALE;
			continue;
		}

		w = SHORT (hu_font[c]->width)*HUD_SCALE;
		if (cx+w > SCREENWIDTH)
			break;
		V_DrawPatch(cx, cy, SCREEN_FRAMEBUFFER, hu_font[c]);
		cx+=w;
	}
}



/* CONTROL PANEL */

/* M_Responder */
d_bool M_Responder (event_t* ev)
{
	int             ch;
	int             i;
	static  int     joywait = 0;
	static  int     mousewait = 0;
	static  int     mousey = 0;
	static  int     lasty = 0;
	static  int     mousex = 0;
	static  int     lastx = 0;

	ch = -1;

	if (ev->type == ev_joystick && joywait < I_GetTime())
	{
		if (ev->data3 == -1)
		{
			ch = KEY_UPARROW;
			joywait = I_GetTime() + 5;
		}
		else if (ev->data3 == 1)
		{
			ch = KEY_DOWNARROW;
			joywait = I_GetTime() + 5;
		}

		if (ev->data2 == -1)
		{
			ch = KEY_LEFTARROW;
			joywait = I_GetTime() + 2;
		}
		else if (ev->data2 == 1)
		{
			ch = KEY_RIGHTARROW;
			joywait = I_GetTime() + 2;
		}

		if (ev->data1&1)
		{
			ch = KEY_ENTER;
			joywait = I_GetTime() + 5;
		}
		if (ev->data1&2)
		{
			ch = KEY_BACKSPACE;
			joywait = I_GetTime() + 5;
		}
	}
	else
	{
		if (ev->type == ev_mouse && mousewait < I_GetTime())
		{
			mousey += ev->data3;
			mousex += ev->data2;

			if (mouse_menu_pointing && mouse_ungrab_on_pause) {
				/* new behaviour: hover over a menu item to select it */

				const int max = currentMenu->numitems;
				for (i = 0; i < max; i++)
				{
					const int y = Y_CENTRE(currentMenu->yRaw) + i * LINEHEIGHT;

					if (currentMenu->menuitems[i].name[0]
						&& currentMenu->menuitems[i].status != -1
						&& ev->data5 >= y && ev->data5 <= y + LINEHEIGHT
						&& itemOn != i) {
						itemOn = i;
						S_StartSound(NULL,sfx_pstop);
					}
				}

			} else {
				/* old behaviour: move mouse up/down to change selection */

				if (mousey < lasty-30*4)
				{
					ch = KEY_DOWNARROW;
					mousewait = I_GetTime() + 5;
					mousey = lasty -= 30*4;
				}
				else if (mousey > lasty+30*4)
				{
					ch = KEY_UPARROW;
					mousewait = I_GetTime() + 5;
					mousey = lasty += 30*4;
				}
				if (mousex < lastx-30*4)
				{
					ch = KEY_LEFTARROW;
					mousewait = I_GetTime() + 5;
					mousex = lastx -= 30*4;
				}
				else if (mousex > lastx+30*44)
				{
					ch = KEY_RIGHTARROW;
					mousewait = I_GetTime() + 5;
					mousex = lastx += 30*4;
				}
			}

			if (ev->data1&1)
			{
				ch = KEY_ENTER;
				mousewait = I_GetTime() + 15;
			}

			if (ev->data1&2)
			{
				ch = KEY_BACKSPACE;
				mousewait = I_GetTime() + 15;
			}
		}
		else if (ev->type == ev_keydown)
		{
			ch = ev->data1;
		}
	}

	if (ch == -1)
		return d_false;


	/* Save Game string input */
	if (saveStringEnter)
	{
		switch(ch)
		{
		  case KEY_BACKSPACE:
			if (saveCharIndex > 0)
			{
				saveCharIndex--;
				savegamestrings[saveSlot][saveCharIndex] = 0;
			}
			break;

		  case KEY_ESCAPE:
			saveStringEnter = 0;
			strcpy(&savegamestrings[saveSlot][0],saveOldString);
			break;

		  case KEY_ENTER:
			saveStringEnter = 0;
			if (savegamestrings[saveSlot][0])
				M_DoSave(saveSlot);
			break;

		  default:
			ch = toupper(ch);
			if (ch != 32)
				if (ch-HU_FONTSTART < 0 || ch-HU_FONTSTART >= HU_FONTSIZE)
					break;
			if (ch >= 32 && ch <= 127 &&
				saveCharIndex < SAVESTRINGSIZE-1 &&
				M_StringWidth(savegamestrings[saveSlot]) <
				(SAVESTRINGSIZE-2)*8*HUD_SCALE)
			{
				savegamestrings[saveSlot][saveCharIndex++] = ch;
				savegamestrings[saveSlot][saveCharIndex] = 0;
			}
			break;
		}
		return d_true;
	}

	/* Take care of any messages that need input */
	if (messageToPrint)
	{
		if (messageNeedsInput == d_true &&
			!(ch == ' ' || ch == 'n' || ch == 'y' || ch == KEY_ESCAPE))
			return d_false;

		menuactive = messageLastMenuActive;
		messageToPrint = 0;
		if (messageRoutine)
			messageRoutine(ch);

		menuactive = d_false;
		S_StartSound(NULL,sfx_swtchx);
		return d_true;
	}

	if (devparm && ch == KEY_F1)
	{
		G_ScreenShot ();
		return d_true;
	}


	/* F-Keys */
	if (!menuactive)
		switch(ch)
		{
		  case KEY_MINUS:         /* Screen size down */
			if (automapactive || chat_on)
				return d_false;
			M_SizeDisplay(0);
			S_StartSound(NULL,sfx_stnmov);
			return d_true;

		  case KEY_EQUALS:        /* Screen size up */
			if (automapactive || chat_on)
				return d_false;
			M_SizeDisplay(1);
			S_StartSound(NULL,sfx_stnmov);
			return d_true;

		  case KEY_F1:            /* Help key */
			M_StartControlPanel ();

			if ( gamemode == retail )
			  currentMenu = &ReadDef2;
			else
			  currentMenu = &ReadDef1;

			itemOn = 0;
			S_StartSound(NULL,sfx_swtchn);
			return d_true;

		  case KEY_F2:            /* Save */
			M_StartControlPanel();
			S_StartSound(NULL,sfx_swtchn);
			M_SaveGame(0);
			return d_true;

		  case KEY_F3:            /* Load */
			M_StartControlPanel();
			S_StartSound(NULL,sfx_swtchn);
			M_LoadGame(0);
			return d_true;

		  case KEY_F4:            /* Sound Volume */
			M_StartControlPanel ();
			currentMenu = &SoundDef;
			itemOn = sfx_vol;
			S_StartSound(NULL,sfx_swtchn);
			return d_true;

		  case KEY_F5:            /* Detail toggle */
			M_ChangeDetail(0);
			S_StartSound(NULL,sfx_swtchn);
			return d_true;

		  case KEY_F6:            /* Quicksave */
			S_StartSound(NULL,sfx_swtchn);
			M_QuickSave();
			return d_true;

		  case KEY_F7:            /* End game */
			S_StartSound(NULL,sfx_swtchn);
			M_EndGame(0);
			return d_true;

		  case KEY_F8:            /* Toggle messages */
			M_ChangeMessages(0);
			S_StartSound(NULL,sfx_swtchn);
			return d_true;

		  case KEY_F9:            /* Quickload */
			S_StartSound(NULL,sfx_swtchn);
			M_QuickLoad();
			return d_true;

		  case KEY_F10:           /* Quit DOOM */
			S_StartSound(NULL,sfx_swtchn);
			M_QuitDOOM(0);
			return d_true;

		  case KEY_F11:           /* gamma toggle */
			usegamma++;
			if (usegamma > 4)
				usegamma = 0;
			players[consoleplayer].message = gammamsg[usegamma];
			V_SetPalette(0);
			return d_true;

		}


	/* Pop-up menu? */
	if (!menuactive)
	{
		if (ch == KEY_ESCAPE)
		{
			M_StartControlPanel ();
			S_StartSound(NULL,sfx_swtchn);
			return d_true;
		}
		return d_false;
	}


	/* Keys usable within menu */
	switch (ch)
	{
	  case KEY_DOWNARROW:
		do
		{
			if (itemOn+1 > currentMenu->numitems-1)
				itemOn = 0;
			else itemOn++;
			S_StartSound(NULL,sfx_pstop);
		} while(currentMenu->menuitems[itemOn].status==-1);
		return d_true;

	  case KEY_UPARROW:
		do
		{
			if (!itemOn)
				itemOn = currentMenu->numitems-1;
			else itemOn--;
			S_StartSound(NULL,sfx_pstop);
		} while(currentMenu->menuitems[itemOn].status==-1);
		return d_true;

	  case KEY_LEFTARROW:
		if (currentMenu->menuitems[itemOn].routine &&
			currentMenu->menuitems[itemOn].status == 2)
		{
			S_StartSound(NULL,sfx_stnmov);
			currentMenu->menuitems[itemOn].routine(0);
		}
		return d_true;

	  case KEY_RIGHTARROW:
		if (currentMenu->menuitems[itemOn].routine &&
			currentMenu->menuitems[itemOn].status == 2)
		{
			S_StartSound(NULL,sfx_stnmov);
			currentMenu->menuitems[itemOn].routine(1);
		}
		return d_true;

	  case KEY_ENTER:
		if (currentMenu->menuitems[itemOn].routine &&
			currentMenu->menuitems[itemOn].status)
		{
			currentMenu->lastOn = itemOn;
			if (currentMenu->menuitems[itemOn].status == 2)
			{
				currentMenu->menuitems[itemOn].routine(1);      /* right arrow */
				S_StartSound(NULL,sfx_stnmov);
			}
			else
			{
				currentMenu->menuitems[itemOn].routine(itemOn);
				S_StartSound(NULL,sfx_pistol);
			}
		}
		return d_true;

	  case KEY_ESCAPE:
		currentMenu->lastOn = itemOn;
		M_ClearMenus ();
		S_StartSound(NULL,sfx_swtchx);
		return d_true;

	  case KEY_BACKSPACE:
		currentMenu->lastOn = itemOn;
		if (currentMenu->prevMenu)
		{
			currentMenu = currentMenu->prevMenu;
			itemOn = currentMenu->lastOn;
			S_StartSound(NULL,sfx_swtchn);
		}
		return d_true;

	  default:
		for (i = itemOn+1;i < currentMenu->numitems;i++)
			if (currentMenu->menuitems[i].alphaKey == ch)
			{
				itemOn = i;
				S_StartSound(NULL,sfx_pstop);
				return d_true;
			}
		for (i = 0;i <= itemOn;i++)
			if (currentMenu->menuitems[i].alphaKey == ch)
			{
				itemOn = i;
				S_StartSound(NULL,sfx_pstop);
				return d_true;
			}
		break;

	}

	return d_false;
}



/* M_StartControlPanel */
void M_StartControlPanel (void)
{
	/* intro might call this repeatedly */
	if (menuactive)
		return;

	menuactive = 1;
	currentMenu = &MainDef;         /* JDC */
	itemOn = currentMenu->lastOn;   /* JDC */
}


/* M_Drawer */
/* Called after the view has been rendered, */
/* but before it has been blitted. */
void M_Drawer (void)
{
	int          x;
	int          y;
	size_t       i;
	size_t       max;

	inhelpscreens = d_false;


	/* Horiz. & Vertically center string and print it. */
	if (messageToPrint)
	{
		size_t start, length;

		y = Y_CENTRE(ORIGINAL_SCREEN_HEIGHT / 2) - M_StringHeight(messageString) / 2;

		for (start = 0; ; start += length + 1)
		{
			char string[40 + 1];
			const size_t string_length = strcspn(&messageString[start], "\n");

			length = D_MIN(string_length, sizeof(string) - 1);
			memcpy(string, &messageString[start], length);
			string[length] = '\0';

			x = X_CENTRE(ORIGINAL_SCREEN_WIDTH/2) - M_StringWidth(string) / 2;
			M_WriteText(x, y, string);
			y += SHORT(hu_font[0]->height)*HUD_SCALE;

			if (messageString[start + length] == '\0')
				break;
		}

		return;
	}

	if (!menuactive)
		return;

	if (currentMenu->routine)
		currentMenu->routine();         /* call Draw routine */

	/* DRAW MENU */
	x = X_CENTRE(currentMenu->xRaw);
	y = Y_CENTRE(currentMenu->yRaw);
	max = currentMenu->numitems;

	for (i=0;i<max;i++)
	{
		if (currentMenu->menuitems[i].name[0])
			V_DrawPatch (x,y,SCREEN_FRAMEBUFFER,
							   (patch_t*)W_CacheLumpName(currentMenu->menuitems[i].name ,PU_CACHE));
		y += LINEHEIGHT;
	}


	/* DRAW SKULL */
	V_DrawPatch(x + SKULLXOFF,Y_CENTRE(currentMenu->yRaw) - 5*HUD_SCALE + itemOn*LINEHEIGHT, SCREEN_FRAMEBUFFER,
					  (patch_t*)W_CacheLumpName(skullName[whichSkull],PU_CACHE));

}


/* M_ClearMenus */
void M_ClearMenus (void)
{
	menuactive = 0;
	/* if (!netgame && usergame && paused) */
	/*       sendpause = true; */
}




/* M_SetupNextMenu */
void M_SetupNextMenu(menu_t *menudef)
{
	currentMenu = menudef;
	itemOn = currentMenu->lastOn;
}


/* M_Ticker */
void M_Ticker (void)
{
	if (--skullAnimCounter <= 0)
	{
		whichSkull ^= 1;
		skullAnimCounter = 8;
	}
}


/* M_Init */
void M_Init (void)
{
	currentMenu = &MainDef;
	menuactive = 0;
	itemOn = currentMenu->lastOn;
	whichSkull = 0;
	skullAnimCounter = 10;
	screenSize = screenblocks - 3;
	messageToPrint = 0;
	messageString = NULL;
	messageLastMenuActive = menuactive;
	quickSaveSlot = -1;

	/* Here we could catch other version dependencies, */
	/*  like HELP1/2, and four episodes. */


	switch ( gamemode )
	{
	  case commercial:
		/* This is used because DOOM 2 had only one HELP */
		/*  page. I use CREDIT as second page now, but */
		/*  kept this hack for educational purposes. */
		MainMenu[readthis] = MainMenu[quitdoom];
		MainDef.numitems--;
		MainDef.yRaw += 8;
		NewDef.prevMenu = &MainDef;
		ReadDef1.routine = M_DrawReadThis1;
		ReadDef1.xRaw = 330;
		ReadDef1.yRaw = 165;
		ReadMenu1[0].routine = M_FinishReadThis;
		break;
	  case shareware:
		/* Episode 2 and 3 are handled, */
		/*  branching to an ad screen. */
	  case registered:
		/* We need to remove the fourth episode. */
		EpiDef.numitems--;
		break;
	  case retail:
		/* We are fine. */
	  default:
		break;
	}

}

