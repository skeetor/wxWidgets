#ifndef _WX_DOCKING_OVERLAY_H_
#define _WX_DOCKING_OVERLAY_H_

#if wxUSE_DOCKING

#include <memory>

#include <wx/docking/dockingevent.h>

class WXDLLIMPEXP_DOCKING wxIDockingOverlay
{
public:
	virtual ~wxIDockingOverlay() {}

	virtual void ProcessOverlay(wxDockingEvent &event) = 0;
	virtual void Refresh() = 0;
};

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_OVERLAY_H_
