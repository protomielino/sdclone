package gui.properties;

public class GraphicObjectData
{
	String	name;
	Integer	color;
	Double  trackX;
	Double  trackY;
	Double	orientation;

	GraphicObjectData(String name, Integer color, double trackX, double trackY, double orientation)
	{
		this.name = name;
		this.color = color;
		this.trackX = trackX;
		this.trackY = trackY;
		this.orientation = orientation;
	}
}
