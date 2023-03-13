package gui;

import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JTextField;

import utils.circuit.ObjShapeObject;
import utils.circuit.TrackObject;

public class TrackObjectDialog extends JDialog
{
	private EditorFrame			editorFrame				= null;
	private ObjShapeObject		objectShape				= null;
	private Vector<TrackObject> objects					= null;
	private boolean				ignoreActions			= true;
	private boolean				changed					= false;

	private int					rgb						= 0;

	private JCheckBox			defaultCheckBox			= new JCheckBox();

	private JLabel				objectLabel				= new JLabel();
	private JComboBox<String>	objectComboBox			= null;

	private JLabel				colorLabel				= new JLabel();
	private JTextField			colorTextField			= new JTextField();

	private JLabel				imageLocationLabel		= new JLabel();
	private JTextField			imageLocationTextField	= new JTextField();

	private JLabel				trackLocationLabel		= new JLabel();
	private JTextField			trackLocationTextField	= new JTextField();

	private JButton				applyButton				= new JButton();
	private JButton				cancelButton			= new JButton();

	public TrackObjectDialog(EditorFrame editorFrame, ObjShapeObject objectShape, int x, int y)
	{
		super();
		this.objectShape = objectShape;
		setTitle("Add Object");
		this.editorFrame = editorFrame;
		initialize(x, y);
	}

	public TrackObjectDialog(EditorFrame editorFrame, boolean all, ObjShapeObject objectShape, int x, int y)
	{
		super();
		this.objectShape = objectShape;
		if (all)
			setTitle("Edit All Objects");
		else
			setTitle("Edit Object");
		this.editorFrame = editorFrame;
		initialize(x, y);
	}

	public boolean isChanged()
	{
		return changed;
	}

	private void initialize(int x, int y)
	{
		setLayout(null);
		setSize(320, 225);
		setResizable(false);
		setLocation(x, y);

		defaultCheckBox.setText("Default Objects");
		defaultCheckBox.setBounds(120, 10, 150, 23);
		defaultCheckBox.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				defaultCheckBoxChanged();
			}
		});

		objectLabel.setText("Name");
		objectLabel.setBounds(10, 37, 120, 23);

		objectComboBox = new JComboBox<String>();
		objectComboBox.setBounds(120, 37, 170, 23);
		objectComboBox.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				objectComboBoxChanged();
			}
		});

		// search for color in track objects first
		objects = editorFrame.getTrackData().getTrackObjects();
		int objectIndex	= -1;
		for (int i = 0; i < objects.size(); i++)
		{
			if (objects.get(i).getColor() == objectShape.getRGB())
			{
				defaultCheckBox.setSelected(false);
				objectIndex = i;
				break;
			}
		}

		// search in default objects if not found
		if (objectIndex == -1)
		{
			objects = editorFrame.getDefaultObjects();
			for (int i = 0; i < objects.size(); i++)
			{
				if (objects.get(i).getColor() == objectShape.getRGB())
				{
					defaultCheckBox.setSelected(true);
					objectIndex = i;
					break;
				}
			}
		}

		if (objectIndex == -1)
		{
			// no object has this color so show default objects
			defaultCheckBox.setSelected(true);
		}

		for (int i = 0; i < objects.size(); i++)
		{
			objectComboBox.addItem(objects.get(i).getName());
		}

		colorLabel.setText("Color");
		colorLabel.setBounds(10, 64, 120, 23);

		colorTextField.setBounds(120, 64, 170, 23);
		colorTextField.setEnabled(false);

		imageLocationLabel.setText("Image Location");
		imageLocationLabel.setBounds(10, 91, 120, 23);

		imageLocationTextField.setBounds(120, 91, 170, 23);
		imageLocationTextField.setEnabled(false);

		trackLocationLabel.setText("Track Location");
		trackLocationLabel.setBounds(10, 118, 120, 23);

		trackLocationTextField.setBounds(120, 118, 170, 23);
		trackLocationTextField.setEnabled(false);

		applyButton.setBounds(50, 150, 70, 25);
		applyButton.setText("Apply");
		applyButton.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				applyButtonChanged();
			}
		});

		cancelButton.setBounds(185, 150, 70, 25);
		cancelButton.setText("Cancel");
		cancelButton.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				cancelButtonChanged();
			}
		});

		add(objectLabel);
		add(objectComboBox);
		add(defaultCheckBox);
		add(colorLabel);
		add(colorTextField);
		add(imageLocationLabel);
		add(imageLocationTextField);
		add(trackLocationLabel);
		add(trackLocationTextField);
		add(applyButton);
		add(cancelButton);

		ignoreActions = false;

		objectComboBox.setSelectedIndex(objectIndex);
	}

	private void objectComboBoxChanged()
	{
		if (ignoreActions)
			return;
		
		if (objectComboBox.getSelectedIndex() == -1)
		{
			colorTextField.setBackground(Color.WHITE);
			colorTextField.setForeground(Color.BLACK);
			colorTextField.setText("");
			imageLocationTextField.setText("");
			trackLocationTextField.setText("");
			rgb = 0;
			return;
		}

		imageLocationTextField.setText(objectShape.getImageX() + ", " + objectShape.getImageY());
		trackLocationTextField.setText(String.format("%.3f", objectShape.getTrackLocation().x) + ", " + String.format("%.3f", objectShape.getTrackLocation().y));

		rgb = objects.get(objectComboBox.getSelectedIndex()).getColor();
		
		Color backgroundColor = new Color((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
		colorTextField.setBackground(backgroundColor);
		if ((backgroundColor.getRed()*0.299 + backgroundColor.getGreen()*0.587 + backgroundColor.getBlue()*0.114) > 186)
		{
			colorTextField.setForeground(Color.BLACK);
		}
		else
		{
			colorTextField.setForeground(Color.WHITE);
		}
		colorTextField.setText(String.format("0x%06X", rgb));
	}

	private void defaultCheckBoxChanged()
	{
		if (ignoreActions)
			return;
		
		if (defaultCheckBox.isSelected())
		{
			objects = editorFrame.getDefaultObjects();
		}
		else
		{
			objects = editorFrame.getTrackData().getObjects();
		}

		ignoreActions = true;
		objectComboBox.removeAllItems();
		int objectIndex = -1;

		for (int i = 0; i < objects.size(); i++)
		{
			objectComboBox.addItem(objects.get(i).getName());

			if (objects.get(i).getColor() == objectShape.getRGB())
			{
				objectIndex = i;
			}
		}

		ignoreActions = false;
		
		objectComboBox.setSelectedIndex(objectIndex);
	}

	private void applyButtonChanged()
	{
		if (objectComboBox.getSelectedIndex() == -1)
		{
			JOptionPane.showMessageDialog(this, "No object selected!", "Error", JOptionPane.ERROR_MESSAGE);
			return;
		}

		objectShape.setRGB(rgb);
		changed = true;
		dispose();
	}

	private void cancelButtonChanged()
	{
		dispose();
	}
}
