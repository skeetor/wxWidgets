# wxDocking extension for wxWidgets<br>
## Written by Gerhard W. Gruber &lt;sparhawk@gmx.at&gt; in 2021


## Design

The wxDocking components are an extension for the wxWidgets library, to add support for designing a modern look and feel. The goal was, to have a full set of API functions which allows the user a maximum of freedom while at the same time minimizes the need for new components or requires the user to use replacements for already established components.<br>
wxDocking was implemented by making use of existing components, so that it should work on most platforms supported by wxWidgets without special, platform dependent code.<br>
Important Note: wxDocking is not compatible with wxAUI and is completely independent for it, to adress problems I had while using wxAUI. So if you want to use it with an existing project, you need to convert it.<br>
<br>
wxDocking can be used as is. However, the events and the API are designed in such a way that you can customize all aspsects of the behavior or even replace it, without the need of modifying the wxDocking code itself. If you can't do that, it may be a bug and should be reported to [wxDocking](https://github.com/skeetor/wxDocking/issues) or  &lt;sparhawk@gmx.at&gt;<br>
IMPORTANT NOTE: wxDocking is not yet a part of wxWidgets though I try to integrate it. Asking questions, report bugs or request additional features should be directed to [wxDocking](https://github.com/skeetor/wxDocking) as wxWidgets can't help there. Of course you can always ask them to integrate it, as this would also be my intention. :)
<br>

## How to build

When building the main library, follow the instructions here [wxWidgets build](../../README.md)<br>

When cloning the source from git, make sure that you use `--recurse-submodules` as described in here [Git-specific notes](../../README-GIT.md)<br>
Building the 

```build\msw\```<br>
and choose one of the solutions appropriate for your VS version i.E. ```wx_vc9.sln```.<br>
First build the whole main library. I had to repeat this a few times, because it happend that not all projects could be built sucesfully on the first run, but this is a problem of wxWidgets itself and not related to this project.<br>
<br>
After that navigate to the ```samples/docking``` and build it.<br>

### TODO/Enhancements

* When moving an item, we have to check if it is a known source/target. Source has to be already checked when the docking is initiated.<br>
* Remove panel and frame from the event structures.<br>
* Custom buttons dont appear when dragging starts in a floating frame (or other frame then frame[0])<br>
* Placement of overlay buttons on frame (and also maximized)<br>
* Move custom buttons near border when mouse is outside the window, otherwise the user cannot reach the buttons without crossing an application window.<br>
* When floating window is moved into a splitter, it should get the height of the original window<br>
* Save layout, use a callback mechanism so that the user can add custom information<br>
* Restore layout, use a callback mechanism so that the user can create windows as required<br>
* Should floating windows be marked as floater?<br>
* Add support for toolbars<br>
* Seperate docking event handlers from wxDockingFrame. Internaly this should already be seperated, so it should be possible to move it into a seperate class<br>
* Toolbar ribbon movement (maybe should become part of wxToolbar?)<br>
* Toolbar item movement (maybe should become part of wxToolbar?)<br>
* Apply wxWidgets coding guidelines https://wxwidgets.org/develop/coding-guidelines/ and specifically https://wxwidgets.org/develop/coding-guidelines/#use_cpp_ext<br>

* <s>When converting a window to floating there is a second frame underneath it.</s><br>
* <s>Moving floating outside the app window crashes</s><br>
* <s>Add a mechanism to prevent dragging of panels via API</s><br>
* <s>Add a mechanism to lock panels via API to prevent automatic deletion of unneeded docking containers</s><br>
* <s>Use a global array for keeping track of created notebooks and splitters which can also store extra metadata like title and iconid</s><br>
* <s>Create layout via API</s><br>
* <s>Fix resize problem when adding splitters</s><br>
* <s>Fix Add tab in bottom splitter of demo tab screws up</s><br>
* <s>Move windows via API</s><br>
* <s>Tabcontrol movement (maybe should be part of wxNotebook?)</s><br>
* <s>Moving a window into a notebook page it should simply split, but it creates another notebook as well</s><br>
* <s>Test if notebook is empty and doesn't has any tabs</s><br>
* <s>Fix flickering on resize (after removing panels?)</s><br>
* <s>Refactor code to make smaller functions</s><br>
* <s>When moving a splitter and it has the same orientation, we should try to keep the proportion intact</s><br>
* <s>Splitter can be a root window as well, requires a placeholder window</s><br>
* <s>Moving a window into it's child is not allowed</s><br>
* <s>Do we have to loop through the pages if a notebook is removed? (no)</s><br>
* <s>Moving the mouse outside a frame will cause a nullpointer</s><br>
* <s>Tabname is wrong when window is converted to notebook</s><br>
* <s>Store name property on user window instead of splitter</s><br>
* <s>When targeting a docking window with overlapping frames, the bars are drawn over multiple frames. Should be client DC instead?</s><br>
* <s>The main application crashes when a window is converted to a floating and then destroyed. Active window is not detected.</s><br>
* <s>Moving a panel to outside the frame the hint is not shown correct</s><br>
* <s>Moving a panel into free tab area the hint is not shown correct</s><br>
* <s>Moving a panel to outside the frame the size is wrong.</s><br>
* <s>Grabbing a tabpanel should move the tabs into the new tabcontrol if the target is tab control</s><br>
* <s>Notebook tabs with different orientation</s><br>
* <s>Floating windows (multiple separate frames)</s><br>
* <s>Highlight docking target and show selector frame for floating or border</s><br>
* <s>Tabcontrols using LEXT/RIGHT style are stacked when they get to much (why not showing a scrollbox similar to TOP/BOTTOM?). They are changing place seemingly randomly. : Disabled page activation in that case</s><br>
* <s>Tabcontrol rectangle pullrequest for wxWidgets</s><br>
* <s>When moving inside a tab back and forth the hint is not always correct</s><br>
* <s>Maybe need to disable page activation on notebook. It can be confusing for the user if it works for most cases but not always</s><br>
* <s>Events for handling docking events</s><br>
* * <s>Start docking(veto) If vetoed, docking is disabled for this cycle. Has to prevent repeated start events</s><br>
* * <s>Track dragging (no veto)</s><br>
* * <s>End docking, can not be vetoed because when the user releases the mouse button, that's it</s><br>
* * <s>Allow move (veto)</s><br>
* * <s>Try move (veto)</s><br>
* * <s>Create panel (no veto)</s><br>
* * <s>Destroy panel (no veto</s>)<br>
<br>

#### Demoapplication

* Highlight active panel<br>
* Toogle menus requiring an active panel<br>
* Lock/Unlock active panel (panel stays instead of getting deleted when removed)<br>
