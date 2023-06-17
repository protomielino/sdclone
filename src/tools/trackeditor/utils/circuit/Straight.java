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
		double trackWidth = editorFrame.getTrackData().getMainTrack().getWidth();
		double leftBorderWidth = getValidLeftBorderWidth(editorFrame);
		double rightBorderWidth = getValidRightBorderWidth(editorFrame);
		double leftSideStartWidth = getValidLeftSideStartWidth(editorFrame);
		double leftSideEndWidth = getValidLeftSideEndWidth(editorFrame);
		double rightSideStartWidth = getValidRightSideStartWidth(editorFrame);
		double rightSideEndWidth = getValidRightSideEndWidth(editorFrame);
		double leftBarrierWidth = getValidLeftBarrierWidth(editorFrame);
		double rightBarrierWidth = getValidRightBarrierWidth(editorFrame);

		if (points == null || points.length != 4 * (7 + (showArrows > 0.0 ? 1 : 0)))
		{
			points = new Point2D.Double[4 * (7 + (showArrows > 0.0 ? 1 : 0))]; 
			for (int i = 0; i < points.length; i++)
				points[i] = new Point2D.Double();

			trPoints = new Point2D.Double[4 * (7 + (showArrows > 0.0 ? 1 : 0))];
			for (int i = 0; i < trPoints.length; i++)
				trPoints[i] = new Point2D.Double();
		}
		double cos = Math.cos(currentA) * length;
		double sin = Math.sin(currentA) * length;

		double cosTransLeft = Math.cos(currentA + Math.PI / 2);
		double sinTransLeft = Math.sin(currentA + Math.PI / 2);

		// track
		points[0].x = currentX + cosTransLeft * trackWidth / 2;
		points[0].y = currentY + sinTransLeft * trackWidth / 2;

		points[1].x = points[0].x + cos;
		points[1].y = points[0].y + sin;

		points[3].x = currentX - cosTransLeft * trackWidth / 2;
		points[3].y = currentY - sinTransLeft * trackWidth / 2;

		points[2].x = points[3].x + cos;
		points[2].y = points[3].y + sin;

		// left border

		points[4].x = currentX + cosTransLeft * (trackWidth / 2 + leftBorderWidth);
		points[4].y = currentY + sinTransLeft * (trackWidth / 2 + leftBorderWidth);

		points[5].x = points[4].x + cos;
		points[5].y = points[4].y + sin;

		points[7].x = points[0].x;
		points[7].y = points[0].y;

		points[6].x = points[1].x;
		points[6].y = points[1].y;

		// left side

		points[8].x = currentX + cosTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideStartWidth);
		points[8].y = currentY + sinTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideStartWidth);

		points[9].x = currentX + cos + cosTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideEndWidth);
		points[9].y = currentY + sin + sinTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideEndWidth);

		points[10].x = points[5].x;
		points[10].y = points[5].y;

		points[11].x = points[4].x;
		points[11].y = points[4].y;
		
		// left barrier

		points[12].x = currentX + cosTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideStartWidth + leftBarrierWidth);
		points[12].y = currentY + sinTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideStartWidth + leftBarrierWidth);

		points[13].x = currentX + cos + cosTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideEndWidth + leftBarrierWidth);
		points[13].y = currentY + sin + sinTransLeft * (trackWidth / 2 + leftBorderWidth + leftSideEndWidth + leftBarrierWidth);

		points[15].x = points[8].x;
		points[15].y = points[8].y;

		points[14].x = points[9].x;
		points[14].y = points[9].y;
	
		// right border

		points[16].x = currentX - cosTransLeft * (trackWidth / 2 + rightBorderWidth);
		points[16].y = currentY - sinTransLeft * (trackWidth / 2 + rightBorderWidth);

		points[17].x = points[16].x + cos;
		points[17].y = points[16].y + sin;

		points[19].x = points[3].x;
		points[19].y = points[3].y;

		points[18].x = points[2].x;
		points[18].y = points[2].y;

		// right side

		points[20].x = currentX - cosTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideStartWidth);
		points[20].y = currentY - sinTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideStartWidth);

		points[21].x = currentX + cos - cosTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideEndWidth);
		points[21].y = currentY + sin - sinTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideEndWidth);

		points[22].x = points[17].x;
		points[22].y = points[17].y;

		points[23].x = points[16].x;
		points[23].y = points[16].y;

		// right barrier

		points[24].x = currentX - cosTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideStartWidth + rightBarrierWidth);
		points[24].y = currentY - sinTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideStartWidth + rightBarrierWidth);

		points[25].x = currentX + cos - cosTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideEndWidth + rightBarrierWidth);
		points[25].y = currentY + sin - sinTransLeft * (trackWidth / 2 + rightBorderWidth + rightSideEndWidth + rightBarrierWidth);

		points[27].x = points[20].x;
		points[27].y = points[20].y;

		points[26].x = points[21].x;
		points[26].y = points[21].y;
	
		if (showArrows > 0.0)
		{
			// arrow
			points[28].x = currentX + cosTransLeft * trackWidth / 2;
			points[28].y = currentY + sinTransLeft * trackWidth / 2;

			points[29].x = points[0].x + cos - (cosTransLeft * trackWidth / 2) * 0.99999;
			points[29].y = points[0].y + sin - (sinTransLeft * trackWidth / 2) * 0.99999;

			points[31].x = currentX - cosTransLeft * trackWidth / 2;
			points[31].y = currentY - sinTransLeft * trackWidth / 2;

			points[30].x = points[23].x + cos + (cosTransLeft * trackWidth / 2) * 0.99999;
			points[30].y = points[23].y + sin + (sinTransLeft * trackWidth / 2) * 0.99999;
		}
		// move track center
		this.dx = cos;
		this.dy = sin;
		currentX += cos;
		currentY += sin;

		endTrackCenter.setLocation(currentX, currentY);
		endTrackAlpha = startTrackAlpha;

		//System.out.println("X = "+tmpX+" , Y = "+tmpY+" , Length = "+this.length);
		
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