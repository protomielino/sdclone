/*
 *   TrackProperties.java
 *   Created on 27 ??? 2005
 *
 *    The TrackProperties.java is part of TrackEditor-0.3.1.
 *
 *    TrackEditor-0.3.1 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    TrackEditor-0.3.1 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with TrackEditor-0.3.1; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
package gui.properties;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Arrays;
import java.util.Collections;
import java.util.Vector;

import javax.swing.DefaultComboBoxModel;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTabbedPane;
import javax.swing.JTextField;

import gui.EditorFrame;
import utils.Editor;
import utils.circuit.SegmentSide;
import utils.circuit.Surface;
/**
 * @author babis
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class TrackProperties extends PropertyPanel
{
	private JLabel				widthLabel					= new JLabel();
	private JTextField			widthTextField				= new JTextField();
	private JLabel				surfaceLabel				= new JLabel();
	private JComboBox<String>	surfaceComboBox				= null;
	private JLabel				profilStepsLengthLabel		= new JLabel();
	private JTextField			profilStepsLengthTextField	= new JTextField();
	private JLabel				racelineWidthscaleLabel		= new JLabel();
	private JTextField			racelineWidthscaleTextField	= new JTextField();
	private JLabel				racelineIntLabel			= new JLabel();
	private JTextField			racelineIntTextField		= new JTextField();
	private JLabel				racelineExtLabel			= new JLabel();
	private JTextField			racelineExtTextField		= new JTextField();
	private JTabbedPane			tabbedPane					= null;

	private String[]			roadSurfaceItems			=
	{"asphalt-lines", "asphalt-l-left", "asphalt-l-right",
     "asphalt-l-both", "asphalt-pits", "asphalt", "dirt", "dirt-b", "asphalt2", "road1", "road1-pits",
     "road1-asphalt", "asphalt-road1", "b-road1", "b-road1-l2", "b-road1-l2p", "concrete", "concrete2",
     "concrete3", "b-asphalt", "b-asphalt-l1", "b-asphalt-l1p", "asphalt2-lines", "asphalt2-l-left",
     "asphalt2-l-right", "asphalt2-l-both", "grass", "grass3", "grass5", "grass6", "grass7", "gravel", "sand3",
     "sand", "curb-5cm-r", "curb-5cm-l", "curb-l", "tar-grass3-l", "tar-grass3-r", "tar-sand", "b-road1-grass6",
     "b-road1-grass6-l2", "b-road1-gravel-l2", "b-road1-sand3", "b-road1-sand3-l2", "b-asphalt-grass7",
     "b-asphalt-grass7-l1", "b-asphalt-grass6", "b-asphalt-grass6-l1", "b-asphalt-sand3", "b-asphalt-sand3-l1",
     "barrier", "barrier2", "barrier-turn", "barrier-grille", "wall", "wall2", "tire-wall"};
	private Vector<String>		roadSurfaceVector			= new Vector<String>(Arrays.asList(roadSurfaceItems));

	private String[]			styleItems					= {"none", "plan", "wall", "fence", "curb"};
	
	private String[]			borderSurfaceItems			=
	{"curb-5cm-r", "curb-5cm-l", "curb-l", "tar-grass3-l",
	 "tar-grass3-r", "tar-sand", "b-road1-grass6", "b-road1-grass6-l2", "b-road1-gravel-l2", "b-road1-sand3",
	 "b-road1-sand3-l2", "b-asphalt-grass7", "b-asphalt-grass7-l1", "b-asphalt-grass6", "b-asphalt-grass6-l1",
	 "b-asphalt-sand3", "b-asphalt-sand3-l1", "grass", "grass3", "grass5", "grass6", "grass7", "gravel",
	 "sand3", "sand", "asphalt-lines", "asphalt-l-left", "asphalt-l-right", "asphalt-l-both", "asphalt-pits",
	 "asphalt", "dirt", "dirt-b", "asphalt2", "road1", "road1-pits", "road1-asphalt", "asphalt-road1",
	 "b-road1", "b-road1-l2", "b-road1-l2p", "concrete", "concrete2", "concrete3", "b-asphalt", "b-asphalt-l1",
	 "b-asphalt-l1p", "asphalt2-lines", "asphalt2-l-left", "asphalt2-l-right", "asphalt2-l-both", "barrier",
	 "barrier2", "barrier-turn", "barrier-grille", "wall", "wall2", "tire-wall"};
	private Vector<String>		borderSurfaceVector			= new Vector<String>(Arrays.asList(borderSurfaceItems));
	private String[]			sideSurfaceItems			=
	{"grass", "grass3", "grass5", "grass6", "grass7", "gravel",
	 "sand3", "sand", "asphalt-lines", "asphalt-l-left", "asphalt-l-right", "asphalt-l-both", "asphalt-pits",
	 "asphalt", "dirt", "dirt-b", "asphalt2", "road1", "road1-pits", "road1-asphalt", "asphalt-road1",
	 "b-road1", "b-road1-l2", "b-road1-l2p", "concrete", "concrete2", "concrete3", "b-asphalt", "b-asphalt-l1",
	 "b-asphalt-l1p", "asphalt2-lines", "asphalt2-l-left", "asphalt2-l-right", "asphalt2-l-both", "curb-5cm-r",
	 "curb-5cm-l", "curb-l", "tar-grass3-l", "tar-grass3-r", "tar-sand", "b-road1-grass6", "b-road1-grass6-l2",
	 "b-road1-gravel-l2", "b-road1-sand3", "b-road1-sand3-l2", "b-asphalt-grass7", "b-asphalt-grass7-l1",
	 "b-asphalt-grass6", "b-asphalt-grass6-l1", "b-asphalt-sand3", "b-asphalt-sand3-l1", "barrier", "barrier2",
	 "barrier-turn", "barrier-grille", "wall", "wall2", "tire-wall"};
	private Vector<String>		sideSurfaceVector			= new Vector<String>(Arrays.asList(sideSurfaceItems));
	private String[]			barrierSurfaceItems			=
	{"barrier", "barrier2", "barrier-turn", "barrier-grille",
	 "wall", "wall2", "tire-wall", "asphalt-lines", "asphalt-l-left", "asphalt-l-right", "asphalt-l-both",
	 "asphalt-pits", "asphalt", "dirt", "dirt-b", "asphalt2", "road1", "road1-pits", "road1-asphalt",
	 "asphalt-road1", "b-road1", "b-road1-l2", "b-road1-l2p", "concrete", "concrete2", "concrete3", "b-asphalt",
	 "b-asphalt-l1", "b-asphalt-l1p", "asphalt2-lines", "asphalt2-l-left", "asphalt2-l-right",
	 "asphalt2-l-both", "curb-5cm-r", "curb-5cm-l", "curb-l", "tar-grass3-l", "tar-grass3-r", "tar-sand",
	 "b-road1-grass6", "b-road1-grass6-l2", "b-road1-gravel-l2", "b-road1-sand3", "b-road1-sand3-l2",
	 "b-asphalt-grass7", "b-asphalt-grass7-l1", "b-asphalt-grass6", "b-asphalt-grass6-l1", "b-asphalt-sand3",
	 "b-asphalt-sand3-l1", "grass", "grass3", "grass5", "grass6", "grass7", "gravel", "sand3", "sand"};
	private Vector<String>		barrierSurfaceVector		= new Vector<String>(Arrays.asList(barrierSurfaceItems));

	/**
	 *
	 */
	public TrackProperties(EditorFrame frame)
	{
		super(frame);
		initialize();
	}

	/**
	 * This method initializes this
	 *
	 * @return void
	 */
	private void initialize()
	{
        this.setLayout(null);
        this.setBorder(javax.swing.BorderFactory.createEtchedBorder(javax.swing.border.EtchedBorder.LOWERED));

		addLabel(this, 0, widthLabel, "Width", 140);
		addLabel(this, 1, surfaceLabel, "Surface", 140);
		addLabel(this, 2, profilStepsLengthLabel, "Profil Steps Length", 140);
		addLabel(this, 3, racelineWidthscaleLabel, "Raceline Width Scale", 140);
		addLabel(this, 4, racelineIntLabel, "Raceline Int", 140);
		addLabel(this, 5, racelineExtLabel, "Raceline Ext", 140);

		addTextField(this, 0, widthTextField, Editor.getProperties().getMainTrack().getWidth(), 150, 50);

        this.add(getSurfaceComboBox(), null);

		addTextField(this, 2, profilStepsLengthTextField, Editor.getProperties().getMainTrack().getProfilStepsLength(), 150, 50);
		addTextField(this, 3, racelineWidthscaleTextField, Editor.getProperties().getMainTrack().getRacelineWidthscale(), 150, 50);
		addTextField(this, 4, racelineIntTextField, Editor.getProperties().getMainTrack().getRacelineInt(), 150, 50);
		addTextField(this, 5, racelineExtTextField, Editor.getProperties().getMainTrack().getRacelineExt(), 150, 50);

		this.add(getTabbedPane(), null);

		addDefaultSurfaces(roadSurfaceVector);
		addDefaultSurfaces(sideSurfaceVector);
		addDefaultSurfaces(borderSurfaceVector);
		addDefaultSurfaces(barrierSurfaceVector);
	}

	private void addDefaultSurfaces(Vector<String> surfaceVector)
	{
        Vector<Surface> surfaces = Editor.getProperties().getSurfaces();
        for (int i = 0; i < surfaces.size(); i++)
        {
			String surface = surfaces.elementAt(i).getName();
			if (surface != null)
			{
				boolean found = false;
				for (int j = 0; j < surfaceVector.size(); j++)
				{
					if (surfaceVector.elementAt(i).equals(surfaces.elementAt(i).getName()))
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					surfaceVector.add(surface);
				}
			}
        }
		Collections.sort(surfaceVector);
	}

	private void addSurface(Vector<String> surfaceVector, String surface)
	{
		// add this surface if it's not found in default list
		if (surface != null)
		{
			boolean found = false;
			for (int i = 0; i < surfaceVector.size(); i++)
			{
				if (surfaceVector.elementAt(i).equals(surface))
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				surfaceVector.add(surface);
				Collections.sort(surfaceVector);
			}
		}
	}

	/**
	 * This method initializes surfaceComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	private JComboBox<String> getSurfaceComboBox()
	{
		if (surfaceComboBox == null)
		{
			surfaceComboBox = new JComboBox<String>();
			surfaceComboBox.setBounds(150, 37, 180, 23);
			String surface = Editor.getProperties().getMainTrack().getSurface();
			addSurface(roadSurfaceVector, surface);
			surfaceComboBox.setModel(new DefaultComboBoxModel<String>(roadSurfaceVector));
			surfaceComboBox.setSelectedItem(surface);
		}
		return surfaceComboBox;
	}

	/**
	 * This method initializes tabbedPane
	 *
	 * @return javax.swing.JTabbedPane
	 */
	private JTabbedPane getTabbedPane()
	{
		if (tabbedPane == null)
		{
			tabbedPane = new JTabbedPane();
			tabbedPane.setTabLayoutPolicy(JTabbedPane.SCROLL_TAB_LAYOUT);
			tabbedPane.setBounds(10, 172, 460, 370);

			tabbedPane.addTab("Left", null, new SidePanel(Editor.getProperties().getMainTrack().getLeft()), null);
			tabbedPane.addTab("Right", null, new SidePanel(Editor.getProperties().getMainTrack().getRight()), null);
		}
		return tabbedPane;
	}

	private class SidePanel extends JPanel
	{
		private SegmentSide 		side;
		private JLabel				sideStartWidthLabel			= new JLabel();
		private JTextField 			sideStartWidthTextField		= new JTextField();
		private JLabel				sideEndWidthLabel			= new JLabel();
		private JTextField 			sideEndWidthTextField		= new JTextField();
		private JLabel				sideSurfaceLabel			= new JLabel();
		private JComboBox<String>	sideSurfaceComboBox			= null;
		private JLabel				sideBankingTypeLabel		= new JLabel();
		private JComboBox<String>	sideBankingTypeComboBox		= null;
		private JLabel				borderWidthLabel			= new JLabel();
		private JTextField			borderWidthTextField		= new JTextField();
		private JLabel				borderHeightLabel			= new JLabel();
		private JTextField			borderHeightTextField		= new JTextField();
		private JLabel				borderSurfaceLabel			= new JLabel();
		private JComboBox<String>	borderSurfaceComboBox		= null;
		private JLabel				borderStyleLabel			= new JLabel();
		private JComboBox<String>	borderStyleComboBox			= null;
		private JLabel				barrierWidthLabel			= new JLabel();
		private JTextField			barrierWidthTextField		= new JTextField();
		private JLabel				barrierHeightLabel			= new JLabel();
		private JTextField			barrierHeightTextField		= new JTextField();
		private JLabel				barrierSurfaceLabel			= new JLabel();
		private JComboBox<String>	barrierSurfaceComboBox		= null;
		private JLabel				barrierStyleLabel			= new JLabel();
		private JComboBox<String>	barrierStyleComboBox		= null;

		/**
		 *
		 */
		public SidePanel(SegmentSide side)
		{
			super();
			this.side = side;
			initialize();
		}

		/**
		 *
		 */
		private void initialize()
		{
			setLayout(null);
			
			addLabel(this, 0, sideStartWidthLabel, "Side Start Width", 130);
			addLabel(this, 1, sideEndWidthLabel, "Side End Width", 130);
			addLabel(this, 2, sideSurfaceLabel, "Side Banking Surface", 130);
			addLabel(this, 3, sideBankingTypeLabel, "Side Type", 130);
			addLabel(this, 4, borderWidthLabel, "Border Width", 130);
			addLabel(this, 5, borderHeightLabel, "Border Height", 130);
			addLabel(this, 6, borderSurfaceLabel, "Border Surface", 130);
			addLabel(this, 7, borderStyleLabel, "Border Style", 130);
			addLabel(this, 8, barrierWidthLabel, "Barrier Width", 130);
			addLabel(this, 9, barrierHeightLabel, "Barrier Height", 130);
			addLabel(this, 10, barrierSurfaceLabel, "Barrier Surface", 130);
			addLabel(this, 11, barrierStyleLabel, "Barrier Style", 130);
			
			addTextField(this, 0, sideStartWidthTextField, side.getSideStartWidth(), 140, 60);
			addTextField(this, 1, sideEndWidthTextField, side.getSideEndWidth(), 140, 60);

			add(getSideSurfaceComboBox(), null);
			add(getSideBankingTypeComboBox(), null);

			addTextField(this, 4, borderWidthTextField, side.getBorderWidth(), 140, 60);
			addTextField(this, 5, borderHeightTextField, side.getBorderHeight(), 140, 60);

			add(getBorderSurfaceComboBox(), null);
			add(getBorderStyleComboBox(), null);

			addTextField(this, 8, barrierWidthTextField, side.getBarrierWidth(), 140, 60);
			addTextField(this, 9, barrierHeightTextField, side.getBarrierHeight(), 140, 60);
			
			add(getBarrierSurfaceComboBox(), null);
			add(getBarrierStyleComboBox(), null);
		}
		
		/**
		 * This method initializes sideSurfaceComboBox
		 *
		 * @return gui.segment.SegmentComboBox
		 */
		private JComboBox<String> getSideSurfaceComboBox()
		{
			if (sideSurfaceComboBox == null)
			{
				sideSurfaceComboBox = new JComboBox();
				sideSurfaceComboBox.setBounds(140, 64, 180, 23);
				addSurface(sideSurfaceVector, side.getSideSurface());
				sideSurfaceComboBox.setModel(new DefaultComboBoxModel<String>(sideSurfaceVector));
				sideSurfaceComboBox.setSelectedItem(side.getSideSurface());
				sideSurfaceComboBox.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						if (sideSurfaceComboBox.getSelectedItem() != null)
							side.setSideSurface(sideSurfaceComboBox.getSelectedItem()+"");
					}
				});
			}
			return sideSurfaceComboBox;
		}

		/**
		 * This method initializes sideBankingTypeComboBox
		 *
		 * @return gui.segment.SegmentComboBox
		 */
		private JComboBox<String> getSideBankingTypeComboBox()
		{
			if (sideBankingTypeComboBox == null)
			{
				String[] items = {"none", "level", "tangent"};
				sideBankingTypeComboBox = new JComboBox();
				sideBankingTypeComboBox.setBounds(140, 91, 100, 23);
				sideBankingTypeComboBox.setModel(new DefaultComboBoxModel<String>(items));
				String type = side.getSideBankingType();
				if (type == null || type.isEmpty())
					type = "none";
				sideBankingTypeComboBox.setSelectedItem(type);
				sideBankingTypeComboBox.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						String type = sideBankingTypeComboBox.getSelectedItem().toString();
						if (type == "none")
							type = "";
						side.setSideBankingType(type);
					}
				});
			}
			return sideBankingTypeComboBox;
		}

		/**
		 * This method initializes borderSurfaceComboBox
		 *
		 * @return gui.segment.SegmentComboBox
		 */
		private JComboBox<String> getBorderSurfaceComboBox()
		{
			if (borderSurfaceComboBox == null)
			{
				borderSurfaceComboBox = new JComboBox();
				borderSurfaceComboBox.setBounds(140, 172, 180, 23);
				addSurface(borderSurfaceVector, side.getBorderSurface());
				borderSurfaceComboBox.setModel(new DefaultComboBoxModel<String>(borderSurfaceVector));
				borderSurfaceComboBox.setSelectedItem(side.getBorderSurface());
				borderSurfaceComboBox.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						if (borderSurfaceComboBox.getSelectedItem() != null)
							side.setBorderSurface(borderSurfaceComboBox.getSelectedItem()+"");
					}
				});
			}
			return borderSurfaceComboBox;
		}

		/**
		 * This method initializes borderStyleComboBox
		 *
		 * @return gui.segment.SegmentComboBox
		 */
		private JComboBox<String> getBorderStyleComboBox()
		{
			if (borderStyleComboBox == null)
			{
				borderStyleComboBox = new JComboBox();
				borderStyleComboBox.setBounds(140, 199, 100, 23);
				borderStyleComboBox.setModel(new DefaultComboBoxModel<String>(styleItems));
				String style = side.getBorderStyle();
				if (style == null || style.isEmpty())
					style = "none";
				borderStyleComboBox.setSelectedItem(style);
				borderStyleComboBox.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						String style = borderStyleComboBox.getSelectedItem().toString();
						if (style == "none")
							style = "";
						side.setBorderStyle(style);
					}
				});
			}
			return borderStyleComboBox;
		}

		/**
		 * This method initializes barrierSurfaceComboBox
		 *
		 * @return gui.segment.SegmentComboBox
		 */
		private JComboBox<String> getBarrierSurfaceComboBox()
		{
			if (barrierSurfaceComboBox == null)
			{
				barrierSurfaceComboBox = new JComboBox();
				barrierSurfaceComboBox.setBounds(140, 280, 180, 23);
				addSurface(barrierSurfaceVector, side.getBarrierSurface());
				barrierSurfaceComboBox.setModel(new DefaultComboBoxModel<String>(barrierSurfaceVector));
				barrierSurfaceComboBox.setSelectedItem(side.getBarrierSurface());
				barrierSurfaceComboBox.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						if (barrierSurfaceComboBox.getSelectedItem() != null)
							side.setBarrierSurface(barrierSurfaceComboBox.getSelectedItem()+"");
					}
				});
			}
			return barrierSurfaceComboBox;
		}

		/**
		 * This method initializes barrierStyleComboBox
		 *
		 * @return gui.segment.SegmentComboBox
		 */
		private JComboBox<String> getBarrierStyleComboBox()
		{
			if (barrierStyleComboBox == null)
			{
				barrierStyleComboBox = new JComboBox();
				barrierStyleComboBox.setBounds(140, 307, 100, 23);
				barrierStyleComboBox.setModel(new DefaultComboBoxModel<String>(styleItems));
				String style = side.getBarrierStyle();
				if (style == null || style.isEmpty())
					style = "none";
				barrierStyleComboBox.setSelectedItem(style);
				barrierStyleComboBox.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						String style = barrierStyleComboBox.getSelectedItem().toString();
						if (style == "none")
							style = "";
						side.setBarrierStyle(style);
					}
				});
			}
			return barrierStyleComboBox;
		}
	}
	
	/**
	 *
	 */
	public void exit()
	{
		MutableString stringResult = new MutableString();
		MutableDouble doubleResult = new MutableDouble();

        if (isDifferent(widthTextField.getText(),
            Editor.getProperties().getMainTrack().getWidth(), doubleResult))
        {
            Editor.getProperties().getMainTrack().setWidth(doubleResult.getValue());
            frame.documentIsModified = true;
        }

		if (isDifferent((String) surfaceComboBox.getSelectedItem(),
            Editor.getProperties().getMainTrack().getSurface(), stringResult))
		{
			Editor.getProperties().getMainTrack().setSurface(stringResult.getValue());
			frame.documentIsModified = true;
		}

        if (isDifferent(profilStepsLengthTextField.getText(),
            Editor.getProperties().getMainTrack().getProfilStepsLength(), doubleResult))
        {
            Editor.getProperties().getMainTrack().setProfilStepsLength(doubleResult.getValue());
            frame.documentIsModified = true;
        }

        if (isDifferent(racelineWidthscaleTextField.getText(),
            Editor.getProperties().getMainTrack().getRacelineWidthscale(), doubleResult))
        {
            Editor.getProperties().getMainTrack().setRacelineWidthscale(doubleResult.getValue());
            frame.documentIsModified = true;
        }

        if (isDifferent(racelineIntTextField.getText(),
            Editor.getProperties().getMainTrack().getRacelineInt(), doubleResult))
        {
            Editor.getProperties().getMainTrack().setRacelineInt(doubleResult.getValue());
            frame.documentIsModified = true;
        }

        if (isDifferent(racelineExtTextField.getText(),
            Editor.getProperties().getMainTrack().getRacelineExt(), doubleResult))
        {
            Editor.getProperties().getMainTrack().setRacelineExt(doubleResult.getValue());
            frame.documentIsModified = true;
        }

        SidePanel left = (SidePanel) tabbedPane.getComponentAt(0);

        if (isDifferent(left.sideStartWidthTextField.getText(),
        	Editor.getProperties().getMainTrack().getLeft().getSideStartWidth(), doubleResult))
        {
            Editor.getProperties().getMainTrack().getLeft().setSideStartWidth(doubleResult.getValue());
            frame.documentIsModified = true;        	
        }
        if (isDifferent(left.sideEndWidthTextField.getText(),
            Editor.getProperties().getMainTrack().getLeft().getSideEndWidth(), doubleResult))
        {
            Editor.getProperties().getMainTrack().getLeft().setSideEndWidth(doubleResult.getValue());
            frame.documentIsModified = true;        	
        }
		if (isDifferent((String) left.sideSurfaceComboBox.getSelectedItem(),
            Editor.getProperties().getMainTrack().getLeft().getSideSurface(), stringResult))
		{
			Editor.getProperties().getMainTrack().getLeft().setSideSurface(stringResult.getValue());
			frame.documentIsModified = true;
		}
		if (isDifferent((String) left.sideBankingTypeComboBox.getSelectedItem(),
            Editor.getProperties().getMainTrack().getLeft().getSideBankingType(), stringResult))
		{
			Editor.getProperties().getMainTrack().getLeft().setSideBankingType(stringResult.getValue());
			frame.documentIsModified = true;
		}
        if (isDifferent(left.borderWidthTextField.getText(),
            Editor.getProperties().getMainTrack().getLeft().getBorderWidth(), doubleResult))
        {
            Editor.getProperties().getMainTrack().getLeft().setBorderWidth(doubleResult.getValue());
            frame.documentIsModified = true;        	
        }
        if (isDifferent(left.borderHeightTextField.getText(),
            Editor.getProperties().getMainTrack().getLeft().getBorderHeight(), doubleResult))
        {
            Editor.getProperties().getMainTrack().getLeft().setBorderHeight(doubleResult.getValue());
            frame.documentIsModified = true;        	
        }
		if (isDifferent((String) left.borderSurfaceComboBox.getSelectedItem(),
            Editor.getProperties().getMainTrack().getLeft().getBorderSurface(), stringResult))
		{
			Editor.getProperties().getMainTrack().getLeft().setBorderSurface(stringResult.getValue());
			frame.documentIsModified = true;
		}
		if (isDifferent((String) left.borderStyleComboBox.getSelectedItem(),
            Editor.getProperties().getMainTrack().getLeft().getBorderStyle(), stringResult))
		{
			Editor.getProperties().getMainTrack().getLeft().setBorderStyle(stringResult.getValue());
			frame.documentIsModified = true;
		}
        if (isDifferent(left.barrierWidthTextField.getText(),
            Editor.getProperties().getMainTrack().getLeft().getBarrierWidth(), doubleResult))
        {
            Editor.getProperties().getMainTrack().getLeft().setBarrierWidth(doubleResult.getValue());
            frame.documentIsModified = true;        	
        }
        if (isDifferent(left.barrierHeightTextField.getText(),
            Editor.getProperties().getMainTrack().getLeft().getBarrierHeight(), doubleResult))
        {
            Editor.getProperties().getMainTrack().getLeft().setBarrierHeight(doubleResult.getValue());
            frame.documentIsModified = true;        	
        }
		if (isDifferent((String) left.barrierSurfaceComboBox.getSelectedItem(),
            Editor.getProperties().getMainTrack().getLeft().getBarrierSurface(), stringResult))
		{
			Editor.getProperties().getMainTrack().getLeft().setBarrierSurface(stringResult.getValue());
			frame.documentIsModified = true;
		}
		if (isDifferent((String) left.barrierStyleComboBox.getSelectedItem(),
            Editor.getProperties().getMainTrack().getLeft().getBarrierStyle(), stringResult))
		{
			Editor.getProperties().getMainTrack().getLeft().setBarrierStyle(stringResult.getValue());
			frame.documentIsModified = true;
		}

        SidePanel right = (SidePanel) tabbedPane.getComponentAt(1);

        if (isDifferent(right.sideStartWidthTextField.getText(),
        	Editor.getProperties().getMainTrack().getRight().getSideStartWidth(), doubleResult))
        {
            Editor.getProperties().getMainTrack().getRight().setSideStartWidth(doubleResult.getValue());
            frame.documentIsModified = true;        	
        }
        if (isDifferent(right.sideEndWidthTextField.getText(),
            Editor.getProperties().getMainTrack().getRight().getSideEndWidth(), doubleResult))
        {
            Editor.getProperties().getMainTrack().getRight().setSideEndWidth(doubleResult.getValue());
            frame.documentIsModified = true;        	
        }
		if (isDifferent((String) right.sideSurfaceComboBox.getSelectedItem(),
            Editor.getProperties().getMainTrack().getRight().getSideSurface(), stringResult))
		{
			Editor.getProperties().getMainTrack().getRight().setSideSurface(stringResult.getValue());
			frame.documentIsModified = true;
		}
		if (isDifferent((String) right.sideBankingTypeComboBox.getSelectedItem(),
            Editor.getProperties().getMainTrack().getRight().getSideBankingType(), stringResult))
		{
			Editor.getProperties().getMainTrack().getRight().setSideBankingType(stringResult.getValue());
			frame.documentIsModified = true;
		}
        if (isDifferent(right.borderWidthTextField.getText(),
            Editor.getProperties().getMainTrack().getRight().getBorderWidth(), doubleResult))
        {
            Editor.getProperties().getMainTrack().getRight().setBorderWidth(doubleResult.getValue());
            frame.documentIsModified = true;        	
        }
        if (isDifferent(right.borderHeightTextField.getText(),
            Editor.getProperties().getMainTrack().getRight().getBorderHeight(), doubleResult))
        {
            Editor.getProperties().getMainTrack().getRight().setBorderHeight(doubleResult.getValue());
            frame.documentIsModified = true;        	
        }
		if (isDifferent((String) right.borderSurfaceComboBox.getSelectedItem(),
            Editor.getProperties().getMainTrack().getRight().getBorderSurface(), stringResult))
		{
			Editor.getProperties().getMainTrack().getRight().setBorderSurface(stringResult.getValue());
			frame.documentIsModified = true;
		}
		if (isDifferent((String) right.borderStyleComboBox.getSelectedItem(),
            Editor.getProperties().getMainTrack().getRight().getBorderStyle(), stringResult))
		{
			Editor.getProperties().getMainTrack().getRight().setBorderStyle(stringResult.getValue());
			frame.documentIsModified = true;
		}
        if (isDifferent(right.barrierWidthTextField.getText(),
            Editor.getProperties().getMainTrack().getRight().getBarrierWidth(), doubleResult))
        {
            Editor.getProperties().getMainTrack().getRight().setBarrierWidth(doubleResult.getValue());
            frame.documentIsModified = true;        	
        }
        if (isDifferent(right.barrierHeightTextField.getText(),
            Editor.getProperties().getMainTrack().getRight().getBarrierHeight(), doubleResult))
        {
            Editor.getProperties().getMainTrack().getRight().setBarrierHeight(doubleResult.getValue());
            frame.documentIsModified = true;        	
        }
		if (isDifferent((String) right.barrierSurfaceComboBox.getSelectedItem(),
            Editor.getProperties().getMainTrack().getRight().getBarrierSurface(), stringResult))
		{
			Editor.getProperties().getMainTrack().getRight().setBarrierSurface(stringResult.getValue());
			frame.documentIsModified = true;
		}
		if (isDifferent((String) right.barrierStyleComboBox.getSelectedItem(),
            Editor.getProperties().getMainTrack().getRight().getBarrierStyle(), stringResult))
		{
			Editor.getProperties().getMainTrack().getRight().setBarrierStyle(stringResult.getValue());
			frame.documentIsModified = true;
		}
	}
}  //  @jve:decl-index=0:visual-constraint="10,10"
