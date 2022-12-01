package utils.ac3d;

import java.io.FileWriter;
import java.io.IOException;
import java.util.Vector;

public class Ac3dObject
{
	private int					linenum;
	private String				type;
	private String				name;
	private String				data;
	private String				texture;
	private double[]			texrep;
	private double[]			texoff;
	private Integer				subdiv;
	private Double				crease;
	private double[]			rot;
	private double[]			loc;
	private String				url;
	private Boolean				hidden;
	private Boolean				locked;
	private Boolean				folded;
	private Vector<double[]>	vertices	= new Vector<double[]>();
	private Vector<Ac3dSurface>	surfaces	= new Vector<Ac3dSurface>();
	private Vector<Ac3dObject>	kids		= new Vector<Ac3dObject>();

	Ac3dObject(String type, int linenum)
	{
		this.type = type;
		this.linenum = linenum;
	}

	public void write(FileWriter file) throws IOException
	{
		file.write("OBJECT " + type + "\n");

		if (name != null && !name.isEmpty())
		{
			file.write("name " + name + "\n");
		}

		if (texture != null && !texture.isEmpty())
		{
			file.write("texture " + texture + "\n");
		}

		if (texrep != null && texrep.length == 2)
		{
			file.write("texrep " + texrep[0] + " " + texrep[1] + "\n");
		}

		if (texoff != null && texoff.length == 2)
		{
			file.write("texoff " + texoff[0] + " " + texoff[1] + "\n");
		}

		if (rot != null && rot.length == 9)
		{
			file.write("rot " + rot[0] + " " + rot[1] + " " + rot[2] + " " + rot[3] + " " + rot[4] + " " + rot[5] + " " + rot[6] + " " + rot[7] + " " + rot[8] + "\n");
		}

		if (loc != null && loc.length == 3)
		{
			file.write("loc " + loc[0] + " " + loc[1] + " " + loc[2] + "\n");
		}

		if (crease != null)
		{
			file.write("crease " + crease + "\n");
		}

		if (subdiv != null)
		{
			file.write("subdiv " + subdiv + "\n");
		}

		if (hidden != null && hidden == true)
		{
			file.write("hidden\n");
		}

		if (locked != null && locked == true)
		{
			file.write("locked\n");
		}

		if (folded != null && folded == true)
		{
			file.write("folded\n");
		}

		if (data != null && !data.isEmpty())
		{
			file.write("data " + data.length() + "\n" + data + "\n");
		}

		if (vertices.size() > 0)
		{
			file.write("numvert " + vertices.size() + "\n");

			for (int i = 0; i < vertices.size(); i++)
			{
				double vertex[] = vertices.get(i);

				file.write(vertex[0] + " " + vertex[1] + " " + vertex[2] + "\n");
			}
		}

		if (surfaces.size() > 0)
		{
			file.write("numsurf " + surfaces.size() + "\n");

			for (int i = 0; i < surfaces.size(); i++)
			{
				Ac3dSurface surface = surfaces.get(i);

				surface.write(file);
			}
		}

		file.write("kids " + kids.size() + "\n");

		for (int i = 0; i < kids.size(); i++)
		{
			kids.get(i).write(file);
		}
	}

	public String getType()
	{
		return type;
	}

	public void setType(String type)
	{
		this.type = type;
	}

	public String getName()
	{
		return name;
	}

	public void setName(String name)
	{
		this.name = name;
	}

	public String getData()
	{
		return data;
	}

	public void setData(String data)
	{
		this.data = data;
	}

	public String getTexture()
	{
		return texture;
	}

	public void setTexture(String texture)
	{
		this.texture = texture;
	}

	public double[] getTexrep()
	{
		return texrep;
	}

	public void setTexrep(String[] tokens)
	{
		this.texrep = new double[2];

		texrep[0] = Double.parseDouble(tokens[1]);
		texrep[1] = Double.parseDouble(tokens[2]);
	}

	public void setTexrep(double[] texrep)
	{
		this.texrep = texrep;
	}

	public double[] getTexoff()
	{
		return texoff;
	}

	public void setTexoff(String[] tokens)
	{
		this.texoff = new double[2];

		texoff[0] = Double.parseDouble(tokens[1]);
		texoff[1] = Double.parseDouble(tokens[2]);
	}

	public void setTexoff(double[] texoff)
	{
		this.texoff = texoff;
	}

	public int getSubdiv()
	{
		return subdiv;
	}

	public void setSubdiv(String[] tokens)
	{
		this.subdiv = Integer.parseInt(tokens[1]);
	}

	public void setSubdiv(int subdiv)
	{
		this.subdiv = subdiv;
	}

	public double getCrease()
	{
		return crease;
	}

	public void setCrease(String[] tokens)
	{
		this.crease = Double.parseDouble(tokens[1]);
	}

	public void setCrease(double crease)
	{
		this.crease = crease;
	}

	public double[] getRot()
	{
		return rot;
	}

	public void setRot(String[] tokens)
	{
		rot = new double[9];

		for (int i = 0; i < 9; i++)
			rot[i] = Double.parseDouble(tokens[i + 1]);
	}

	public void setRot(double[] rot)
	{
		this.rot = rot;
	}

	public double[] getLoc()
	{
		return loc;
	}

	public void setLoc(String[] tokens)
	{
		loc = new double[3];

		for (int i = 0; i < 3; i++)
			loc[i] = Double.parseDouble(tokens[i + 1]);
	}

	public void setLoc(double[] loc)
	{
		this.loc = loc;
	}

	public String getUrl()
	{
		return url;
	}

	public void setUrl(String url)
	{
		this.url = url;
	}

	public Vector<double[]> getVertices()
	{
		return vertices;
	}

	public void setVertices(Vector<double[]> vertices)
	{
		this.vertices = vertices;
	}

	public void addVertex(String line)
	{
		addVertex(line.split("\\s+"));
	}

	public void addVertex(String[] tokens)
	{
		double vertex[] = new double[3];

		for (int i = 0; i < 3; i++)
			vertex[i] = Double.parseDouble(tokens[i]);

		vertices.add(vertex);
	}

	public Vector<Ac3dSurface> getSurfaces()
	{
		return surfaces;
	}

	public void setSurfaces(Vector<Ac3dSurface> surfaces)
	{
		this.surfaces = surfaces;
	}

	public void addSurface(Ac3dSurface surface)
	{
		surfaces.add(surface);
	}

	public Vector<Ac3dObject> getKids()
	{
		return kids;
	}

	public void setKids(Vector<Ac3dObject> kids)
	{
		this.kids = kids;
	}

	public void addKid(Ac3dObject kid)
	{
		kids.add(kid);
	}

	public Boolean getHidden()
	{
		return hidden;
	}

	public void setHidden(Boolean hidden)
	{
		this.hidden = hidden;
	}

	public Boolean getLocked()
	{
		return locked;
	}

	public void setLocked(Boolean locked)
	{
		this.locked = locked;
	}

	public Boolean getFolded()
	{
		return folded;
	}

	public void setFolded(Boolean folded)
	{
		this.folded = folded;
	}

	public void setSubdiv(Integer subdiv)
	{
		this.subdiv = subdiv;
	}

	public void setCrease(Double crease)
	{
		this.crease = crease;
	}

	public int getLinenum()
	{
		return linenum;
	}

	public void setLinenum(int linenum)
	{
		this.linenum = linenum;
	}
}
