// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace NFmiIndexMaskTools
 *
 * \section sec_NFmiIndexMaskTools_details Implementation details
 *
 * Most mask creation operations need to find the points closest
 * to some given point or path. For this purpose it is useful
 * not to iterate through the entire grid, querying the distances
 * for each point, but to iterate the nearest points first, until
 * at some point we can decide that no more points can be as near.
 *
 * Note that for complicated SVG paths the point inside polygon
 * test may be quite inefficient (linear). To speed up for example
 * MaskInside, we may calculate the insidedness and distance from
 * the mask for a fixed point. Any point within the radius established
 * by the distance of the fixed point to the edge of the polygon
 * must belong to the same region as the fixed point. We may update
 * the fixed point whenever we have to calculate the distance and
 * insidedness again to move the fixed point closer to the points
 * being investigated. Most mask operations can be optimized
 * similarly.
 */
// ======================================================================
/*!
 * \namespace NFmiIndexMaskTools
 *
 * \brief Tools for creating and analyzing NFmiIndexMask objects
 *
 * One can create masks from paths and points:
 * \code
 * using namespace NFmiIndexMaskTools;
 * NFmiIndexMask mask = MaskInside(grid,path);
 * NFmiIndexMask mask = MaskOutside(grid,path);
 * NFmiIndexMask mask = MaskExpand(grid,path,distance);
 * NFmiIndexMask mask = MaskShrink(grid,path,distance);
 * NFmiIndexMask mask = MaskDistance(grid,path,distance);
 * NFmiIndexMask mask = MaskDistance(grid,point,distance);
 * \endcode
 * or one can create masks from the grid values themselves
 * \code
 * NFmiIndexMask mask = MaskBelow(grid,limit);
 * NFmiIndexMask mask = MaskAbove(grid,limit);
 * NFmiIndexMask mask = MaskRange(grid,lolimit,hilimit);
 * \endcode
 * The above calls are especially useful when used with
 * the NFmiSvgTools to create NFmiSvgPath distance grids.
 *
 * The regression test directory contains two sample files,
 * suomi.svg and vesiraja.svg. Below are sample results
 * for various function calls, as visualized by MaskString.
 *
 * \htmlonly
 * <font size="1">
 * \endhtmlonly
 * \code
 * MaskInside(grid,suomi):
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............XX..............
 * ..............XXXX............
 * ..............XXXX............
 * .............XXXX.............
 * .......XX....XXXX.............
 * .......XX....XXXX.............
 * ........XXXX.XXXX.............
 * .........XXXXXXXX.............
 * ..........XXXXXXX.............
 * ..........XXXXXXXX............
 * ...........XXXXXXXX...........
 * ..........XXXXXXXXX...........
 * ..........XXXXXXXXX...........
 * ...........XXXXXXX............
 * ...........XXXXXXX............
 * ...........XXXXXXX............
 * ...........XXXXXXXX...........
 * ...........XXXXXXXX...........
 * ...........XXXXXXXX...........
 * ...........XXXXXXXXX..........
 * ...........XXXXXXXXX..........
 * ............XXXXXXX...........
 * .............XXXXXX...........
 * .............XXXXXXX..........
 * .............XXXXXX...........
 * .............XXXXXXX..........
 * ............XXXXXXXX..........
 * ...........XXXXXXXXX..........
 * ...........XXXXXXXXXX.........
 * ..........XXXXXXXXXXX.........
 * ..........XXXXXXXXXX..........
 * .........XXXXXXXXXXXX.........
 * ........XXXXXXXXXXXXX.........
 * ......X.XXXXXXXXXXXXXX........
 * .......XXXXXXXXXXXXXXXX.......
 * ......XXXXXXXXXXXXXXXXX.......
 * ......XXXXXXXXXXXXXXXXX.......
 * ......XXXXXXXXXXXXXXXXX.......
 * ......XXXXXXXXXXXXXXXX........
 * ......XXXXXXXXXXXXXXXX........
 * ......XXXXXXXXXXXXXXX.........
 * ......XXXXXXXXXXXXXXX.........
 * .......XXXXXXXXXXXXX..........
 * ......XXXXXXXXXXXXXX..........
 * ......XXXXXXXXXXXXX...........
 * ......XXXXXXXXXXXXX...........
 * ......XXXXXXXXXXXX............
 * ......XXXXXXXXXXX.............
 * ...X...XXXXXXXX...............
 * ...X..X.XXXXX.................
 * ........XXXX..................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * \endcode
 *
 * \code
 * MaskOutside(grid,suomi):
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXX..XXXXXXXXXXXXXX
 * XXXXXXXXXXXXXX....XXXXXXXXXXXX
 * XXXXXXXXXXXXXX....XXXXXXXXXXXX
 * XXXXXXXXXXXXX....XXXXXXXXXXXXX
 * XXXXXXX..XXXX....XXXXXXXXXXXXX
 * XXXXXXX..XXXX....XXXXXXXXXXXXX
 * XXXXXXXX....X....XXXXXXXXXXXXX
 * XXXXXXXXX........XXXXXXXXXXXXX
 * XXXXXXXXXX.......XXXXXXXXXXXXX
 * XXXXXXXXXX........XXXXXXXXXXXX
 * XXXXXXXXXXX........XXXXXXXXXXX
 * XXXXXXXXXX.........XXXXXXXXXXX
 * XXXXXXXXXX.........XXXXXXXXXXX
 * XXXXXXXXXXX.......XXXXXXXXXXXX
 * XXXXXXXXXXX.......XXXXXXXXXXXX
 * XXXXXXXXXXX.......XXXXXXXXXXXX
 * XXXXXXXXXXX........XXXXXXXXXXX
 * XXXXXXXXXXX........XXXXXXXXXXX
 * XXXXXXXXXXX........XXXXXXXXXXX
 * XXXXXXXXXXX.........XXXXXXXXXX
 * XXXXXXXXXXX.........XXXXXXXXXX
 * XXXXXXXXXXXX.......XXXXXXXXXXX
 * XXXXXXXXXXXXX......XXXXXXXXXXX
 * XXXXXXXXXXXXX.......XXXXXXXXXX
 * XXXXXXXXXXXXX......XXXXXXXXXXX
 * XXXXXXXXXXXXX.......XXXXXXXXXX
 * XXXXXXXXXXXX........XXXXXXXXXX
 * XXXXXXXXXXX.........XXXXXXXXXX
 * XXXXXXXXXXX..........XXXXXXXXX
 * XXXXXXXXXX...........XXXXXXXXX
 * XXXXXXXXXX..........XXXXXXXXXX
 * XXXXXXXXX............XXXXXXXXX
 * XXXXXXXX.............XXXXXXXXX
 * XXXXXX.X..............XXXXXXXX
 * XXXXXXX................XXXXXXX
 * XXXXXX.................XXXXXXX
 * XXXXXX.................XXXXXXX
 * XXXXXX.................XXXXXXX
 * XXXXXX................XXXXXXXX
 * XXXXXX................XXXXXXXX
 * XXXXXX...............XXXXXXXXX
 * XXXXXX...............XXXXXXXXX
 * XXXXXXX.............XXXXXXXXXX
 * XXXXXX..............XXXXXXXXXX
 * XXXXXX.............XXXXXXXXXXX
 * XXXXXX.............XXXXXXXXXXX
 * XXXXXX............XXXXXXXXXXXX
 * XXXXXX...........XXXXXXXXXXXXX
 * XXX.XXX........XXXXXXXXXXXXXXX
 * XXX.XX.X.....XXXXXXXXXXXXXXXXX
 * XXXXXXXX....XXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * \endcode
 *
 * \code
 * MaskExpand(grid,suomi,50):
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ...............XX.............
 * .............XXXXX............
 * ............XXXXXXX...........
 * ............XXXXXXX...........
 * ......XXX...XXXXXXX...........
 * ......XXXX..XXXXXXX...........
 * .....XXXXXXXXXXXXXX...........
 * .....XXXXXXXXXXXXXX...........
 * .....XXXXXXXXXXXXXX...........
 * ......XXXXXXXXXXXXX...........
 * .......XXXXXXXXXXXX...........
 * .......XXXXXXXXXXXXX..........
 * ........XXXXXXXXXXXX..........
 * .........XXXXXXXXXXXX.........
 * .........XXXXXXXXXXXX.........
 * .........XXXXXXXXXXXX.........
 * .........XXXXXXXXXXX..........
 * .........XXXXXXXXXXX..........
 * .........XXXXXXXXXXX..........
 * .........XXXXXXXXXXX..........
 * .........XXXXXXXXXXXX.........
 * .........XXXXXXXXXXXX.........
 * .........XXXXXXXXXXXX.........
 * .........XXXXXXXXXXXXX........
 * .........XXXXXXXXXXXXX........
 * ..........XXXXXXXXXXX.........
 * ..........XXXXXXXXXXX.........
 * ..........XXXXXXXXXXXX........
 * ..........XXXXXXXXXXXX........
 * ..........XXXXXXXXXXXX........
 * .........XXXXXXXXXXXXX........
 * ........XXXXXXXXXXXXXXX.......
 * .......XXXXXXXXXXXXXXXX.......
 * .......XXXXXXXXXXXXXXX........
 * .....XXXXXXXXXXXXXXXXXX.......
 * .....XXXXXXXXXXXXXXXXXXX......
 * ....XXXXXXXXXXXXXXXXXXXXX.....
 * .....XXXXXXXXXXXXXXXXXXXX.....
 * ....XXXXXXXXXXXXXXXXXXXXX.....
 * ....XXXXXXXXXXXXXXXXXXXXX.....
 * ....XXXXXXXXXXXXXXXXXXXX......
 * ....XXXXXXXXXXXXXXXXXXXX......
 * ....XXXXXXXXXXXXXXXXXXXX......
 * ....XXXXXXXXXXXXXXXXXXX.......
 * .....XXXXXXXXXXXXXXXXXX.......
 * .....XXXXXXXXXXXXXXXXX........
 * .....XXXXXXXXXXXXXXXXX........
 * ....XXXXXXXXXXXXXXXXX.........
 * ...XXXXXXXXXXXXXXXXXX.........
 * ..XXXXXXXXXXXXXXXXXX..........
 * .XXXXXXXXXXXXXXXXXXX..........
 * .XXXXXXXXXXXXXXXXXX...........
 * .XXXXXXXXXXXXXXXXXX...........
 * .XXXXXXXXXXXXXXX..............
 * ..XXXXXXXXXXXXX...............
 * ...XX..XXXXXX.................
 * ........XX....................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * \endcode
 *
 * \code
 * MaskShrink(grid,suomi,25):
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ...............X..............
 * ..............XXX.............
 * ..............XX..............
 * ..............XX..............
 * ..............XX..............
 * ..............XX..............
 * ...........X.XXX..............
 * ...........XXXXX..............
 * ...........XXXXXX.............
 * ...........XXXXXXX............
 * ...........XXXXXXX............
 * ...........XXXXXXX............
 * ...........XXXXXXX............
 * ...........XXXXXX.............
 * ............XXXXX.............
 * ............XXXXXX............
 * ...........XXXXXXX............
 * ...........XXXXXXX............
 * ............XXXXXXX...........
 * .............XXXXXX...........
 * .............XXXXXX...........
 * ..............XXXXX...........
 * ..............XXXXX...........
 * ..............XXXX............
 * ..............XXXXX...........
 * ..............XXXXX...........
 * ............XXXXXXX...........
 * ............XXXXXXXX..........
 * ...........XXXXXXXXX..........
 * ...........XXXXXXXXX..........
 * ..........XXXXXXXXXX..........
 * .........XXXXXXXXXXX..........
 * .........XXXXXXXXXXXX.........
 * ........XXXXXXXXXXXXXX........
 * .......XXXXXXXXXXXXXXX........
 * .......XXXXXXXXXXXXXXX........
 * .......XXXXXXXXXXXXXXX........
 * .......XXXXXXXXXXXXXX.........
 * .......XXXXXXXXXXXXXX.........
 * .......XXXXXXXXXXXXX..........
 * .......XXXXXXXXXXXXX..........
 * .......XXXXXXXXXXXX...........
 * .......XXXXXXXXXXXX...........
 * .......XXXXXXXXXXX............
 * .......XXXXXXXXXXX............
 * .......XXXXXXXXXX.............
 * .......XXXXXXX................
 * ........XXXX..................
 * .........X....................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * \endcode
 *
 * \code
 * MaskDistance(grid,suomi,25):
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ...............XX.............
 * ..............XXX.............
 * .............XXXXX............
 * .............XX.XXX...........
 * .......XX....X...XX...........
 * ......XXX....X..XX............
 * ......XXXX.XXX..XX............
 * ......XXXXXXXX..XX............
 * .......XXXXXXX..XX............
 * ........XXX.X...XX............
 * .........XX.....XXX...........
 * .........XX......XX...........
 * ..........X.......XX..........
 * ..........X.......XX..........
 * .........XX.......XX..........
 * ..........X.......X...........
 * ..........X......XX...........
 * ..........XX.....XX...........
 * ..........XX......XX..........
 * ..........X.......XX..........
 * ..........X.......XX..........
 * ..........XX.......XX.........
 * ..........XXX......XX.........
 * ..........XXX......XX.........
 * ...........XXX.....X..........
 * ...........XXX.....XX.........
 * ...........XXX....XXX.........
 * ...........XXX.....XX.........
 * ...........XXX.....XX.........
 * ..........XX.......XX.........
 * ..........XX........XX........
 * .........XX.........XX........
 * ........XXX.........X.........
 * ........XX..........XX........
 * ......XXX...........XXX.......
 * .....XXXX............XX.......
 * .....XXX..............XX......
 * .....XX...............XX......
 * .....XX...............XX......
 * .....XX...............X.......
 * .....XX..............XX.......
 * .....XX..............XX.......
 * .....XX.............XX........
 * ......X.............XX........
 * ......X............XX.........
 * .....XX............XX.........
 * .....XX...........XX..........
 * .....XX...........XX..........
 * .....XX..........XX...........
 * ..XX.XX.......XXXXX...........
 * ..XXXXXX....XXXXXX............
 * ..XXXXXXX.XXXXXX..............
 * ..XXX.XXXXXXXX................
 * ...X...XXXXX..................
 * ........X.....................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * \endcode
 *
 * \code
 * MaskDistance(grid,vesiraja,25):
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ........XX....................
 * ........XXXXX.................
 * ........XXXXX.................
 * ......XXXX.XXX................
 * ......XXX..XXX................
 * ......XX...XXX................
 * ......XX...XXX................
 * ......X....XXX................
 * ......XX..XX..................
 * .....XX...XX..................
 * .....XX..XX...................
 * ....XX..XXX...................
 * ...XXX..XX....................
 * ..XXX.XXX.....................
 * XXXX.XXXX.....................
 * XXX..XXX......................
 * XX...XX.......................
 * X....XX.......................
 * X....XX.......................
 * .....XX.......................
 * .....XX.......................
 * .....XX...............XX......
 * ......X..............XXX......
 * ......X..............XXXX.....
 * .....XX.............XX..XX....
 * .....XX.............XX..XXX...
 * .....XX..............XX..XXX..
 * .....XX..........XX..XX...X...
 * X.XX.XX.......XXXXXX..XX..X...
 * XXXXXXXX....XXXXXXXXXXXXXXX...
 * XXXXXXXXX.XXXXXX..XXXXXXXXX...
 * XXXXX.XXXXXXXX....XXXXXXX.....
 * XX.X...XXXXX.....XXXXXX.......
 * XX......X..XXXXX.XXX..........
 * XX........XXXXXXXXX...........
 * XX.......XXX...XXXX...........
 * X........X....................
 * .........X....................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * \endcode
 *
 * \code
 * NFmiPoint hki(25,60);
 * MaskDistance(grid,hki,100):
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ...........XXX................
 * ..........XXXXX...............
 * .........XXXXXX...............
 * .........XXXXXXX..............
 * .........XXXXXXX..............
 * .........XXXXXXX..............
 * .........XXXXXX...............
 * ..........XXXXX...............
 * ..........XXXX................
 * ............X.................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * ..............................
 * \endcode
 * \htmlonly
 * </font>
 * \endhtmlonly
 */
// ======================================================================

#include "NFmiIndexMaskTools.h"

#include "NFmiCalculationCondition.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiGeoTools.h"
#include "NFmiGrid.h"
#include "NFmiIndexMask.h"
#include "NFmiNearTree.h"
#include "NFmiPoint.h"
#include "NFmiSvgPath.h"
#include "NFmiSvgTools.h"

#include <cassert>

// Implementation hiding detail functions

namespace
{
//! The factor by which we reduce grid resolution for distance calculations

const double resolution_factor = 1.0 / 4;

// ----------------------------------------------------------------------
/*!
 * \brief Insert a line into the given NFmiNearTree
 *
 * This is used when inserting a NFmiSvgPath into the NFmiNearTree
 * with some fixed resolution.
 *
 * \param theTree The tree to insert the data into
 * \param theStart The starting X-coordinate
 * \param theEnd The end X-coordinate
 * \param theResolution The maximum allowed edge distance
 */
// ----------------------------------------------------------------------

void Insert(NFmiNearTree<NFmiPoint> &theTree,
            const NFmiPoint &theStart,
            const NFmiPoint &theEnd,
            double theResolution)
{
  // Safety against infinite recursion
  if (theResolution <= 0)
  {
    theTree.Insert(theStart);
    theTree.Insert(theEnd);
  }
  else
  {
    // if edge length is small enough, stop recursion
    const double dist = theStart.Distance(theEnd);
    if (dist <= theResolution)
    {
      theTree.Insert(theStart);
      theTree.Insert(theEnd);
    }
    else
    {
      // subdivide and recurse
      NFmiPoint mid((theStart.X() + theEnd.X()) / 2, (theStart.Y() + theEnd.Y()) / 2);
      Insert(theTree, theStart, mid, theResolution);
      Insert(theTree, theEnd, mid, theResolution);
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Insert the SVG path into the given NFmiNearTree
 *
 * The purpose here is to provide means for calculating
 * the distance of some point from the NFmiSvgPath. The user
 * is expected to provide some suitable resolution for
 * subdividing too long edges into more vertices.
 *
 * \param theTree The tree to insert the data into
 * \param thePath The path to insert
 * \param theResolution The maximum allowed edge distance
 */
// ----------------------------------------------------------------------

void Insert(NFmiNearTree<NFmiPoint> &theTree, const NFmiSvgPath &thePath, double theResolution)
{
  if (thePath.empty()) return;

  NFmiPoint firstPoint(thePath.front().itsX, thePath.front().itsY);

  NFmiPoint lastPoint(0, 0);

  for (const auto &it : thePath)
  {
    switch (it.itsType)
    {
      case NFmiSvgPath::kElementMoveto:
        lastPoint.Set(it.itsX, it.itsY);
        firstPoint = lastPoint;
        break;
      case NFmiSvgPath::kElementClosePath:
      {
        Insert(theTree, lastPoint, firstPoint, theResolution);
        lastPoint = firstPoint;
        break;
      }
      case NFmiSvgPath::kElementLineto:
      {
        NFmiPoint nextPoint(it.itsX, it.itsY);
        Insert(theTree, lastPoint, nextPoint, theResolution);
        lastPoint = nextPoint;
        break;
      }
      case NFmiSvgPath::kElementNotValid:
        return;
    }
  }
}

}  // namespace

namespace NFmiIndexMaskTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Return area inside the path
 *
 * If the path is empty, so is the mask.
 *
 * Optimization algorithm:
 * -# Store last calculated insidedness and distance
 * -# If some point is within distance of stored point
 *    -# It must have same insidedness
 * -# Set this as the new last point and calculate as required
 *
 * \param theGrid The grid
 * \param thePath The path (closed)
 * \return The mask for the area inside the path
 */
// ----------------------------------------------------------------------

NFmiIndexMask MaskInside(const NFmiGrid &theGrid, const NFmiSvgPath &thePath)
{
  NFmiIndexMask mask;

  // Handle empty paths
  if (thePath.empty()) return mask;

  // Establish grid resolution

  const double dx = theGrid.Area()->WorldXYWidth() / theGrid.XNumber();
  const double dy = theGrid.Area()->WorldXYHeight() / theGrid.YNumber();

  // Fast lookup tree for distance calculations

  NFmiSvgPath projectedPath(thePath);
  NFmiSvgTools::LatLonToWorldXY(projectedPath, *theGrid.Area());
  NFmiNearTree<NFmiPoint> tree;
  Insert(tree, projectedPath, FmiMin(dx, dy) * resolution_factor);

  // Optimization

  bool lastPointOn = false;
  NFmiPoint lastPoint;
  double lastDistance = 0.0;
  bool lastInside = false;

  // Non-optimal solution loops through the entire grid

  const unsigned long nx = theGrid.XNumber();
  const unsigned long ny = theGrid.YNumber();

  for (unsigned long j = 0; j < ny; j++)
    for (unsigned long i = 0; i < nx; i++)
    {
      const unsigned long idx = j * nx + i;
      const NFmiPoint p = theGrid.LatLon(idx);

      const NFmiPoint xy = theGrid.GridToWorldXY(i, j);

      bool recalculate = true;

      if (lastPointOn)
      {
        double distance = xy.Distance(lastPoint);
        if (distance < lastDistance)
        {
          recalculate = false;
          if (lastInside) mask.insert(idx);
        }
      }

      if (recalculate)
      {
        NFmiPoint nearest;
        tree.NearestPoint(nearest, xy);

        lastPointOn = true;
        lastPoint = xy;
        lastInside = NFmiSvgTools::IsInside(thePath, p);
        lastDistance = xy.Distance(nearest);

        if (lastInside) mask.insert(idx);
      }
    }
  return mask;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return area outside the path
 *
 * If the path is empty, the mask is full
 *
 * Optimization algorithm:
 * -# Store last calculated insidedness and distance
 * -# If some point is within distance of stored point
 *    -# It must have same insidedness
 * -# Set this as the new last point and calculate as required
 *
 * \param theGrid The grid
 * \param thePath The path (closed)
 * \return The mask for the area outside the path
 */
// ----------------------------------------------------------------------

NFmiIndexMask MaskOutside(const NFmiGrid &theGrid, const NFmiSvgPath &thePath)
{
  NFmiIndexMask mask;

  // Establish grid resolution

  const double dx = theGrid.Area()->WorldXYWidth() / theGrid.XNumber();
  const double dy = theGrid.Area()->WorldXYHeight() / theGrid.YNumber();

  // Fast lookup tree for distance calculations

  NFmiSvgPath projectedPath(thePath);
  NFmiSvgTools::LatLonToWorldXY(projectedPath, *theGrid.Area());
  NFmiNearTree<NFmiPoint> tree;
  Insert(tree, projectedPath, FmiMin(dx, dy) * resolution_factor);

  // Optimization

  bool lastPointOn = false;
  NFmiPoint lastPoint;
  double lastDistance = 0;
  bool lastInside = false;

  // Non-optimal solution loops through the entire grid

  const unsigned long nx = theGrid.XNumber();
  const unsigned long ny = theGrid.YNumber();

  if (thePath.empty())
  {
    for (unsigned long j = 0; j < ny; j++)
      for (unsigned long i = 0; i < nx; i++)
        mask.insert(j * nx + i);
  }
  else
  {
    for (unsigned long j = 0; j < ny; j++)
      for (unsigned long i = 0; i < nx; i++)
      {
        const unsigned long idx = j * nx + i;
        const NFmiPoint p = theGrid.LatLon(idx);

        const NFmiPoint xy = theGrid.GridToWorldXY(i, j);

        bool recalculate = true;

        if (lastPointOn)
        {
          double distance = xy.Distance(lastPoint);
          if (distance < lastDistance)
          {
            recalculate = false;
            if (!lastInside) mask.insert(idx);
          }
        }

        if (recalculate)
        {
          NFmiPoint nearest;
          tree.NearestPoint(nearest, xy);

          lastPointOn = true;
          lastPoint = xy;
          lastInside = NFmiSvgTools::IsInside(thePath, p);
          lastDistance = xy.Distance(nearest);

          if (!lastInside) mask.insert(idx);
        }
      }
  }
  return mask;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return area inside the path plus surrounding area
 *
 * If the path is empty, so is the mask
 *
 * Optimization algorithm:
 * -# Store last calculated insidedness and distance
 * -# If last point was inside
 *    -# If some point is within distance+expand of stored point
 *       it is also inside
 * -# Else
 *    -# If some point is within distance-expand of stored point
 *       it is also outside
 * -# Upgrade last point when result is unknown
 *
 * \param theGrid The grid
 * \param thePath The path (closed)
 * \param theDistance The maximum allowed distance from the path
 * \return The mask for the area
 */
// ----------------------------------------------------------------------

NFmiIndexMask MaskExpand(const NFmiGrid &theGrid, const NFmiSvgPath &thePath, double theDistance)
{
  if (theDistance == 0) return MaskInside(theGrid, thePath);
  if (theDistance < 0) return MaskShrink(theGrid, thePath, -theDistance);

  NFmiIndexMask mask;

  // Handle empty paths
  if (thePath.empty()) return mask;

  // Establish grid resolution

  const double dx = theGrid.Area()->WorldXYWidth() / theGrid.XNumber();
  const double dy = theGrid.Area()->WorldXYHeight() / theGrid.YNumber();

  // Fast lookup tree for distance calculations

  NFmiSvgPath projectedPath(thePath);
  NFmiSvgTools::LatLonToWorldXY(projectedPath, *theGrid.Area());
  NFmiNearTree<NFmiPoint> tree;
  Insert(tree, projectedPath, FmiMin(dx, dy) * resolution_factor);

  // Optimization

  bool lastPointOn = false;
  NFmiPoint lastPoint;
  double lastDistance = 0;
  bool lastInside = false;

  // Non-optimal solution loops through the entire grid

  const unsigned long nx = theGrid.XNumber();
  const unsigned long ny = theGrid.YNumber();

  for (unsigned long j = 0; j < ny; j++)
    for (unsigned long i = 0; i < nx; i++)
    {
      const unsigned long idx = j * nx + i;

      const NFmiPoint p = theGrid.LatLon(idx);
      const NFmiPoint xy = theGrid.GridToWorldXY(i, j);

      bool recalculate = true;

      if (lastPointOn)
      {
        double distance = xy.Distance(lastPoint);
        if (lastInside)
        {
          if (distance < lastDistance + theDistance * 1000)
          {
            recalculate = false;
            mask.insert(idx);
          }
        }
        else
        {
          recalculate = (distance >= lastDistance - theDistance * 1000);
        }
      }

      if (recalculate)
      {
        NFmiPoint nearest;
        tree.NearestPoint(nearest, xy);

        lastPointOn = true;
        lastPoint = xy;
        lastInside = NFmiSvgTools::IsInside(thePath, p);
        lastDistance = xy.Distance(nearest);

        if (lastInside || lastDistance <= theDistance * 1000) mask.insert(idx);
      }
    }
  return mask;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return area inside the path minus border areas
 *
 * If the path is empty, so is the mask
 *
 * \param theGrid The grid
 * \param thePath The path (closed)
 * \param theDistance The required distance from the path
 * \return The mask for the area
 */
// ----------------------------------------------------------------------

NFmiIndexMask MaskShrink(const NFmiGrid &theGrid, const NFmiSvgPath &thePath, double theDistance)
{
  if (theDistance == 0) return MaskInside(theGrid, thePath);
  if (theDistance < 0) return MaskExpand(theGrid, thePath, -theDistance);

  NFmiIndexMask mask;

  // Handle empty paths
  if (thePath.empty()) return mask;

  // Establish grid resolution

  const double dx = theGrid.Area()->WorldXYWidth() / theGrid.XNumber();
  const double dy = theGrid.Area()->WorldXYHeight() / theGrid.YNumber();

  // Fast lookup tree for distance calculations

  NFmiSvgPath projectedPath(thePath);
  NFmiSvgTools::LatLonToWorldXY(projectedPath, *theGrid.Area());
  NFmiNearTree<NFmiPoint> tree;
  Insert(tree, projectedPath, FmiMin(dx, dy) * resolution_factor);

  // Non-optimal solution loops through the entire grid

  const unsigned long nx = theGrid.XNumber();
  const unsigned long ny = theGrid.YNumber();

  NFmiPoint nearestPoint, p, xy;
  for (unsigned long j = 0; j < ny; j++)
    for (unsigned long i = 0; i < nx; i++)
    {
      const unsigned long idx = j * nx + i;
      p = theGrid.LatLon(idx);
      xy = theGrid.GridToWorldXY(i, j);
      if (NFmiSvgTools::IsInside(thePath, p) &&
          !tree.NearestPoint(nearestPoint, xy, theDistance * 1000))
        mask.insert(idx);
    }
  return mask;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return area withing given distance from path
 *
 * If the path is empty, so is the mask.
 *
 * \param theGrid The grid
 * \param thePath The path (not necessarily closed)
 * \param theDistance The maximum allowed distance from the path
 * \return The mask for the area within the required distance
 */
// ----------------------------------------------------------------------

NFmiIndexMask MaskDistance(const NFmiGrid &theGrid, const NFmiSvgPath &thePath, double theDistance)
{
  NFmiIndexMask mask;

  if (theDistance < 0) return mask;

  // Handle empty paths
  if (thePath.empty()) return mask;

  // Establish grid resolution

  const double dx = theGrid.Area()->WorldXYWidth() / theGrid.XNumber();
  const double dy = theGrid.Area()->WorldXYHeight() / theGrid.YNumber();

  // Fast lookup tree for distance calculations

  NFmiSvgPath projectedPath(thePath);
  NFmiSvgTools::LatLonToWorldXY(projectedPath, *theGrid.Area());
  NFmiNearTree<NFmiPoint> tree;
  Insert(tree, projectedPath, FmiMin(dx, dy) * resolution_factor);

  // Non-optimal solution loops through the entire grid

  const unsigned long nx = theGrid.XNumber();
  const unsigned long ny = theGrid.YNumber();

  NFmiPoint nearestPoint, xy;
  for (unsigned long j = 0; j < ny; j++)
    for (unsigned long i = 0; i < nx; i++)
    {
      const unsigned long idx = j * nx + i;
      xy = theGrid.GridToWorldXY(i, j);
      if (tree.NearestPoint(nearestPoint, xy, theDistance * 1000)) mask.insert(idx);
    }
  return mask;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return area withing given distance from point
 *
 * \param theGrid The grid
 * \param thePoint The point
 * \param theDistance The maximum allowed distance from the point
 * \return The mask for the area within the required distance
 */
// ----------------------------------------------------------------------

NFmiIndexMask MaskDistance(const NFmiGrid &theGrid, const NFmiPoint &thePoint, double theDistance)
{
  NFmiIndexMask mask;

  if (theDistance < 0) return mask;

  NFmiPoint p = theGrid.Area()->LatLonToWorldXY(thePoint);

  // Non-optimal solution loops through the entire grid

  const unsigned long nx = theGrid.XNumber();
  const unsigned long ny = theGrid.YNumber();

  NFmiPoint xy;
  for (unsigned long j = 0; j < ny; j++)
    for (unsigned long i = 0; i < nx; i++)
    {
      const unsigned long idx = j * nx + i;
      xy = theGrid.GridToWorldXY(i, j);
      if (p.Distance(xy) <= theDistance * 1000) mask.insert(idx);
    }
  return mask;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return areas inside the path plus surrounding areas
 *
 * If the path is empty, so is the mask.
 *
 * \param theGrid The grid
 * \param thePath The path (closed)
 * \param theDistances Vector of maximum allowed distance from the path
 * \return Vector of masks for each distance
 */
// ----------------------------------------------------------------------

std::vector<NFmiIndexMask> MaskExpand(const NFmiGrid &theGrid,
                                      const NFmiSvgPath &thePath,
                                      std::vector<double> theDistances)
{
  std::vector<NFmiIndexMask> masks;
  masks.resize(theDistances.size());

  // Handle special cases
  if (theDistances.empty() || thePath.empty()) return masks;

  // Establish grid resolution

  const double dx = theGrid.Area()->WorldXYWidth() / theGrid.XNumber();
  const double dy = theGrid.Area()->WorldXYHeight() / theGrid.YNumber();

  // Fast lookup tree for distance calculations

  NFmiSvgPath projectedPath(thePath);
  NFmiSvgTools::LatLonToWorldXY(projectedPath, *theGrid.Area());
  NFmiNearTree<NFmiPoint> tree;
  Insert(tree, projectedPath, FmiMin(dx, dy) * resolution_factor);

  // Non-optimal solution loops through the entire grid

  const unsigned long nx = theGrid.XNumber();
  const unsigned long ny = theGrid.YNumber();

  NFmiPoint nearestPoint, xy;

  for (unsigned long j = 0; j < ny; j++)
    for (unsigned long i = 0; i < nx; i++)
    {
      const unsigned long idx = j * nx + i;

      xy = theGrid.GridToWorldXY(i, j);

      bool isinside = NFmiSvgTools::IsInside(thePath, theGrid.LatLon(idx));
      bool foundnear = tree.NearestPoint(nearestPoint, xy);
      double dist = (!foundnear ? 0.0 : nearestPoint.Distance(xy));

      for (unsigned int k = 0; k < theDistances.size(); k++)
      {
        const double limit = 1000 * theDistances[k];

        if (isinside && limit >= 0)  // inside expand?
          masks[k].insert(idx);
        else if (!isinside && limit < 0)  // outside shrink?
          ;
        else if (limit >= 0 && foundnear && dist <= limit)
          masks[k].insert(idx);
        else if (limit < 0 && foundnear && dist > (-limit))
          masks[k].insert(idx);
      }
    }
  return masks;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the mask as a string visualization
 *
 * \param theMask The mask to visualize
 * \param theWidth The width of the mask
 * \param theHeight The height of the mask
 */
// ----------------------------------------------------------------------

std::string MaskString(const NFmiIndexMask &theMask,
                       unsigned long theWidth,
                       unsigned long theHeight)
{
  std::string out;
  for (long j = theHeight - 1; j >= 0; j--)
  {
    for (unsigned long i = 0; i < theWidth; i++)
    {
      const unsigned long idx = j * theWidth + i;
      if (theMask.find(idx) != theMask.end())
        out += 'X';
      else
        out += '.';
    }
    out += '\n';
  }
  return out;
}

// ----------------------------------------------------------------------
/*!
 * Makes mask to wanted grid from info-grid and calculated by given
 * condition in info's current time, param and level.´The return
 * mask can be in different grid than the source-info grid.
 * If condition return missing-value, mask to the current point is not set.
 *
 * \param theGrid is wanted mask-grid.
 * \param theInfo contains data that is used to calculate the mask (time,param and level are set
 * outside).
 * \param theCondition contains the wanted mask condition e.g. greater than 5 (>5).
 * \return Calculated IndexMask-object.
 */
// ----------------------------------------------------------------------

NFmiIndexMask MaskCondition(const NFmiGrid &theGrid,
                            NFmiFastQueryInfo &theInfo,
                            const NFmiCalculationCondition &theCondition)
{
  NFmiIndexMask mask;

  if (!theInfo.IsGrid()) return mask;

  const unsigned long nx = theGrid.XNumber();
  const unsigned long ny = theGrid.YNumber();

  NFmiPoint latlon;
  for (unsigned long j = 0; j < ny; j++)
    for (unsigned long i = 0; i < nx; i++)
    {
      const unsigned long idx = j * nx + i;
      latlon = theGrid.GridToLatLon(i, j);
      double value = theInfo.InterpolatedValue(latlon);
      if (theCondition.IsMasked(value)) mask.insert(idx);
    }
  return mask;
}

}  // namespace NFmiIndexMaskTools

// ======================================================================
