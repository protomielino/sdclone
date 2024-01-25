package utils.circuit;

import java.awt.geom.Point2D;

public class Point3D extends Point2D.Double
{
	public double z;

	public Point3D()
	{
		super();
	}

	public Point3D(double x, double y, double z)
	{
		super(x, y);
		this.z = z;
	}

    public String toString()
    {
        return "Point3D["+x+", "+y+", "+z+"]";
    }
}
