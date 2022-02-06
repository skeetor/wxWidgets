#ifndef _WX_DOCKING_BUTTON_OVERLAY_H_
#define _WX_DOCKING_BUTTON_OVERLAY_H_

#if wxUSE_DOCKING

#include <memory>

#include <wx/defs.h>
#include <wx/frame.h>
#include <wx/docking/docking_defs.h>
#include <wx/docking/dockinginfo.h>
#include <wx/docking/dockingevent.h>

class wxDockingButton;

class WXDLLIMPEXP_DOCKING wxIDockingButtonOverlay
{
public:
	virtual ~wxIDockingButtonOverlay() {}

	virtual void ProcessOverlay(wxDockingEvent &event) = 0;
	virtual void Refresh() = 0;
};

class WXDLLIMPEXP_DOCKING wxDockingButtonOverlay
	: public wxIDockingButtonOverlay
{
public:
	wxDockingButtonOverlay(wxWindow *parent);
	~wxDockingButtonOverlay() override;

	void ProcessOverlay(wxDockingEvent &event) override;
	void Refresh() override;

protected:
	virtual void Hide();
	virtual void Show();

	// Check if a GUI element was triggered, returns true if the event was updated..
	virtual bool ButtonHitUpdate(wxDockingEvent &event);

	// Returns true if our own button was hit.
	virtual bool IsButtonHit(wxDockingEvent const &event);

	wxDockingButton const *GetTopPanelButton() const { return m_topPanel.get(); }
	wxDockingButton *GetTopPanelButton() { return m_topPanel.get(); }
	wxWindow *GetParent() const { return m_parent; }

private:
	void DoLayout(wxDockingEntity const &target);

private:
	wxWindow *m_parent;
	std::unique_ptr<wxDockingButton> m_topPanel;
};

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_BUTTON_OVERLAY_H_
