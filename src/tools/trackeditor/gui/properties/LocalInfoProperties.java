/*
 *   LocalInfoProperties.java
 *   Created on 3 June 2022
 *
 *    The LocalInfoProperties.java is part of TrackEditor-0.7.0.
 *
 *    TrackEditor-0.7.0 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    TrackEditor-0.7.0 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with TrackEditor-0.7.0; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
package gui.properties;

import javax.swing.JLabel;
import javax.swing.JTextField;

import gui.EditorFrame;

/**
 * @author Robert Reif
 *
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class LocalInfoProperties extends PropertyPanel
{
	private JLabel		stationLabel					= new JLabel();
	private JTextField	stationTextField				= new JTextField();
	private JLabel		timezoneLabel					= new JLabel();
	private JTextField	timezoneTextField				= new JTextField();
	private JLabel		overallRainLikelyhoodLabel		= new JLabel();
	private JTextField	overallRainLikelyhoodTextField	= new JTextField();
	private JLabel		littleRainLikelyhoodLabel		= new JLabel();
	private JTextField	littleRainLikelyhoodTextField	= new JTextField();
	private JLabel		mediumRainLikelyhoodLabel		= new JLabel();
	private JTextField	mediumRainLikelyhoodTextField	= new JTextField();
	private JLabel		timeOfDayLabel					= new JLabel();
	private JTextField	timeOfDayTextField				= new JTextField();
	private JLabel		sunAscensionLabel				= new JLabel();
	private JTextField	sunAscensionTextField			= new JTextField();
	private JLabel		altitudeLabel					= new JLabel();
	private JTextField	altitudeTextField				= new JTextField();

	/**
	 *
	 */
	public LocalInfoProperties(EditorFrame frame)
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
		setLayout(null);
		setBorder(javax.swing.BorderFactory.createEtchedBorder(javax.swing.border.EtchedBorder.LOWERED));

		addLabel(this, 0, stationLabel, "Station", 150);
		addLabel(this, 1, timezoneLabel, "Timezone", 150);
		addLabel(this, 2, overallRainLikelyhoodLabel, "Overall Rain Likelyhood", 150);
		addLabel(this, 3, littleRainLikelyhoodLabel, "Little Rain Likelyhood", 150);
		addLabel(this, 4, mediumRainLikelyhoodLabel, "Medium Rain Likelyhood", 150);
		addLabel(this, 5, timeOfDayLabel, "Time Of Day", 150);
		addLabel(this, 6, sunAscensionLabel, "Sun Ascension", 150);
		addLabel(this, 7, altitudeLabel, "Altitude", 150);

		addTextField(this, 0, stationTextField, getEditorFrame().getTrackData().getLocalInfo().getStation(), 170, 125);
		addTextField(this, 1, timezoneTextField, getEditorFrame().getTrackData().getLocalInfo().getTimezone(), 170, 125);
		addTextField(this, 2, overallRainLikelyhoodTextField, getEditorFrame().getTrackData().getLocalInfo().getOverallRainLikelyhood(), 170, 125);
		addTextField(this, 3, littleRainLikelyhoodTextField, getEditorFrame().getTrackData().getLocalInfo().getLittleRainLikelyhood(), 170, 125);
		addTextField(this, 4, mediumRainLikelyhoodTextField, getEditorFrame().getTrackData().getLocalInfo().getMediumRainLikelyhood(), 170, 125);
		addTextField(this, 5, timeOfDayTextField, getEditorFrame().getTrackData().getLocalInfo().getTimeOfDay(), 170, 125);
		addTextField(this, 6, sunAscensionTextField, getEditorFrame().getTrackData().getLocalInfo().getSunAscension(), 170, 125);
		addTextField(this, 7, altitudeTextField, getEditorFrame().getTrackData().getLocalInfo().getAltitude(), 170, 125);
	}

	/**
	 *
	 */
	public void exit()
	{
		MutableString stringResult = new MutableString();
		MutableDouble doubleResult = new MutableDouble();

		if (isDifferent(stationTextField.getText(),
			getEditorFrame().getTrackData().getLocalInfo().getStation(), stringResult))
		{
			getEditorFrame().getTrackData().getLocalInfo().setStation(stringResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(timezoneTextField.getText(),
			getEditorFrame().getTrackData().getLocalInfo().getTimezone(), doubleResult))
		{
			getEditorFrame().getTrackData().getLocalInfo().setTimezone(doubleResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(overallRainLikelyhoodTextField.getText(),
			getEditorFrame().getTrackData().getLocalInfo().getOverallRainLikelyhood(), doubleResult))
		{
			getEditorFrame().getTrackData().getLocalInfo().setOverallRainLikelyhood(doubleResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(littleRainLikelyhoodTextField.getText(),
			getEditorFrame().getTrackData().getLocalInfo().getLittleRainLikelyhood(), doubleResult))
		{
			getEditorFrame().getTrackData().getLocalInfo().setLittleRainLikelyhood(doubleResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(mediumRainLikelyhoodTextField.getText(),
			getEditorFrame().getTrackData().getLocalInfo().getMediumRainLikelyhood(), doubleResult))
		{
			getEditorFrame().getTrackData().getLocalInfo().setMediumRainLikelyhood(doubleResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(timeOfDayTextField.getText(),
			getEditorFrame().getTrackData().getLocalInfo().getTimeOfDay(), doubleResult))
		{
			getEditorFrame().getTrackData().getLocalInfo().setTimeOfDay(doubleResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(sunAscensionTextField.getText(),
			getEditorFrame().getTrackData().getLocalInfo().getSunAscension(), doubleResult))
		{
			getEditorFrame().getTrackData().getLocalInfo().setSunAscension(doubleResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(altitudeTextField.getText(),
			getEditorFrame().getTrackData().getLocalInfo().getAltitude(), doubleResult))
		{
			getEditorFrame().getTrackData().getLocalInfo().setAltitude(doubleResult.getValue());
			getEditorFrame().documentIsModified = true;
		}
	}
} //  @jve:decl-index=0:visual-constraint="10,10"
