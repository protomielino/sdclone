package gui;

import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.geom.Point2D;
import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JTextField;

import utils.circuit.GraphicObject;
import utils.circuit.ObjShapeObject;
import utils.circuit.ObjectData;
import utils.circuit.TrackObject;

public class TrackObjectDialog extends JDialog
{
	private EditorFrame			editorFrame				= null;
	private GraphicObject		graphicObject			= null;
	private ObjShapeObject		objectShape				= null;
	private ObjectData			objectData				= null;
	private Vector<TrackObject> objects					= null;
	private boolean				ignoreActions			= true;
	private boolean				changed					= false;
	private boolean				isGraphicObject			= false;

	private int					rgb						= 0;

	private JCheckBox			defaultCheckBox			= new JCheckBox();

	private JLabel				nameLabel				= new JLabel();
	private JTextField			nameTextField			= new JTextField();

	private JLabel				objectLabel				= new JLabel();
	private JComboBox<String>	objectComboBox			= null;

	private JLabel				colorLabel				= new JLabel();
	private JTextField			colorTextField			= new JTextField();

	private JLabel				trackLocationLabel		= new JLabel();
	private JTextField			trackLocationTextField	= new JTextField();

	private JLabel				imageLocationLabel		= new JLabel();
	private JTextField			imageLocationTextField	= new JTextField();

	private JLabel				orientationLabel		= new JLabel();
	private JTextField			orientationTextField	= new JTextField();

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

	public TrackObjectDialog(EditorFrame editorFrame, GraphicObject graphicObject, int x, int y)
	{
		super();
		this.graphicObject = graphicObject;
		this.objectShape = graphicObject.getShape();
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

	public TrackObjectDialog(EditorFrame editorFrame, boolean all, GraphicObject graphicObject, int x, int y)
	{
		super();
		this.graphicObject = graphicObject;
		this.objectShape = graphicObject.getShape();
		if (all)
			setTitle("Edit All Objects");
		else
			setTitle("Edit Object");
		this.editorFrame = editorFrame;
		initialize(x, y);
	}

	public TrackObjectDialog(EditorFrame editorFrame, boolean all, ObjectData objectData)
	{
		super();
		this.objectData = objectData;
		setTitle("Add Object");
		this.editorFrame = editorFrame;
		initialize(0, 0);
	}

	public boolean isChanged()
	{
		return changed;
	}

	private int getRGB()
	{
		if (objectShape != null)
		{
			return objectShape.getRGB();
		}

		return objectData.color;
	}
	private void setRGB(int rgb)
	{
		if (objectShape != null)
		{
			objectShape.setRGB(rgb);
			return;
		}

		objectData.color = rgb;
	}

	private String getObjectName()
	{
		if (objectShape != null)
		{
			return objectShape.getName();
		}

		return objectData.name;
	}
	private void setObjectName(String name)
	{
		if (objectShape != null)
		{
			objectShape.setName(name);
			return;
		}

		objectData.name = name;
	}

	private Point2D.Double getTrackLocation()
	{
		if (objectShape != null)
		{
			return objectShape.getTrackLocation();
		}

		return new Point2D.Double(objectData.trackX, objectData.trackY);
	}

	private double getImageX()
	{
		if (objectShape != null)
		{
			return objectShape.getImageX();
		}

		return objectData.imageX;
	}

	private double getImageY()
	{
		if (objectShape != null)
		{
			return objectShape.getImageY();
		}

		return objectData.imageY;
	}

	private void initialize(int x, int y)
	{
		isGraphicObject = objectShape != null && objectShape.getType().equals("graphic object");

		setLayout(null);
		setSize(320, 252);
		setResizable(false);
		if (objectData == null)
		{
			setLocation(x, y);
		}
		else
		{
			setLocationRelativeTo(getParent());
		}
		defaultCheckBox.setText("Default Objects");
		defaultCheckBox.setBounds(120, 10, 150, 23);
		defaultCheckBox.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				defaultCheckBoxChanged();
			}
		});

		objectLabel.setText("Object");
		objectLabel.setBounds(10, 64, 120, 23);

		objectComboBox = new JComboBox<String>();
		objectComboBox.setBounds(120, 64, 170, 23);
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
			if (objects.get(i).getColor() == getRGB())
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
				if (objects.get(i).getColor() == getRGB())
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

		if (isGraphicObject)
		{
			nameLabel.setText("Name");
			nameLabel.setBounds(10, 37, 120, 23);

			nameTextField.setText(getObjectName());
			nameTextField.setBounds(120, 37, 170, 23);
		}

		colorLabel.setText("Color");
		colorLabel.setBounds(10, 91, 120, 23);

		colorTextField.setBounds(120, 91, 170, 23);
		colorTextField.setEnabled(false);

		trackLocationLabel.setText("Track Location");
		trackLocationLabel.setBounds(10, 118, 120, 23);

		trackLocationTextField.setText(String.format("%.3f", getTrackLocation().x) + ", " + String.format("%.3f", getTrackLocation().y));
		trackLocationTextField.setBounds(120, 118, 170, 23);
		trackLocationTextField.setEnabled(false);

		if (!isGraphicObject)
		{
			imageLocationLabel.setText("Image Location");
			imageLocationLabel.setBounds(10, 145, 120, 23);

			imageLocationTextField.setText(getImageX() + ", " + getImageY());
			imageLocationTextField.setBounds(120, 145, 170, 23);
			imageLocationTextField.setEnabled(false);
		}
		else
		{
			orientationLabel.setText("Orientation");
			orientationLabel.setBounds(10, 145, 120, 23);

			if (!Double.isNaN(graphicObject.getOrientation()))
				orientationTextField.setText("" + graphicObject.getOrientation());
			orientationTextField.setBounds(120, 145, 170, 23);
		}

		applyButton.setBounds(50, 177, 70, 25);
		applyButton.setText("Apply");
		applyButton.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				applyButtonChanged();
			}
		});

		cancelButton.setBounds(185, 177, 70, 25);
		cancelButton.setText("Cancel");
		cancelButton.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				cancelButtonChanged();
			}
		});

		if (isGraphicObject)
		{
			add(nameLabel);
			add(nameTextField);
		}

		add(objectLabel);
		add(objectComboBox);
		add(defaultCheckBox);
		add(colorLabel);
		add(colorTextField);
		add(trackLocationLabel);
		add(trackLocationTextField);

		if (!isGraphicObject)
		{
			add(imageLocationLabel);
			add(imageLocationTextField);
		}
		else
		{
			add(orientationLabel);
			add(orientationTextField);
		}

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
			rgb = 0;
			return;
		}

		if (!isGraphicObject || objectData != null)
		{
			imageLocationTextField.setText(getImageX() + ", " + getImageY());
		}

		trackLocationTextField.setText(String.format("%.3f", getTrackLocation().x) + ", " + String.format("%.3f", getTrackLocation().y));

		setObjectName(objects.get(objectComboBox.getSelectedIndex()).getName());
		
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

			if (objects.get(i).getColor() == getRGB())
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

		setRGB(rgb);

		if (isGraphicObject)
		{
			String newName = nameTextField.getText();

			for (GraphicObject object : editorFrame.getGraphicObjects())
			{
				if (object.getShape() != objectShape)
				{
					if (object.getName().equals(newName))
					{
						JOptionPane.showMessageDialog(this, "Object name already used!", "Error", JOptionPane.ERROR_MESSAGE);
						return;
					}
				}
			}
			objectShape.setName(newName);

			String orientationText = orientationTextField.getText();
			if (orientationText == null || orientationText.isEmpty())
			{
				graphicObject.setOrientation(Double.NaN);
			}
			else
			{
				try
				{
					graphicObject.setOrientation(Double.parseDouble(orientationText));
				}
				catch (NumberFormatException e)
				{
				}
			}
		}

		changed = true;
		dispose();
	}

	private void cancelButtonChanged()
	{
		dispose();
	}
}
