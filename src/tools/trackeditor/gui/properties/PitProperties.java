/*
 *   PitProperties.java
 *   Created on 27 ??? 2005
 *
 *    The PitProperties.java is part of TrackEditor-0.3.1.
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
import java.util.Iterator;
import java.util.Vector;

import javax.swing.DefaultComboBoxModel;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JTextField;

import gui.EditorFrame;
import utils.SurfaceComboBox;
import utils.circuit.Pits;
import utils.circuit.Segment;
import utils.circuit.SegmentSide;
import utils.circuit.Surface;

/**
 * @author babis
 *
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class PitProperties extends PropertyPanel
{
	private JLabel				styleLabel				= new JLabel();
	private JComboBox<String>	styleComboBox			= null;
	private JLabel				sideLabel				= new JLabel();
	private JComboBox<String>	sideComboBox			= null;
	private JLabel				entryLabel				= new JLabel();
	private JTextField			entryTextField			= new JTextField();
	private JLabel				startLabel				= new JLabel();
	private JTextField			startTextField			= new JTextField();
	private JLabel				startBuildingsLabel		= new JLabel();
	private JTextField			startBuildingsTextField	= new JTextField();
	private JLabel				stopBuildingsLabel		= new JLabel();
	private JTextField			stopBuildingsTextField	= new JTextField();
	private JLabel				maxPitsLabel			= new JLabel();
	private JTextField			maxPitsTextField		= new JTextField();
	private JLabel				endLabel				= new JLabel();
	private JTextField			endTextField			= new JTextField();
	private JLabel				exitLabel				= new JLabel();
	private JTextField			exitTextField			= new JTextField();
	private JLabel				widthLabel				= new JLabel();
	private JTextField			widthTextField			= new JTextField();
	private JLabel				lengthLabel				= new JLabel();
	private JTextField			lengthTextField			= new JTextField();
	private JLabel				indicatorLabel			= new JLabel();
	private JComboBox<String>	indicatorComboBox		= null;
	private JLabel				speedLimitLabel			= new JLabel();
	private JTextField			speedLimitTextField		= new JTextField();
	private JLabel				entrySurfaceLabel		= new JLabel();
	private JComboBox<String>	entrySurfaceComboBox	= null;
	private JLabel				pitSurfaceLabel			= new JLabel();
	private SurfaceComboBox		pitSurfaceComboBox		= null;
	private JLabel				pitWallLabel			= new JLabel();
	private JCheckBox			pitWallCheckBox			= null;
	private JLabel				pitWallHeightLabel		= new JLabel();
	private JTextField			pitWallHeightTextField	= new JTextField();
	private JLabel				pitWallWidthLabel		= new JLabel();
	private JTextField			pitWallWidthTextField	= new JTextField();
	private JLabel				pitWallSurfaceLabel		= new JLabel();
	private SurfaceComboBox		pitWallSurfaceComboBox	= null;
	private JLabel				exitSurfaceLabel		= new JLabel();
	private SurfaceComboBox		exitSurfaceComboBox		= null;
	private JLabel 				generatePitsLabel 		= new JLabel();
	private JCheckBox 			generatePitsCheckBox 	= null;
	private boolean 			generatePits			= false;

	private String[]			sideSurfaceItems		=
														{"grass", "grass3", "grass5", "grass6", "grass7", "gravel",
			"sand3", "sand", "asphalt-lines", "asphalt-l-left", "asphalt-l-right", "asphalt-l-both", "asphalt-pits",
			"asphalt", "dirt", "dirt-b", "asphalt2", "road1", "road1-pits", "road1-asphalt", "asphalt-road1",
			"b-road1", "b-road1-l2", "b-road1-l2p", "concrete", "concrete2", "concrete3", "b-asphalt", "b-asphalt-l1",
			"b-asphalt-l1p", "asphalt2-lines", "asphalt2-l-left", "asphalt2-l-right", "asphalt2-l-both", "curb-5cm-r",
			"curb-5cm-l", "curb-l", "tar-grass3-l", "tar-grass3-r", "tar-sand", "b-road1-grass6", "b-road1-grass6-l2",
			"b-road1-gravel-l2", "b-road1-sand3", "b-road1-sand3-l2", "b-asphalt-grass7", "b-asphalt-grass7-l1",
			"b-asphalt-grass6", "b-asphalt-grass6-l1", "b-asphalt-sand3", "b-asphalt-sand3-l1", "barrier", "barrier2",
			"barrier-turn", "barrier-grille", "wall", "wall2", "tire-wall"};
	private Vector<String>		sideSurfaceVector		= new Vector<String>(Arrays.asList(sideSurfaceItems));

	/**
	 *
	 */
	public PitProperties(EditorFrame editorFrame)
	{
		super(editorFrame);
		addDefaultSurfaces(sideSurfaceVector);
		initialize();
	}

	/**
	 * This method initializes this
	 *
	 * @return void
	 */
	private void initialize()
	{
		setLayout(null);
		setBorder(javax.swing.BorderFactory.createEtchedBorder(javax.swing.border.EtchedBorder.LOWERED));

		addLabel(this, 0, styleLabel, "Type", 110);
		addLabel(this, 1, sideLabel, "Side", 110);
		addLabel(this, 2, entryLabel, "Entry", 110);
		addLabel(this, 3, startLabel, "Start", 110);
		addLabel(this, 4, startBuildingsLabel, "Start Buildings", 110);
		addLabel(this, 5, stopBuildingsLabel, "Stop Buildings", 110);
		addLabel(this, 6, maxPitsLabel, "Max Pits", 110);
		addLabel(this, 7, endLabel, "End", 110);
		addLabel(this, 8, exitLabel, "Exit", 110);
		addLabel(this, 9, widthLabel, "Width", 110);
		addLabel(this, 10, lengthLabel, "Length", 110);
		addLabel(this, 11, indicatorLabel, "Indicator", 110);
		addLabel(this, 12, speedLimitLabel, "Speed Limit", 110);
		addLabel(this, 14, generatePitsLabel, "Generate Pits", 110);
		addLabel(this, 16, entrySurfaceLabel, "Entry Surface", 110);
		addLabel(this, 17, pitSurfaceLabel, "Pit Surface", 110);
		addLabel(this, 18, pitWallLabel, "Pit Wall", 110);
		addLabel(this, 19, pitWallHeightLabel, "Wall Height", 110);
		addLabel(this, 20, pitWallWidthLabel, "Wall Width", 110);
		addLabel(this, 21, pitWallSurfaceLabel, "Wall Surface", 110);
		addLabel(this, 22, exitSurfaceLabel, "Exit Surface", 110);

		add(getStyleComboBox(), null);
		add(getSideComboBox(), null);

		Pits pits = getEditorFrame().getTrackData().getMainTrack().getPits();

		addTextField(this, 2, entryTextField, pits.getEntry(), 120, 100);
		addTextField(this, 3, startTextField, pits.getStart(), 120, 100);
		addTextField(this, 4, startBuildingsTextField, pits.getStartBuildings(), 120, 100);
		addTextField(this, 5, stopBuildingsTextField, pits.getStopBuildings(), 120, 100);
		addTextField(this, 6, maxPitsTextField, pits.getMaxPits(), 120, 100);
		addTextField(this, 7, endTextField, pits.getEnd(), 120, 100);
		addTextField(this, 8, exitTextField, pits.getExit(), 120, 100);
		addTextField(this, 9, widthTextField, pits.getWidth(), 120, 40);
		addTextField(this, 10, lengthTextField, pits.getLength(), 120, 40);

		add(getIndicatorComboBox(), null);

		addTextField(this, 12, speedLimitTextField, pits.getSpeedLimit(), 120, 40);

		add(getGeneratePitsCheckBox(), null);
		add(getEntrySurfaceComboBox(), null);
		add(getPitSurfaceComboBox(), null);		
		add(getPitWallCheckBox(), null);
		addTextField(this, 19, pitWallHeightTextField, null, 120, 100);
		addTextField(this, 20, pitWallWidthTextField, null, 120, 100);
		add(getPitWallSurfaceComboBox(), null);
		add(getExitSurfaceComboBox(), null);
	}
	
	private void addDefaultSurfaces(Vector<String> surfaceVector)
	{
        Vector<Surface> surfaces = getEditorFrame().getTrackData().getSurfaces();
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
	
	/**
	 * This method initializes styleComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	private JComboBox<String> getStyleComboBox()
	{
		if (styleComboBox == null)
		{
			String[] items = {"none", "no pits", "on track side", "on separate path", "no building"};
			styleComboBox = new JComboBox<String>(items);
			styleComboBox.setBounds(120, 10, 120, 23);
			int style = getEditorFrame().getTrackData().getMainTrack().getPits().getStyle();
			if (style == Integer.MAX_VALUE)
				style = 0;
			else
				style++;
			styleComboBox.setSelectedIndex(style);
		}
		return styleComboBox;
	}

	/**
	 * This method initializes sideComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	private JComboBox<String> getSideComboBox()
	{
		if (sideComboBox == null)
		{
			String[] items = {"none", "right", "left"};
			sideComboBox = new JComboBox<String>(items);
			sideComboBox.setBounds(120, 37, 80, 23);
			String side = getEditorFrame().getTrackData().getMainTrack().getPits().getSide();
			if (side == null || side.isEmpty())
				side = "none";
			sideComboBox.setSelectedItem(side);
		}
		return sideComboBox;
	}

	/**
	 * This method initializes indicatorComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	private JComboBox<String> getIndicatorComboBox()
	{
		if (indicatorComboBox == null)
		{
			String[] items = {"none", "no", "yes"};
			indicatorComboBox = new JComboBox<String>(items);
			indicatorComboBox.setBounds(120, 307, 80, 23);
			int indicator = getEditorFrame().getTrackData().getMainTrack().getPits().getIndicator();
			if (indicator == Integer.MAX_VALUE)
				indicator = 0;
			else
				indicator++;
			indicatorComboBox.setSelectedIndex(indicator);
		}
		return indicatorComboBox;
	}

 	/**
	 * This method initializes generatePitsCheckBox
 	 *
	 * @return javax.swing.JCheckBox
 	 */
	private JCheckBox getGeneratePitsCheckBox()
	{
		if (generatePitsCheckBox == null)
		{
			generatePitsCheckBox = new JCheckBox();
			generatePitsCheckBox.setBounds(120, 388, 20, 20);
			generatePitsCheckBox.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					if (generatePitsCheckBox.isSelected())
					{
						entrySurfaceComboBox.setEnabled(true);
						entrySurfaceComboBox.setSelectedItem("road1");
						pitSurfaceComboBox.setEnabled(true);
						pitSurfaceComboBox.setSelectedItem("road1-pits");
						exitSurfaceComboBox.setEnabled(true);
						exitSurfaceComboBox.setSelectedItem("road1");
						pitWallCheckBox.setEnabled(true);
						pitWallCheckBox.setSelected(false);
						generatePits = true;
					}
					else
					{
						entrySurfaceComboBox.setEnabled(false);
						entrySurfaceComboBox.setSelectedItem("");
						pitSurfaceComboBox.setEnabled(false);
						pitSurfaceComboBox.setSelectedItem("");
						exitSurfaceComboBox.setEnabled(false);
						exitSurfaceComboBox.setSelectedItem("");
						pitWallCheckBox.setEnabled(false);
						pitWallCheckBox.setSelected(false);
						pitWallHeightTextField.setEnabled(false);
						pitWallHeightTextField.setText(null);
						pitWallWidthTextField.setEnabled(false);
						pitWallWidthTextField.setText(null);
						pitWallSurfaceComboBox.setEnabled(false);
						pitWallSurfaceComboBox.setSelectedItem("");
						generatePits = false;
					}
				}
			});
		}
		return generatePitsCheckBox;
	}
	
	/**
	 * This method initializes entrySurfaceComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	private JComboBox<String> getEntrySurfaceComboBox()
	{
		if (entrySurfaceComboBox == null)
		{
			entrySurfaceComboBox = new JComboBox<String>();
			entrySurfaceComboBox.setBounds(120, 442, 180, 23);
			entrySurfaceComboBox.setModel(new DefaultComboBoxModel<String>(sideSurfaceVector));
			entrySurfaceComboBox.setEnabled(false);
			entrySurfaceComboBox.setSelectedItem(null);
		}
		return entrySurfaceComboBox;
	}

	/**
	 * This method initializes pitSurfaceComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	private JComboBox<String> getPitSurfaceComboBox()
	{
		if (pitSurfaceComboBox == null)
		{
			pitSurfaceComboBox = new SurfaceComboBox(getEditorFrame(), sideSurfaceVector);
			pitSurfaceComboBox.setBounds(120, 469, 180, 23);
			pitSurfaceComboBox.setEnabled(false);
			pitSurfaceComboBox.setSelectedItem(null);
		}
		return pitSurfaceComboBox;
	}

 	/**
	 * This method initializes pitWallCheckBox
 	 *
	 * @return javax.swing.JCheckBox
 	 */
	private JCheckBox getPitWallCheckBox()
	{
		if (pitWallCheckBox == null)
		{
			pitWallCheckBox = new JCheckBox();
			pitWallCheckBox.setBounds(120, 496, 20, 20);
			pitWallCheckBox.setEnabled(false);
			pitWallCheckBox.setSelected(false);
			pitWallCheckBox.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					if (pitWallCheckBox.isSelected())
					{
						pitWallHeightTextField.setEnabled(true);
						pitWallHeightTextField.setText("1.0");
						pitWallWidthTextField.setEnabled(true);
						pitWallWidthTextField.setText("0.5");
						pitWallSurfaceComboBox.setEnabled(true);
						pitWallSurfaceComboBox.setSelectedItem("wall");
					}
					else
					{
						pitWallHeightTextField.setEnabled(false);
						pitWallHeightTextField.setText(null);
						pitWallWidthTextField.setEnabled(false);
						pitWallWidthTextField.setText(null);
						pitWallSurfaceComboBox.setEnabled(false);
						pitWallSurfaceComboBox.setSelectedItem("");
					}
				}
			});
		}
		return pitWallCheckBox;
	}
	
	/**
	 * This method initializes pitWallSurfaceComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	private JComboBox<String> getPitWallSurfaceComboBox()
	{
		if (pitWallSurfaceComboBox == null)
		{
			pitWallSurfaceComboBox = new SurfaceComboBox(getEditorFrame(), sideSurfaceVector);
			pitWallSurfaceComboBox.setBounds(120, 577, 180, 23);
			pitWallSurfaceComboBox.setEnabled(false);
			pitWallSurfaceComboBox.setSelectedItem(null);
		}
		return pitWallSurfaceComboBox;
	}

	/**
	 * This method initializes exitSurfaceComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	private JComboBox<String> getExitSurfaceComboBox()
	{
		if (exitSurfaceComboBox == null)
		{
			exitSurfaceComboBox = new SurfaceComboBox(getEditorFrame(), sideSurfaceVector);
			exitSurfaceComboBox.setBounds(120, 604, 180, 23);
			exitSurfaceComboBox.setEnabled(false);
			exitSurfaceComboBox.setSelectedItem(null);
		}
		return exitSurfaceComboBox;
	}

	/**
	 *
	 */
	public void exit()
	{
		int index = getStyleComboBox().getSelectedIndex();
		int style = getEditorFrame().getTrackData().getMainTrack().getPits().getStyle();
		if (index == 0)
		{
			if (style != Integer.MAX_VALUE)
			{
				getEditorFrame().getTrackData().getMainTrack().getPits().setStyle(Integer.MAX_VALUE);
				getEditorFrame().documentIsModified = true;
			}
		}
		else if (style == Integer.MAX_VALUE || style != index - 1)
		{
			getEditorFrame().getTrackData().getMainTrack().getPits().setStyle(index - 1);
			getEditorFrame().documentIsModified = true;
		}

		MutableString stringResult = new MutableString();
		MutableDouble doubleResult = new MutableDouble();
		MutableInteger integerResult = new MutableInteger();

		if (isDifferent((String) getSideComboBox().getSelectedItem(),
			getEditorFrame().getTrackData().getMainTrack().getPits().getSide(), stringResult))
		{
			getEditorFrame().getTrackData().getMainTrack().getPits().setSide(stringResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(entryTextField.getText(),
			getEditorFrame().getTrackData().getMainTrack().getPits().getEntry(), stringResult))
		{
			getEditorFrame().getTrackData().getMainTrack().getPits().setEntry(stringResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(startTextField.getText(),
			getEditorFrame().getTrackData().getMainTrack().getPits().getStart(), stringResult))
		{
			getEditorFrame().getTrackData().getMainTrack().getPits().setStart(stringResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(startBuildingsTextField.getText(),
			getEditorFrame().getTrackData().getMainTrack().getPits().getStartBuildings(), stringResult))
		{
			getEditorFrame().getTrackData().getMainTrack().getPits().setStartBuildings(stringResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(stopBuildingsTextField.getText(),
			getEditorFrame().getTrackData().getMainTrack().getPits().getStopBuildings(), stringResult))
		{
			getEditorFrame().getTrackData().getMainTrack().getPits().setStopBuildings(stringResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(maxPitsTextField.getText(),
			getEditorFrame().getTrackData().getMainTrack().getPits().getMaxPits(), integerResult))
		{
			getEditorFrame().getTrackData().getMainTrack().getPits().setMaxPits(integerResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(endTextField.getText(),
			getEditorFrame().getTrackData().getMainTrack().getPits().getEnd(), stringResult))
		{
			getEditorFrame().getTrackData().getMainTrack().getPits().setEnd(stringResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(exitTextField.getText(),
			getEditorFrame().getTrackData().getMainTrack().getPits().getExit(), stringResult))
		{
			getEditorFrame().getTrackData().getMainTrack().getPits().setExit(stringResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(widthTextField.getText(),
			getEditorFrame().getTrackData().getMainTrack().getPits().getWidth(), doubleResult))
		{
			getEditorFrame().getTrackData().getMainTrack().getPits().setWidth(doubleResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(lengthTextField.getText(),
			getEditorFrame().getTrackData().getMainTrack().getPits().getLength(), doubleResult))
		{
			getEditorFrame().getTrackData().getMainTrack().getPits().setLength(doubleResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		index = getIndicatorComboBox().getSelectedIndex();
		int indicator = getEditorFrame().getTrackData().getMainTrack().getPits().getIndicator();
		if (index == 0)
		{
			if (indicator != Integer.MAX_VALUE)
			{
				getEditorFrame().getTrackData().getMainTrack().getPits().setIndicator(Integer.MAX_VALUE);
				getEditorFrame().documentIsModified = true;
			}
		}
		else if (indicator == Integer.MAX_VALUE || indicator != index - 1)
		{
			getEditorFrame().getTrackData().getMainTrack().getPits().setIndicator(index - 1);
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(speedLimitTextField.getText(),
			getEditorFrame().getTrackData().getMainTrack().getPits().getSpeedLimit(), doubleResult))
		{
			getEditorFrame().getTrackData().getMainTrack().getPits().setSpeedLimit(doubleResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (getGeneratePitsCheckBox().isSelected())
		{
			createPits();
			getEditorFrame().documentIsModified = true;
		}
	}

	/**
	 *
	 */
	private void createPits()
	{
		Pits pits = getEditorFrame().getTrackData().getMainTrack().getPits();
		Vector<Segment> data = getEditorFrame().getTrackData().getSegments();
		Segment pitEntry = null;
		Segment pitStart = null;
		Segment pitStartBuildings = null;
		Segment pitStopBuildings = null;
		Segment pitEnd = null;
		Segment pitExit = null;

		Iterator<Segment> it = data.iterator();
		while (it.hasNext())
		{
			Segment obj = it.next();
			String name = obj.getName();
			if (name.equals(pits.getEntry()))
			{
				pitEntry = obj;
			}
			if (name.equals(pits.getStart()))
			{
				pitStart = obj;
			}
			if (name.equals(pits.getStartBuildings()))
			{
				pitStartBuildings = obj;
			}
			if (name.equals(pits.getStopBuildings()))
			{
				pitStopBuildings = obj;
			}
			if (name.equals(pits.getEnd()))
			{
				pitEnd = obj;
			}
			if (name.equals(pits.getExit()))
			{
				pitExit = obj;
			}
		}

		if (pits.getSide() == null)
 		{
			JOptionPane.showMessageDialog(this, "No pit side specified", "Pit Side", JOptionPane.ERROR_MESSAGE);
			return;
 		}
		
		if (pitEntry == null)
 		{
			if (pits.getEntry() == null || pits.getEntry().isEmpty())
			{
				JOptionPane.showMessageDialog(this, "No pit entry specified.", "Pit Entry", JOptionPane.ERROR_MESSAGE);
			}
			else
			{
				JOptionPane.showMessageDialog(this, "Pit entry " + pits.getEntry() + " not found.", "Pit Entry", JOptionPane.ERROR_MESSAGE);
			}
 			return;
 		}

		if (pitStart == null)
 		{
			if (pits.getStart() == null || pits.getStart().isEmpty())
			{
				JOptionPane.showMessageDialog(this, "No pit start specified.", "Pit Start", JOptionPane.WARNING_MESSAGE);
			}
			else
			{
				JOptionPane.showMessageDialog(this, "Pit start " + pits.getStart() + " not found.", "Pit Start", JOptionPane.WARNING_MESSAGE);
			}
			return;
		}
		
		if (pitEnd == null)
 		{
			if (pits.getEnd() == null || pits.getEnd().isEmpty())
			{
				JOptionPane.showMessageDialog(this, "No pit end specified.", "Pit End", JOptionPane.WARNING_MESSAGE);
			}
			else
			{
				JOptionPane.showMessageDialog(this, "Pit end " + pits.getEnd() + " not found.", "Pit End", JOptionPane.WARNING_MESSAGE);
			}
			return;
 		}

		if (pitExit == null)
 		{
			if (pits.getExit() == null || pits.getExit().isEmpty())
			{
				JOptionPane.showMessageDialog(this, "No pit exit specified.", "Pit Exit", JOptionPane.ERROR_MESSAGE);
			}
			else
			{
				JOptionPane.showMessageDialog(this, "Pit exit " + pits.getExit() + " not found.", "Pit Exit", JOptionPane.ERROR_MESSAGE);
			}
 			return;
 		}	

		SegmentSide side = null;
		double borderWidth = 0;
		double sideWidth = 0;
		Segment nextSegment = pitExit.getNextShape();
		if (nextSegment == null)
			nextSegment = data.get(0);
		if (pits.getSide().equals("left"))
		{
			nextSegment.getLeft().setBorderWidth(nextSegment.getValidLeftBorderWidth(getEditorFrame()));
			nextSegment.getLeft().setBorderHeight(nextSegment.getValidLeftBorderHeight(getEditorFrame()));
			nextSegment.getLeft().setBorderSurface(nextSegment.getValidLeftBorderSurface(getEditorFrame()));
			nextSegment.getLeft().setBorderStyle(nextSegment.getValidLeftBorderStyle(getEditorFrame()));

			nextSegment.getLeft().setSideStartWidth(nextSegment.getValidLeftSideStartWidth(getEditorFrame()));
			nextSegment.getLeft().setSideSurface(nextSegment.getValidLeftSideSurface(getEditorFrame()));
			nextSegment.getLeft().setSideBankingType(nextSegment.getValidLeftSideBankingType(getEditorFrame()));

			nextSegment.getLeft().setBarrierWidth(nextSegment.getValidLeftBarrierWidth(getEditorFrame()));
			nextSegment.getLeft().setBarrierHeight(nextSegment.getValidLeftBarrierHeight(getEditorFrame()));
			nextSegment.getLeft().setBarrierSurface(nextSegment.getValidLeftBarrierSurface(getEditorFrame()));
			nextSegment.getLeft().setBarrierStyle(nextSegment.getValidLeftBarrierStyle(getEditorFrame()));

			side = pitExit.getLeft();
			borderWidth = nextSegment.getLeft().getBorderWidth();
			sideWidth = nextSegment.getLeft().getSideStartWidth();
		}
		else
		{
			nextSegment.getRight().setBorderWidth(nextSegment.getValidRightBorderWidth(getEditorFrame()));
			nextSegment.getRight().setBorderHeight(nextSegment.getValidRightBorderHeight(getEditorFrame()));
			nextSegment.getRight().setBorderSurface(nextSegment.getValidRightBorderSurface(getEditorFrame()));
			nextSegment.getRight().setBorderStyle(nextSegment.getValidRightBorderStyle(getEditorFrame()));

			nextSegment.getRight().setSideStartWidth(nextSegment.getValidRightSideStartWidth(getEditorFrame()));
			nextSegment.getRight().setSideSurface(nextSegment.getValidRightSideSurface(getEditorFrame()));
			nextSegment.getRight().setSideBankingType(nextSegment.getValidRightSideBankingType(getEditorFrame()));

			nextSegment.getRight().setBarrierWidth(nextSegment.getValidRightBarrierWidth(getEditorFrame()));
			nextSegment.getRight().setBarrierHeight(nextSegment.getValidRightBarrierHeight(getEditorFrame()));
			nextSegment.getRight().setBarrierSurface(nextSegment.getValidRightBarrierSurface(getEditorFrame()));
			nextSegment.getRight().setBarrierStyle(nextSegment.getValidRightBarrierStyle(getEditorFrame()));

			side = pitExit.getRight();
			borderWidth = nextSegment.getRight().getBorderWidth();
			sideWidth = nextSegment.getRight().getSideStartWidth();
		}
		side.setBorderHeight(0);
		side.setBorderWidth(0);
		side.setSideStartWidth(pits.getWidth()*3);
		side.setSideEndWidth(borderWidth + sideWidth);
		side.setSideSurface(exitSurfaceComboBox.getSelectedItem().toString());
		side.setBarrierHeight(1);
		side.setBarrierWidth(0.1);

		Segment previousSegment = pitEntry.getPreviousShape();
		if (previousSegment == null)
			previousSegment = data.get(data.size() - 1);			
		if (pits.getSide().equals("left"))
		{
			previousSegment.getLeft().setBorderWidth(previousSegment.getValidLeftBorderWidth(getEditorFrame()));
			previousSegment.getLeft().setBorderHeight(previousSegment.getValidLeftBorderHeight(getEditorFrame()));
			previousSegment.getLeft().setBorderSurface(previousSegment.getValidLeftBorderSurface(getEditorFrame()));
			previousSegment.getLeft().setBorderStyle(previousSegment.getValidLeftBorderStyle(getEditorFrame()));

			previousSegment.getLeft().setSideStartWidth(previousSegment.getValidLeftSideStartWidth(getEditorFrame()));
			previousSegment.getLeft().setSideSurface(previousSegment.getValidLeftSideSurface(getEditorFrame()));
			previousSegment.getLeft().setSideBankingType(previousSegment.getValidLeftSideBankingType(getEditorFrame()));

			previousSegment.getLeft().setBarrierWidth(previousSegment.getValidLeftBarrierWidth(getEditorFrame()));
			previousSegment.getLeft().setBarrierHeight(previousSegment.getValidLeftBarrierHeight(getEditorFrame()));
			previousSegment.getLeft().setBarrierSurface(previousSegment.getValidLeftBarrierSurface(getEditorFrame()));
			previousSegment.getLeft().setBarrierStyle(previousSegment.getValidLeftBarrierStyle(getEditorFrame()));

			side = pitEntry.getLeft();
			borderWidth = previousSegment.getLeft().getBorderWidth();
			sideWidth = previousSegment.getLeft().getSideStartWidth();
		}
		else
		{
			previousSegment.getRight().setBorderWidth(previousSegment.getValidRightBorderWidth(getEditorFrame()));
			previousSegment.getRight().setBorderHeight(previousSegment.getValidRightBorderHeight(getEditorFrame()));
			previousSegment.getRight().setBorderSurface(previousSegment.getValidRightBorderSurface(getEditorFrame()));
			previousSegment.getRight().setBorderStyle(previousSegment.getValidRightBorderStyle(getEditorFrame()));

			previousSegment.getRight().setSideStartWidth(previousSegment.getValidRightSideStartWidth(getEditorFrame()));
			previousSegment.getRight().setSideSurface(previousSegment.getValidRightSideSurface(getEditorFrame()));
			previousSegment.getRight().setSideBankingType(previousSegment.getValidRightSideBankingType(getEditorFrame()));

			previousSegment.getRight().setBarrierWidth(previousSegment.getValidRightBarrierWidth(getEditorFrame()));
			previousSegment.getRight().setBarrierHeight(previousSegment.getValidRightBarrierHeight(getEditorFrame()));
			previousSegment.getRight().setBarrierSurface(previousSegment.getValidRightBarrierSurface(getEditorFrame()));
			previousSegment.getRight().setBarrierStyle(previousSegment.getValidRightBarrierStyle(getEditorFrame()));

			side = pitEntry.getRight();
			borderWidth = previousSegment.getRight().getBorderWidth();
			sideWidth = previousSegment.getRight().getSideStartWidth();
		}
		side.setBorderHeight(0);
		side.setBorderWidth(0);
		side.setSideStartWidth(borderWidth + sideWidth);
		side.setSideEndWidth(pits.getWidth()*3);
		side.setSideSurface(entrySurfaceComboBox.getSelectedItem().toString());
		side.setBarrierHeight(1);
		side.setBarrierWidth(0.1);

		int start = data.indexOf(pitStart);
		int end = data.indexOf(pitEnd);
		int count;
		if (start > end)
			count = end + data.size() - start + 1;
		else
			count = end - start + 1;

		for (int i = 0; i < count; i++)
		{
			int index = (start + i) % data.size();
			if (pits.getSide().equals("left"))
			{
				side = ((Segment) data.get(index)).getLeft();
			}
			else
			{
				side = ((Segment) data.get(index)).getRight();
			}
			double width = pits.getWidth()*3;
			if (pitWallCheckBox.isSelected())
			{
				side.setBorderHeight(getDouble(pitWallHeightTextField.getText()));
				side.setBorderWidth(getDouble(pitWallWidthTextField.getText()));
				side.setBorderSurface(pitWallSurfaceComboBox.getSelectedItem().toString());
				side.setBorderStyle("wall");
				width -= side.getBorderWidth();
			}
			else
			{
				side.setBorderHeight(0);
				side.setBorderWidth(0);
				side.setBorderSurface(null);
				side.setBorderStyle(null);
			}
			side.setSideStartWidth(width);
			side.setSideEndWidth(width);
			side.setSideSurface(pitSurfaceComboBox.getSelectedItem().toString());
			side.setBarrierHeight(1);
			side.setBarrierWidth(0.1);
		}
	}
} //  @jve:decl-index=0:visual-constraint="10,10"
