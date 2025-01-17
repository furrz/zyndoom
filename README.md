# zyndoom

This is a fork of [clownacy/clowndoom](https://github.com/Clownacy/clowndoom/) where
I mess around with the DOOM codebase.

This is by no means meant to be a 'better' DOOM port than a modernized version like gzdoom,
nor is it meant to be 'faithful' to the original codebase (though I do not intend to change
it drastically.)


## Changes

- Add macOS support.
- Use SDL2 only, removing vestigial SDL1 and X11 support when SDL2 already covers it.
- Add `-skipmodwarning` command-line flag to skip the modified game warning prompt.
- Add `mouse_ungrab_on_pause` option to ungrab the mouse when you pause the game.
- Add `mouse_menu_pointing` option, allowing you to mouse over menu items to select them.
  Will only work when `mouse_ungrab_on_pause` is also enabled.
- If you have multiple DOOM IWADs in the wad dir, override which one to play using `-game` (with no `.WAD` on the end):
  - `doom2f`: DOOM II Commercial French
  - `doom2`: DOOM II Commercial
  - `plutonia`: The Plutonia Experiment
  - `tnt`: TNT: Evilution
  - `doomu`: DOOM I Retail
  - `doom`: DOOM I Registered
  - `doom1`: DOOM I Shareware
- New "smart" system for identifying the DOOM configuration path:
  - Checks the following directories, in order:
    - `./`
    - `$XDG_CONFIG_HOME/`
    - `$HOME/.config/`
    - `$HOME/Library/Application Support/`
    - `$LOCALAPPDATA/`
    - `$APPDATA/`
    - `/etc/doom/`
  - For the following files, in order:
    - `zyndoom.cfg`
    - `doom.cfg`
    - `default.cfg`
  - If none exist, defaults to `./zyndoom.cfg`.
  - This system is overridden by the `-config` cmdline option.


## Recommendations

- To get music working, unzip [this archive of GUS patches](https://www.doomworld.com/idgames/music/dgguspat) to a
  folder on your computer, and then set `wildmidi_config_path` in your `doom.cfg` to the full path to one of the .cfg
  files in that folder. `timidity.cfg` seems to work well.

# Original clowndoom README

# clowndoom

Yet another purist DOOM port.

This project aims to repair the Linux Doom source code, restoring features that
were lost from the DOS version, and implementing opt-in quality-of-life
improvements such as widescreen rendering.

## Features
### Repaired Downgrades
- Features that were broken by the original Linux port:
  - Music support (through WildMIDI).
  - Complete sound effect support (restored missing features such as stopping
    mid-playback and updating positional effects).
  - Low-detail mode.
- Features that were broken by the 1997 source release:
  - Animated Doom 1 intermission screens.
  - Ultimate Doom wall switch textures.
  - Volume levels and volume control.
### Portability Improvements
- Addressed various compiler warnings.
- Support for 64-bit CPUs.
- Support for Windows and modern Linux:
  - The X11 code has been converted from 8bpp to 24bpp, which is supported by
    modern X11 servers.
  - The audio code has been migrated from OSS to the miniaudio middleware
    library (supports OSS, ALSA, PulseAudio, JACK, and more).
  - As an alternative to X11 and miniaudio, SDL1 and SDL2 backends are
    available.
  - The codebase can be compiled with MSVC.
  - Windows networking code has been added.
- Eliminated reliance on C99 and C extensions, making the codebase pure ANSI C.
### Quality-of-Life Improvements
- New configuration options:
  - `novert` - Prevents the player character from walking when the mouse is
    moved.
  - `always_run` - Makes the player character run instead of walk, and vice
    versa.
  - `aspect_ratio_correction` - Restores the original 4:3 aspect ratio by
    making pixels rectangular.
  - `full_colour` - Render with more than 256 colours.
  - `prototype_light_amplification_visor_effect` - Restore the 'night vision'
    effect for the light amplification visor from the Press Release Pre-Beta.
  - `screen_width` - The screen's horizontal resolution. The maximum is 5040.
  - `screen_height` - The screen's vertical resolution. The maximum is 1800.
  - `hud_scale` - Multiplies the size of the user interface. 1 renders elements
    at their native resolutions, 2 renders them at double their resolution,
    etc. The maximum value is 9.
- The 'iddt' cheat enables kill/item/secret totals in the automap, as well as
  notifications when finding secrets.
- Assorted bug fixes.
- Better than CuckyDOOM.

## Configuration
Additional settings are found in the configuration file - 'clowndoomrc'. On
Unix platforms, it can be found in the user's standard configuration directory
(`XDG_CONFIG_HOME`, or `~/.config/` if it is undefined) named 'clowndoomrc'. On
other platforms, it can be found in the same directory as the executable (or
whichever directory the executable was invoked from) with the name
'default.cfg'.

WildMIDI requires a collection of GUS patches in order to work. The patches
should come with a Timidity-compatible '.cfg'. file. Set the
`wildmidi_config_path` option in 'clowndoomrc' to the path of this file, and
music should now work.
