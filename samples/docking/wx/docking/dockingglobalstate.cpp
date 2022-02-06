
#if wxUSE_DOCKING

#include <wx/docking/dockingglobalstate.h>

static wxDockingEntityState gDummyPanelState;
static wxDockingGlobalState gGlobals;

wxDockingGlobalState &wxDockingGlobalState::GetInstance()
{
	return gGlobals;
}

wxDockingGlobalState::wxDockingGlobalState()
	: draggingPos()
	, mouseCaptured(false)
	, ignoreDocking(false)
	, waitMouseBtnUp(false)
	, m_checkDragging(false)
	, m_isDragging(false)
{
}

void wxDockingGlobalState::SetLock(wxDockingEntity const &panel, bool locked)
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

bool wxDockingGlobalState::IsLocked(wxDockingEntity const &panel) const
{
	wxDockingEntityStates::const_iterator pos = std::find(panels.begin(), panels.end(), panel);

	if (pos == panels.end())
		return false;

	wxDockingEntityState const &ps = *pos;

	return ps.IsLocked();
}

void wxDockingGlobalState::SetDraggable(wxDockingEntity const &panel, bool dragging)
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

bool wxDockingGlobalState::IsDraggable(wxDockingEntity const &panel) const
{
	wxDockingEntityStates::const_iterator pos = std::find(panels.begin(), panels.end(), panel);

	if (pos == panels.end())
		return false;

	wxDockingEntityState const &ps = *pos;

	return ps.IsDraggable();
}

bool wxDockingGlobalState::IsKnownPanel(wxDockingEntity const &panel) const
{
	bool isKnown = false;
	PanelState(panel, &isKnown);
	return isKnown;
}

wxDockingEntityState &wxDockingGlobalState::PanelState(wxDockingEntity const &panel, bool force, bool *found)
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

wxDockingEntityState const &wxDockingGlobalState::PanelState(wxDockingEntity const &panel, bool *found) const
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
