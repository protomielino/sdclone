package utils.circuit;

public class Sector
{
	private String	name;
	private double	distanceFromStart;
	
	public String getName()
	{
		return name;
	}
	public void setName(String name)
	{
		this.name = name;
	}

	public double getDistanceFromStart()
	{
		return distanceFromStart;
	}
	public void setDistanceFromStart(double distanceFromStart)
	{
		this.distanceFromStart = distanceFromStart;
	}

	public void dump(String indent)
    {
		System.out.println(indent + "Sector");
		System.out.println(indent + "  name              : " + name);
		System.out.println(indent + "  distanceFromStart : " + distanceFromStart);
    }
}
