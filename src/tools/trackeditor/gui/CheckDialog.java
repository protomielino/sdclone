package gui;

import java.awt.Point;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.WindowEvent;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.Vector;

import javax.swing.JDialog;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.WindowConstants;

import utils.Editor;
import utils.TrackData;
import utils.circuit.EnvironmentMapping;
import utils.circuit.Segment;
import utils.circuit.Surface;
import utils.circuit.TrackObject;

public class CheckDialog extends JDialog
{
	private final String 		sep 			= System.getProperty("file.separator");
	private JScrollPane			scrollPane		= null;
	private JTextArea			textArea		= null;
	private EditorFrame			editorFrame		= null;
	private Vector<Surface>		defaultSurfaces	= null;
	private Vector<TrackObject>	defaultObjects	= null;
	private String				dataDirectory	= null;
	private TrackData			trackData		= null;

	public CheckDialog(EditorFrame editorFrame)
	{
		super();
		this.editorFrame = editorFrame;
		defaultSurfaces = editorFrame.getDefaultSurfaces();
		defaultObjects = editorFrame.getDefaultObjects();
		dataDirectory = editorFrame.getDataDirectory();
		trackData = editorFrame.getTrackData();
		initialize();
	}

	/**
	 * This method initializes this
	 * 
	 * @return void
	 */
	private void initialize()
	{	
		this.setTitle("Check Track");
		this.setSize(editorFrame.getProject().getCheckDialogWidth(), editorFrame.getProject().getCheckDialogHeight());
		this.setResizable(true);
		Point p = new Point();
		p.x = editorFrame.getProject().getCheckDialogX();
		p.y = editorFrame.getProject().getCheckDialogY();
		this.setLocation(p);
		
		textArea = new JTextArea();
	    textArea.setLineWrap(false);
	    textArea.setEditable(false);
	    textArea.setVisible(true);
	    
	    scrollPane = new JScrollPane (textArea);
	    scrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
	    scrollPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
		
	    add(scrollPane);
	    
	    addComponentListener(new ComponentAdapter()
	    {
	    	public void componentShown(ComponentEvent e)
	    	{
	    		checkSurfaces();
	    		checkObjects();
	    		checkTerrainGeneration();
	    		checkEnvironmentMapping();
			
	    		textArea.append("Checking complete!");	
	    	}
	    });
	    this.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
	}

	private void checkEnvironmentMapping()
	{
		Vector<EnvironmentMapping>	envMaps = trackData.getGraphic().getEnvironmentMapping();
		
		for (int i = 0; i < envMaps.size(); i++)
		{
			String mapImage = envMaps.get(i).getEnvMapImage();

			if (mapImage != null && !mapImage.isEmpty())
			{
				File file = new File(Editor.getProperties().getPath() + sep + mapImage);

				if (!file.exists())
				{
					textArea.append("Environment Map " + envMaps.get(i).getName() + " file " + mapImage + " not found\n");
				}
			}
			else
			{
				textArea.append("Environment Map " + envMaps.get(i).getName() + " missing image file\n");
			}
		}
	}
	
	private void checkTerrainGeneration()
	{
		String reliefFile = trackData.getGraphic().getTerrainGeneration().getReliefFile();

		if (reliefFile != null && !reliefFile.isEmpty())
		{
			File file = findObjectFile(reliefFile);

			if (file == null)
			{
				textArea.append("Terrain Generation relief file " + reliefFile + " not found\n");
			}

			try
			{
				BufferedReader br = new BufferedReader(new FileReader(file));
				String line = "";

				while ((line = br.readLine()) != null)
				{
					if (line.startsWith("name"))
					{
						String name = line.substring(line.indexOf("\"") + 1, line.lastIndexOf("\""));

						if (!(name.equals("interior") || name.equals("exterior")))
						{
							textArea.append("Terrain Generation relief file " + reliefFile + " invalid\n");
							break;
						}
					}
				}
				br.close();
			} 
			catch (Exception e)
			{
				e.printStackTrace();
			}
		}

		String elevationFile = trackData.getGraphic().getTerrainGeneration().getElevationMap();

		if (elevationFile != null && !elevationFile.isEmpty())
		{
			File file = new File(Editor.getProperties().getPath() + sep + elevationFile);

			if (!file.exists())
			{
				textArea.append("Terrain Generation elevation file " + elevationFile + " not found\n");
			}
		}	
	}

	private File findObjectFile(String object)
	{
		File file = new File(Editor.getProperties().getPath() + sep + object);

		if (!file.exists())
		{
			file = new File(dataDirectory + sep + "data" + sep + "objects" + sep + object);
			if (!file.exists())
			{
				return null;
			}
		}

		return file;
	}
	private void checkTrackObject(TrackObject trackObject, String type)
	{
		String	object = trackObject.getObject();

		if (object == null || object.isEmpty())
		{
			textArea.append(type + " object " + object + " missing model\n");
			return;
		}

		File file = findObjectFile(object);
		
		if (file == null)
		{
			textArea.append(type + " object " + trackObject.getName() + " model " + object + " not found\n");
			return;
		}
		
		try
		{
			BufferedReader br = new BufferedReader(new FileReader(file));
		    String line = "";

		    while ((line = br.readLine()) != null)
		    {
		        if (line.startsWith("texture"))
		        {
		        	String texture = line.substring(line.indexOf("\"") + 1, line.lastIndexOf("\""));

		        	file = findTextureFile(texture);

		        	if (file == null)
		        	{
		        		textArea.append(type + " object " + trackObject.getName() + " model " + object + " texture " + texture + " not found\n");
		        	}

					if (texture.endsWith(".rgb"))
					{
						textArea.append(type + " object " + trackObject.getName() + " model " + object + " texture " + texture + " should be converted to png format\n");
					}
		        }
		    }
		    br.close();
		} 
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
	private void checkObjects()
	{
		// check for duplicate object names
		for (int i = 0; i < trackData.getObjects().size(); i++)
		{
			String name = trackData.getObjects().get(i).getName();
			
			if (name == null || name.isEmpty())
			{
				textArea.append("Track object " + (i + 1) + " missing name\n");
			}
			else
			{
				for (int j = i + 1; j < trackData.getObjects().size(); j++)
				{
					if (name.equals(trackData.getObjects().get(j).getName()))
					{
						textArea.append("Track object " + (i + 1) + " " + name + " has same name as Track object " + (j + 1) + " " + trackData.getObjects().get(j).getName() + "\n");						
					}
				}

				for (int j = 0; j < defaultObjects.size(); j++)
				{
					if (name.equals(defaultObjects.get(j).getName()))
					{
						textArea.append("Track object " + (i + 1) + " " + name + " has same name as Default object " + (j + 1) + " " + defaultObjects.get(j).getName() + "\n");						
					}
				}
			}
		}

		// check for duplicate colors
		for (int i = 0; i < trackData.getObjects().size(); i++)
		{
			String name = trackData.getObjects().get(i).getName();
			int color = trackData.getObjects().get(i).getColor();
			
			if (color == Integer.MAX_VALUE)
			{
				textArea.append("Track object " + (i + 1) + " missing color\n");
			}
			else
			{
				for (int j = i + 1; j < trackData.getObjects().size(); j++)
				{
					if (color == trackData.getObjects().get(j).getColor())
					{
						textArea.append("Track object " + (i + 1) + " " + name + " has same color as Track object " + (j + 1) + " " + trackData.getObjects().get(j).getName() + "\n");						
					}
				}

				for (int j = 0; j < defaultObjects.size(); j++)
				{
					if (color == defaultObjects.get(j).getColor())
					{
						textArea.append("Track object " + (i + 1) + " " + name + " has same color as Default object " + (j + 1) + " " + defaultObjects.get(j).getName() + "\n");						
					}
				}
			}
		}

		for (int i = 0; i < trackData.getObjects().size(); i++)
		{
			checkTrackObject(trackData.getObjects().get(i), "Track");
		}
		for (int i = 0; i < defaultObjects.size(); i++)
		{
			checkTrackObject(defaultObjects.get(i), "Default");
		}
	}
	private File findTextureFile(String texture)
	{
		File file = new File(Editor.getProperties().getPath() + sep + texture);

		if (!file.exists())
		{
			if (dataDirectory != null)
			{
				file = new File(dataDirectory + sep + "data" + sep + "textures" + sep + texture);
				if (!file.exists())
				{
					return null;
				}
			}
		}
		return file;
	}
	private void checkSurface(String surface, String description)
	{
		if (surface == null)
			return;

		for (int i = 0; i < trackData.getSurfaces().size(); i++)
		{
			if (trackData.getSurfaces().get(i).getName().equals(surface))
			{
				String texture = trackData.getSurfaces().get(i).getTextureName();
				if (texture == null || texture.isEmpty())
				{
					textArea.append(description + " surface " + surface + " missing texture\n");
				}
				else
				{
					File textureFile = findTextureFile(texture);
					if (textureFile == null)
					{
						textArea.append(description + " surface " + surface + " texture " + texture + " not found\n");
					}

					if (texture.endsWith(".rgb"))
					{
						textArea.append(description + " surface " + surface + " texture " + texture + " should be converted to png format\n");
					}
				}

				texture = trackData.getSurfaces().get(i).getBumpName();
				if (texture != null && !texture.isEmpty())
				{
					File textureFile = findTextureFile(texture);
					if (textureFile == null)
					{
						textArea.append(description + " surface " + surface + " Bump texture " + texture + " not found\n");
					}

					if (texture.endsWith(".rgb"))
					{
						textArea.append(description + " surface " + surface + " Bump texture " + texture + " should be converted to png format\n");
					}
				}

				texture = trackData.getSurfaces().get(i).getRacelineName();
				if (texture != null && !texture.isEmpty())
				{
					File textureFile = findTextureFile(texture);
					if (textureFile == null)
					{
						textArea.append(description + " surface " + surface + " Raceline texture " + texture + " not found\n");
					}

					if (texture.endsWith(".rgb"))
					{
						textArea.append(description + " surface " + surface + " Raceline texture " + texture + " should be converted to png format\n");
					}
				}

				return;
			}
		}
		for (int i = 0; i < defaultSurfaces.size(); i++)
		{
			if (defaultSurfaces.get(i).getName().equals(surface))
			{
				String texture = defaultSurfaces.get(i).getTextureName();
				if (texture == null || texture.isEmpty())
				{
					textArea.append(description + " surface " + surface + " missing texture\n");
				}
				else
				{
					File textureFile = findTextureFile(texture);
					if (textureFile == null)
					{
						textArea.append(description + " surface " + surface + " texture " + texture + " not found\n");
					}

					if (texture.endsWith(".rgb"))
					{
						textArea.append(description + " surface " + surface + " texture " + texture + " should be converted to png format\n");
					}
				}

				texture = defaultSurfaces.get(i).getBumpName();
				if (texture != null && !texture.isEmpty())
				{
					File textureFile = findTextureFile(texture);
					if (textureFile == null)
					{
						textArea.append(description + " surface " + surface + " Bump texture " + texture + " not found\n");
					}

					if (texture.endsWith(".rgb"))
					{
						textArea.append(description + " surface " + surface + " Bump texture " + texture + " should be converted to png format\n");
					}
				}

				texture = defaultSurfaces.get(i).getRacelineName();
				if (texture != null && !texture.isEmpty())
				{
					File textureFile = findTextureFile(texture);
					if (textureFile == null)
					{
						textArea.append(description + " surface " + surface + " Raceline texture " + texture + " not found\n");
					}

					if (texture.endsWith(".rgb"))
					{
						textArea.append(description + " surface " + surface + " Raceline texture " + texture + " should be converted to png format\n");
					}
				}

				return;
			}
		}
		
		textArea.append(description + " surface " + surface + " not found\n");
	}
	private void checkSurfaces()
	{
		// check for duplicate surface names
		for (int i = 0; i < trackData.getSurfaces().size(); i++)
		{
			String name = trackData.getSurfaces().get(i).getName();
			
			if (name == null || name.isEmpty())
			{
				textArea.append("Track surface " + (i + 1) + " missing name\n");
			}
			else
			{
				for (int j = i + 1; j < trackData.getSurfaces().size(); j++)
				{
					if (name.equals(trackData.getSurfaces().get(j).getName()))
					{
						textArea.append("Track surface " + (i + 1) + " " + name + " has same name as Track surface " + (j + 1) + " " + trackData.getSurfaces().get(i).getName() + "\n");						
					}
				}
				
				for (int j = 0; j < defaultSurfaces.size(); j++)
				{
					if (name.equals(defaultSurfaces.get(j).getName()))
					{
						textArea.append("Track surface " + (i + 1) + " " + name + " has same name as Default surface " + (j + 1) + " " + defaultSurfaces.get(i).getName() + "\n");						
					}
				}
			}
		}

		checkSurface(trackData.getMainTrack().getSurface(), "Main Track");
		checkSurface(trackData.getMainTrack().getLeft().getBorderSurface(), "Main Track Left Border");
		checkSurface(trackData.getMainTrack().getLeft().getSideSurface(), "Main Track Left Side");
		checkSurface(trackData.getMainTrack().getLeft().getBarrierSurface(), "Main Track Left Barrier");
		checkSurface(trackData.getMainTrack().getRight().getBorderSurface(), "Main Track Right Border");
		checkSurface(trackData.getMainTrack().getRight().getSideSurface(), "Main Track Right Side");
		checkSurface(trackData.getMainTrack().getRight().getBarrierSurface(), "Main Track Right Barrier");
		for (int i = 0; i < trackData.getSegments().size(); i++)
		{
			Segment segment = trackData.getSegments().get(i);
			
			checkSurface(segment.getSurface(), "Segment " + segment.getName() + " Track");
			checkSurface(segment.getLeft().getBorderSurface(), "Segment " + segment.getName() + " Left Border");
			checkSurface(segment.getLeft().getSideSurface(), "Segment " + segment.getName() + " Left Side");
			checkSurface(segment.getLeft().getBarrierSurface(), "Segment " + segment.getName() + " Left Barrier");
			checkSurface(segment.getRight().getBorderSurface(), "Segment " + segment.getName() + " Right Border");
			checkSurface(segment.getRight().getSideSurface(), "Segment " + segment.getName() + " Right Side");
			checkSurface(segment.getRight().getBarrierSurface(), "Segment " + segment.getName() + " Right Barrier");
		}
		checkSurface(trackData.getGraphic().getTerrainGeneration().getSurface(), "Terrain");
	}

	protected void processWindowEvent(WindowEvent e)
	{
		super.processWindowEvent(e);
		if (e.getID() == WindowEvent.WINDOW_CLOSING)
		{
			editorFrame.getProject().setCheckDialogX(this.getX());
			editorFrame.getProject().setCheckDialogY(this.getY());
			editorFrame.getProject().setCheckDialogWidth(this.getWidth());
			editorFrame.getProject().setCheckDialogHeight(this.getHeight());
		}
	}
}
