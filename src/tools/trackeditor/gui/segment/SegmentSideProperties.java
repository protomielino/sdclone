/*
 *   SegmentSideProperties.java
 *   Created on Aug 25, 2004
 *
 *    The SegmentSideProperties.java is part of TrackEditor-0.2.0.
 *
 *    TrackEditor-0.2.0 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    TrackEditor-0.2.0 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with TrackEditor-0.2.0; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
package gui.segment;

import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Arrays;
import java.util.Collections;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.border.EtchedBorder;

import utils.SurfaceComboBox;
import utils.circuit.SegmentSide;
import utils.circuit.Surface;
import bsh.EvalError;
import bsh.Interpreter;
/**
 * @author babis
 *
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class SegmentSideProperties extends JPanel implements SliderListener
{
	private SegmentEditorDlg	editor;
	private SegmentSide			side;
	private SegmentEditorDlg	parent;

	private String[]			borderStyleItems		= {"none", "plan", "wall", "curb"};
	private String[]			borderSurfaceItems		=
														{"curb-5cm-r", "curb-5cm-l", "curb-l", "tar-grass3-l",
			"tar-grass3-r", "tar-sand", "b-road1-grass6", "b-road1-grass6-l2", "b-road1-gravel-l2", "b-road1-sand3",
			"b-road1-sand3-l2", "b-asphalt-grass7", "b-asphalt-grass7-l1", "b-asphalt-grass6", "b-asphalt-grass6-l1",
			"b-asphalt-sand3", "b-asphalt-sand3-l1", "grass", "grass3", "grass5", "grass6", "grass7", "gravel",
			"sand3", "sand", "asphalt-lines", "asphalt-l-left", "asphalt-l-right", "asphalt-l-both", "asphalt-pits",
			"asphalt", "dirt", "dirt-b", "asphalt2", "road1", "road1-pits", "road1-asphalt", "asphalt-road1",
			"b-road1", "b-road1-l2", "b-road1-l2p", "concrete", "concrete2", "concrete3", "b-asphalt", "b-asphalt-l1",
			"b-asphalt-l1p", "asphalt2-lines", "asphalt2-l-left", "asphalt2-l-right", "asphalt2-l-both", "barrier",
			"barrier2", "barrier-turn", "barrier-grille", "wall", "wall2", "tire-wall"};
	private Vector<String>		borderSurfaceVector		= new Vector<String>();
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
	private Vector<String>		sideSurfaceVector		= new Vector<String>();
	private String[]			barrierStyleItems		= {"none", "no barrier", "wall", "fence"};
	private String[]			barrierSurfaceItems		=
														{"barrier", "barrier2", "barrier-turn", "barrier-grille",
			"wall", "wall2", "tire-wall", "asphalt-lines", "asphalt-l-left", "asphalt-l-right", "asphalt-l-both",
			"asphalt-pits", "asphalt", "dirt", "dirt-b", "asphalt2", "road1", "road1-pits", "road1-asphalt",
			"asphalt-road1", "b-road1", "b-road1-l2", "b-road1-l2p", "concrete", "concrete2", "concrete3", "b-asphalt",
			"b-asphalt-l1", "b-asphalt-l1p", "asphalt2-lines", "asphalt2-l-left", "asphalt2-l-right",
			"asphalt2-l-both", "curb-5cm-r", "curb-5cm-l", "curb-l", "tar-grass3-l", "tar-grass3-r", "tar-sand",
			"b-road1-grass6", "b-road1-grass6-l2", "b-road1-gravel-l2", "b-road1-sand3", "b-road1-sand3-l2",
			"b-asphalt-grass7", "b-asphalt-grass7-l1", "b-asphalt-grass6", "b-asphalt-grass6-l1", "b-asphalt-sand3",
			"b-asphalt-sand3-l1", "grass", "grass3", "grass5", "grass6", "grass7", "gravel", "sand3", "sand"};
	private Vector<String>		barrierSurfaceVector	= new Vector<String>();

	public JPanel				panel					= null;
	private JLabel				borderLabel				= null;
	private JLabel				borderSurfaceLabel		= null;
	private JLabel				borderStyleLabel		= null;
	private JLabel				sideLabel				= null;
	private JLabel				sideSurfaceLabel		= null;
	private JLabel				sideBankingTypeLabel	= null;
	private JLabel				barrierLabel			= null;
	private JLabel				barrierStyleLabel		= null;
	private JLabel				barrierSurfaceLabel		= null;
	public JLabel				titleLabel				= null;
	private SegmentSlider		barrierHeightSlider		= null;
	private SurfaceComboBox		borderSurfaceComboBox	= null;
	private JComboBox<String>	borderStyleComboBox		= null;
	private SurfaceComboBox		sideSurfaceComboBox		= null;
	private JComboBox<String>	sideBankingTypeComboBox	= null;
	private SurfaceComboBox		barrierSurfaceComboBox	= null;
	private JComboBox<String>	barrierStyleComboBox	= null;
	private SegmentSlider		sideStartWidthSlider	= null;
	private SegmentSlider		borderWidthSlider		= null;
	private SegmentSlider		sideEndWidthSlider		= null;
	private SegmentSlider		barrierWidthSlider		= null;
	private SegmentSlider		borderHeightSlider		= null;

	private boolean				setSource				= false;

	/**
	 *
	 */
	public SegmentSideProperties(SegmentEditorDlg parent, SegmentSide side)
	{
		this.parent = parent;
		
		// add new surfaces from Surfaces
		addDefaultSurfaces(sideSurfaceVector, sideSurfaceItems);
		addDefaultSurfaces(borderSurfaceVector, borderSurfaceItems);
		addDefaultSurfaces(barrierSurfaceVector, barrierSurfaceItems);
		
		setSide(side);
		initialize();
	}

	/**
	 * This method initializes this
	 *
	 * @return void
	 */
	private void initialize()
	{
		titleLabel = new JLabel();
		this.setLayout(null);
		this.setBorder(BorderFactory.createEtchedBorder(EtchedBorder.LOWERED));
		this.setLocation(0, 0);
		titleLabel.setBounds(396, 5, 70, 23);
		titleLabel.setText("Right");
		titleLabel.setFont(new Font("Dialog", Font.BOLD, 18));
		this.add(getPanel(), null);
		this.add(titleLabel, null);
	}

	public void refresh()
	{
		sideSurfaceVector.clear();
		addDefaultSurfaces(sideSurfaceVector, sideSurfaceItems);
		panel.remove(sideSurfaceComboBox);
		sideSurfaceComboBox = null;
		panel.add(getSideSurfaceComboBox(), null);

		borderSurfaceVector.clear();
		addDefaultSurfaces(borderSurfaceVector, borderSurfaceItems);
		panel.remove(borderSurfaceComboBox);
		borderSurfaceComboBox = null;
		panel.add(getBorderSurfaceComboBox(), null);

		barrierSurfaceVector.clear();
		addDefaultSurfaces(barrierSurfaceVector, barrierSurfaceItems);
		panel.remove(barrierSurfaceComboBox);
		barrierSurfaceComboBox = null;
		panel.add(getBarrierSurfaceComboBox(), null);
	}

	private void addDefaultSurfaces(Vector<String> surfaceVector, String[] fallback)
	{
		// use the default surfaces if available
		Vector<Surface> defaultSurfaces = parent.editorFrame.getDefaultSurfaces();

		if (defaultSurfaces.isEmpty())
		{
			// add the ones found
			surfaceVector.addAll(Arrays.asList(fallback));
		}
		else
		{
			// add the ones that should be there
	        for (int i = 0; i < defaultSurfaces.size(); i++)
	        {
	        	surfaceVector.add(defaultSurfaces.get(i).getName());
	        }
		}

        Vector<Surface> surfaces = parent.editorFrame.getTrackData().getSurfaces();
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
	 * This method initializes panel
	 *
	 * @return javax.swing.JPanel
	 */
	public JPanel getPanel()
	{
		if (panel == null)
		{
			borderLabel = new JLabel();
			borderStyleLabel = new JLabel();
			borderSurfaceLabel = new JLabel();
			panel = new JPanel();
			sideSurfaceLabel = new JLabel();
			sideLabel = new JLabel();
			sideBankingTypeLabel = new JLabel();
			barrierSurfaceLabel = new JLabel();
			barrierStyleLabel = new JLabel();
			barrierLabel = new JLabel();
			panel.setLayout(null);
			panel.setBorder(BorderFactory.createEtchedBorder(EtchedBorder.LOWERED));
			borderLabel.setBounds(698, 2, 50, 23);
			borderLabel.setText("Border");
			borderSurfaceLabel.setBounds(698, 325, 55, 23);
			borderSurfaceLabel.setText("Surface");
			borderStyleLabel.setBounds(703, 375, 55, 23);
			borderStyleLabel.setText("Style");
			sideBankingTypeLabel.setBounds(376, 375, 90, 23);
			sideBankingTypeLabel.setText("Banking Type");
			panel.add(borderSurfaceLabel, null);
			panel.add(borderStyleLabel, null);
			panel.add(borderLabel, null);
			panel.add(sideLabel, null);
			panel.add(sideSurfaceLabel, null);
			panel.add(barrierLabel, null);
			panel.add(barrierStyleLabel, null);
			panel.add(barrierSurfaceLabel, null);
			panel.add(sideBankingTypeLabel, null);
			panel.setLocation(3, 35);
			panel.setSize(823, 431);
			sideLabel.setText("Side");
			sideLabel.setSize(45, 23);
			sideLabel.setLocation(401, 2);
			sideSurfaceLabel.setText("Surface");
			sideSurfaceLabel.setBounds(391, 325, 55, 23);
			barrierLabel.setText("Barrier");
			barrierLabel.setBounds(85, 2, 50, 23);
			barrierStyleLabel.setText("Style");
			barrierStyleLabel.setBounds(90, 375, 55, 23);
			barrierSurfaceLabel.setText("Surface");
			barrierSurfaceLabel.setBounds(85, 325, 55, 23);
			panel.add(getBarrierHeightSlider(), null);
			panel.add(getBorderSurfaceComboBox(), null);
			panel.add(getBorderStyleComboBox(), null);
			panel.add(getSideSurfaceComboBox(), null);
			panel.add(getSideBankingTypeComboBox(), null);
			panel.add(getBarrierSurfaceComboBox(), null);
			panel.add(getBarrierStyleComboBox(), null);
			panel.add(getSideStartWidthSlider(), null);
			panel.add(getBorderWidthSlider(), null);
			panel.add(getSideEndWidthSlider(), null);
			panel.add(getBarrierWidthSlider(), null);
			panel.add(getBorderHeightSlider(), null);
		}
		return panel;
	}

	/**
	 * This method initializes borderSurfaceComboBox
	 *
	 * @return utils.SurfaceComboBox
	 */
	private SurfaceComboBox getBorderSurfaceComboBox()
	{
		if (borderSurfaceComboBox == null)
		{
			borderSurfaceComboBox = new SurfaceComboBox(parent.editorFrame, borderSurfaceVector);
			borderSurfaceComboBox.setBounds(618, 350, 200, 23);
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
	 * @return javax.swing.JComboBox
	 */
	private JComboBox<String> getBorderStyleComboBox()
	{
		if (borderStyleComboBox == null)
		{
			borderStyleComboBox = new JComboBox<String>();
			borderStyleComboBox.setBounds(658, 400, 120, 23);
			borderStyleComboBox.setModel(new DefaultComboBoxModel<String>(borderStyleItems));
			borderStyleComboBox.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					String oldStyle = side.getBorderStyle();
					if (oldStyle == null || oldStyle.isEmpty())
						oldStyle = "none";
					String newStyle = borderStyleComboBox.getSelectedItem().toString();
					boolean styleChanged = !oldStyle.equals(newStyle);
					String style = newStyle;
					if (style == "none")
						style = "";
					side.setBorderStyle(style);
					if (setSource || !styleChanged)
					{
						return;
					}
					switch (newStyle)
					{
					case "none":
						side.setBorderSurface(null);
						side.setBorderHeight(Double.NaN);
						side.setBorderWidth(Double.NaN);
						getBorderSurfaceComboBox().setSelectedIndex(-1);
						getBorderSurfaceComboBox().setEnabled(false);
						getBorderHeightSlider().setValue(side.getBorderHeight());
						getBorderHeightSlider().setEnabled(false);
						getBorderWidthSlider().setValue(side.getBorderWidth());
						getBorderWidthSlider().setEnabled(false);
						break;
					case "plan":
						if (side.isRight())
						{
							side.setBorderSurface(SegmentSide.DEFAULT_BORDER_PLAN_RIGHT_SURFACE);
						}
						else
						{
							side.setBorderSurface(SegmentSide.DEFAULT_BORDER_PLAN_LEFT_SURFACE);
						}
						side.setBorderHeight(SegmentSide.DEFAULT_BORDER_PLAN_HEIGHT);
						side.setBorderWidth(SegmentSide.DEFAULT_BORDER_PLAN_WIDTH);
						getBorderSurfaceComboBox().setEnabled(true);
						getBorderSurfaceComboBox().setSelectedItem(side.getBorderSurface());
						getBorderHeightSlider().setEnabled(true);
						getBorderHeightSlider().setValueFrozen(side.getBorderHeight());
						getBorderWidthSlider().setEnabled(true);
						getBorderWidthSlider().setValue(side.getBorderWidth());
						break;
					case "wall":
						side.setBorderSurface(SegmentSide.DEFAULT_BORDER_WALL_SURFACE);
						side.setBorderHeight(SegmentSide.DEFAULT_BORDER_WALL_HEIGHT);
						side.setBorderWidth(SegmentSide.DEFAULT_BORDER_WALL_WIDTH);
						getBorderSurfaceComboBox().setEnabled(true);
						getBorderSurfaceComboBox().setSelectedItem(side.getBorderSurface());
						getBorderHeightSlider().setEnabled(true);
						getBorderHeightSlider().setValue(side.getBorderHeight());
						getBorderWidthSlider().setEnabled(true);
						getBorderWidthSlider().setValue(side.getBorderWidth());
						break;
					case "curb":
						if (side.isRight())
						{
							side.setBorderSurface(SegmentSide.DEFAULT_BORDER_CURB_RIGHT_SURFACE);
						}
						else
						{
							side.setBorderSurface(SegmentSide.DEFAULT_BORDER_CURB_LEFT_SURFACE);
						}
						side.setBorderHeight(SegmentSide.DEFAULT_BORDER_CURB_HEIGHT);
						side.setBorderWidth(SegmentSide.DEFAULT_BORDER_CURB_WIDTH);
						getBorderSurfaceComboBox().setEnabled(true);
						getBorderSurfaceComboBox().setSelectedItem(side.getBorderSurface());
						getBorderHeightSlider().setEnabled(true);
						getBorderHeightSlider().setValue(side.getBorderHeight());
						getBorderWidthSlider().setEnabled(true);
						getBorderWidthSlider().setValue(side.getBorderWidth());
						break;
					}
					parent.sideChanged();
				}
			});
		}
		return borderStyleComboBox;
	}
	/**
	 * This method initializes sideSurfaceComboBox
	 *
	 * @return utils.SurfaceComboBox
	 */
	private SurfaceComboBox getSideSurfaceComboBox()
	{
		if (sideSurfaceComboBox == null)
		{
			sideSurfaceComboBox = new SurfaceComboBox(parent.editorFrame, sideSurfaceVector);
			sideSurfaceComboBox.setBounds(311, 350, 200, 23);
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
	 * @return javax.swing.JComboBox
	 */
	private JComboBox<String> getSideBankingTypeComboBox()
	{
		if (sideBankingTypeComboBox == null)
		{
			String[] items = {"none", "level", "tangent"};
			sideBankingTypeComboBox = new JComboBox<String>();
			sideBankingTypeComboBox.setBounds(351, 400, 120, 23);
			sideBankingTypeComboBox.setModel(new DefaultComboBoxModel<String>(items));
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
	 * This method initializes barrierSurfaceComboBox
	 *
	 * @return utils.SurfaceComboBox
	 */
	private SurfaceComboBox getBarrierSurfaceComboBox()
	{
		if (barrierSurfaceComboBox == null)
		{
			barrierSurfaceComboBox = new SurfaceComboBox(parent.editorFrame, barrierSurfaceVector);
			barrierSurfaceComboBox.setBounds(5, 350, 200, 23);
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
	 * @return javax.swing.JComboBox
	 */
	private JComboBox<String> getBarrierStyleComboBox()
	{
		if (barrierStyleComboBox == null)
		{
			barrierStyleComboBox = new JComboBox<String>();
			barrierStyleComboBox.setBounds(45, 400, 120, 23);
			barrierStyleComboBox.setModel(new DefaultComboBoxModel<String>(barrierStyleItems));
			barrierStyleComboBox.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					String oldStyle = side.getBarrierStyle();
					if (oldStyle == null || oldStyle.isEmpty())
						oldStyle = "none";
					String newStyle = barrierStyleComboBox.getSelectedItem().toString();
					boolean styleChanged = !oldStyle.equals(newStyle);
					String style = newStyle;
					if (style == "none")
						style = "";
					side.setBarrierStyle(style);
					if (setSource || !styleChanged)
						return;
					switch (newStyle)
					{
					case "none":
						side.setBarrierSurface(null);
						side.setBarrierHeight(Double.NaN);
						side.setBarrierWidth(Double.NaN);
						getBarrierSurfaceComboBox().setSelectedIndex(-1);
						getBarrierSurfaceComboBox().setEnabled(false);
						getBarrierHeightSlider().setValue(side.getBarrierHeight());
						getBarrierHeightSlider().setEnabled(false);
						getBarrierWidthSlider().setValue(side.getBarrierWidth());
						getBarrierWidthSlider().setEnabled(false);
						break;
					case "no barrier":
						side.setBarrierSurface(null);
						side.setBarrierHeight(0.0);
						side.setBarrierWidth(0.0);
						getBarrierSurfaceComboBox().setSelectedIndex(-1);
						getBarrierSurfaceComboBox().setEnabled(false);
						getBarrierHeightSlider().setEnabled(true);
						getBarrierHeightSlider().setValueFrozen(side.getBarrierHeight());
						getBarrierWidthSlider().setEnabled(true);
						getBarrierWidthSlider().setValueFrozen(side.getBarrierWidth());
						break;
					case "wall":
						side.setBarrierSurface(SegmentSide.DEFAULT_BARRIER_WALL_SURFACE);
						side.setBarrierHeight(SegmentSide.DEFAULT_BARRIER_WALL_HEIGHT);
						side.setBarrierWidth(SegmentSide.DEFAULT_BARRIER_WALL_WIDTH);
						getBarrierSurfaceComboBox().setEnabled(true);
						getBarrierSurfaceComboBox().setSelectedItem(side.getBarrierSurface());
						getBarrierHeightSlider().setEnabled(true);
						getBarrierHeightSlider().setValue(side.getBarrierHeight());
						getBarrierWidthSlider().setEnabled(true);
						getBarrierWidthSlider().setValue(side.getBarrierWidth());
						break;
					case "fence":
						side.setBarrierSurface(SegmentSide.DEFAULT_BARRIER_FENCE_SURFACE);
						side.setBarrierHeight(SegmentSide.DEFAULT_BARRIER_FENCE_HEIGHT);
						side.setBarrierWidth(SegmentSide.DEFAULT_BARRIER_FENCE_WIDTH);
						getBarrierSurfaceComboBox().setEnabled(true);
						getBarrierSurfaceComboBox().setSelectedItem(side.getBarrierSurface());
						getBarrierHeightSlider().setEnabled(true);
						getBarrierHeightSlider().setValue(side.getBarrierHeight());
						getBarrierWidthSlider().setEnabled(true);
						getBarrierWidthSlider().setValueFrozen(side.getBarrierWidth());
						break;
					}
					parent.sideChanged();
				}
			});
		}
		return barrierStyleComboBox;
	}
	/**
	 * This method initializes barrierHeightSlider
	 *
	 * @return gui.SegmentSlider
	 */
	private SegmentSlider getBarrierHeightSlider()
	{
		if (barrierHeightSlider == null)
		{
			barrierHeightSlider = new SegmentSlider(0, 50, 0.5, side.getBarrierHeight(), "Height", "", "BarrierHeight", true);
			barrierHeightSlider.setBounds(52, 25, 50, 290);
			barrierHeightSlider.addSliderListener(this);
		}
		return barrierHeightSlider;
	}

	/**
	 * This method initializes sideStartWidthSlider
	 *
	 * @return gui.SegmentSlider
	 */
	private SegmentSlider getSideStartWidthSlider()
	{
		if (sideStartWidthSlider == null)
		{
			sideStartWidthSlider = new SegmentSlider(0, 100, 0.5, side.getSideStartWidth(), "Width", "Start", "SideStartWidth", true);
			sideStartWidthSlider.setBounds(358, 25, 50, 290);
			sideStartWidthSlider.addSliderListener(this);
		}
		return sideStartWidthSlider;
	}
	/**
	 * This method initializes borderWidthSlider
	 *
	 * @return gui.SegmentSlider
	 */
	private SegmentSlider getBorderWidthSlider()
	{
		if (borderWidthSlider == null)
		{
			borderWidthSlider = new SegmentSlider(0, 5, 0.05, side.getBorderWidth(), "Width", "", "BorderWidth", true);
			borderWidthSlider.setBounds(720, 25, 50, 290);
			borderWidthSlider.addSliderListener(this);
		}
		return borderWidthSlider;
	}
	/**
	 * This method initializes sideEndWidthSlider
	 *
	 * @return gui.SegmentSlider
	 */
	private SegmentSlider getSideEndWidthSlider()
	{
		if (sideEndWidthSlider == null)
		{
			sideEndWidthSlider = new SegmentSlider(0, 100, 0.5, side.getSideEndWidth(), "Width", "End", "SideEndWidth", true);
			sideEndWidthSlider.setBounds(413, 25, 50, 290);
			sideEndWidthSlider.addSliderListener(this);
		}
		return sideEndWidthSlider;
	}
	/**
	 * This method initializes barrierWidthSlider
	 *
	 * @return gui.segment.SegmentSlider
	 */
	private SegmentSlider getBarrierWidthSlider()
	{
		if (barrierWidthSlider == null)
		{
			barrierWidthSlider = new SegmentSlider(0, 50, 0.1, side.getBarrierWidth(), "Width", "", "BarrierWidth", true);
			barrierWidthSlider.setBounds(107, 25, 50, 290);
			barrierWidthSlider.addSliderListener(this);
		}
		return barrierWidthSlider;
	}
	/**
	 * This method initializes borderHeightSlider
	 *
	 * @return gui.segment.SegmentSlider
	 */
	private SegmentSlider getBorderHeightSlider()
	{
		if (borderHeightSlider == null)
		{
			borderHeightSlider = new SegmentSlider(0, 5, 0.005, side.getBorderHeight(), "Height", "", "BorderHeight", true);
			borderHeightSlider.setBounds(665, 25, 50, 290);
			borderHeightSlider.addSliderListener(this);
		}
		return borderHeightSlider;
	}

	/**
	 * @param editor
	 *            The editor to set.
	 */
	public void setEditor(SegmentEditorDlg editor)
	{
		this.editor = editor;
	}

	public void update()
	{
	}

	/**
	 * @return Returns the titleLabel.
	 */
	public String getTitle()
	{
		return titleLabel.getText();
	}
	/**
	 * @param titleLabel
	 *            The titleLabel to set.
	 */
	public void setTitle(String title)
	{
		this.titleLabel.setText(title);
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

	public void setSide(SegmentSide side)
	{
		setSource = true;

		this.side = side;

		// update side
		String surface = this.side.getSideSurface();
		addSurface(sideSurfaceVector, surface);

		if (surface != null)
			this.getSideSurfaceComboBox().setSelectedItem(surface);
		else
			this.getSideSurfaceComboBox().setSelectedIndex(-1);

		String bankingType = this.side.getSideBankingType();
		if (bankingType == null || bankingType.isEmpty())
			bankingType = "none";
		this.getSideBankingTypeComboBox().setSelectedItem(bankingType);

		this.getSideStartWidthSlider().setValue(side.getSideStartWidth());
		this.getSideEndWidthSlider().setValue(side.getSideEndWidth());

		// update border
		surface = this.side.getBorderSurface();
		addSurface(borderSurfaceVector, surface);	

		if (surface != null && !surface.isEmpty())
			this.getBorderSurfaceComboBox().setSelectedItem(surface);
		else
			this.getBorderSurfaceComboBox().setSelectedIndex(-1);

		String style = side.getBorderStyle();
		if (style == null || style.isEmpty())
			style = "none";
		switch (style)
		{
		case "none":
			getBorderHeightSlider().setValue(side.getBorderHeight());
			getBorderHeightSlider().setEnabled(false);
			getBorderWidthSlider().setValue(side.getBorderWidth());
			getBorderWidthSlider().setEnabled(false);
			break;
		case "plan":
			/*
			// fix up bad value (height should be 0)
			if (!Double.isNaN(side.getBorderWidth()) && side.getBorderHeight() != 0)
			{
				String[] options = { "Set height to 0.0", "Set style to wall", "Ignore" };
				switch (JOptionPane.showOptionDialog(null, "Found " + side.getName() + " border style \"plan\" with height " + side.getBorderHeight() + "", "Invalid Border Height", 0, 3, null, options, options[2]))
				{
				case 0:
					side.setBorderHeight(0);
					break;
				case 1:
					side.setBorderStyle("wall");
					break;
				case 2:
					// nothing
					break;
				}
			}
			*/
			getBorderHeightSlider().setEnabled(true);
			getBorderHeightSlider().setValueFrozen(side.getBorderHeight());
			getBorderWidthSlider().setEnabled(true);
			getBorderWidthSlider().setValue(side.getBorderWidth());
			break;
		case "wall":
			getBorderHeightSlider().setEnabled(true);
			getBorderHeightSlider().setValue(side.getBorderHeight());
			getBorderWidthSlider().setEnabled(true);
			getBorderWidthSlider().setValue(side.getBorderWidth());
			break;
		case "curb":
			getBorderHeightSlider().setEnabled(true);
			getBorderHeightSlider().setValue(side.getBorderHeight());
			getBorderWidthSlider().setEnabled(true);
			getBorderWidthSlider().setValue(side.getBorderWidth());
			break;
		}
		this.getBorderStyleComboBox().setSelectedItem(style);
		
		// update barrier
		surface = this.side.getBarrierSurface();
		addSurface(barrierSurfaceVector, surface);

		if (surface != null)
			this.getBarrierSurfaceComboBox().setSelectedItem(surface);
		else
			this.getBarrierSurfaceComboBox().setSelectedIndex(-1);
		
		this.getBarrierHeightSlider().setValue(side.getBarrierHeight());
		this.getBarrierWidthSlider().setValue(side.getBarrierWidth());

		style = side.getBarrierStyle();
		if (style == null || style.isEmpty())
			style = "none";
		switch (style)
		{
		case "none":
			getBarrierHeightSlider().setValue(side.getBarrierHeight());
			getBarrierHeightSlider().setEnabled(false);
			getBarrierWidthSlider().setValue(side.getBarrierWidth());
			getBarrierWidthSlider().setEnabled(false);
			break;
		case "no barrier":
			/*
			// fix up bad values (width and height should be 0)
			if (Double.isNaN(side.getBarrierHeight()) || side.getBarrierHeight() != 0)
			{
				if (JOptionPane.showConfirmDialog(null, "Fix barrier height : " + side.getBarrierHeight() + " ?\n\nShould be 0.0.", "Fix Barrier Height", JOptionPane.YES_NO_OPTION) == JOptionPane.YES_OPTION)
				{
					side.setBarrierHeight(0);
				}
			}
			if (Double.isNaN(side.getBarrierWidth()) || side.getBarrierWidth() != 0)
			{
				if (JOptionPane.showConfirmDialog(null, "Fix barrier width : " + side.getBarrierWidth() + " ?\n\nShould be 0.0.", "Fix Barrier Width", JOptionPane.YES_NO_OPTION) == JOptionPane.YES_OPTION)
				{
					side.setBarrierWidth(0);
				}
			}
			*/
			getBarrierHeightSlider().setEnabled(true);
			getBarrierHeightSlider().setValueFrozen(side.getBarrierHeight());
			getBarrierWidthSlider().setEnabled(true);
			getBarrierWidthSlider().setValueFrozen(side.getBarrierWidth());
			break;
		case "wall":
			getBarrierHeightSlider().setEnabled(true);
			getBarrierHeightSlider().setValue(side.getBarrierHeight());
			getBarrierWidthSlider().setEnabled(true);
			getBarrierWidthSlider().setValue(side.getBarrierWidth());
			break;
		case "fence":
			/*
			// fix up bad value (width should be 0)
			if (!Double.isNaN(side.getBarrierWidth()) && side.getBarrierWidth() != 0)
			{
				String[] options = { "Set width to 0.0", "Set style to wall", "Ignore" };
				switch (JOptionPane.showOptionDialog(null, "Found " + side.getName() + " barrier style \"fence\" with width " + side.getBarrierWidth() + "", "Invalid Barrier Width", 0, 3, null, options, options[2]))
				{
				case 0:
					side.setBarrierWidth(0);
					break;
				case 1:
					side.setBarrierStyle("wall");
					break;
				case 2:
					// nothing
					break;
				}
			}
			*/
			getBarrierHeightSlider().setEnabled(true);
			getBarrierHeightSlider().setValue(side.getBarrierHeight());
			getBarrierWidthSlider().setEnabled(true);
			getBarrierWidthSlider().setValueFrozen(side.getBarrierWidth());
			break;
		}

		if (style == null || style.isEmpty())
			style = "none";
		this.getBarrierStyleComboBox().setSelectedItem(style);
		
		this.validate();
		this.repaint();

		setSource = false;
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see gui.segment.SliderListener#valueChanged(gui.segment.SegmentSlider)
	 */
	public void sliderChanged(SegmentSlider slider)
	{
		Interpreter line = new Interpreter();
		String command = "";

		try
		{
			line.set("side", side);

			String method = slider.getMethod();

			if (Double.isNaN(slider.getValue()))
				command = "side.set" + method + "(Double.NaN)";
			else
				command = "side.set" + method + "(" + slider.getValue() + ")";

			line.eval(command);
			side = (SegmentSide) line.get("side");
		} catch (EvalError e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		parent.update();
	}

} //  @jve:decl-index=0:visual-constraint="42,-30"
