package utils.circuit;

import java.awt.Graphics;
import java.awt.Rectangle;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.util.Vector;

public class ObjShapeRelief extends Segment
{
	private static int			POINT_RADIUS	= 4;
	private static int			POINT_DIAMETER	= POINT_RADIUS * 2;

	public enum Type {	Interior, Exterior };
	public enum LineType { Polygon, Polyline };

	private Type				type;
	private LineType			lineType;
	private Vector<double[]>	vertices = new Vector<double[]>();

	public ObjShapeRelief(Type type, LineType lineType, Vector<double[]> vertices)
	{
		super("relief");
		this.type = type;
		this.lineType = lineType;
		this.vertices = vertices;
	}

	public boolean isInterior()
	{
		return type == Type.Interior;
	}
	public boolean isExterior()
	{
		return type == Type.Exterior;
	}
	public boolean isPolygon()
	{
		return lineType == LineType.Polygon;
	}
	public boolean isPolyline()
	{
		return lineType == LineType.Polyline;
	}	
	public Vector<double[]> getVertices() {
		return vertices;
	}

	public void setVertices(Vector<double[]> vertices) {
		this.vertices = vertices;
	}

	public void calcShape(Rectangle2D.Double boundingRectangle)
	{
		if (points == null)
		{
			xToDraw	= new int[vertices.size()];
			yToDraw = new int[vertices.size()];

			points = new Point2D.Double[vertices.size()];
			for (int i = 0; i < points.length; i++)
				points[i] = new Point2D.Double();

			trPoints = new Point2D.Double[vertices.size()];
			for (int i = 0; i < trPoints.length; i++)
				trPoints[i] = new Point2D.Double();
		}

		for (int i = 0; i < points.length; i++)
		{
			points[i].x = vertices.get(i)[0] + boundingRectangle.x;
			points[i].y = -vertices.get(i)[2] + boundingRectangle.y;
		}
	}

	public void draw(Graphics g, AffineTransform affineTransform)
	{
		if (points == null)
			return;

		affineTransform.transform(points, 0, trPoints, 0, points.length);

		Rectangle clipBound = g.getClipBounds();
		int minX = Integer.MAX_VALUE;
		int maxX = Integer.MIN_VALUE;
		int minY = Integer.MAX_VALUE;
		int maxY = Integer.MIN_VALUE;

		for (int i = 0; i < points.length; i++)
		{
			if (minX > trPoints[i].x)
				minX = (int) (trPoints[i].x);

			if (maxX < trPoints[i].x)
				maxX = (int) (trPoints[i].x);

			if (minY > trPoints[i].y)
				minY = (int) (trPoints[i].y);

			if (maxY < trPoints[i].y)
				maxY = (int) (trPoints[i].y);
		}

		if (clipBound.intersects(minX, minY, maxX - minX, maxY - minY))
		{
			for (int i = 0; i < points.length; i++)
			{
				xToDraw[i] = (int) trPoints[i].x;
				yToDraw[i] = (int) trPoints[i].y;

				g.fillOval(xToDraw[i] - POINT_RADIUS, yToDraw[i] - POINT_RADIUS, POINT_DIAMETER, POINT_DIAMETER);
			}

			if (lineType == LineType.Polygon)
			{
				g.drawPolygon(xToDraw, yToDraw, points.length);
			}
			else
			{
				g.drawPolyline(xToDraw, yToDraw, points.length);
			}
		}
	}

	public boolean contains(Point2D.Double point)
	{
		if (points == null)
			return false;

		for (int i = 0; i < points.length; i++)
		{
			if (points[i].distance(point) <= POINT_RADIUS)
			{
				return true;
			}
		}

		return false;
	}

	public Point2D.Double getPoint2D(Point2D.Double point)
	{
		if (points == null)
			return null;

		for (int i = 0; i < points.length; i++)
		{
			if (points[i].distance(point) <= POINT_RADIUS)
			{
				return points[i];
			}
		}

		return null;
	}

	public void setPoint2D(int index, Point2D.Double point, Rectangle2D.Double boundingRectangle)
	{
		if (points == null)
			return;

		if (index < 0 || index >= points.length)
			return;

		points[index].x = point.x;
		points[index].y = point.y;

		vertices.get(index)[0] = point.x - boundingRectangle.x;
		vertices.get(index)[2] = -(point.y - boundingRectangle.y);
	}

	public int getPointIndex(Point2D.Double point)
	{
		if (points == null)
			return -1;

		for (int i = 0; i < points.length; i++)
		{
			if (points[i].distance(point) <= POINT_RADIUS)
			{
				return i;
			}
		}

		return -1;
	}

	public void dump(String indent)
	{
		super.dump(indent);

		System.out.println(indent + "  ObjShapeRelief");	
		System.out.println(indent + "    type        : " + (isInterior() ? "Interior" : "Exterior"));
		System.out.println(indent + "    lineType    : " + (isPolyline() ? "Polyline" : "Polygon"));
		System.out.println(indent + "    vertices    : " + vertices.size());
		for (int i = 0; i < vertices.size(); i++)
		{
			System.out.println(indent + "      vertex[" + i + "] : " + vertices.get(i)[0] + ", " + vertices.get(i)[1] + ", " + vertices.get(i)[2]);
		}
	}
}
