package utils.circuit;

import java.awt.Color;
import java.awt.geom.Point2D;

public class ObjShapeObject extends Segment
{
	static public final double	defaultSize	= 10;

	// color
	private int					rgb;
	private Color				color;
	
	// position
	private int					imageX;
	private int					imageY;
	
	// shape info
	private Point2D.Double		location;
	private double				width		= defaultSize;
	private double				height		= defaultSize;
	
	public ObjShapeObject(int rgb, int imageX, int imageY)
	{
		super("object");
		this.rgb = rgb & 0x00ffffff;
		this.color = new Color((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
		this.imageX = imageX;
		this.imageY = imageY;
	}
			
	public int getRGB() {
		return rgb;
	}

	public void setRGB(int rgb) {
		this.rgb = rgb;
	}

	public Color getColor() {
		return color;
	}

	public void setColor(Color color) {
		this.color = color;
	}

	public int getImageX() {
		return imageX;
	}

	public void setImageX(int imageX) {
		this.imageX = imageX;
	}

	public int getImageY() {
		return imageY;
	}

	public void setImageY(int imageY) {
		this.imageY = imageY;
	}

	public Point2D.Double getLocation() {
		return location;
	}

	public void setLocation(Point2D.Double location) {
		this.location = location;
	}

	public void calcShape(Point2D.Double location)
	{
		this.location = location;
		
		if (points == null)
		{
			points = new Point2D.Double[4]; // 4 points in 2D
			for (int i = 0; i < points.length; i++)
				points[i] = new Point2D.Double();

			trPoints = new Point2D.Double[4];
			for (int i = 0; i < trPoints.length; i++)
				trPoints[i] = new Point2D.Double();
		}

		points[0].x = this.location.getX() - width / 2;
		points[0].y = this.location.getY() - height / 2;

		points[1].x = this.location.getX() + width / 2;
		points[1].y = this.location.getY() - height / 2;

		points[2].x = this.location.getX() + width / 2;
		points[2].y = this.location.getY() + height / 2;

		points[3].x = this.location.getX() - width / 2;
		points[3].y = this.location.getY() + height / 2;
	}
}
