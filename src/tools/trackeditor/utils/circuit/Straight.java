/*
 *   Straight.java
 *   Created on 9 ??? 2005
 *
 *    The Straight.java is part of TrackEditor-0.6.0.
 *
 *    TrackEditor-0.6.0 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    TrackEditor-0.6.0 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with TrackEditor-0.6.0; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
package utils.circuit;

import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;

import gui.EditorFrame;
import utils.Editor;


/**
 * @author Patrice Espie , Charalampos Alexopoulos
 * 
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */

public class Straight extends Segment
{
	public Straight()
	{
		this(null);
	}

	public Straight(Segment prev)
	{
		super("str");
		this.previousShape = prev;
	}

	@Override
	public void set(Segment segment)
	{
		super.set(segment);
		Straight straight = (Straight) segment;
	}

	public Rectangle2D.Double getBounds()
	{
		if (points == null || points.length == 0)
			return (new Rectangle2D.Double(0, 0, 0, 0));

		double minX = Double.MAX_VALUE;
		double maxX = -Double.MAX_VALUE;
		double minY = Double.MAX_VALUE;
		double maxY = -Double.MAX_VALUE;

		for (int i = 0; i < points.length; i++)
		{
			// don't use barrier points
			if ((i >= 12 && i <= 15) || (i >= 24 && i <= 27))
				continue;

			if (minX > points[i].x)
				minX = points[i].x;
			if (maxX < points[i].x)
				maxX = points[i].x;
			if (minY > points[i].y)
				minY = points[i].y;
			if (maxY < points[i].y)
				maxY = points[i].y;
		}

		return (new Rectangle2D.Double(minX, minY, maxX - minX, maxY - minY));
	}

	public void calcShape(EditorFrame editorFrame) throws Exception
	{
		double currentX = Editor.getProperties().getCurrentX();
		double currentY = Editor.getProperties().getCurrentY();
		double currentA = Editor.getProperties().getCurrentA();
		double showArrows = Editor.getProperties().getShowArrows();
		double profilStepsLength = getValidProfilStepsLength(editorFrame);
		double trackWidth = editorFrame.getTrackData().getMainTrack().getWidth();
		double leftBorderWidth = getValidLeftBorderWidth(editorFrame);
		double rightBorderWidth = getValidRightBorderWidth(editorFrame);
		double leftSideStartWidth = getValidLeftSideStartWidth(editorFrame);
		double leftSideEndWidth = getValidLeftSideEndWidth(editorFrame);
		double rightSideStartWidth = getValidRightSideStartWidth(editorFrame);
		double rightSideEndWidth = getValidRightSideEndWidth(editorFrame);
		double leftBarrierWidth = getValidLeftBarrierWidth(editorFrame);
		double rightBarrierWidth = getValidRightBarrierWidth(editorFrame);
		double leftStartHeight = this.getCalculatedHeightStartLeft();
		double rightStartHeight = this.getCalculatedHeightStartRight();
		double leftEndHeight = this.getCalculatedHeightEndLeft();
		double rightEndHeight = this.getCalculatedHeightEndRight();

		int nbSteps = 1;
		if (hasProfilSteps())
		{
			nbSteps = getProfilSteps();
		}
		else
		{
			nbSteps = (int) (length / profilStepsLength + 0.5) + 1;
		}
		double stepLength = length / nbSteps;

		if (points == null || points.length != 4 * (7 + (showArrows > 0.0 ? 1 : 0)) * nbSteps)
		{
			points = new Point3D[4 * (7 + (showArrows > 0.0 ? 1 : 0)) * nbSteps];

			for (int i = 0; i < points.length; i++)
				points[i] = new Point3D();

			trPoints = new Point2D.Double[4 * (7 + (showArrows > 0.0 ? 1 : 0)) * nbSteps];
			for (int i = 0; i < trPoints.length; i++)
				trPoints[i] = new Point2D.Double();
		}

		int currentSubSeg = 0;
		double leftSideDeltaStep = (leftSideEndWidth - leftSideStartWidth) / nbSteps;
		double rightSideDeltaStep = (rightSideEndWidth - rightSideStartWidth) / nbSteps;

		double leftHeightDeltaStep = (leftEndHeight - leftStartHeight) / nbSteps;
		double rightHeightDeltaStep = (rightEndHeight - rightStartHeight) / nbSteps;

		double cos = Math.cos(currentA) * stepLength;
		double sin = Math.sin(currentA) * stepLength;

		double cosTransLeft = Math.cos(currentA + Math.PI / 2);
		double sinTransLeft = Math.sin(currentA + Math.PI / 2);

		boolean linear = getValidProfil(editorFrame).equals("linear");

		double T1l = getCalculatedStartTangentLeft() * getLength();
		double T2l = getCalculatedEndTangentLeft() * getLength();
		double tl = 0.0;
		double dtl = 1.0 / nbSteps;
		double T1r = getCalculatedStartTangentRight() * getLength();
		double T2r = getCalculatedEndTangentRight() * getLength();
		double tr = 0.0;
		double dtr = 1.0 / nbSteps;
		double curzsl = leftStartHeight;
		double curzsr = rightStartHeight;
		double curzel = leftStartHeight;
		double curzer = rightStartHeight;

		for (int nStep = 0; nStep < nbSteps; nStep++)
		{
			// track
			points[currentSubSeg + 0].x = currentX + cosTransLeft * trackWidth / 2;
			points[currentSubSeg + 0].y = currentY + sinTransLeft * trackWidth / 2;

			points[currentSubSeg + 1].x = points[currentSubSeg + 0].x + cos;
			points[currentSubSeg + 1].y = points[currentSubSeg + 0].y + sin;

			points[currentSubSeg + 3].x = currentX - cosTransLeft * trackWidth / 2;
			points[currentSubSeg + 3].y = currentY - sinTransLeft * trackWidth / 2;

			points[currentSubSeg + 2].x = points[currentSubSeg + 3].x + cos;
			points[currentSubSeg + 2].y = points[currentSubSeg + 3].y + sin;

			if (linear)
			{
				points[currentSubSeg + 0].z = leftStartHeight + leftHeightDeltaStep * nStep;
				points[currentSubSeg + 1].z = leftStartHeight + leftHeightDeltaStep * (nStep + 1);
				points[currentSubSeg + 2].z = rightStartHeight + rightHeightDeltaStep * (nStep + 1);
				points[currentSubSeg + 3].z = rightStartHeight + rightHeightDeltaStep * nStep;
			}
			else
			{
				tl += dtl;
				tr += dtr;

				curzsl = curzel;
				curzel = trackSpline(leftStartHeight, leftEndHeight, T1l, T2l, tl);

				curzsr = curzer;
				curzer = trackSpline(rightStartHeight, rightEndHeight, T1r, T2r, tr);

				points[currentSubSeg + 0].z = curzsl;
				points[currentSubSeg + 1].z = curzel;
				points[currentSubSeg + 2].z = curzer;
				points[currentSubSeg + 3].z = curzsr;
			}

			// left border

			points[currentSubSeg + 4].x = currentX + cosTransLeft * (trackWidth / 2 + leftBorderWidth);
			points[currentSubSeg + 4].y = currentY + sinTransLeft * (trackWidth / 2 + leftBorderWidth);

			points[currentSubSeg + 5].x = points[currentSubSeg + 4].x + cos;
			points[currentSubSeg + 5].y = points[currentSubSeg + 4].y + sin;

			points[currentSubSeg + 7].x = points[currentSubSeg + 0].x;
			points[currentSubSeg + 7].y = points[currentSubSeg + 0].y;

			points[currentSubSeg + 6].x = points[currentSubSeg + 1].x;
			points[currentSubSeg + 6].y = points[currentSubSeg + 1].y;

			// left side

			points[currentSubSeg + 8].x = currentX + cosTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideStartWidth + (leftSideDeltaStep * nStep));
			points[currentSubSeg + 8].y = currentY + sinTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideStartWidth + (leftSideDeltaStep * nStep));

			points[currentSubSeg + 9].x = currentX + cos + cosTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideStartWidth + (leftSideDeltaStep * (nStep + 1)));
			points[currentSubSeg + 9].y = currentY + sin + sinTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideStartWidth + (leftSideDeltaStep * (nStep + 1)));

			points[currentSubSeg + 10].x = points[currentSubSeg + 5].x;
			points[currentSubSeg + 10].y = points[currentSubSeg + 5].y;

			points[currentSubSeg + 11].x = points[currentSubSeg + 4].x;
			points[currentSubSeg + 11].y = points[currentSubSeg + 4].y;

			// left barrier

			points[currentSubSeg + 12].x = currentX + cosTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideStartWidth + (leftSideDeltaStep * nStep) + leftBarrierWidth);
			points[currentSubSeg + 12].y = currentY + sinTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideStartWidth + (leftSideDeltaStep * nStep) + leftBarrierWidth);

			points[currentSubSeg + 13].x = currentX + cos + cosTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideStartWidth + (leftSideDeltaStep * (nStep + 1)) + leftBarrierWidth);
			points[currentSubSeg + 13].y = currentY + sin + sinTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideStartWidth + (leftSideDeltaStep * (nStep + 1)) + leftBarrierWidth);

			points[currentSubSeg + 15].x = points[currentSubSeg + 8].x;
			points[currentSubSeg + 15].y = points[currentSubSeg + 8].y;

			points[currentSubSeg + 14].x = points[currentSubSeg + 9].x;
			points[currentSubSeg + 14].y = points[currentSubSeg + 9].y;

			// right border

			points[currentSubSeg + 16].x = currentX - cosTransLeft * (trackWidth / 2 + rightBorderWidth);
			points[currentSubSeg + 16].y = currentY - sinTransLeft * (trackWidth / 2 + rightBorderWidth);

			points[currentSubSeg + 17].x = points[currentSubSeg + 16].x + cos;
			points[currentSubSeg + 17].y = points[currentSubSeg + 16].y + sin;

			points[currentSubSeg + 19].x = points[currentSubSeg + 3].x;
			points[currentSubSeg + 19].y = points[currentSubSeg + 3].y;

			points[currentSubSeg + 18].x = points[currentSubSeg + 2].x;
			points[currentSubSeg + 18].y = points[currentSubSeg + 2].y;

			// right side

			points[currentSubSeg + 20].x = currentX - cosTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideStartWidth + (rightSideDeltaStep * nStep));
			points[currentSubSeg + 20].y = currentY - sinTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideStartWidth + (rightSideDeltaStep * nStep));

			points[currentSubSeg + 21].x = currentX + cos - cosTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideStartWidth + (rightSideDeltaStep * (nStep + 1)));
			points[currentSubSeg + 21].y = currentY + sin - sinTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideStartWidth + (rightSideDeltaStep * (nStep + 1)));

			points[currentSubSeg + 22].x = points[currentSubSeg + 17].x;
			points[currentSubSeg + 22].y = points[currentSubSeg + 17].y;

			points[currentSubSeg + 23].x = points[currentSubSeg + 16].x;
			points[currentSubSeg + 23].y = points[currentSubSeg + 16].y;

			// right barrier

			points[currentSubSeg + 24].x = currentX - cosTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideStartWidth + (rightSideDeltaStep * nStep) + rightBarrierWidth);
			points[currentSubSeg + 24].y = currentY - sinTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideStartWidth + (rightSideDeltaStep * nStep) + rightBarrierWidth);

			points[currentSubSeg + 25].x = currentX + cos - cosTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideStartWidth + (rightSideDeltaStep * (nStep + 1)) + rightBarrierWidth);
			points[currentSubSeg + 25].y = currentY + sin - sinTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideStartWidth + (rightSideDeltaStep * (nStep + 1)) + rightBarrierWidth);

			points[currentSubSeg + 27].x = points[currentSubSeg + 20].x;
			points[currentSubSeg + 27].y = points[currentSubSeg + 20].y;

			points[currentSubSeg + 26].x = points[currentSubSeg + 21].x;
			points[currentSubSeg + 26].y = points[currentSubSeg + 21].y;

			if (showArrows > 0.0)
			{
				// arrow
				points[currentSubSeg + 28].x = currentX + cosTransLeft * trackWidth / 2;
				points[currentSubSeg + 28].y = currentY + sinTransLeft * trackWidth / 2;

				points[currentSubSeg + 29].x = points[currentSubSeg + 0].x + cos - (cosTransLeft * trackWidth / 2) * 0.99999;
				points[currentSubSeg + 29].y = points[currentSubSeg + 0].y + sin - (sinTransLeft * trackWidth / 2) * 0.99999;

				points[currentSubSeg + 31].x = currentX - cosTransLeft * trackWidth / 2;
				points[currentSubSeg + 31].y = currentY - sinTransLeft * trackWidth / 2;

				points[currentSubSeg + 30].x = points[currentSubSeg + 23].x + cos + (cosTransLeft * trackWidth / 2) * 0.99999;
				points[currentSubSeg + 30].y = points[currentSubSeg + 23].y + sin + (sinTransLeft * trackWidth / 2) * 0.99999;

				currentSubSeg += 32;
			}
			else
			{
				currentSubSeg += 28;
			}

			// move track center
			this.dx = cos;
			this.dy = sin;
			currentX += cos;
			currentY += sin;
		}

		endTrackCenter.setLocation(currentX, currentY);
		endTrackAlpha = startTrackAlpha;

		Editor.getProperties().setCurrentA(currentA);
		Editor.getProperties().setCurrentX(currentX);
		Editor.getProperties().setCurrentY(currentY);
	}

	public void drag(Point2D.Double dragDelta)
	{
	}


	public Object clone()
	{
		Straight s;
		s = (Straight) super.clone();

		return s; // return the clone
	}

}
