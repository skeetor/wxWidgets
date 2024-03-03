#ifndef _WX_DOCKING_DEFS_H_
#define _WX_DOCKING_DEFS_H_

#include <wx/defs.h>
#include <wx/cpp.h>
#include <wx/dlimpexp.h>

// TODO: Should be moved to dlimpexp.h
#ifdef WXMAKINGDLL_DOCKING
#    define WXDLLIMPEXP_DOCKING WXEXPORT
#elif defined(WXUSINGDLL)
#    define WXDLLIMPEXP_DOCKING WXIMPORT
#else /* not making nor using DLL */
#    define WXDLLIMPEXP_DOCKING
#endif

// Usage: #pragma WARNING(message)
#define WARNING(txt) message(__FILE__ "(" wxSTRINGIZE(__LINE__)"): user warning: " txt)

// Width/height of how many pixels the window can be grabbed from the border to trigger the docking
#define wxDOCKING_TRIGGER_WIDTH		20
#define wxDOCKING_TRIGGER_HEIGHT	20
// Widht of the indicator when targeting a tab control
#define wxDOCKING_TAB_WIDTH			10
#define wxDOCKING_TAB_HEIGHT			10

class wxWindow;
class wxDockingFrame;
class wxSplitterWindow;
class wxNotebook;

// TODO: Can be removed if this is added to wxWidgets.
#ifndef wxCENTRAL
#define wxCENTRAL   ((wxDirection)0u)
#endif

#ifndef wxFLOATING
#define wxFLOATING   ((wxDirection)0x0100)
#endif

#ifndef wxNONE
#define wxNONE   ((wxOrientation)0u)
#endif

typedef enum
{
	wxDOCKING_NONE,
	wxDOCKING_WINDOW,
	wxDOCKING_SPLITTER,
	wxDOCKING_NOTEBOOK,
	wxDOCKING_TOOLBAR,
	wxDOCKING_FRAME,
	wxDOCKING_PLACEHOLDER
} wxDockingEntityType;

#define wxDOCKING_HINT_ALLOW_RED 0
#define wxDOCKING_HINT_ALLOW_GREEN 200
#define wxDOCKING_HINT_ALLOW_BLUE 0

#define wxDOCKING_HINT_FLOATING_RED 0
#define wxDOCKING_HINT_FLOATING_GREEN 130
#define wxDOCKING_HINT_FLOATING_BLUE 220

#define wxDOCKING_HINT_DISALLOW_RED 255
#define wxDOCKING_HINT_DISALLOW_GREEN 0
#define wxDOCKING_HINT_DISALLOW_BLUE 0

#define wxDOCKING_HINT_OVERLAY_ALPHA 0.4f

#endif // _WX_DOCKING_DEFS_H_
