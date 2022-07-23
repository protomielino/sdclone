package utils;

import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

import javax.swing.DefaultListCellRenderer;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JList;

import gui.EditorFrame;
import utils.circuit.Surface;

public class SurfaceComboBox extends JComboBox<String>
{
	private EditorFrame		editorFrame;
	private Vector<String>	surfaceVector;

	private String 			sep	= System.getProperty("file.separator");

	public SurfaceComboBox(EditorFrame editorFrame, Vector<String> surfaceVector)
	{
		super(surfaceVector);
		
		this.editorFrame = editorFrame;
		this.surfaceVector = surfaceVector;
		
		initialize();
	}

	private void initialize()
	{
		ComboboxToolTipRenderer	renderer = new ComboboxToolTipRenderer();
		renderer.setTooltips(getSurfaceImages());
		setRenderer(renderer);
	}

	private class ComboboxToolTipRenderer extends DefaultListCellRenderer
	{
		List<String> tooltips;

		@Override
		public JComponent getListCellRendererComponent(JList list, Object value,
			int index, boolean isSelected, boolean cellHasFocus)
		{
			JComponent comp = (JComponent) super.getListCellRendererComponent(
				list, value, index, isSelected, cellHasFocus);

			if (-1 < index && null != value && null != tooltips)
			{
				list.setToolTipText(tooltips.get(index));
			}
			return comp;
		}

		public void setTooltips(List<String> tooltips)
		{
			this.tooltips = tooltips;
		}
	};

	private List<String> getSurfaceImages()
	{
		List<String> tooltips = new ArrayList<String>();
		Vector<Surface>	surfaces = editorFrame.getTrackData().getSurfaces();
		for (int i = 0; i < surfaceVector.size(); i++)
		{
			String tooltipText = null;
			String surfaceName = surfaceVector.get(i);
			for (int j = 0; j < surfaces.size(); j++)
			{
				Surface surface = surfaces.get(j);					
				if (surface.getName().equals(surfaceName))
				{
					String textureName = surface.getTextureName();
					if (textureName != null && !textureName.isEmpty())
					{
						String fileName = Editor.getProperties().getPath() + sep + textureName;
						File file = new File(fileName);							
						if (file.canRead())
						{
							try
							{
								URL url = file.toURI().toURL();
								tooltipText = "<html><img src=" + url + "></html>";
							}
							catch (MalformedURLException e)
							{
								System.out.println("tooltipText " + tooltipText);
							}
						}
					}
					break;
				}
			}
			tooltips.add(tooltipText);
		}
		return tooltips;
	}
}
