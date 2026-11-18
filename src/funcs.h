#ifndef _FUNCS_H
#define _FUNCS_H

// 017e..0877: the runtime library (SLIBCR.LIB, LIBH.LIB; src/u5d.lnk). tools/match.py and
// tools/rootmap.py read the names of its routines from these lines.

// 017e: _astart (crt0)
// 0230: _cinit
// 02f4: exit
// 037d: (dynamic near call)
// 038c: (dynamic far call)
// 03a0: aNuldiv (32bit division)
// 0402: memchr
// 0426: itoa
// 0442: aNlmul (32bit multiply)
// aNulmul == aNlmul?
// 0476: aNNalshl (32bit shl)
// 0496: aNldiv (32bit division)
// 0532: _NMSG_TEXT
// 055d: _NMSG_WRITE
// 0586: _FF_MSGBANNER
// 05a6: _setargv
// 0734: _setenvp
// 07a2: _nullcheck
// 07c4: _fptrap
// 07d6: _cxtoa
// 082a: aNlshl
// 0836: _myalloc

#include "0000.h"
#include "1000.h"
#include "2000.h"
#include "3000.h"
#include "4000.h"
#include "5000.h"
#include "6000.h"
#include "7000.h"

#endif
