#ifndef _WX_DOCKING_UTILS_H_
#define _WX_DOCKING_UTILS_H_

#include "wx/defs.h"

#if wxUSE_DOCKING

#include <wx/toolbar.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include <wx/docking/docking_defs.h>
#include <wx/bitmap.h>
#include <wx/colour.h>
#include <wx/vector.h>

class wxDockingEntity;
class wxDockingInfo;

namespace wxDockingUtils
{
	/**
	 * Returns true, if window is any child of parent.
	 */
	WXDLLIMPEXP_DOCKING bool IsParentOf(wxWindow *parent, wxWindow *window);

	/**
	 * Find the wxDockingFrame from the specified window.
	 */
	WXDLLIMPEXP_DOCKING wxDockingFrame *DockingFrameFromWindow(wxWindow *window);

	/**
	 * Find the parent we can dock to, which is either a splitter, notebook or a frame.
	 * If the provided window is already dockable, this is returned and child might be null.
	 * 
	 * dockingChild returns the child window which is directly connected to the dockingpanel.
	 * If window is the dockingPanel itself, then dockingChild is nullptr.
	 */
	WXDLLIMPEXP_DOCKING wxDockingEntity FindParentPanel(wxDockingEntity const &window, wxWindow **dockingChild = nullptr);

	/**
	 * Returns true if the window can be docked to. By default a window can be docked to
	 * if it is either a docking panel itself or a direct child of one of them.
	 * If the window is dockable and is a known type, then this will be reported if the
	 * pointer is not a nullptr.
	 */
	WXDLLIMPEXP_DOCKING bool IsDockable(wxDockingEntity const &window);

	WXDLLIMPEXP_DOCKING bool IsPanel(wxDockingEntity const &window);

	/**
	 * Calculate the distance of a point to a rectangle
	 */
	WXDLLIMPEXP_DOCKING double RectDistance(wxRect const &tabRect, wxPoint const &coordinate);

	WXDLLIMPEXP_DOCKING wxString TypeToString(wxDockingEntityType type);

	/**
	 * Returns the area of the tabs.
	 */
	WXDLLIMPEXP_DOCKING wxRect GetTabArea(wxNotebook *notebook, bool *multiWidth = nullptr, bool *multiHeight = nullptr);

	/**
	 * Calculate the free are next to the tabs
	 */
	WXDLLIMPEXP_DOCKING wxRect GetTabOpenArea(wxNotebook *notebook);

	/**
	 * Returns true if the child is a child of parent. This function will traverse the parent hierarchy, so
	 * if true is returned it doesn't neccessarily mean that the child is a direct child of the parent, but
	 * is A child somewhere in the hierarchy.
	 */
	WXDLLIMPEXP_DOCKING bool IsChildOf(wxWindow *parent, wxWindow *child);

	/**
	 * Validate if the target is allowed for docking
	 */
	WXDLLIMPEXP_DOCKING bool ValidateTarget(wxDockingInfo &src, wxDockingInfo &tgt);

	/**
	 * Calculate the distance depending on the direction. 'orientation' specifies the direction of the object
	 * i.E. Tab- or Toolbar orientation.
	 */
	WXDLLIMPEXP_DOCKING int32_t CalcDistance(wxDirection orientation, wxPoint const &left, wxPoint const &right);

	WXDLLIMPEXP_DOCKING void GrabScreenshot(wxDC &dc, wxRect const &coordinates, wxBitmap &screenshot);

	/**
	 * Blend source into target image. Alpha value must be 0 .. 1.
	 * Both bitmaps must match in size.
	 */
	WXDLLIMPEXP_DOCKING bool Blend(wxBitmap &src, wxBitmap &target, float alpha);

	/**
	 * Apply a filter color to the bitmap
	 */
	WXDLLIMPEXP_DOCKING bool ApplyFilter(wxColor &color, wxBitmap &target, float alpha);

	WXDLLIMPEXP_DOCKING void PaintRect(wxRect &rect, wxDockingEntity const &panel, wxColor rgb = wxColor(0, 200, 0));

	/**
	 *Return the border aligned rectangles for the tab from a notebook.
	 */
	WXDLLIMPEXP_DOCKING wxRect GetAlignedTabRect(wxNotebook *notebook, wxRect const &openRect, wxRect const &pageRect, size_t page);

	/**
	 * Check if the coordinates are on the sash. The coordinates are in the splitters local window coordinates.
	 */
	bool IsOnSash(wxSplitterWindow *splitter, wxPoint const &coordinates);

	/**
	 * Calculate the scaling factors to fit the desired size.
	 */
	inline double ScalingFactor(int length, int newLength)
	{
		// no division by zero !
		if (newLength > 0)
		{
			// calculate the scaling factor for the 2 dimensions
			return (double)newLength / (double)length;
		}

		return 1.0f;
	}

	/**
	 * Contains only unique values. Only recommended for small number of elements.
	 */
	template <typename T>
	class wxUniqueVector
		: public wxVector<T>
	{
	public:
		bool exists(T const &element) const
		{
			return std::find(begin(), end(), element) != end();
		}

		T &append(T const &element)
		{
			wxUniqueVector<T>::iterator pos = std::find(begin(), end(), element);

			if (pos != end())
				return *pos;

			emplace_back(element);

			return back();
		}

		bool remove(T const &element)
		{
			for (wxVector<T>::iterator it = begin(); it != end(); ++it)
			{
				T &p = *it;
				if (p == element)
				{
					erase(it);
					return true;
				}
			}

			return false;
		}
	};
}

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_UTILS_H_
