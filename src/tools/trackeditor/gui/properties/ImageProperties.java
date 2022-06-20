/*
 *   ImageProperties.java
 *   Created on 27 ��� 2005
 *
 *    The ImageProperties.java is part of TrackEditor-0.3.1.
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

import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.UIManager;
import javax.swing.filechooser.FileNameExtensionFilter;

import gui.EditorFrame;
import utils.Editor;

/**
 * @author babis
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class ImageProperties extends PropertyPanel
{
	private JLabel		pathLabel				= new JLabel();
	private JTextField	pathTextField			= new JTextField();
	private JButton		pathButton				= null;
	private JLabel 		imageScaleLabel			= new JLabel();
	private JTextField	imageScaleTextField		= new JTextField();

	private final String sep = System.getProperty("file.separator");

	/**
	 *
	 */
	public ImageProperties(EditorFrame frame)
	{
		super(frame);
		initialize();
	}

	/**
	 *
	 */
	private void initialize()
	{
		setLayout(null);
		setBorder(javax.swing.BorderFactory.createEtchedBorder(javax.swing.border.EtchedBorder.LOWERED));

		addLabel(this, 0, imageScaleLabel, "Path", 80);
		addLabel(this, 1, pathLabel, "Image scale", 80);

		addTextField(this, 0, pathTextField, Editor.getProperties().getImage(), 100, 285);
		addTextField(this, 1, imageScaleTextField, Editor.getProperties().getImageScale(), 100, 50);

		this.add(getPathButton(), null);
	}

	/**
	 * This method initializes pathButton
	 *
	 * @return javax.swing.JButton
	 */
	public JButton getPathButton()
	{
		if (pathButton == null)
		{
			pathButton = new JButton();
			pathButton.setBounds(390, 7, 80, 25);
			pathButton.setText("Browse");
			pathButton.addActionListener(new java.awt.event.ActionListener()
			{
				public void actionPerformed(java.awt.event.ActionEvent e)
				{
					selectPath();
				}
			});
		}
		return pathButton;
	}

	/**
	 *
	 */
	protected void selectPath()
	{
		String tmp = "";
		String filename = Editor.getProperties().getImage();
		Boolean old = UIManager.getBoolean("FileChooser.readOnly");  
		UIManager.put("FileChooser.readOnly", Boolean.TRUE);  
		JFileChooser fc = new JFileChooser();
		fc.setSelectedFiles(null);
		fc.setSelectedFile(null);
		fc.rescanCurrentDirectory();
		fc.setApproveButtonMnemonic(0);
		fc.setDialogTitle("Background image file selection");
		fc.setVisible(true);
		if(Editor.getProperties().getImage().equals(""))
		{
			fc.setCurrentDirectory(new File(System.getProperty("user.dir") +sep+ "tracks"));
		}
		else
		{
			String tmpFile = Editor.getProperties().getImage().substring(0,Editor.getProperties().getImage().lastIndexOf(sep));
			File file = new File(tmpFile);
			fc.setCurrentDirectory(file);
		}
		fc.setAcceptAllFileFilterUsed(false);
		FileNameExtensionFilter filter = new FileNameExtensionFilter("RGB and PNG images", "rgb", "png");
		fc.addChoosableFileFilter(filter);
		int result = fc.showOpenDialog(this);
		UIManager.put("FileChooser.readOnly", old);
		if (result == JFileChooser.APPROVE_OPTION)
		{
			pathTextField.setText(fc.getSelectedFile().toString());
		}
	}

	public void exit()
	{
		MutableString stringResult = new MutableString();
		MutableDouble doubleResult = new MutableDouble();

		if (isDifferent(pathTextField.getText(), Editor.getProperties().getImage(), stringResult))
		{
			Editor.getProperties().setImage(stringResult.getValue());
			frame.documentIsModified = true;
		}

		if (isDifferent(imageScaleTextField.getText(),
			Editor.getProperties().getImageScale(), doubleResult))
		{
			Editor.getProperties().setImageScale(doubleResult.getValue());
			frame.documentIsModified = true;
		}
	}
}  //  @jve:decl-index=0:visual-constraint="10,10"
