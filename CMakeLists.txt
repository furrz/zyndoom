cmake_minimum_required(VERSION 3.19)

project(zyndoom LANGUAGES C)

option(WILDMIDI "Render MIDI using the WildMIDI library" ON)
option(FRENCH "Change the language of various strings to French" OFF)

add_executable(zyndoom WIN32
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/clownlibs/clowncommon/clowncommon.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/clownlibs/dictionary.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/clownlibs/dictionary.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/am_map.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/am_map.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/d_englsh.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/d_event.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/d_french.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/d_items.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/d_items.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/d_main.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/d_main.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/d_net.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/d_net.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/doomdata.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/doomdef.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/doomdef.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/doomstat.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/doomstat.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/doomtype.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/d_player.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/dstrings.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/d_textur.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/d_think.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/d_ticcmd.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/f_finale.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/f_finale.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/f_wipe.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/f_wipe.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/g_game.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/g_game.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/hu_lib.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/hu_lib.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/hu_stuff.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/hu_stuff.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/ib_sound.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/ib_system.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/ib_video.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/i_main.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/i_net.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/i_net.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/info.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/info.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/i_sound.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/i_sound.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/i_system.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/i_system.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/i_video.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/i_video.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_argv.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_argv.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_bbox.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_bbox.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_cheat.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_cheat.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_fixed.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_fixed.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_menu.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_menu.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_misc.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_misc.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_random.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_random.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_swap.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_ceilng.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_doors.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_enemy.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_floor.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_inter.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_inter.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_lights.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_local.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_map.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_maputl.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_mobj.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_mobj.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_plats.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_pspr.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_pspr.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_saveg.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_saveg.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_setup.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_setup.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_sight.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_spec.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_spec.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_switch.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_telept.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_tick.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_tick.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/p_user.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_bsp.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_bsp.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_data.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_data.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_defs.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_draw.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_draw.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_local.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_main.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_main.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_plane.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_plane.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_segs.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_segs.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_sky.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_sky.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_state.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_things.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/r_things.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/sounds.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/sounds.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/s_sound.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/s_sound.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/st_lib.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/st_lib.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/st_stuff.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/st_stuff.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/tables.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/tables.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/v_video.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/v_video.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/wi_stuff.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/wi_stuff.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/w_wad.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/w_wad.h"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/z_zone.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/z_zone.h"
)

set_target_properties(zyndoom PROPERTIES C_STANDARD 90)

if(CMAKE_C_BYTE_ORDER STREQUAL "BIG_ENDIAN")
	target_compile_definitions(zyndoom PRIVATE PLATFORM_IS_BIG_ENDIAN)
	target_sources(zyndoom "${PROJECT_SOURCE_DIR}/linuxdoom-1.10/m_swap.c")
endif()

if(WIN32)
	target_link_libraries(zyndoom PRIVATE ws2_32)
endif()

if(FRENCH)
	target_compile_definitions(zyndoom PRIVATE FRENCH)
endif()

if(NOT MSVC)
	target_link_libraries(zyndoom PRIVATE m)
endif()

# Handling linking/building WildMIDI
if(WILDMIDI)
	target_compile_definitions(zyndoom PRIVATE WILDMIDI)

	find_package(WildMidi)

	if(WildMidi_FOUND)
		target_link_libraries(zyndoom PRIVATE WildMidi::libwildmidi)
	else()
		# Set and hide these internal options
		set(WANT_ALSA OFF CACHE INTERNAL "" FORCE)
		set(WANT_DEVTEST OFF CACHE INTERNAL "" FORCE)
		set(WANT_OPENAL OFF CACHE INTERNAL "" FORCE)
		set(WANT_OSS OFF CACHE INTERNAL "" FORCE)
		set(WANT_OSX_DEPLOYMENT OFF CACHE INTERNAL "" FORCE)
		set(WANT_STATIC ON CACHE INTERNAL "" FORCE)

		add_subdirectory("external/wildmidi" EXCLUDE_FROM_ALL SYSTEM)
		target_compile_options(libwildmidi-static PRIVATE "-w")
		target_link_libraries(zyndoom PRIVATE libwildmidi-static)
	endif()
endif()

target_sources(zyndoom PRIVATE
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/ib_sound/sdl.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/ib_system/sdl.c"
	"${PROJECT_SOURCE_DIR}/linuxdoom-1.10/ib_video/sdl.c"
)

find_package(SDL2)

if(NOT SDL2_FOUND)
	add_subdirectory("external/SDL" EXCLUDE_FROM_ALL SYSTEM)
	if(SDL_STATIC)
		target_link_libraries(zyndoom PRIVATE SDL2::SDL2main SDL2::SDL2-static)
	else()
		target_link_libraries(zyndoom PRIVATE SDL2::SDL2main SDL2::SDL2)
	endif()
else()
	target_link_libraries(zyndoom PRIVATE SDL2::SDL2main SDL2::SDL2)
endif()

target_compile_definitions(zyndoom PRIVATE INCLUDE_SDL2_MAIN)
