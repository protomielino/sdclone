/*
 *   Trackgen.java
 *   Created on 27 ??? 2005
 *
 *    The Trackgen.java is part of TrackEditor-0.3.1.
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
package gui;

import java.awt.Point;
import java.awt.event.WindowEvent;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.WindowConstants;

import utils.Editor;

/**
 * @author babis
 * 
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class TrackgenPanel extends JDialog implements Runnable
{
	private final static String newline 		= "\n";
	private final static String sep				= System.getProperty("file.separator");
	private EditorFrame			editorFrame;
	private Thread 				ac3d 			= new Thread(this);

	private JPanel				jPanel			= null;
	private JLabel				nameLabel		= null;
	private JLabel				authorLabel		= null;
	private JLabel				fileNameLabel	= null;
	private JLabel				lengthLabel		= null;
	private JLabel				widthLabel		= null;
	private JLabel				xSizeLabel		= null;
	private JLabel				ySizeLabel		= null;
	private JTextField			nodesTextField	= null;
	private JPanel				jPanel1			= null;
	private JLabel				trackgenLabel	= null;
	private JLabel				waitLabel		= null;
	private JPanel				jPanel2			= null;
	private JTextArea			errorsTextArea	= null;
	
	public TrackgenPanel(EditorFrame editorFrame)
	{
		super();
		this.editorFrame = editorFrame;
		initialize();
		ac3d.start();
	}

	/**
	 * This method initializes this
	 * 
	 * @return void
	 */
	private void initialize()
	{
		this.setContentPane(getJPanel());
		this.setTitle("Trackgen");
		this.setSize(800, 520);
		this.setResizable(false);
		Point p = new Point();
		p.x = editorFrame.getProject().getTrackgenDialogX();
		p.y = editorFrame.getProject().getTrackgenDialogY();
		this.setLocation(p);
		this.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
	}

	public void run()
	{
		String path = Editor.getProperties().getPath();
		String trackName = path.substring(path.lastIndexOf(sep) + 1);
		String category = " -c " + editorFrame.getTrackData().getHeader().getCategory();
		String name = " -n " + trackName;
		String args = " -a" + category + name;

		System.out.println(args);

		try
		{
			String ls_str;
			String tmp = "";
			String trackgen = "sd2-trackgen";
			if (editorFrame.getBinDirectory() != null && !editorFrame.getBinDirectory().isEmpty())
			{
				trackgen = editorFrame.getBinDirectory() + sep + trackgen;
			}
			
			Process ls_proc = Runtime.getRuntime().exec(trackgen + args);
			// get its output (your input) stream
			BufferedReader ls_in = new BufferedReader(new InputStreamReader(ls_proc.getInputStream()));
			BufferedReader ls_err = new BufferedReader(new InputStreamReader(ls_proc.getErrorStream()));

			try
			{
				while (true)
				{
					// done when process terminated and nothing to read
					if (!ls_proc.isAlive() && !ls_in.ready() && !ls_err.ready())
						break;
					
					if (ls_err.ready())
					{
						String str = ls_err.readLine();
						int index = str.indexOf("Error");
						if (index != -1)
						{
							if (!str.contains("not released"))
							{
								errorsTextArea.append(str.substring(index) + newline);
							}
						}
						index = str.indexOf("FATAL:");
						if (index != -1)
						{
							errorsTextArea.append(str.substring(index) + newline);
						}
						index = str.indexOf("WARNING:");
						if (index != -1)
						{
							errorsTextArea.append(str.substring(index) + newline);
						}
						index = str.indexOf("libpng warning:");
						if (index != -1)
						{
							errorsTextArea.append(str.substring(index) + newline);
						}
					}
				
					if (ls_in.ready()) 
					{
						ls_str = ls_in.readLine();
						if (ls_str.indexOf(" ") != -1)
						{
							tmp = ls_str.substring(0, ls_str.indexOf(" "));
							if (tmp.equals("name"))
							{
								nameLabel.setText(ls_str);
							} else if (tmp.equals("authors"))
							{
								this.authorLabel.setText(ls_str);
							} else if (tmp.equals("filename"))
							{
								this.fileNameLabel.setText(ls_str);
							}else if (tmp.equals("length"))
							{
								this.lengthLabel.setText(ls_str);
							}else if (tmp.equals("width"))
							{
								this.widthLabel.setText(ls_str);
							}
							else if (tmp.equals("XSize"))
							{
								this.xSizeLabel.setText(ls_str);
							}else if (tmp.equals("YSize"))
							{
								this.ySizeLabel.setText(ls_str);
							}else if (tmp.equals("FATAL:"))
							{
								errorsTextArea.append(ls_str + newline);
							}else if (tmp.equals("WARNING:"))
							{
								errorsTextArea.append(ls_str + newline);
							}else
							{
								this.nodesTextField.setText(ls_str);
							}
						}
					}
				}
			} catch (IOException e)
			{
				e.printStackTrace();
			}
		} catch (IOException e1)
		{
			JOptionPane.showMessageDialog(this, e1.getLocalizedMessage(), "Export AC3D", JOptionPane.ERROR_MESSAGE);
		}
		this.waitLabel.setText("Track finished");
	}

	/**
	 * This method initializes jPanel
	 * 
	 * @return javax.swing.JPanel
	 */
	private JPanel getJPanel()
	{
		if (jPanel == null)
		{
			trackgenLabel = new JLabel();
			waitLabel = new JLabel();
			jPanel = new JPanel();
			jPanel.setLayout(null);
			trackgenLabel.setBounds(350, 10, 200, 20);
			trackgenLabel.setText("Track data");
			trackgenLabel.setFont(new java.awt.Font("Dialog", java.awt.Font.BOLD, 18));
			waitLabel.setBounds(10, 260, 290, 25);
			waitLabel.setText("Constructing the .ac file. Please wait...");
			jPanel.add(trackgenLabel, null);
			jPanel.add(waitLabel, null);
	
			jPanel.add(getJPanel1(), null);
			jPanel.add(getNodesTextField(), null);
			jPanel.add(getJPanel2(), null);
		}
		return jPanel;
	}
	/**
	 * This method initializes nodesTextField
	 * 
	 * @return javax.swing.JTextField
	 */
	private JTextField getNodesTextField()
	{
		if (nodesTextField == null)
		{
			nodesTextField = new JTextField();
			nodesTextField.setBounds(10, 230, 760, 25);
			nodesTextField.setEditable(false);
			nodesTextField.setText("");
		}
		return nodesTextField;
	}
	/**
	 * This method initializes jPanel1	
	 * 	
	 * @return javax.swing.JPanel	
	 */    
	private JPanel getJPanel1() {
		if (jPanel1 == null) {
			jPanel1 = new JPanel();
			jPanel1.setLayout(null);
			jPanel1.setBounds(10, 40, 760, 180);
			jPanel1.setBorder(javax.swing.BorderFactory.createEtchedBorder(javax.swing.border.EtchedBorder.LOWERED));
			nameLabel = new JLabel();
			authorLabel = new JLabel();
			fileNameLabel = new JLabel();
			lengthLabel = new JLabel();
			widthLabel = new JLabel();
			xSizeLabel = new JLabel();
			ySizeLabel = new JLabel();
			nameLabel.setText("");
			nameLabel.setBounds(5, 5, 740, 20);
			authorLabel.setText("");
			authorLabel.setBounds(5, 30, 740, 20);
			fileNameLabel.setText("");
			fileNameLabel.setBounds(5, 55, 740, 20);
			lengthLabel.setText("");
			lengthLabel.setBounds(5, 80, 740, 20);
			widthLabel.setText("");
			widthLabel.setBounds(5, 105, 740, 20);
			xSizeLabel.setText("");
			xSizeLabel.setBounds(5, 130, 740, 20);
			ySizeLabel.setText("");
			ySizeLabel.setBounds(5, 155, 740, 20);
			jPanel1.add(ySizeLabel, null);
			jPanel1.add(xSizeLabel, null);
			jPanel1.add(widthLabel, null);
			jPanel1.add(lengthLabel, null);
			jPanel1.add(fileNameLabel, null);
			jPanel1.add(authorLabel, null);
			jPanel1.add(nameLabel, null);
		}
		return jPanel1;
	}

	/**
	 * This method initializes jPanel2	
	 * 	
	 * @return javax.swing.JPanel	
	 */    
	private JPanel getJPanel2() {
		if (jPanel2 == null) {
			jPanel2 = new JPanel();
			jPanel2.setLayout(null);
			jPanel2.setBounds(10, 290, 760, 180);
			jPanel2.setBorder(javax.swing.BorderFactory.createEtchedBorder(javax.swing.border.EtchedBorder.LOWERED));
			jPanel2.add(getErrorsTextArea(), null);
		}
		return jPanel2;
	}
	/**
	 * This method initializes errorsTextArea
	 * 
	 * @return javax.swing.JTextArea
	 */
	private JTextArea getErrorsTextArea()
	{
		if (errorsTextArea == null)
		{
			errorsTextArea = new JTextArea();
			errorsTextArea.setBounds(5, 10, 750, 165);
			errorsTextArea.setEditable(false);
			errorsTextArea.setText("");
		}
		return errorsTextArea;
	}

	protected void processWindowEvent(WindowEvent e)
	{
		super.processWindowEvent(e);
		if (e.getID() == WindowEvent.WINDOW_CLOSING)
		{
			editorFrame.getProject().setTrackgenDialogX(this.getX());
			editorFrame.getProject().setTrackgenDialogY(this.getY());
		}
	}
} //  @jve:decl-index=0:visual-constraint="10,10"

