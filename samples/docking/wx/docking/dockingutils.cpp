
#if wxUSE_DOCKING

#include <wx/object.h>
#include <wx/toolbar.h>

#include <wx/docking/dockingframe.h>
#include <wx/docking/dockingutils.h>
#include <wx/docking/dockinginfo.h>
#include <wx/docking/dockingstate.h>
#include <wx/dcmemory.h>
#include <wx/rawbmp.h>
#include <wx/vector.h>
#include <wx/dcscreen.h>

#ifndef wxUSE_NOTEBOOK
#error wxDockingFrame requires wxNotebook enabled
#endif

#ifndef wxUSE_SPLITTER
#error wxDockingFrame requires wxSplitterWindow enabled
#endif

#define TYPESTRING(x) case (x): return wxSTRINGIZE(x)

namespace wxDockingUtils
{
	wxString TypeToString(wxDockingEntityType type)
	{
		wxString s;

		switch (type)
		{
			TYPESTRING(wxDOCKING_NONE);
			TYPESTRING(wxDOCKING_WINDOW);
			TYPESTRING(wxDOCKING_SPLITTER);
			TYPESTRING(wxDOCKING_NOTEBOOK);
			TYPESTRING(wxDOCKING_TOOLBAR);
			TYPESTRING(wxDOCKING_FRAME);
			TYPESTRING(wxDOCKING_PLACEHOLDER);
		}

		return s << type;
	}

	wxDockingFrame *DockingFrameFromWindow(wxWindow *w)
	{
		while (w)
		{
			wxDockingFrame *frame = wxDynamicCast(w, wxDockingFrame);
			if (frame)
				return frame;

			w = w->GetParent();
		}

		return nullptr;
	}

	wxDockingEntity FindParentPanel(wxDockingEntity const &window, wxWindow **dockingChild)
	{
		if (dockingChild)
			*dockingChild = nullptr;

		wxDockingEntity w = window;
		wxWindow *child = nullptr;
		wxDockingState const &ds = wxDockingState::GetInstance();

		while (w)
		{
			if (!ds.IsKnownPanel(w))
			{
				child = w;
				w = w->GetParent();
				continue;
			}

			wxDockingEntityType pt = w.GetType();
			if (pt == wxDOCKING_SPLITTER || pt == wxDOCKING_NOTEBOOK || pt == wxDOCKING_FRAME)
			{
				if (pt == wxDOCKING_NOTEBOOK)
				{
					wxNotebook *nb = w.GetNotebook();
					child = nb->GetCurrentPage();
				}

				// child will be null if the provided window was already a splitter or a frame.
				if (dockingChild)
					*dockingChild = child;

				return w;
			}

			child = w;
			w = window->GetParent();
		}

		return static_cast<wxWindow *>(nullptr);
	}

	bool IsPanel(wxDockingEntity const &window)
	{
		if (!window)
			return false;

		wxDockingEntityType pt = window.GetType();
		switch (pt)
		{
			case wxDOCKING_SPLITTER:
			case wxDOCKING_NOTEBOOK:
				return true;
		}

		return false;
	}

	bool IsDockable(wxDockingEntity const &window)
	{
		if (IsPanel(window))
			return true;

		if (!window)
			return false;

		// If the window itself was not a dockable, we check if the parent is a dockable.
		// we can dock to any window which is a docking panel itself, or a direct
		// descendant of it.
		return IsPanel(window->GetParent());
	}

	bool IsParentOf(wxWindow *parent, wxWindow *window)
	{
		if (!parent || !window)
			return false;

		while (window)
		{
			if (window == parent)
				return true;

			window = window->GetParent();
		}

		return false;
	}

	wxRect GetTabArea(wxNotebook *notebook, bool *multiWidth, bool *multiHeight)
	{
		size_t tabCnt = notebook->GetPageCount();
		if (!tabCnt)
			return wxRect();

		// If we have stacked tabs, they have different x (or y) coordinates, depending on the orientation.
		// Otherweise they are on the same row/column, like in TOP/BOTTOM on Windows. With left/right it
		// depends how much room there is.
		if (multiWidth)
			*multiWidth = false;

		if (multiHeight)
			*multiHeight = false;

		wxRect tabRect(INT_MAX, INT_MAX, INT_MIN, INT_MIN);
		for (int i = 0; i < tabCnt; i++)
		{
			wxRect curRect = notebook->GetTabRect(i);

			if (curRect.x < tabRect.x)
			{
				if (multiWidth && tabRect.x != INT_MAX)
					*multiWidth = true;
				tabRect.x = curRect.x;
			}

			if (curRect.y < tabRect.y)
			{
				if (multiHeight && tabRect.y != INT_MAX)
					*multiHeight = true;

				tabRect.y = curRect.y;
			}

			int v = curRect.x + curRect.width;
			if (v > tabRect.width)
				tabRect.width = v;

			v = curRect.y + curRect.height;
			if (v > tabRect.height)
				tabRect.height = v;
		}

		return tabRect;
	}

	wxRect GetTabOpenArea(wxNotebook *notebook)
	{
		wxRect openRect = wxRect();

		size_t tabCnt = notebook->GetPageCount();
		if (!tabCnt)
			return openRect;

		// The tabs may be on a single line, but they don't strictly have to. Depending on the system they can
		// be stacked if they exceed the limit of visible tabs.
		// In such a case, we want to report the whole area, but we can't rely on the last tab being the
		// rightmost as it may not be aligned, so we have to find out the tab area by looping through all tabs.
		wxRect tabRect = GetTabArea(notebook);
		wxRect pageRect = notebook->GetPage(0)->GetRect();
		wxDirection tabDir = notebook->GetTabOrientation();

		// tabRect contains now coordinates instead of width/height, so they have to be adjusted.
		openRect = notebook->GetRect();
		switch (tabDir)
		{
			case wxTOP:
			{
				openRect.height = pageRect.y;
				openRect.width = openRect.width - tabRect.width;
				openRect.x = tabRect.width;
			}
			break;

			case wxBOTTOM:
			{
				openRect.y = pageRect.y + pageRect.height;
				openRect.height -= openRect.y;
				openRect.width = openRect.width - tabRect.width;
				openRect.x = tabRect.width;
			}
			break;

			case wxLEFT:
			{
				openRect.width = pageRect.x;
				int v = tabRect.y + tabRect.height;
				if (v <= openRect.height)
				{
					openRect.height -= v;
					openRect.y = v;
					if (openRect.height < 0)
					{
						openRect.height = 0;
						openRect.y = v;
					}
				}
			}
			break;

			case wxRIGHT:
			{
				wxRect notebookRect = notebook->GetRect();
				int v;

				openRect.x = (pageRect.x + pageRect.width);
				openRect.width = (notebookRect.x + notebookRect.width) - openRect.x;
				v = tabRect.y + tabRect.height;
				openRect.y = v;
				openRect.height = (notebookRect.y + notebookRect.height) - v;
			}
			break;
		}

		return openRect;
	}

	double RectDistance(wxRect const &tabRect, wxPoint const &coordinate)
	{
		double cx = std::max(std::min(coordinate.x, tabRect.x + tabRect.width), tabRect.x);
		double cy = std::max(std::min(coordinate.y, tabRect.y + tabRect.height), tabRect.y);
		return std::sqrt(((double)coordinate.x - cx) * ((double)coordinate.x - cx) + ((double)coordinate.y - cy) * ((double)coordinate.y - cy));
	}

	bool ValidateNotebookTarget(wxDockingInfo src, wxDockingInfo tgt, bool *isValid)
	{
		wxDockingEntity const &sw = src.GetDockingEntity();

		*isValid = false;

		if (sw.GetType() == wxDOCKING_WINDOW)
			src.UpdateToParent();

		wxDockingEntity const &tw = tgt.GetDockingEntity();
		if (tw.GetType() == wxDOCKING_WINDOW)
			tgt.UpdateToParent();

		// If both windows are not notebooks we are done.
		if (sw.GetType() != wxDOCKING_NOTEBOOK && tw.GetType() != wxDOCKING_NOTEBOOK)
			return false;

		// Both panels must be known docking entities. If one is not, it may be a notebook i.E. inside some
		// window which is not intended for docking (like an option dialog or such).
		wxDockingState const &gs = wxDockingState::GetInstance();
		if (!gs.IsKnownPanel(sw) || !gs.IsKnownPanel(tw))
			return false;

		bool isChild = IsChildOf(sw, tw);

		// If the notebooks are different, they may still be nested inside each other.
		if (sw.GetRawWindow() != tw.GetRawWindow())
		{
			// If the target notebook is not nested inside our source notebook, we are save.
			if (!isChild)
			{
				*isValid = true;
				return true;
			}

			// If the notebook is a nested child, we are save if we just move a single tab.
			if (src.IsOnTab())
			{
				*isValid = true;
				return true;
			}

			// We can not move the whole notebook into a nested notebook.
			*isValid = false;
			return true;
		}

		// We are moving within our notebook

		// We can not move the whole notebook into ourself.
		if (!src.IsOnTab() || src.GetPage() == wxNOT_FOUND)
		{
			*isValid = false;
			return true;
		}

		// It has to be a differen tab we want to move into.
		if (src.GetPage() == tgt.GetPage())
		{
			*isValid = false;
			return true;
		}

		// Check if we move the last page into the last page.
		wxNotebook *sn = sw.GetNotebook();
		if (src.GetPage() == sn->GetPageCount() && tgt.GetPage() == wxNOT_FOUND)
		{
			*isValid = false;
			return true;
		}

		*isValid = true;
		return true;
	}

	bool ValidateTarget(wxDockingInfo const &src, wxDockingInfo const &tgt)
	{
		wxDockingEntity const tw = tgt.GetDockingEntity();
		if (!tw)
			return false;

		wxDockingEntity const sw = src.GetDockingEntity();
		if (!sw)
			return false;

		bool isValid = false;
		if (ValidateNotebookTarget(src, tgt, &isValid))
			return isValid;
		else if (tw.GetType() == wxDOCKING_SPLITTER)
		{
			// We have to prevent the case where the window is moved into it's own splitter
			wxDockingEntity const &sp = src.GetWindow();
			if (sp == tw)
				return false;
		}

		// Check if the source is a parent of the target, which we want to avoid.
		// i.E. The user grabs a splitter and tries to move it into a page of itself. Bad idea!:)
		return !IsChildOf(sw, tw);
	}

	bool IsChildOf(wxWindow *parent, wxWindow *child)
	{
		if (!child)
			return false;

		if (child == parent)
			return true;

		wxWindow *w = parent;
		while (w)
		{
			w = child->GetParent();
			if (w == parent)
				return true;

			child = w;
		}

		return false;
	}

	int32_t CalcDistance(wxDirection dir, wxPoint const &left, wxPoint const &right)
	{
		switch (dir)
		{
			case wxLEFT:
			case wxRIGHT:
				return left.y - right.y;

			case wxTOP:
			case wxBOTTOM:
				return left.x - right.x;
		}

		return 0;
	}

	void GrabScreenshot(wxDC &dc, wxRect const &rect, wxBitmap &screenshot)
	{
		screenshot.Create(rect.width, rect.height);
		wxMemoryDC memDC;
		memDC.SelectObject(screenshot);

		memDC.Blit(0, 0, rect.width,rect.height, &dc, rect.x, rect.y);
		memDC.SelectObject(wxNullBitmap);
	}

	bool Blend(wxBitmap &src, wxBitmap &target, float alpha)
	{
		// https://stackoverflow.com/questions/746899/how-to-calculate-an-rgb-colour-by-specifying-an-alpha-blending-amount

		wxSize sz = src.GetSize();
		wxSize tsz = target.GetSize();

		if (sz != tsz)
			return false;

		wxNativePixelData srcData(src, wxPoint(0, 0), sz);
		wxNativePixelData::Iterator srcPtr(srcData);

		wxNativePixelData tgtData(target, wxPoint(0, 0), sz);
		wxNativePixelData::Iterator tgtPtr(tgtData);

		for (int y = 0; y < sz.y; ++y)
		{
			wxNativePixelData::Iterator srcStart = srcPtr;
			wxNativePixelData::Iterator tgtStart = tgtPtr;

			for (int x = 0; x < sz.x; ++x)
			{
				unsigned int r = tgtPtr.Red();
				unsigned int g = tgtPtr.Green();
				unsigned int b = tgtPtr.Blue();

				r = alpha * srcPtr.Red() + (1 - alpha) * r;
				g = alpha * srcPtr.Green() + (1 - alpha) * g;
				b = alpha * srcPtr.Blue() + (1 - alpha) * b;

				tgtPtr.Red() = r;
				tgtPtr.Green() = g;
				tgtPtr.Blue() = b;

				++srcPtr;
				++tgtPtr;
			}

			srcPtr = srcStart;
			tgtPtr = tgtStart;

			srcPtr.OffsetY(srcData, 1);
			tgtPtr.OffsetY(tgtData, 1);
		}

		return true;
	}

	bool ApplyFilter(wxColor &color, wxBitmap &target, float alpha)
	{
		wxSize sz = target.GetSize();

		wxNativePixelData tgtData(target, wxPoint(0, 0), sz);
		wxNativePixelData::Iterator tgtPtr(tgtData);

		unsigned int cr = color.Red();
		unsigned int cg = color.Green();
		unsigned int cb = color.Blue();

		for (int y = 0; y < sz.y; ++y)
		{
			wxNativePixelData::Iterator tgtStart = tgtPtr;

			for (int x = 0; x < sz.x; ++x)
			{
				unsigned int r = tgtPtr.Red();
				unsigned int g = tgtPtr.Green();
				unsigned int b = tgtPtr.Blue();

				r = alpha * cr + (1 - alpha) * r;
				g = alpha * cg + (1 - alpha) * g;
				b = alpha * cb + (1 - alpha) * b;

				tgtPtr.Red() = r;
				tgtPtr.Green() = g;
				tgtPtr.Blue() = b;

				++tgtPtr;
			}

			tgtPtr = tgtStart;

			tgtPtr.OffsetY(tgtData, 1);
		}

		return true;
	}

	void PaintRect(wxRect &rectangle, wxDockingEntity const &panel, wxColor rgb)
	{
		wxScreenDC dc;
		wxBrush brush(rgb);

		wxRect rect = rectangle;
		panel->ClientToScreen(&rect.x, &rect.y);

		dc.SetBrush(brush);
		dc.DrawRectangle(rect);

		// Draw a border around the highlight
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen(*wxBLACK_PEN);
		dc.DrawRectangle(rect);
	}

	bool IsOnSash(wxSplitterWindow *splitter, wxPoint const &coordinates)
	{
		wxSplitMode md = splitter->GetSplitMode();
		wxRect sashArea;

		wxWindow *w = splitter->GetWindow1();
		if (md == wxSPLIT_HORIZONTAL)
		{
			wxSize sz = splitter->GetSize();
			sashArea.width = sz.GetWidth();

			wxPoint pt1 = w->GetPosition();
			wxPoint pt2 = splitter->GetWindow2()->GetPosition();
			sashArea.height = pt2.y - pt1.y;

			sz = w->GetSize();
			sashArea.x = 0;
			sashArea.y = sz.GetHeight();
		}
		else
		{
			wxSize sz = splitter->GetSize();
			sashArea.height = sz.GetHeight();

			wxPoint pt1 = w->GetPosition();
			wxPoint pt2 = splitter->GetWindow2()->GetPosition();
			sashArea.width = pt2.x - pt1.x;

			sz = w->GetSize();
			sashArea.y = 0;
			sashArea.x = sz.GetWidth();
		}

		if (!sashArea.Contains(coordinates))
			return false;

		return true;
	}

	wxRect GetAlignedTabRect(wxNotebook *notebook, wxRect const &openRect, wxRect const &pageRect, size_t page)
	{
		wxRect tabRect = notebook->GetTabRect(page);

		// If the coordinate is smaller than the height, it means there is no room for
		// an additional tab and the tab is on the border, so we have to align it.
		// The same applies for the other directions.
		switch (notebook->GetTabOrientation())
		{
			case wxTOP:
			{
				if (tabRect.y < tabRect.height)
					tabRect.y = 0;

				if (tabRect.x < tabRect.width)
					tabRect.x = 0;

				int v = pageRect.y - (tabRect.x + tabRect.height);
				if (v < tabRect.height)
					tabRect.height += v;
			}
			break;

			case wxBOTTOM:
			{
				if (tabRect.x < tabRect.width)
					tabRect.x = 0;

				int v = (pageRect.y + pageRect.height) - tabRect.y;
				if (v < tabRect.height)
				{
					tabRect.y -= v;
					tabRect.height += v;
				}
			}
			break;

			case wxLEFT:
			{
				if (tabRect.y < tabRect.height)
					tabRect.y = 0;

				if (tabRect.x < tabRect.width)
					tabRect.x = 0;

				int v = pageRect.x - (tabRect.x + tabRect.width);
				if (v < tabRect.width)
					tabRect.width += v;

				v = openRect.y - (tabRect.y + tabRect.height);
				if (v < tabRect.height)
					tabRect.height += v;
			}
			break;

			case wxRIGHT:
			{
				if (tabRect.y < tabRect.height)
					tabRect.y = 0;

				int v = tabRect.x - (pageRect.x + pageRect.width);
				if (v < tabRect.width)
				{
					tabRect.x -= v;
					tabRect.width += v;
				}

				wxRect notebookRect = notebook->GetRect();
				v = (tabRect.x + tabRect.width) - (notebookRect.x - notebookRect.width);
				if (v < tabRect.width)
					tabRect.width += v;
			}
			break;
		}

		return tabRect;
	}
}

#endif // wxUSE_DOCKING
