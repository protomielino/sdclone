package gui.properties;

public class GraphicObjectData
{
	public String	name;
	public Integer	color;
	public Double	trackX;
	public Double	trackY;
	public Double	orientation;
	public Double	height;

	GraphicObjectData(String name, Integer color, double trackX, double trackY, double orientation, double height)
	{
		this.name = name;
		this.color = color;
		this.trackX = trackX;
		this.trackY = trackY;
		this.orientation = orientation;
		this.height = height;
	}
}
