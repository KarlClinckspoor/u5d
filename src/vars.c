#include "common/common.h"

#include "tiles.h"

// This object holds nothing: it is the one file of the pre-loaded section (DGROUP, DATA.OVL;
// src/u5d.lnk). Every initialised variable is defined by the module that owns it, every
// communal (uninitialised) one by ultima.c or the overlay module that uses it (vars.h, DS:a9bc).
#include "vars.h"

#include "macros.h"

// 00 00

// 00 00

// textWindows: filled with 0

//u16 D_5394 = 0x1232;

// NOTE: [55a6..6606): filled with 0
// NOTE: [6608..6a08): filled with 0

