#ifndef _CUSTOM_BUTTON_OVERLAY_H_
#define _CUSTOM_BUTTON_OVERLAY_H_

#include <memory>

#include <wx/docking/docking.h>
#include <wx/docking/buttonoverlay.h>
#include <wx/docking/dockingbutton.h>

class WXDLLIMPEXP_DOCKING CustomOverlay
	: public wxDockingButtonOverlay
{
public:
	CustomOverlay(wxWindow *parent);
	~CustomOverlay() override;

	void ProcessOverlay(wxDockingEvent &event) override;
	void Refresh() override;

protected:
	void Init();

	void DoLayout(wxDockingEntity const &target, wxDockingEntity const &frame);
	bool IsButtonHit(wxDockingEvent const &event) override;
	bool ButtonHitUpdate(wxDockingEvent &event) override;
	void Hide() override;
	void Show() override;

	void UpdateEvent(wxDockingEvent &event, wxDirection direction, bool toFrame);

	void CalcSize();

private:
	wxWindow *m_parent;

	typedef enum
	{
		// Top to bottom
		IDX_FRAME_TOP = 0,
		IDX_PANEL_TOP,			// From baseclass
		IDX_FLOATING,
		IDX_CENTER,
		IDX_TAB,
		IDX_PANEL_BOTTOM,
		IDX_FRAME_BOTTOM,

		// Left to right
		IDX_FRAME_LEFT = 0,
		IDX_PANEL_LEFT,
		IDX_PANEL_RIGHT,
		IDX_FRAME_RIGHT

	} ButtonIndex;

	// Total rectangle size of the button cross
	wxSize m_buttonRect;

	std::vector<std::unique_ptr<wxDockingButton>> m_buttons;
	std::vector<wxDockingButton *> m_topToBottom;
	std::vector<wxDockingButton *> m_leftToRight;

private:
	typedef wxDockingButtonOverlay super;
};

#endif // _CUSTOM_BUTTON_OVERLAY_H_
