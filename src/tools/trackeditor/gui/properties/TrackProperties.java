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

	private String[]			roadSurfaceItems		=
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

		Vector<Surface> surfaces = Editor.getProperties().getSurfaces();
        for (int i = 0; i < surfaces.size(); i++)
        {
			String surface = surfaces.elementAt(i).getName();
			boolean found = false;
			for (int j = 0; j < roadSurfaceVector.size(); j++)
			{
				if (roadSurfaceVector.elementAt(i).equals(surfaces.elementAt(i).getName()))
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				roadSurfaceVector.add(surface);
			}
        }
		Collections.sort(roadSurfaceVector);
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
			if (surface != null)
			{
				boolean found = false;
				for (int i = 0; i < roadSurfaceVector.size(); i++)
				{
					if (roadSurfaceVector.elementAt(i).equals(surface))
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					roadSurfaceVector.add(surface);
				}
			}
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
		private JLabel		sideStartWidthLabel			= new JLabel();
		private JTextField 	sideStartWidthTextField		= new JTextField();
		private JLabel		sideEndWidthLabel			= new JLabel();
		private JTextField 	sideEndWidthTextField		= new JTextField();
		private JLabel		sideSurfaceLabel			= new JLabel();
		private JLabel		sideBankingTypeLabel		= new JLabel();
		private JLabel		borderWidthLabel			= new JLabel();
		private JTextField	borderWidthTextField		= new JTextField();
		private JLabel		borderHeightLabel			= new JLabel();
		private JTextField	borderHeightTextField		= new JTextField();
		private JLabel		borderSurfaceLabel			= new JLabel();
		private JLabel		borderStyleLabel			= new JLabel();
		private JLabel		barrierWidthLabel			= new JLabel();
		private JTextField	barrierWidthTextField		= new JTextField();
		private JLabel		barrierHeightLabel			= new JLabel();
		private JTextField	barrierHeightTextField		= new JTextField();
		private JLabel		barrierSurfaceLabel			= new JLabel();
		private JLabel		barrierStyleLabel			= new JLabel();

		/**
		 *
		 */
		public SidePanel(SegmentSide side)
		{
			super();
			initialize(side);
		}

		/**
		 *
		 */
		private void initialize(SegmentSide side)
		{
			setLayout(null);
			
			addLabel(this, 0, sideStartWidthLabel, "Side Start Width", 120);
			addLabel(this, 1, sideEndWidthLabel, "Side End Width", 120);
			addLabel(this, 2, sideSurfaceLabel, "Side Surface", 120);
			addLabel(this, 3, sideBankingTypeLabel, "Side Type", 120);
			addLabel(this, 4, borderWidthLabel, "Border Width", 120);
			addLabel(this, 5, borderHeightLabel, "Border Height", 120);
			addLabel(this, 6, borderSurfaceLabel, "Border Surface", 120);
			addLabel(this, 7, borderStyleLabel, "Border Style", 120);
			addLabel(this, 8, barrierWidthLabel, "Barrier Width", 120);
			addLabel(this, 9, barrierHeightLabel, "Barrier Height", 120);
			addLabel(this, 10, barrierSurfaceLabel, "Barrier Surface", 120);
			addLabel(this, 11, barrierStyleLabel, "Barrier Style", 120);
			
			addTextField(this, 0, sideStartWidthTextField, side.getSideStartWidth(), 130, 60);
			addTextField(this, 1, sideEndWidthTextField, side.getSideEndWidth(), 130, 60);

			addTextField(this, 4, borderWidthTextField, side.getBorderWidth(), 130, 60);
			addTextField(this, 5, borderHeightTextField, side.getBorderHeight(), 130, 60);

			addTextField(this, 8, barrierWidthTextField, side.getBarrierWidth(), 130, 60);
			addTextField(this, 9, barrierHeightTextField, side.getBarrierHeight(), 130, 60);		
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
	}
 }  //  @jve:decl-index=0:visual-constraint="10,10"
