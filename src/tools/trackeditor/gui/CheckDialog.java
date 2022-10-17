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
import utils.SegmentVector;
import utils.TrackData;
import utils.circuit.Curve;
import utils.circuit.EnvironmentMapping;
import utils.circuit.Pits;
import utils.circuit.Segment;
import utils.circuit.Surface;
import utils.circuit.TrackLight;
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
	    		checkTrackHeight();
	    		checkSurfaces();
	    		checkObjects();
	    		checkTerrainGeneration();
	    		checkEnvironmentMapping();
	    		checkTrackLights();
	    		checkGraphic();
	    		checkPits();
	    		checkTrack();
			
	    		textArea.append("Checking complete!");	
	    	}
	    });
	    this.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
	}

	private void checkTrack()
	{
		SegmentVector segments = trackData.getSegments();

		for (int i = 0; i < segments.size(); i++)
		{
			Segment segment = segments.get(i);

			if (!segment.getType().equals("str"))
			{
				Curve curve = (Curve) segment;
				double startWidth = trackData.getMainTrack().getWidth() / 2.0;
				double endWidth = startWidth;

				if (segment.getType().equals("rgt"))
				{
					startWidth += curve.getValidRightBorderWidth(editorFrame);
					endWidth = startWidth;
					
					startWidth += curve.getValidRightSideStartWidth(editorFrame);
					endWidth += curve.getValidRightSideEndWidth(editorFrame);
				}
				else
				{
					startWidth += curve.getValidLeftBorderWidth(editorFrame);
					endWidth = startWidth;
					
					startWidth += curve.getValidLeftSideStartWidth(editorFrame);
					endWidth += curve.getValidLeftSideEndWidth(editorFrame);
				}

				if (startWidth > curve.getRadiusStart())
				{
					textArea.append("Segment " + curve.getName() + " Distance to barrier: " + startWidth + " greater than start radius: " + curve.getRadiusStart() + "\n");
				}
				if (endWidth > curve.getRadiusEnd())
				{
					textArea.append("Segment " + curve.getName() + " Distance to barrier: " + endWidth + " greater than end radius: " + curve.getRadiusEnd() + "\n");
				}
			}
		}
	}

	private void checkPits()
	{
		Pits pits = trackData.getMainTrack().getPits();

		if (pits == null)
			return;

		boolean noSegments = !hasText(pits.getEntry()) && !hasText(pits.getStart()) && !hasText(pits.getEnd())
				&& !hasText(pits.getExit());

		if (!noSegments)
		{
			SegmentVector segments = trackData.getSegments();

			if (!hasText(pits.getEntry()))
				textArea.append("Missing pit entry\n");
			else if (segments.getSegmentFromName(pits.getEntry()) == null)
				textArea.append("Invalid pit entry: " + pits.getEntry() + "\n");

			if (!hasText(pits.getStart()))
				textArea.append("Missing pit start\n");
			else if (segments.getSegmentFromName(pits.getStart()) == null)
				textArea.append("Invalid pit start: " + pits.getStart() + "\n");

			if (!hasText(pits.getEnd()))
				textArea.append("Missing pit end\n");
			else if (segments.getSegmentFromName(pits.getEnd()) == null)
				textArea.append("Invalid pit end: " + pits.getEnd() + "\n");

			if (!hasText(pits.getExit()))
				textArea.append("Missing pit exit\n");
			else if (segments.getSegmentFromName(pits.getExit()) == null)
				textArea.append("Invalid pit exit: " + pits.getExit() + "\n");

			if (!hasText(pits.getSide()))
				textArea.append("Missing pit side\n");
			else if (!(pits.getSide().equals("left") || pits.getSide().equals("right")))
				textArea.append("Invalid pit side: " + pits.getSide() + "\n");
		}
	}

	private void checkTrackHeight()
	{
		SegmentVector segments = trackData.getSegments();

		double width = trackData.getMainTrack().getWidth();

		double firstHeightStartLeft = Double.NaN;
		double firstHeightStartRight = Double.NaN;
		double previousHeightEndLeft = Double.NaN;
		double previousHeightEndRight = Double.NaN;
		
		for (int i = 0; i < segments.size(); i++)
		{
			Segment segment = segments.get(i);
			
			String	segmentInfo = "Segment " + segment.getName() + " : ";

			double heightStartLeft = segment.getHeightStartLeft();
			boolean hasHeightStartLeft = !Double.isNaN(heightStartLeft);

			double heightStartRight = segment.getHeightStartRight();			
			boolean hasHeightStartRight = !Double.isNaN(heightStartRight);
			
			boolean hasHeightStart = hasHeightStartLeft && hasHeightStartRight;
			boolean hasBankingStartFromHeights = hasHeightStart && heightStartLeft != heightStartRight;

			double bankingStart = segment.getBankingStart();
			boolean hasBankingStart = !Double.isNaN(bankingStart);

			// Track should start at an elevation of 0.0.
			if (i == 0)
			{
				firstHeightStartLeft = heightStartLeft;
				firstHeightStartRight = heightStartRight;

				if (hasHeightStartLeft && hasHeightStartRight)
				{
					double centerHeight = (heightStartLeft + heightStartRight) / 2.0;

					if (centerHeight != 0.0)
					{
						// disabled because many tracks don't start at 0 because of banking 
						// or because they use the actual elevation
						//textArea.append(segmentInfo + "Track height at start should be 0. Actual height: " + centerHeight +"\n");
					}
				}
			}

			if (hasBankingStart && hasBankingStartFromHeights)
			{
				textArea.append(segmentInfo + "Banking start angle and banking from heights\n");

				double bankingFromHeights = Math.atan2(heightStartLeft - heightStartRight, width) * 180.0 / Math.PI;

				if (bankingStart != bankingFromHeights)
				{
					textArea.append(segmentInfo + "Banking start: " + bankingStart + " doesn't match banking from heights: " + bankingFromHeights + "\n");							
				}
			}

			double heightEndLeft = segment.getHeightEndLeft();
			boolean hasHeightEndLeft = !Double.isNaN(heightEndLeft);

			double heightEndRight = segment.getHeightEndRight();			
			boolean hasHeightEndRight = !Double.isNaN(heightEndRight);

			boolean hasHeightEnd = hasHeightEndLeft && hasHeightEndRight;
			boolean hasBankingEndFromHeights = hasHeightEnd && heightEndLeft != heightEndRight;

			double bankingEnd = segment.getBankingEnd();
			boolean hasBankingEnd = !Double.isNaN(bankingEnd);

			if (hasBankingEnd && hasBankingEndFromHeights)
			{
				textArea.append(segmentInfo + "Banking end angle and banking from heights\n");
				
				double bankingFromHeights = Math.atan2(heightEndLeft - heightEndRight, width) * 180.0 / Math.PI;
		
				if (bankingEnd != bankingFromHeights)
				{
					textArea.append(segmentInfo + "Banking end: " + bankingEnd + " doesn't match banking from heights: " + bankingFromHeights + "\n");							
				}
			}

			double grade = segment.getGrade();
			boolean hasGrade = !Double.isNaN(grade);

			if (hasGrade && hasHeightEnd)
			{
				if (hasBankingEndFromHeights)
				{
					textArea.append(segmentInfo + "Grade and end banking from heights\n");
				}
				else
				{
					textArea.append(segmentInfo + "Grade and end height\n");
				}
			}
			
			Segment previous = segment.previousShape;
			Segment next = segment.nextShape;
			
			if (previous != null && hasHeightStart)
			{
				if (!Double.isNaN(previousHeightEndLeft) && previousHeightEndLeft != heightStartLeft)
				{
					textArea.append(segmentInfo + "Previous height end left : " + previousHeightEndLeft + " doesn't match " + heightStartLeft + "\n");
				}
				
				if (!Double.isNaN(previousHeightEndRight) && previousHeightEndRight != heightStartRight)
				{
					textArea.append(segmentInfo + "Previous height end right : " + previousHeightEndRight + " doesn't match " + heightStartRight + "\n");
				}
			}
			else if (next == null && hasHeightEnd)
			{
				if (!Double.isNaN(firstHeightStartLeft) && firstHeightStartLeft != heightEndLeft)
				{
					textArea.append(segmentInfo + "Height end left : " + heightEndLeft + " doesn't match " + firstHeightStartLeft + "\n");
				}
				
				if (!Double.isNaN(firstHeightStartRight) && firstHeightStartRight != heightEndRight)
				{
					textArea.append(segmentInfo + "Height end right : " + heightEndRight + " doesn't match " + firstHeightStartRight + "\n");
				}
			}
			
			previousHeightEndLeft = heightEndLeft;
			previousHeightEndRight = heightEndRight;
		}
	}

	private void checkGraphic()
	{
		String image = trackData.getGraphic().getBackgroundImage();
		if (hasText(image))
			checkTexture("Graphic Background Image ", image);
	}
	
	private void checkTrackLights()
	{
		Vector<TrackLight> lightData = trackData.getTrackLights();

		for (int i = 0; i < lightData.size(); i++)
		{
			TrackLight light = lightData.get(i);

			String texture = light.getTextureOn();
			checkTexture("Track Lights " + light.getName() + " Texture On ", texture);

			texture = light.getTextureOff();
			checkTexture("Track Lights " + light.getName() + " Texture Off ", texture);
		}
	}
	private void checkEnvironmentMapping()
	{
		Vector<EnvironmentMapping>	envMaps = trackData.getGraphic().getEnvironmentMapping();
		
		for (int i = 0; i < envMaps.size(); i++)
		{
			String texture = envMaps.get(i).getEnvMapImage();
			checkTexture("Environment Map " + envMaps.get(i).getName(), texture);
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
			else
			{
				try
				{
					BufferedReader br = new BufferedReader(new FileReader(file));
					String line = "";

					while ((line = br.readLine()) != null)
					{
						if (line.startsWith("data"))
						{
							String name = br.readLine();

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
	private Boolean hasText(String text)
	{
		return !(text == null || text.trim().isEmpty());
	}
	private void checkTrackObject(TrackObject trackObject, String type)
	{
		String	object = trackObject.getObject();

		if (!hasText(object))
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
		        	checkTexture(type + " object " + trackObject.getName() + " model " + object, texture);
		        }
		    }
		    br.close();
		} 
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
	private void checkTexture(String description, String texture)
	{
		Vector<String> messages = new Vector<String>();
		if (!hasText(texture))
		{
			messages.add(" missing texture\n");
		}
		else
		{
			File textureFile = findTextureFile(texture);
			if (textureFile == null)
			{
				messages.add(" texture " + texture + " not found\n");
			}

			if (texture.endsWith(".rgb"))
			{
				messages.add(" texture " + texture + " should be converted to png format\n");
			}
			
			if (texture.contains("/") || texture.contains("\\"))
			{
				messages.add(" texture " + texture + " should remove path\n");
			}
		}
    	for (int i = 0; i < messages.size(); i++)
    	{
    		textArea.append(description + messages.get(i));
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
		if (surface == null || surface.isEmpty())
			return;
		
		String message = description + " surface " + surface;

		for (int i = 0; i < trackData.getSurfaces().size(); i++)
		{
			if (trackData.getSurfaces().get(i).getName().equals(surface))
			{
				String texture = trackData.getSurfaces().get(i).getTextureName();
				checkTexture(message, texture);

				texture = trackData.getSurfaces().get(i).getBumpName();
				if (hasText(texture))
					checkTexture(message + " Bump ", texture);

				texture = trackData.getSurfaces().get(i).getRacelineName();
				if (hasText(texture))
					checkTexture(message + " Raceline ", texture);

				return;
			}
		}
		for (int i = 0; i < defaultSurfaces.size(); i++)
		{
			if (defaultSurfaces.get(i).getName().equals(surface))
			{
				String texture = defaultSurfaces.get(i).getTextureName();
				checkTexture(message, texture);

				texture = defaultSurfaces.get(i).getBumpName();
				if (hasText(texture))
					checkTexture(message + " Bump ", texture);

				texture = defaultSurfaces.get(i).getRacelineName();
				if (hasText(texture))
					checkTexture(message + " Raceline ", texture);

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
			
			if (!hasText(name))
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
