/*
 *   Segment.java
 *   Created on 9 ??? 2005
 *
 *    The Segment.java is part of TrackEditor-0.6.0.
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

import java.awt.Graphics;
import java.awt.Rectangle;
import java.awt.event.ActionListener;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.util.Arrays;
import java.util.Vector;

import gui.EditorFrame;

/**
 * @author Patrice Espie , Charalampos Alexopoulos
 *
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */

public class Segment implements Cloneable
{
	private Vector<SegmentSideListener>	segmentListeners	= new Vector<SegmentSideListener>();

	// neighbours
	public Segment			previousShape;
	public Segment			nextShape;

	protected String		name					= "";
	protected String		comment					= null;

	protected SegmentSide	left					= new SegmentSide(false);
	protected SegmentSide	right					= new SegmentSide(true);
	//	 type
	protected String		type;
	protected int			count;
	public double			startTrackAlpha;
	public double			endTrackAlpha;
	public Point2D.Double	startTrackCenter		= new Point2D.Double();
	public Point2D.Double	endTrackCenter			= new Point2D.Double();
	public double			distFromCircuitStart;

	// All datas
	protected double		length					= Double.NaN;
	protected String		surface					= null;

	protected double		heightStartLeft			= Double.NaN;
	protected double		heightStartRight		= Double.NaN;
	protected double		heightEndLeft			= Double.NaN;
	protected double		heightEndRight			= Double.NaN;

	protected double		grade					= Double.NaN;
	protected double		bankingStart			= Double.NaN;
	protected double		bankingEnd				= Double.NaN;

	protected String		profil					= null;
	protected double		profilSteps				= Double.NaN;
	protected double		profilStepsLength		= Double.NaN;
	protected double		profilStartTangent		= Double.NaN;
	protected double		profilEndTangent		= Double.NaN;
	protected double		profilStartTangentLeft	= Double.NaN;
	protected double		profilEndTangentLeft	= Double.NaN;
	protected double		profilStartTangentRight	= Double.NaN;
	protected double		profilEndTangentRight	= Double.NaN;

	// shape to be drawn
	public Point2D.Double	points[];
	public Point2D.Double	trPoints[];

	// datas for fast 'draw' process
	int						xToDraw[]			= new int[4];
	int						yToDraw[]			= new int[4];

	double dx;
	double dy;

	public Segment()
	{
		this(null);
	}

	public Segment(String type)
	{
		this.type = type;
		setProperties();
	}

	public Segment(String name, String type)
	{
		this.name = name;
		this.type = type;
		setProperties();
	}

	public void set(Segment segment)
	{
		left.set(segment.left);
		right.set(segment.right);
		name = segment.name;
		type = segment.type;
		comment = segment.comment;
		length = segment.length;
		surface = segment.surface;
		heightStartLeft = segment.heightStartLeft;
		heightStartRight = segment.heightStartRight;
		heightEndLeft = segment.heightEndLeft;
		heightEndRight = segment.heightEndRight;
		grade = segment.grade;
		bankingStart = segment.bankingStart;
		bankingEnd = segment.bankingEnd;
		profil = segment.profil;
		profilSteps = segment.profilSteps;
		profilStepsLength = segment.profilStepsLength;
		profilStartTangent = segment.profilStartTangent;
		profilEndTangent = segment.profilEndTangent;
		profilStartTangentLeft = segment.profilStartTangentLeft;
		profilEndTangentLeft = segment.profilEndTangentLeft;
		profilStartTangentRight = segment.profilStartTangentRight;
		profilEndTangentRight = segment.profilEndTangentRight;
		points = Arrays.copyOf(segment.points, segment.points.length);
		trPoints = Arrays.copyOf(segment.trPoints, segment.trPoints.length);
		xToDraw = Arrays.copyOf(segment.xToDraw, segment.xToDraw.length);
		yToDraw = Arrays.copyOf(segment.yToDraw, segment.yToDraw.length);
		dx = segment.dx;
		dy = segment.dy;
	}

	/**
	 * @return Returns the left.
	 */
	public SegmentSide getLeft()
	{
		return left;
	}
	/**
	 * @param left
	 *            The left to set.
	 */
	public void setLeft(SegmentSide left)
	{
		this.left = left;
	}
	/**
	 * @return Returns the right.
	 */
	public SegmentSide getRight()
	{
		return right;
	}
	/**
	 * @param right
	 *            The right to set.
	 */
	public void setRight(SegmentSide right)
	{
		this.right = right;
	}
	/**
	 * @return Returns the name.
	 */
	public String getName()
	{
		int i = -1;
		try
		{
			i = Integer.parseInt(name);
		} catch (NumberFormatException e)
		{

		}

		if (name == null || name.equals("")) // || i > 0)
		{
			name = String.valueOf(count);
		}
		return name;
	}
	/**
	 * @param name
	 *            The name to set.
	 */
	public void setName(String name)
	{
		this.name = name;
	}
	
	public String getComment()
	{
		return comment;
	}

	public void setComment(String comment)
	{
		this.comment = comment;
	}

	/**
	 * @return Returns the surface.
	 */
	public String getSurface()
	{
		return surface;
	}
	/**
	 * @param surface
	 *            The surface to set.
	 */
	public void setSurface(String surface)
	{
		this.surface = surface;
	}
	public Segment copyTo(Segment shape)
	{
		shape.points = points.clone();
		shape.trPoints = trPoints.clone();
		shape.type = new String(type);
		shape.xToDraw = xToDraw.clone();
		shape.yToDraw = yToDraw.clone();

		return shape;
	}

	// adapted from: https://web.archive.org/web/20130126163405/http://geomalgorithms.com/a03-_inclusion.html
	// Copyright 2000 softSurfer, 2012 Dan Sunday
	private double isLeft(Point2D.Double P0, Point2D.Double P1, Point2D.Double P2)
	{
		return ((P1.x - P0.x) * (P2.y - P0.y) - (P2.x -  P0.x) * (P1.y - P0.y));
	}
	
	public boolean contains(Point2D.Double point)
	{
		if (points == null)
			return false;
		
		for (int i = 0; i < points.length; i += 4)
		{
			int count = 0;

			for (int j = 0; j < 4; j++)
			{
				int start = i + j;
				int next = i + ((j + 1) % 4);

				if (points[start].y <= point.y)
				{
					if (points[next].y > point.y)
					{
						if (isLeft(points[start], points[next], point) > 0)
							++count;
					}
				}
				else
				{
					if (points[next].y <= point.y)
					{
						if (isLeft(points[start], points[next], point) < 0)
							--count;
					}
				}
			}

			if (count != 0)
				return true;
		}

		return false;
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
		System.out.println("Segment.calcShape : Start ...");
	}

	public void draw(Graphics g, AffineTransform affineTransform)
	{
		if (points == null || points.length < 4)
			return;
		affineTransform.transform(points, 0, trPoints, 0, points.length);

		Rectangle clipBound = g.getClipBounds();
		int minX, maxX, minY, maxY;

		for (int i = 0; i < points.length; i += 4)
		{
			// clip calcul
			minX = Integer.MAX_VALUE;
			maxX = Integer.MIN_VALUE;
			minY = Integer.MAX_VALUE;
			maxY = Integer.MIN_VALUE;

			for (int j = 0; j < 4; j++)
			{
				if (minX > trPoints[i + j].x)
					minX = (int) (trPoints[i + j].x);

				if (maxX < trPoints[i + j].x)
					maxX = (int) (trPoints[i + j].x);

				if (minY > trPoints[i + j].y)
					minY = (int) (trPoints[i + j].y);

				if (maxY < trPoints[i + j].y)
					maxY = (int) (trPoints[i + j].y);
			}

			if (clipBound.intersects(minX, minY, maxX - minX, maxY - minY))
			{
				for (int j = 0; j < 4; j++)
				{
					xToDraw[j] = (int) (trPoints[i + j].x);
					yToDraw[j] = (int) (trPoints[i + j].y);
				}
				g.drawPolygon(xToDraw, yToDraw, 4);
			}
		}
	}

	public void drag(Point2D.Double dragDelta)
	{
	}

	/**
	 * @return Returns the heightStartLeft.
	 */
	public double getHeightStartLeft()
	{
		return heightStartLeft;
	}
	/**
	 * @param heightStartLeft
	 *            The heightStartLeft to set.
	 */
	public void setHeightStartLeft(double heightStartLeft)
	{
		this.heightStartLeft = heightStartLeft;
	}
	/**
	 * @return Returns the heightStartRight.
	 */
	public double getHeightStartRight()
	{
		return heightStartRight;
	}
	/**
	 * @param heightStart
	 *            The heightStartRight to set.
	 */
	public void setHeightStartRight(double heightStartRight)
	{
		this.heightStartRight = heightStartRight;
	}
	/**
	 * @return Returns the heightEndLeft.
	 */
	public double getHeightEndLeft()
	{
		return heightEndLeft;
	}
	/**
	 * @param heightEndLeft
	 *            The heightEndLeft to set.
	 */
	public void setHeightEndLeft(double heightEndLeft)
	{
		this.heightEndLeft = heightEndLeft;
	}
	/**
	 * @return Returns the heightEndRight.
	 */
	public double getHeightEndRight()
	{
		return heightEndRight;
	}
	/**
	 * @param heightEndRight
	 *            The heightEndRight to set.
	 */
	public void setHeightEndRight(double heightEndRight)
	{
		this.heightEndRight = heightEndRight;
	}
	/**
	 * @return Returns the grade.
	 */
	public double getGrade()
	{
		return grade;
	}
	/**
	 * @param grade
	 *            The grade to set.
	 */
	public void setGrade(double grade)
	{
		this.grade = grade;
	}
	/**
	 * @return Returns the bankingStart.
	 */
	public double getBankingStart()
	{
		return bankingStart;
	}
	/**
	 * @param bankingStart
	 *            The bankingStart to set.
	 */
	public void setBankingStart(double bankingStart)
	{
		this.bankingStart = bankingStart;
	}
	/**
	 * @return Returns the bankingEnd.
	 */
	public double getBankingEnd()
	{
		return bankingEnd;
	}
	/**
	 * @param bankingEnd
	 *            The bankingEnd to set.
	 */
	public void setBankingEnd(double bankingEnd)
	{
		this.bankingEnd = bankingEnd;
	}
	/**
	 * @return Returns the profil.
	 */
	public String getProfil()
	{
		return profil;
	}
	/**
	 * @param profil
	 *            The profil to set.
	 */
	public void setProfil(String profil)
	{
		this.profil = profil;
	}
	/**
	 * @return Returns the profilSteps.
	 */
	public double getProfilSteps()
	{
		return profilSteps;
	}
	/**
	 * @param profilSteps
	 *            The profilSteps to set.
	 */
	public void setProfilSteps(double profilSteps)
	{
		this.profilSteps = profilSteps;
	}
	/**
	 * @return Returns the profilStepsLength.
	 */
	public double getProfilStepsLength()
	{
		return profilStepsLength;
	}
	/**
	 * @param profilStepsLength
	 *            The profilStepsLength to set.
	 */
	public void setProfilStepsLength(double profilStepsLength)
	{
		this.profilStepsLength = profilStepsLength;
	}
	/**
	 * @return Returns the profilStartTangent.
	 */
	public double getProfilStartTangent()
	{
		return profilStartTangent;
	}
	/**
	 * @param profilStartTangent
	 *            The profilStartTangent to set.
	 */
	public void setProfilStartTangent(double profilStartTangent)
	{
		this.profilStartTangent = profilStartTangent;
	}
	/**
	 * @return Returns the profilEndTangent.
	 */
	public double getProfilEndTangent()
	{
		return profilEndTangent;
	}
	/**
	 * @param profilEndTangent
	 *            The profilEndTangent to set.
	 */
	public void setProfilEndTangent(double profilEndTangent)
	{
		this.profilEndTangent = profilEndTangent;
	}
	/**
	 * @return Returns the profilStartTangentLeft.
	 */
	public double getProfilStartTangentLeft()
	{
		return profilStartTangentLeft;
	}
	/**
	 * @param profilStartTangentLeft
	 *            The profilStartTangentLeft to set.
	 */
	public void setProfilStartTangentLeft(double profilStartTangentLeft)
	{
		this.profilStartTangentLeft = profilStartTangentLeft;
	}
	/**
	 * @return Returns the profilEndTangentLeft.
	 */
	public double getProfilEndTangentLeft()
	{
		return profilEndTangentLeft;
	}
	/**
	 * @param profilEndTangentLeft
	 *            The profilEndTangentLeft to set.
	 */
	public void setProfilEndTangentLeft(double profilEndTangentLeft)
	{
		this.profilEndTangentLeft = profilEndTangentLeft;
	}
	/**
	 * @return Returns the profilStartTangentRight.
	 */
	public double getProfilStartTangentRight()
	{
		return profilStartTangentRight;
	}
	/**
	 * @param profilStartTangentRight
	 *            The profilStartTangentRight to set.
	 */
	public void setProfilStartTangentRight(double profilStartTangentRight)
	{
		this.profilStartTangentRight = profilStartTangentRight;
	}
	/**
	 * @return Returns the profilEndTangentRight.
	 */
	public double getProfilEndTangentRight()
	{
		return profilEndTangentRight;
	}
	/**
	 * @param profilEndTangentRight
	 *            The profilEndTangentRight to set.
	 */
	public void setProfilEndTangentRight(double profilEndTangentRight)
	{
		this.profilEndTangentRight = profilEndTangentRight;
	}

	/**
	 * @return Returns the length.
	 */
	public double getLength()
	{
		return length;
	}
	/**
	 * @param length
	 *            The length to set.
	 */
	public void setLength(double length)
	{
		this.length = length;
	}

	/**
	 * @return Returns the type.
	 */
	public String getType()
	{
		return type;
	}
	/**
	 * @param type
	 *            The type to set.
	 */
	public void setType(String type)
	{
		this.type = type;
	}
	/**
	 * @return Returns the count.
	 */
	public int getCount()
	{
		return count;
	}
	/**
	 * @param count
	 *            The count to set.
	 */
	public void setCount(int count)
	{
		this.count = count;
	}
	/**
	 * @return Returns the previousShape.
	 */
	public Segment getPreviousShape()
	{
		return previousShape;
	}
	/**
	 * @param previousShape
	 *            The previousShape to set.
	 */
	public void setPreviousShape(Segment prev)
	{
		this.previousShape = prev;
	}
	/**
	 * @return Returns the nextShape.
	 */
	public Segment getNextShape()
	{
		return nextShape;
	}
	/**
	 * @param previousShape
	 *            The nextShape to set.
	 */
	public void setNextShape(Segment next)
	{
		this.nextShape = next;
	}

	/**
	 *
	 */
	private void setProperties()
	{

	}

	public synchronized void removeSideListener(ActionListener l)
	{

	}

	public synchronized void addSideListener(SegmentSideListener l)
	{
		Vector<SegmentSideListener> v = segmentListeners == null ? new Vector<SegmentSideListener>(2) : (Vector<SegmentSideListener>) segmentListeners.clone();
		if (!v.contains(l))
		{
			v.addElement(l);
			segmentListeners = v;
		}
	}

	public Object clone()
	{
		Segment s = null;
		try
		{
			s = (Segment) super.clone();
			s.left = (SegmentSide) this.left.clone();
			s.right = (SegmentSide) this.right.clone();
			s.name = this.name;
			s.type = this.type;
			s.comment = this.comment;
			s.length = this.length;
			s.surface = this.surface;
			s.heightStartLeft = this.heightStartLeft;
			s.heightStartRight = this.heightStartRight;
			s.heightEndLeft = this.heightEndLeft;
			s.heightEndRight = this.heightEndRight;
			s.grade = this.grade;
			s.bankingStart = this.bankingStart;
			s.bankingEnd = this.bankingEnd;
			s.profil = this.profil;
			s.profilSteps = this.profilSteps;
			s.profilStepsLength = this.profilStepsLength;
			s.profilStartTangent = this.profilStartTangent;
			s.profilEndTangent = this.profilEndTangent;
			s.profilStartTangentLeft = this.profilStartTangentLeft;
			s.profilEndTangentLeft = this.profilEndTangentLeft;
			s.profilStartTangentRight = this.profilStartTangentRight;
			s.profilEndTangentRight = this.profilEndTangentRight;
			s.points = Arrays.copyOf(points, points.length);
			s.trPoints = Arrays.copyOf(trPoints, trPoints.length);
			s.xToDraw = xToDraw.clone();
			s.yToDraw = yToDraw.clone();
			s.dx = dx;
			s.dy = dy;
		} catch (CloneNotSupportedException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return s; // return the clone
	}
    /**
     * @return Returns the points.
     */
    public Point2D.Double[] getPoints()
    {
        return points;
    }
    /**
     * @param points The points to set.
     */
    public void setPoints(Point2D.Double[] points)
    {
        this.points = points;
    }
    /**
     * @return Returns the dx.
     */
    public double getDx()
    {
        return dx;
    }
    /**
     * @return Returns the dy.
     */
    public double getDy()
    {
        return dy;
    }

	public String getValidSurface(EditorFrame editorFrame)
	{
		Segment previous = this;
		String surf;
		while (previous != null)
		{
			surf = surface;
			if (surf != null)
			{
				return surf;
			}
			previous = previous.previousShape;
		}

		surf = editorFrame.getTrackData().getMainTrack().getSurface();
		if (surf == null)
		{
			surf = MainTrack.DEFAULT_SURFACE;
		}
		return surf;
	}

    public double getValidProfilStepsLength(EditorFrame editorFrame)
    {
        Segment previous = this;
        double	length;
        while (previous != null)
		{
			length = profilStepsLength;
			if (!Double.isNaN(profilStepsLength))
			{
				return length;
			}
			previous = previous.previousShape;
		}

        length = editorFrame.getTrackData().getMainTrack().getProfilStepsLength();
        if (Double.isNaN(length))
        {
            length = MainTrack.DEFAULT_PROFIL_STEPS_LENGTH;
        }
        return length;
    }

    public double getValidLeftBarrierWidth(EditorFrame editorFrame)
    {
        Segment previous = this;
        double	value;

        // try to get missing attribute from previous segments first
        while (previous != null)
        {
            value = previous.left.barrierWidth;
            if (!Double.isNaN(value))
            {
                return value;
            }
            else
            {
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getLeft().getBarrierWidth();
        if (Double.isNaN(value))
    	{
        	String style = getValidLeftBarrierStyle(editorFrame);

        	switch (style)
        	{
        	case "fence":
        		value = SegmentSide.DEFAULT_BARRIER_FENCE_WIDTH;
        		break;
        	case "wall":
        		value = SegmentSide.DEFAULT_BARRIER_WALL_WIDTH;
        		break;
        	default:
        		value = Double.NaN;
        		break;
        	}
    	}

        return value;
    }

    public double getValidRightBarrierWidth(EditorFrame editorFrame)
    {
    	Segment previous = this;
        double	value;

    	// try to get missing attribute from previous segments first
        while (previous != null)
    	{
            value = previous.right.barrierWidth;
            if (!Double.isNaN(value))
    		{
                return value;
    		}
    		else
    		{
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getRight().getBarrierWidth();
        if (Double.isNaN(value))
    	{
        	String style = getValidRightBarrierStyle(editorFrame);

        	switch (style)
        	{
        	case "fence":
        		value = SegmentSide.DEFAULT_BARRIER_FENCE_WIDTH;
        		break;
        	case "wall":
        		value = SegmentSide.DEFAULT_BARRIER_WALL_WIDTH;
        		break;
        	default:
        		value = Double.NaN;
        		break;
        	}
    	}

        return value;
    }

    public double getValidLeftBarrierHeight(EditorFrame editorFrame)
    {
        Segment previous = this;
        double	value;

        // try to get missing attribute from previous segments first
        while (previous != null)
        {
            value = previous.left.barrierHeight;
            if (!Double.isNaN(value))
            {
                return value;
            }
            else
            {
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getLeft().getBarrierHeight();
        if (Double.isNaN(value))
    	{
        	String style = getValidLeftBarrierStyle(editorFrame);

        	switch (style)
        	{
        	case "fence":
        		value = SegmentSide.DEFAULT_BARRIER_FENCE_HEIGHT;
        		break;
        	case "wall":
        		value = SegmentSide.DEFAULT_BARRIER_WALL_HEIGHT;
        		break;
        	default:
        		value = Double.NaN;
        		break;
        	}
    	}

        return value;
    }

    public double getValidRightBarrierHeight(EditorFrame editorFrame)
    {
    	Segment previous = this;
        double	value;

    	// try to get missing attribute from previous segments first
        while (previous != null)
    	{
            value = previous.right.barrierHeight;
            if (!Double.isNaN(value))
    		{
                return value;
    		}
    		else
    		{
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getRight().getBarrierHeight();
        if (Double.isNaN(value))
    	{
        	String style = getValidRightBarrierStyle(editorFrame);

        	switch (style)
        	{
        	case "fence":
        		value = SegmentSide.DEFAULT_BARRIER_FENCE_HEIGHT;
        		break;
        	case "wall":
        		value = SegmentSide.DEFAULT_BARRIER_WALL_HEIGHT;
        		break;
        	default:
        		value = Double.NaN;
        		break;
        	}
    	}

        return value;
    }

    public String getValidLeftBarrierSurface(EditorFrame editorFrame)
    {
        Segment previous = this;
        String	value;

        // try to get missing attribute from previous segments first
        while (previous != null)
        {
            value = previous.left.barrierSurface;
            if (value != null && !value.isEmpty())
            {
                return value;
            }
            else
            {
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getLeft().getBarrierSurface();
        if (value == null || value.isEmpty())
    	{
        	String style = getValidLeftBarrierStyle(editorFrame);

        	switch (style)
        	{
        	case "fence":
        		value = SegmentSide.DEFAULT_BARRIER_FENCE_SURFACE;
        		break;
        	case "wall":
        		value = SegmentSide.DEFAULT_BARRIER_WALL_SURFACE;
        		break;
        	default:
        		value = null;
        		break;
        	}
    	}

        return value;
    }

    public String getValidRightBarrierSurface(EditorFrame editorFrame)
    {
    	Segment previous = this;
        String	value;

    	// try to get missing attribute from previous segments first
        while (previous != null)
    	{
            value = previous.right.barrierSurface;
            if (value != null && !value.isEmpty())
    		{
                return value;
    		}
    		else
    		{
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getRight().getBarrierSurface();
        if (value == null || value.isEmpty())
    	{
        	String style = getValidRightBarrierStyle(editorFrame);

        	switch (style)
        	{
        	case "fence":
        		value = SegmentSide.DEFAULT_BARRIER_FENCE_SURFACE;
        		break;
        	case "wall":
        		value = SegmentSide.DEFAULT_BARRIER_WALL_SURFACE;
        		break;
        	default:
        		value = null;
        		break;
        	}
    	}

        return value;
    }

    public String getValidLeftBarrierStyle(EditorFrame editorFrame)
    {
        Segment previous = this;
        String	value;

        // try to get missing attribute from previous segments first
        while (previous != null)
        {
            value = previous.left.barrierStyle;
            if (value != null && !value.isEmpty())
            {
                return value;
            }
            else
            {
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getLeft().getBarrierStyle();
        if (value == null || value.isEmpty())
    	{
            value = SegmentSide.DEFAULT_BARRIER_STYLE;
    	}

        return value;
    }

    public String getValidRightBarrierStyle(EditorFrame editorFrame)
    {
    	Segment previous = this;
        String	value;

    	// try to get missing attribute from previous segments first
        while (previous != null)
    	{
            value = previous.right.barrierStyle;
            if (value != null && !value.isEmpty())
    		{
                return value;
    		}
    		else
    		{
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getRight().getBarrierStyle();
        if (value == null || value.isEmpty())
    	{
            value = SegmentSide.DEFAULT_BARRIER_STYLE;
    	}

        return value;
    }

    public double getValidLeftBorderWidth(EditorFrame editorFrame)
    {
        Segment previous = this;
        double	value;

        // try to get missing attribute from previous segments first
        while (previous != null)
        {
            value = previous.left.borderWidth;
            if (!Double.isNaN(value))
            {
                return value;
            }
            else
            {
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getLeft().getBorderWidth();
        if (Double.isNaN(value))
    	{
        	String style = getValidLeftBorderStyle(editorFrame);

        	switch (style)
        	{
        	case "plan":
        		value = SegmentSide.DEFAULT_BORDER_PLAN_WIDTH;
        		break;
        	case "curb":
        		value = SegmentSide.DEFAULT_BORDER_CURB_WIDTH;
        		break;
        	case "wall":
        		value = SegmentSide.DEFAULT_BORDER_WALL_WIDTH;
        		break;
        	default:
        		value = Double.NaN;
        		break;
        	}
    	}

        return value;
    }

    public double getValidRightBorderWidth(EditorFrame editorFrame)
    {
    	Segment previous = this;
        double	value;

    	// try to get missing attribute from previous segments first
        while (previous != null)
    	{
            value = previous.right.borderWidth;
            if (!Double.isNaN(value))
    		{
                return value;
    		}
    		else
    		{
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getRight().getBorderWidth();
        if (Double.isNaN(value))
    	{
        	String style = getValidRightBorderStyle(editorFrame);

        	switch (style)
        	{
        	case "plan":
        		value = SegmentSide.DEFAULT_BORDER_PLAN_WIDTH;
        		break;
        	case "curb":
        		value = SegmentSide.DEFAULT_BORDER_CURB_WIDTH;
        		break;
        	case "wall":
        		value = SegmentSide.DEFAULT_BORDER_WALL_WIDTH;
        		break;
        	default:
        		value = Double.NaN;
        		break;
        	}
    	}

        return value;
    }

    public double getValidLeftBorderHeight(EditorFrame editorFrame)
    {
        Segment previous = this;
        double	value;

        // try to get missing attribute from previous segments first
        while (previous != null)
        {
            value = previous.left.borderHeight;
            if (!Double.isNaN(value))
            {
                return value;
            }
            else
            {
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getLeft().getBorderHeight();
        if (Double.isNaN(value))
    	{
        	String style = getValidLeftBorderStyle(editorFrame);

        	switch (style)
        	{
        	case "plan":
        		value = SegmentSide.DEFAULT_BORDER_PLAN_HEIGHT;
        		break;
        	case "curb":
        		value = SegmentSide.DEFAULT_BORDER_CURB_HEIGHT;
        		break;
        	case "wall":
        		value = SegmentSide.DEFAULT_BORDER_WALL_HEIGHT;
        		break;
        	default:
        		value = Double.NaN;
        		break;
        	}
    	}

        return value;
    }

    public double getValidRightBorderHeight(EditorFrame editorFrame)
    {
    	Segment previous = this;
        double	value;

    	// try to get missing attribute from previous segments first
        while (previous != null)
    	{
            value = previous.right.borderHeight;
            if (!Double.isNaN(value))
    		{
                return value;
    		}
    		else
    		{
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getRight().getBorderHeight();
        if (Double.isNaN(value))
    	{
        	String style = getValidRightBorderStyle(editorFrame);

        	switch (style)
        	{
        	case "plan":
        		value = SegmentSide.DEFAULT_BORDER_PLAN_HEIGHT;
        		break;
        	case "curb":
        		value = SegmentSide.DEFAULT_BORDER_CURB_HEIGHT;
        		break;
        	case "wall":
        		value = SegmentSide.DEFAULT_BORDER_WALL_HEIGHT;
        		break;
        	default:
        		value = Double.NaN;
        		break;
        	}
    	}

        return value;
    }

    public String getValidLeftBorderSurface(EditorFrame editorFrame)
    {
        Segment previous = this;
        String	value;

        // try to get missing attribute from previous segments first
        while (previous != null)
        {
            value = previous.left.borderSurface;
            if (value != null && !value.isEmpty())
            {
                return value;
            }
            else
            {
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getLeft().getBorderSurface();
        if (value == null || value.isEmpty())
    	{
        	String style = getValidLeftBorderStyle(editorFrame);

        	switch (style)
        	{
        	case "plan":
        		value = SegmentSide.DEFAULT_BORDER_PLAN_LEFT_SURFACE;
        		break;
        	case "curb":
        		value = SegmentSide.DEFAULT_BORDER_CURB_LEFT_SURFACE;
        		break;
        	case "wall":
        		value = SegmentSide.DEFAULT_BORDER_WALL_SURFACE;
        		break;
        	default:
        		value = null;
        		break;
        	}
    	}

        return value;
    }

    public String getValidRightBorderSurface(EditorFrame editorFrame)
    {
    	Segment previous = this;
        String	value;

    	// try to get missing attribute from previous segments first
        while (previous != null)
    	{
            value = previous.right.borderSurface;
            if (value != null && !value.isEmpty())
    		{
                return value;
    		}
    		else
    		{
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getRight().getBorderSurface();
        if (value == null || value.isEmpty())
    	{
        	String style = getValidRightBorderStyle(editorFrame);

        	switch (style)
        	{
        	case "plan":
        		value = SegmentSide.DEFAULT_BORDER_PLAN_RIGHT_SURFACE;
        		break;
        	case "curb":
        		value = SegmentSide.DEFAULT_BORDER_CURB_RIGHT_SURFACE;
        		break;
        	case "wall":
        		value = SegmentSide.DEFAULT_BORDER_WALL_SURFACE;
        		break;
        	default:
        		value = null;
        		break;
        	}
    	}

        return value;
    }

    public String getValidLeftBorderStyle(EditorFrame editorFrame)
    {
        Segment previous = this;
        String	value;

        // try to get missing attribute from previous segments first
        while (previous != null)
        {
            value = previous.left.borderStyle;
            if (value != null && !value.isEmpty())
            {
                return value;
            }
            else
            {
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getLeft().getBorderStyle();
        if (value == null || value.isEmpty())
    	{
            value = SegmentSide.DEFAULT_BORDER_STYLE;
    	}

        return value;
    }

    public String getValidRightBorderStyle(EditorFrame editorFrame)
    {
    	Segment previous = this;
        String	value;

    	// try to get missing attribute from previous segments first
        while (previous != null)
    	{
            value = previous.right.borderStyle;
            if (value != null && !value.isEmpty())
    		{
                return value;
    		}
    		else
    		{
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getRight().getBorderStyle();
        if (value == null || value.isEmpty())
    	{
            value = SegmentSide.DEFAULT_BORDER_STYLE;
    	}

        return value;
    }

    public double getValidLeftSideStartWidth(EditorFrame editorFrame)
    {
    	Segment previous = this;
		double	value;

    	// try to get missing attribute from previous segments first
        while (true)
    	{
            value = previous.left.sideStartWidth;
            if (!Double.isNaN(value))
    		{
                return value;
    		}
            previous = previous.previousShape;
            if (previous == null)
    		{
                break;
            }
            value = previous.left.sideEndWidth;
            if (!Double.isNaN(value))
            {
                return value;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getLeft().getSideStartWidth();
        if (Double.isNaN(value))
    	{
            value = SegmentSide.DEFAULT_SIDE_START_WIDTH;
    	}

        return value;
    }

    public double getValidLeftSideEndWidth(EditorFrame editorFrame)
    {
    	Segment previous = this;
		double	value;

    	// try to get missing attribute from previous segments first
        while (previous != null)
    	{
            value = previous.left.sideEndWidth;
            if (!Double.isNaN(value))
    		{
                return value;
            }
            value = previous.left.sideStartWidth;
            if (!Double.isNaN(value))
    		{
                return value;
    		}
            previous = previous.previousShape;
        }

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getLeft().getSideEndWidth();
        if (Double.isNaN(value))
    	{
            value = SegmentSide.DEFAULT_SIDE_END_WIDTH;
    	}

        return value;
    }

    public double getValidRightSideStartWidth(EditorFrame editorFrame)
    {
    	Segment previous = this;
		double	value;

    	// try to get missing attribute from previous segments first
        while (true)
    	{
            value = previous.right.sideStartWidth;
            if (!Double.isNaN(value))
    		{
                return value;
    		}
            previous = previous.previousShape;
            if (previous == null)
    		{
                break;
            }
            value = previous.right.sideEndWidth;
            if (!Double.isNaN(value))
            {
                return value;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getRight().getSideStartWidth();
        if (Double.isNaN(value))
        {
            value = SegmentSide.DEFAULT_SIDE_START_WIDTH;
    	}

        return value;
    }

    public double getValidRightSideEndWidth(EditorFrame editorFrame)
    {
    	Segment previous = this;
		double	value;

    	// try to get missing attribute from previous segments first
        while (previous != null)
    	{
            value = previous.right.sideEndWidth;
            if (!Double.isNaN(value))
    		{
                return value;
    		}
            value = previous.right.sideStartWidth;
            if (!Double.isNaN(value))
    		{
                return value;
    		}
            previous = previous.previousShape;
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getRight().getSideEndWidth();
        if (Double.isNaN(value))
    	{
            value = SegmentSide.DEFAULT_SIDE_END_WIDTH;
    	}

        return value;
	}
    
    public String getValidLeftSideSurface(EditorFrame editorFrame)
    {
        Segment previous = this;
        String	value;

        // try to get missing attribute from previous segments first
        while (previous != null)
        {
            value = previous.left.sideSurface;
            if (value != null && !value.isEmpty())
            {
                return value;
            }
            else
            {
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getLeft().getSideSurface();
        if (value == null || value.isEmpty())
    	{
            value = SegmentSide.DEFAULT_SIDE_SURFACE;
    	}

        return value;
    }

    public String getValidRightSideSurface(EditorFrame editorFrame)
    {
    	Segment previous = this;
        String	value;

    	// try to get missing attribute from previous segments first
        while (previous != null)
    	{
            value = previous.right.sideSurface;
            if (value != null && !value.isEmpty())
    		{
                return value;
    		}
    		else
    		{
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getRight().getSideSurface();
        if (value == null || value.isEmpty())
    	{
            value = SegmentSide.DEFAULT_SIDE_SURFACE;
    	}

        return value;
    }

    public String getValidLeftSideBankingType(EditorFrame editorFrame)
    {
        Segment previous = this;
        String	value;

        // try to get missing attribute from previous segments first
        while (previous != null)
        {
            value = previous.left.sideBankingType;
            if (value != null && !value.isEmpty())
            {
                return value;
            }
            else
            {
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getLeft().getSideBankingType();
        if (value == null || value.isEmpty())
    	{
            value = SegmentSide.DEFAULT_SIDE_BANKING_TYPE;
    	}

        return value;
    }

    public String getValidRightSideBankingType(EditorFrame editorFrame)
    {
    	Segment previous = this;
        String	value;

    	// try to get missing attribute from previous segments first
        while (previous != null)
    	{
            value = previous.right.sideBankingType;
            if (value != null && !value.isEmpty())
    		{
                return value;
    		}
    		else
    		{
                previous = previous.previousShape;
    		}
    	}

    	// get it from main track when all else fails
        value = editorFrame.getTrackData().getMainTrack().getRight().getSideBankingType();
        if (value == null || value.isEmpty())
    	{
            value = SegmentSide.DEFAULT_SIDE_BANKING_TYPE;
    	}

        return value;
    }
   
    public void inheritProperties(Segment previousShape)
    {
		setSurface(previousShape.getSurface());
		if (!Double.isNaN(previousShape.getHeightEndLeft()))
		{
			setHeightStartLeft(previousShape.getHeightEndLeft());
			setHeightEndLeft(previousShape.getHeightEndLeft());
		}

		if (!Double.isNaN(previousShape.getHeightEndRight()))
		{
			setHeightStartRight(previousShape.getHeightEndRight());
			setHeightEndRight(previousShape.getHeightEndRight());
		}

		setGrade(previousShape.getGrade());
		if (!Double.isNaN(bankingEnd))
		{
			setBankingStart(previousShape.getBankingEnd());
			setBankingEnd(previousShape.getBankingEnd());
		}

		setProfil(previousShape.getProfil());

		getLeft().setSideStartWidth(previousShape.getLeft().getSideEndWidth());
		getLeft().setSideEndWidth(previousShape.getLeft().getSideEndWidth());
		getLeft().setSideSurface(previousShape.getLeft().getSideSurface());
		getLeft().setSideBankingType(previousShape.getLeft().getSideBankingType());

		getLeft().setBarrierWidth(previousShape.getLeft().getBarrierWidth());
		getLeft().setBarrierHeight(previousShape.getLeft().getBarrierHeight());
		getLeft().setBarrierSurface(previousShape.getLeft().getBarrierSurface());
		getLeft().setBarrierStyle(previousShape.getLeft().getBarrierStyle());

		getLeft().setBorderWidth(previousShape.getLeft().getBorderWidth());
		getLeft().setBorderHeight(previousShape.getLeft().getBorderHeight());
		getLeft().setBorderSurface(previousShape.getLeft().getBorderSurface());
		getLeft().setBorderStyle(previousShape.getLeft().getBorderStyle());

		getRight().setSideStartWidth(previousShape.getRight().getSideEndWidth());
		getRight().setSideEndWidth(previousShape.getRight().getSideEndWidth());
		getRight().setSideSurface(previousShape.getRight().getSideSurface());
		getRight().setSideBankingType(previousShape.getRight().getSideBankingType());

		getRight().setBarrierWidth(previousShape.getRight().getBarrierWidth());
		getRight().setBarrierHeight(previousShape.getRight().getBarrierHeight());
		getRight().setBarrierSurface(previousShape.getRight().getBarrierSurface());
		getRight().setBarrierStyle(previousShape.getRight().getBarrierStyle());

		getRight().setBorderWidth(previousShape.getRight().getBorderWidth());
		getRight().setBorderHeight(previousShape.getRight().getBorderHeight());
		getRight().setBorderSurface(previousShape.getRight().getBorderSurface());
		getRight().setBorderStyle(previousShape.getRight().getBorderStyle());
    }

	public void unoptimize(EditorFrame editorFrame)
	{
		surface = getValidSurface(editorFrame);
		profilStepsLength = getValidProfilStepsLength(editorFrame);

		// TODO: elevation, banking and other stuff needs to be implemented

		left.setBarrierWidth(getValidLeftBarrierWidth(editorFrame));
		left.setBarrierHeight(getValidLeftBarrierHeight(editorFrame));
		left.setBarrierSurface(getValidLeftBarrierSurface(editorFrame));
		left.setBarrierStyle(getValidLeftBarrierStyle(editorFrame));

		left.setBorderWidth(getValidLeftBorderWidth(editorFrame));
		left.setBorderHeight(getValidLeftBorderHeight(editorFrame));
		left.setBorderSurface(getValidLeftBorderSurface(editorFrame));
		left.setBorderStyle(getValidLeftBorderStyle(editorFrame));

		left.setSideStartWidth(getValidLeftSideStartWidth(editorFrame));
		left.setSideEndWidth(getValidLeftSideEndWidth(editorFrame));
		left.setSideSurface(getValidLeftSideSurface(editorFrame));
		left.setSideBankingType(getValidLeftSideBankingType(editorFrame));

		right.setBarrierWidth(getValidRightBarrierWidth(editorFrame));
		right.setBarrierHeight(getValidRightBarrierHeight(editorFrame));
		right.setBarrierSurface(getValidRightBarrierSurface(editorFrame));
		right.setBarrierStyle(getValidRightBarrierStyle(editorFrame));

		right.setBorderWidth(getValidRightBorderWidth(editorFrame));
		right.setBorderHeight(getValidRightBorderHeight(editorFrame));
		right.setBorderSurface(getValidRightBorderSurface(editorFrame));
		right.setBorderStyle(getValidRightBorderStyle(editorFrame));

		right.setSideStartWidth(getValidRightSideStartWidth(editorFrame));
		right.setSideEndWidth(getValidRightSideEndWidth(editorFrame));
		right.setSideSurface(getValidRightSideSurface(editorFrame));
		right.setSideBankingType(getValidRightSideBankingType(editorFrame));
	}

	public void dump(String indent)
	{
		System.out.println(indent + "Segment");
		System.out.println(indent + "  previousShape : " + (previousShape != null ? previousShape.name : "null"));
		System.out.println(indent + "  nextShape     : " + (nextShape != null ? nextShape.name : "null"));
		System.out.println(indent + "  name          : " + name);
		System.out.println(indent + "  type          : " + type);
		System.out.println(indent + "  count         : " + count);
		if (points != null)
		{
			System.out.println(indent + "  points        : " + points.length);
			for (int i = 0; i < points.length; i++)
			{
				System.out.println(indent + "    points[" + i + "] " + points[i].x + ", " + points[i].y);
			}
		}
		else
		{
			System.out.println(indent + "  points        : null");
		}
		if (trPoints != null)
		{
			System.out.println(indent + "  trPoints      : " + trPoints.length);
			for (int i = 0; i < trPoints.length; i++)
			{
				System.out.println(indent + "    trPoints[" + i + "] " + trPoints[i].x + ", " + trPoints[i].y);
			}
		}
		else
		{
			System.out.println(indent + "  trPoints      : null");
		}
		if (xToDraw != null)
		{
			System.out.println(indent + "  xToDraw      : " + xToDraw.length);
			for (int i = 0; i < xToDraw.length; i++)
			{
				System.out.println(indent + "    xToDraw[" + i + "] " + xToDraw[i]);
			}
		}
		else
		{
			System.out.println(indent + "  xToDraw      : null");
		}
		if (yToDraw != null)
		{
			System.out.println(indent + "  yToDraw      : " + yToDraw.length);
			for (int i = 0; i < yToDraw.length; i++)
			{
				System.out.println(indent + "    yToDraw[" + i + "] " + yToDraw[i]);
			}
		}
		else
		{
			System.out.println(indent + "  yToDraw      : null");
		}
		System.out.println(indent + "  dx           : " + dx);
		System.out.println(indent + "  dy           : " + dy);
	}
}
