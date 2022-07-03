/*
 *   GeneralProperties.java
 *   Created on 27 ??? 2005
 *
 *    The GeneralProperties.java is part of TrackEditor-0.3.1.
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

import java.io.File;

import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JTextField;

import gui.EditorFrame;
import utils.Editor;

/**
 * @author babis
 *
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class GeneralProperties extends PropertyPanel
{
	private JLabel				nameLabel				= new JLabel();
	private JTextField			nameTextField			= new JTextField();
	private JLabel				categoryLabel			= new JLabel();
	private JComboBox<String>	categoryComboBox		= null;
	private JLabel				subcategoryLabel		= new JLabel();
	private JComboBox<String>	subcategoryComboBox		= null;
	private JLabel				versionLabel			= new JLabel();
	private JComboBox<String>	versionComboBox			= null;
	private JLabel				skyVersionLabel			= new JLabel();
	private JComboBox<String>	skyVersionComboBox		= null;
	private JLabel				authorLabel				= new JLabel();
	private JTextField			authorTextField			= new JTextField();
	private JLabel				descriptionLabel		= new JLabel();
	private JTextField			descriptionTextField	= new JTextField();

	private final String sep = System.getProperty("file.separator");

	/**
	 *
	 */
	public GeneralProperties(EditorFrame editorFrame)
	{
		super(editorFrame);
		initialize();
	}

	/**
	 *
	 */
	private void initialize()
	{
		setLayout(null);
		setBorder(javax.swing.BorderFactory.createEtchedBorder(javax.swing.border.EtchedBorder.LOWERED));

		addLabel(this, 0, nameLabel, "Name", 110);
		addLabel(this, 1, categoryLabel, "Category", 110);
		addLabel(this, 2, subcategoryLabel, "Subcategory", 110);
		addLabel(this, 3, versionLabel, "Version", 110);
		addLabel(this, 4, skyVersionLabel, "Sky Version", 110);
		addLabel(this, 5, authorLabel, "Author", 80);
		addLabel(this, 6, descriptionLabel, "Description", 80);

		addTextField(this, 0, nameTextField, getEditorFrame().getTrackData().getHeader().getName(), 130, 150);

		add(getCategoryComboBox(), null);
		add(getSubcategoryComboBox(), null);
		add(getVersionComboBox(), null);
		add(getSkyVersionComboBox(), null);

		addTextField(this, 5, authorTextField, getEditorFrame().getTrackData().getHeader().getAuthor(), 85, 385);
		addTextField(this, 6, descriptionTextField, getEditorFrame().getTrackData().getHeader().getDescription(), 85, 385);
	}

	/**
	 * This method initializes categoryComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	public JComboBox<String> getCategoryComboBox()
	{
		if (categoryComboBox == null)
		{
			String[] items = {"circuit", "development", "dirt", "gprix", "karting", "oval", "road", "speedway", "test"};
			categoryComboBox = new JComboBox<String>(items);
			categoryComboBox.setBounds(130, 37, 100, 23);
			categoryComboBox.setSelectedItem(getEditorFrame().getTrackData().getHeader().getCategory());
		}
		return categoryComboBox;
	}

	/**
	 * This method initializes subcategoryComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	public JComboBox<String> getSubcategoryComboBox()
	{
		if (subcategoryComboBox == null)
		{
			String[] items = {"none", "short", "long"};
			subcategoryComboBox = new JComboBox<String>(items);
			subcategoryComboBox.setBounds(130, 64, 100, 23);
			String subcategory = getEditorFrame().getTrackData().getHeader().getSubcategory();
			if (subcategory == null)
				subcategory = "none";
			subcategoryComboBox.setSelectedItem(subcategory);
		}
		return subcategoryComboBox;
	}

	/**
	 * This method initializes versionComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	public JComboBox<String> getVersionComboBox()
	{
		if (versionComboBox == null)
		{
			String[] items = {"3", "4", "5"};
			versionComboBox = new JComboBox<String>(items);
			versionComboBox.setBounds(130, 91, 100, 23);
			versionComboBox.setSelectedItem(getEditorFrame().getTrackData().getHeader().getVersion() + "");
		}
		return versionComboBox;
	}

	/**
	 * This method initializes skyVersionComboBox
	 *
	 * @return javax.swing.JComboBox
	 */
	public JComboBox<String> getSkyVersionComboBox()
	{
		if (skyVersionComboBox == null)
		{
			String[] items = {"none", "1"};
			skyVersionComboBox = new JComboBox<String>(items);
			skyVersionComboBox.setBounds(130, 118, 100, 23);
			int version = getEditorFrame().getTrackData().getHeader().getSkyVersion();
			String stringVersion;
			if (version == Integer.MAX_VALUE)
				stringVersion = "none";
			else
				stringVersion = version + "";
			skyVersionComboBox.setSelectedItem(stringVersion);
		}
		return skyVersionComboBox;
	}

	public void exit()
	{
		MutableString stringResult = new MutableString();

		// the path is something/category/track
		String tmpPath = Editor.getProperties().getPath();
		String tmpName = nameTextField.getText();
		String tmpCategory = (String) getCategoryComboBox().getSelectedItem();

		if (isDifferent(nameTextField.getText(), getEditorFrame().getTrackData().getHeader().getName(), stringResult))
		{
			getEditorFrame().getTrackData().getHeader().setName(stringResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent((String) getCategoryComboBox().getSelectedItem(), getEditorFrame().getTrackData().getHeader().getCategory(), stringResult))
		{
			getEditorFrame().getTrackData().getHeader().setCategory(stringResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		// get the track name from the path
		int index = tmpPath.lastIndexOf(sep);
		String nameFromPath = tmpPath.substring(index + 1);
		if (!nameFromPath.equals(tmpName))
		{
			// TODO Which one should we use?
			System.out.println("different!!! name from path: " + nameFromPath + " name: " + tmpName);
		}

		// remove the track name from the path
		String pathToCategory = tmpPath.substring(0, index);

		// get the category from the path
		index = pathToCategory.lastIndexOf(sep);
		String categoryFromPath = pathToCategory.substring(index + 1);
		if (!categoryFromPath.equals(tmpCategory))
		{
			// TODO  Which one should we use?
			System.out.println("category from path : " + categoryFromPath + " category : " + tmpCategory);
		}

		File path = new File(tmpPath);
		if (!path.exists())
		{
			path.mkdirs();
		}

		if (isDifferent((String) getSubcategoryComboBox().getSelectedItem(),
			getEditorFrame().getTrackData().getHeader().getSubcategory(), stringResult))
		{
			getEditorFrame().getTrackData().getHeader().setSubcategory(stringResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		int version = Integer.parseInt((String) getVersionComboBox().getSelectedItem());
		if (version != getEditorFrame().getTrackData().getHeader().getVersion())
		{
			getEditorFrame().getTrackData().getHeader().setVersion(version);
			getEditorFrame().documentIsModified = true;
		}

		String skyVersionString = (String) getSkyVersionComboBox().getSelectedItem();
		int skyVersion;
		if (skyVersionString == "none")
			skyVersion = Integer.MAX_VALUE;
		else
			skyVersion = Integer.parseInt(skyVersionString);
		if (skyVersion != getEditorFrame().getTrackData().getHeader().getSkyVersion())
		{
			getEditorFrame().getTrackData().getHeader().setSkyVersion(skyVersion);
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(authorTextField.getText(),
			getEditorFrame().getTrackData().getHeader().getAuthor(), stringResult))
		{
			getEditorFrame().getTrackData().getHeader().setAuthor(stringResult.getValue());
			getEditorFrame().documentIsModified = true;
		}

		if (isDifferent(descriptionTextField.getText(),
			getEditorFrame().getTrackData().getHeader().getDescription(), stringResult))
		{
			getEditorFrame().getTrackData().getHeader().setDescription(stringResult.getValue());
			getEditorFrame().documentIsModified = true;
		}
	}
} //  @jve:decl-index=0:visual-constraint="10,10"
