
#if wxUSE_DOCKING

#include <wx/docking/dockingstate.h>

static wxDockingEntityState gDummyPanelState;
static wxDockingState gGlobals;

wxDockingState &wxDockingState::GetInstance()
{
	return gGlobals;
}

wxDockingState::wxDockingState()
	: draggingPos()
	, m_mouseCaptured(false)
	, m_ignoreDocking(false)
	, m_waitMouseBtnUp(false)
	, m_checkDragging(false)
	, m_isDragging(false)
	, m_hasOverlayHandler(false)
{
}

void wxDockingState::SetLock(wxDockingEntity const &panel, bool locked)
{
	wxDockingEntityStates::iterator pos = std::find(panels.begin(), panels.end(), panel);

	if (pos == panels.end())
	{
		// This is the default, so no need to add it.
		if (!locked)
			return;

		panels.append(panel);
		pos = --panels.end();
	}

	wxDockingEntityState &ps = *pos;
	ps.SetLocked(locked);
}

bool wxDockingState::IsLocked(wxDockingEntity const &panel) const
{
	wxDockingEntityStates::const_iterator pos = std::find(panels.begin(), panels.end(), panel);

	if (pos == panels.end())
		return false;

	wxDockingEntityState const &ps = *pos;

	return ps.IsLocked();
}

void wxDockingState::SetDraggable(wxDockingEntity const &panel, bool dragging)
{
	wxDockingEntityStates::iterator pos = std::find(panels.begin(), panels.end(), panel);

	if (pos == panels.end())
	{
		// This is the default, so no need to add it.
		if (dragging)
			return;

		panels.append(panel);
		pos = --panels.end();
	}

	wxDockingEntityState &ps = *pos;
	ps.SetDraggable(dragging);
}

bool wxDockingState::IsDraggable(wxDockingEntity const &panel) const
{
	wxDockingEntityStates::const_iterator pos = std::find(panels.begin(), panels.end(), panel);

	if (pos == panels.end())
		return false;

	wxDockingEntityState const &ps = *pos;

	return ps.IsDraggable();
}

bool wxDockingState::IsKnownPanel(wxDockingEntity const &panel) const
{
	bool isKnown = false;
	FindPanelState(panel, &isKnown);
	return isKnown;
}

wxDockingEntityState &wxDockingState::PanelState(wxDockingEntity const &panel, bool force, bool *found)
{
	wxDockingEntityStates::iterator pos = std::find(panels.begin(), panels.end(), panel);

	if (pos == panels.end())
	{
		if (found)
			*found = false;

		if (force)
		{
			panels.append(panel);
			return panels.back();
		}

		// Make sure this is always empty.
		gDummyPanelState = wxDockingEntityState();
		return gDummyPanelState;
	}

	if (found)
		*found = true;

	return *pos;
}

wxDockingEntityState const &wxDockingState::FindPanelState(wxDockingEntity const &panel, bool *found) const
{
	wxDockingEntityStates::const_iterator pos = std::find(panels.begin(), panels.end(), panel);

	if (pos == panels.end())
	{
		if (found)
			*found = false;

		return gDummyPanelState;
	}

	if (found)
		*found = true;

	return *pos;
}

#endif // wxUSE_DOCKING
