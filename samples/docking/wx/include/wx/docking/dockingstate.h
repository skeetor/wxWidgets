#ifndef _WX_DOCKING_STATE_H_
#define _WX_DOCKING_STATE_H_

#if wxUSE_DOCKING

#include <wx/docking/docking_defs.h>
#include <wx/docking/dockingevent.h>
#include <wx/docking/dockingentity.h>
#include <wx/docking/dockingutils.h>
#include <wx/docking/dockingoverlay.h>

typedef wxDockingUtils::wxUniqueVector<wxDockingEntityState> wxDockingEntityStates;
typedef wxDockingUtils::wxUniqueVector<wxDockingFrame *> wxDockingFrames;

class WXDLLIMPEXP_DOCKING wxDockingState
{
public:
	wxDockingState();
	static wxDockingState &GetInstance();

	/**
	 * IsDragging is set to true, when the mouse button has been pressed and the mouse has moved in the meantime. This
	 * seems to be a problem with the MSW build, falsely reporting a mouse drag in some cases, where the mouse is not dragged.
	 * It seems that this is coming from Windows though, so I don't know if this can be fixed within wxWidgets (there is even
	 * a comment about this in the mouse event).
	 * So here we detect the dragging on our own, in order to avoid this problem.
	 */
	bool IsDragging(wxPoint mousePos = ::wxGetMousePosition())
	{
		// When the mousebutton is pressed checkDragging is enabled. Now we keep checking the mouse position until the mouse is moved.
		// Once the dragging is detected, we disable the check, so we don't falsely report the dragging in case the user moves the
		// mouse back to it's original position. Only when the mouse button is released, the check can be enabled again.
		if (m_checkDragging)
		{
			if (draggingPos.x != mousePos.x || draggingPos.y != mousePos.y)
			{
				m_isDragging = true;
				m_checkDragging = false;
			}
		}

		return m_isDragging;
	}

	void EnableDragging()
	{
		m_checkDragging = true;
		m_isDragging = false;
	}

	void ResetDragging()
	{
		// Dragging will no longer be reported after this, until the mousebutton is pressed again.
		m_checkDragging = false;
		m_isDragging = false;
	}

	/**
	 * Returns the panel state for the given panel. If the state was not found, a dummy object is returned
	 * which can be safely used, but will not keep it's state.
	 * 'found' is set to true or false if it is provided.
	 * If force is true, then panel will be added to the list, if it is not yet known. 'found' will still be set to
	 * false in this case, as the panel was not known before.
	 * If force is true and the panel is already known, then nothing happens, as each panelstate is only added once
	 */
	wxDockingEntityState &PanelState(wxDockingEntity const &panel, bool force = true, bool *found = nullptr);
	wxDockingEntityState const &FindPanelState(wxDockingEntity const &panel, bool *found = nullptr) const;

	/**
	 * Returns true if the panel is a known docking target.
	 */
	bool IsKnownPanel(wxDockingEntity const &panel) const;

	/**
	 * Convenience function to check if a panel is currently locked.
	 */
	bool IsLocked(wxDockingEntity const &panel) const;
	void SetLock(wxDockingEntity const &panel, bool locked = true);

	/**
	 * Convenience function to check if a panel can be dragged.
	 */
	bool IsDraggable(wxDockingEntity const &panel) const;
	void SetDraggable(wxDockingEntity const &panel, bool dragging = true);

	bool IsMouseCaptured() const { return m_mouseCaptured; }
	void SetMouseCaptured(bool flag) { m_mouseCaptured = flag; }

	bool IgnoreDocking() const { return m_ignoreDocking; }
	void SetIgnoreDocking(bool flag) { m_ignoreDocking = flag; }

	bool WaitMouseBtnUp() const { return m_waitMouseBtnUp; }
	void SetWaitMouseBtnUp(bool flag) { m_waitMouseBtnUp = flag; }

	/**
	 * This is set, if an overlay handler was installed. This is even then the case, when the
	 * client called it with a null pointer, thus disabling overlays, which is a valid szenario.
	 */
	bool HasOverlayHandler() const { return m_hasOverlayHandler; }
	void SetOverlayHandler(wxIDockingOverlay *overlayHandler, bool hasHandler = true) { m_overlayHandler.reset(overlayHandler); m_hasOverlayHandler = hasHandler; }
	wxIDockingOverlay *GetOverlayHandler() const {  return HasOverlayHandler() ? m_overlayHandler.get() : nullptr; }
	wxIDockingOverlay *GetOverlayHandler() { return m_overlayHandler.get(); }

	// Since this class represents only an internal state, we don't provide getters and setters (for now)
public:
	wxDockingEvent event;					// Record the current state of the docking parameters
	wxDockingFrames frames;
	wxDockingEntityStates panels;			// Panels which have to be tracked
	wxPoint draggingPos;					// Position where we want to check the dragging against.

private:
	std::unique_ptr<wxIDockingOverlay> m_overlayHandler;
	bool m_mouseCaptured:1;
	bool m_ignoreDocking:1;					// If the start event is vetoed we no longer track this until the mouse is released.
	bool m_waitMouseBtnUp:1;
	bool m_checkDragging:1;					// When the mousebutton is pressed we have to check if a dragging is starting
	bool m_isDragging:1;					// Dragging has been activated. This will only be set once per dragging operation
	bool m_hasOverlayHandler : 1;			// Set if an overlayhandler is available. This is needed because a nullptr may
											// also be used as a handler which disables it.

private:
	wxDECLARE_NO_COPY_CLASS(wxDockingState);
};

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_GLOBALSTATE_H_
