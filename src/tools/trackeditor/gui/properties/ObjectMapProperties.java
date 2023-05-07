package gui.properties;

import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics2D;
import java.awt.GridLayout;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Iterator;
import java.util.Set;
import java.util.Vector;

import javax.imageio.ImageIO;
import javax.swing.DefaultCellEditor;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.UIManager;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableColumn;

import gui.EditorFrame;
import utils.Editor;
import utils.circuit.ObjShapeObject;
import utils.circuit.ObjectMap;

public class ObjectMapProperties extends PropertyPanel
{
	private JTabbedPane		tabbedPane				= null;
	private JButton			addObjectMapButton		= null;
	private JButton			deleteObjectMapButton	= null;
	private final String 	sep						= System.getProperty("file.separator");

	ObjectMapProperties(EditorFrame editorFrame)
	{
		super(editorFrame);
		initialize();
	}

	private void initialize()
	{
		setLayout(null);

		add(getTabbedPane(), null);
		add(getAddObjectMapButton(), null);
		add(getDeleteObjectMapButton(), null);
	}

	private JTabbedPane getTabbedPane()
	{
		if (tabbedPane == null)
		{
			tabbedPane = new JTabbedPane();
			tabbedPane.setTabLayoutPolicy(JTabbedPane.SCROLL_TAB_LAYOUT);
			tabbedPane.setBounds(10, 10, 487, 280);

			Vector<ObjectMap> objectMaps = getEditorFrame().getObjectMaps();

			for (int i = 0; i < objectMaps.size(); i++)
	        {
                ObjectMap objectMap = objectMaps.elementAt(i);
				tabbedPane.addTab(objectMap.getName(), null, new ObjectMapPanel(objectMap), null);
			}
		}
		return tabbedPane;
	}

	private JButton getAddObjectMapButton()
	{
		if (addObjectMapButton == null)
		{
			addObjectMapButton = new JButton();
			addObjectMapButton.setBounds(10, 300, 130, 25);
			addObjectMapButton.setText("Add Object Map");
			addObjectMapButton.addActionListener(new java.awt.event.ActionListener()
			{
				public void actionPerformed(java.awt.event.ActionEvent e)
				{
					String		name = "map " + (tabbedPane.getTabCount() + 1);
					ObjectMap	objectMap = new ObjectMap();
					objectMap.setName(name);

					tabbedPane.addTab(name, null, new ObjectMapPanel(objectMap), null);
					tabbedPane.setSelectedIndex(tabbedPane.getTabCount() - 1);
				}
			});
		}
		return addObjectMapButton;
	}

	private JButton getDeleteObjectMapButton()
	{
		if (deleteObjectMapButton == null)
		{
			deleteObjectMapButton = new JButton();
			deleteObjectMapButton.setBounds(155, 300, 140, 25);
			deleteObjectMapButton.setText("Delete Object Map");
			deleteObjectMapButton.addActionListener(new java.awt.event.ActionListener()
			{
				public void actionPerformed(java.awt.event.ActionEvent e)
				{
					if (tabbedPane.getTabCount() > 0)
					{
						tabbedPane.removeTabAt(tabbedPane.getSelectedIndex());
					}
				}
			});
		}
		return deleteObjectMapButton;
	}

	private class ObjectMapPanel extends JPanel
	{
		private JLabel				nameLabel				= new JLabel();
		private JTextField 			nameTextField			= new JTextField();
		private JLabel				objectMapLabel			= new JLabel();
		private JTextField			objectMapTextField		= new JTextField();
		private JButton				objectMapCreateButton 	= null;
		private JButton				objectMapButton			= null;
		private ObjectTablePanel	objectTablePanel		= null;

        public class Data
        {
        	String	name;
        	Integer	color;
        	Integer	x;
        	Integer	y;

        	Data(String name, Integer color, Integer x, Integer y)
        	{
        		this.name = name;
        		this.color = color;
        		this.x = x;
        		this.y = y;
        	}
        }

		private Vector<Data> data = new Vector<Data>();

		public Vector<Data> getData()
		{
			return data;
		}

		/**
		 *
		 */
		public ObjectMapPanel(ObjectMap objectMap)
		{
			super();
			initialize(objectMap);
		}

		/**
		 *
		 */
		private void initialize(ObjectMap objectMap)
		{
			setLayout(null);

			addLabel(this, 0, nameLabel, "Name", 120);
			addLabel(this, 1, objectMapLabel, "Object Map", 120);

			addTextField(this, 0, nameTextField, objectMap.getName(), 115, 100);
			addTextField(this, 1, objectMapTextField, objectMap.getObjectMap(), 115, 275);

			add(getObjectMapCreateButton(), null);
			add(getObjectMapButton(), null);
			add(getObjectTablePanel(objectMap), null);

			objectMapCreateButton.setEnabled(objectMap.getObjectMap() == null || objectMap.getObjectMap().isEmpty());

			objectMapTextField.getDocument().addDocumentListener(new DocumentListener()
			{
				public void changedUpdate(DocumentEvent ev)
				{
					changed();
				}
				public void removeUpdate(DocumentEvent ev)
				{
					changed();
				}
				public void insertUpdate(DocumentEvent ev)
				{
					changed();
				}

				public void changed()
				{
					// update table with new data
					try
					{
						getDataFromImage(Editor.getProperties().getPath() + sep + objectMapTextField.getText());
						objectTablePanel.dataChanged();
					}
					catch (IOException ex)
					{
					}
				}
			});
		}

 		/**
		 * This method initializes objectMapCreateButton
		 *
		 * @return javax.swing.JButton
		 */
		private JButton getObjectMapCreateButton()
		{
			if (objectMapCreateButton == null)
			{
				objectMapCreateButton = new JButton();
				objectMapCreateButton.setBounds(395, 6, 80, 25);
				objectMapCreateButton.setText("Create");
				objectMapCreateButton.addActionListener(new java.awt.event.ActionListener()
				{
					public void actionPerformed(java.awt.event.ActionEvent ev)
					{
						try
						{
							objectMapCreateFile();
						}
						catch (IOException ex)
						{
						}
					}
				});
			}
			return objectMapCreateButton;
		}

		protected void objectMapCreateFile() throws IOException
		{
			int index = tabbedPane.getSelectedIndex() + 1;

			Path filename = Paths.get(Editor.getProperties().getPath() + sep + "object-map" + index + ".png");

			if (filename.toFile().exists())
			{
				String[] options = {"Overwrite", "Use Existing", "Cancel"};

				int option = JOptionPane.showOptionDialog(this, filename.getFileName().toString() + " already exists!",
						"Create Object Map", JOptionPane.DEFAULT_OPTION, JOptionPane.INFORMATION_MESSAGE, null, options, options[0]);

				if (option == 1)
				{
					objectMapTextField.setText(filename.getFileName().toString());

					getDataFromImage(filename.toString());
					objectTablePanel.dataChanged();
					return;
				}
				else if (option == 2)
				{
					return;
				}
			}

			Rectangle2D.Double boundingRectangle = getEditorFrame().getBoundingRectangle();

			int imageWidth = 1024;
			int imageHeight = (int)(1024 * (boundingRectangle.height / boundingRectangle.width));
			BufferedImage image = new BufferedImage(imageWidth, imageHeight, BufferedImage.TYPE_INT_RGB);
			Graphics2D graphic = image.createGraphics();
			graphic.setColor(new Color(0x00000000));
			graphic.fillRect(0, 0, imageWidth, imageHeight);

			graphic.dispose();
			ImageIO.write(image, "png", new File(filename.toString()));

			objectMapTextField.setText(filename.getFileName().toString());

			getDataFromImage(filename.toString());
			objectTablePanel.dataChanged();
		}

		/**
		 * This method initializes objectMapButton
		 *
		 * @return javax.swing.JButton
		 */
		private JButton getObjectMapButton()
		{
			if (objectMapButton == null)
			{
				objectMapButton = new JButton();
				objectMapButton.setBounds(395, 36, 80, 25);
				objectMapButton.setText("Browse");
				objectMapButton.addActionListener(new java.awt.event.ActionListener()
				{
					public void actionPerformed(java.awt.event.ActionEvent e)
					{
						objectMapFile();
					}
				});
			}
			return objectMapButton;
		}

		protected void objectMapFile()
		{
			Boolean old = UIManager.getBoolean("FileChooser.readOnly");
			UIManager.put("FileChooser.readOnly", Boolean.TRUE);
			JFileChooser fc = new JFileChooser();
			fc.setSelectedFiles(null);
			fc.setSelectedFile(null);
			fc.rescanCurrentDirectory();
			fc.setApproveButtonMnemonic(0);
			fc.setDialogTitle("Object Map image file selection");
			fc.setVisible(true);
			fc.setAcceptAllFileFilterUsed(false);
			FileNameExtensionFilter filter = new FileNameExtensionFilter("RGB and PNG images", "rgb", "png");
			fc.addChoosableFileFilter(filter);
			fc.setCurrentDirectory(new File(Editor.getProperties().getPath()));
			int result = fc.showOpenDialog(this);
			UIManager.put("FileChooser.readOnly", old);
			if (result == JFileChooser.APPROVE_OPTION)
			{
				String selectedFile = fc.getSelectedFile().toString();
				String fileName = selectedFile;
				int index = selectedFile.lastIndexOf(sep);
				String pathToFile = selectedFile.substring(0, index);
				if (pathToFile.equals(Editor.getProperties().getPath()))
					fileName = selectedFile.substring(index + 1);
				objectMapTextField.setText(fileName);

				// update table with new data
				try
				{
					getDataFromImage(selectedFile);
					objectTablePanel.dataChanged();
				}
				catch (IOException e)
				{
				}
			}
		}

    	public void getDataFromImage(String fileName) throws IOException
    	{
    		data.clear();

			File file = new File(fileName);

			if (file.exists())
			{
				BufferedImage image = ImageIO.read(file);
				int imageWidth = image.getWidth();
				int imageHeight = image.getHeight();

				for (int x = 0; x < imageWidth; x++)
				{
					for (int y = 0; y < imageHeight; y++)
					{
						int rgb = image.getRGB(x, y) & 0x00ffffff;

						if (rgb != 0x0)
						{
							String name = getEditorFrame().getObjectColorName(rgb);
							if (name == null)
							{
								name = new String("Unknown");
							}

							data.add(new Data(name, rgb, x, y));
						}
					}
				}
			}
    	}

    	public void getDataFromObjectMap(ObjectMap objectMap)
    	{
    		data.clear();

			for (int i = 0; i < objectMap.getObjects().size(); i++)
			{
				ObjShapeObject object = objectMap.getObjects().get(i);

				String name = getEditorFrame().getObjectColorName(object.getRGB());

				if (name == null)
				{
					name = new String("Unknown");
				}

				data.add(new Data(name, object.getRGB(), object.getImageX(), object.getImageY()));
			}
    	}

		private ObjectTablePanel getObjectTablePanel(ObjectMap objectMap)
		{
			if (objectTablePanel == null)
			{
				objectTablePanel = new ObjectTablePanel(objectMap);
				objectTablePanel.setBounds(10, 64, 465, 180);
			}
			return objectTablePanel;
		}

		public class ColorRenderer extends DefaultTableCellRenderer
		{
		    public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected,
		        boolean hasFocus, int row, int column)
		    {
		        Component c = super.getTableCellRendererComponent(table, value, isSelected, hasFocus, row, column);
		        JLabel label = (JLabel)c;

		        if (column == 2)
		    	{
		    		int rgb = Integer.decode(value.toString());
		    		Color color = new Color((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
		    		label.setBackground(color);
		    		if ((color.getRed()*0.299 + color.getGreen()*0.587 + color.getBlue()*0.114) > 186)
		    		{
			        	label.setForeground(Color.BLACK);
		    		}
		    		else
		    		{
			        	label.setForeground(Color.WHITE);
		    		}
		    	}
		        else
		        {
		        	label.setBackground(Color.WHITE);
		        	label.setForeground(Color.BLACK);
		        }

		        return label;
		    }
		}

		class ObjectTableModel extends AbstractTableModel
	    {
	        private final String[] 		columnNames = { null, "Name", "Color", "X", "Y" };
	        private final Class<?>[] 	columnClass = new Class[]
	        {
	        	Integer.class, String.class, Integer.class, Integer.class, Integer.class
	        };
			private ObjectMap 			objectMap = null;

	        ObjectTableModel(ObjectMap objectMap)
			{
	        	this.objectMap = objectMap;
	        	getDataFromObjectMap(objectMap);
	        }

			public int getRowCount()
			{
				return data.size();
			}

			public int getColumnCount()
			{
				return columnNames.length;
			}

	        public String getColumnName(int columnIndex)
	        {
	            return columnNames[columnIndex];
	        }

	        public Class<?> getColumnClass(int columnIndex)
	        {
	            return columnClass[columnIndex];
	        }

	        public boolean isCellEditable(int row, int columnIndex)
	        {
	        	if (columnIndex == 1 || columnIndex == 3 || columnIndex == 4)
	        	{
	        		return true;
	        	}
	        	return false;
	        }

			public Object getValueAt(int rowIndex, int columnIndex)
			{
				Data datum = data.get(rowIndex);

				switch (columnIndex)
				{
				case 0:
					return rowIndex + 1;
				case 1:
					return datum.name;
				case 2:
					return String.format("0x%06X", datum.color);
				case 3:
					return datum.x;
				case 4:
					return datum.y;
				}
				return null;
			}

			public void setValueAt(Object value, int rowIndex, int columnIndex)
			{
				Data datum = data.get(rowIndex);

				switch (columnIndex)
				{
				case 1:
					datum.name = (String) value;
			        fireTableCellUpdated(rowIndex, columnIndex);

			        if (value.equals("Unknown"))
			        {
			        	datum.color = objectMap.getObjects().get(rowIndex).getRGB();
			        }
			        else
			        {
			        	datum.color = getEditorFrame().getObjectColor(datum.name);
			        }
			        fireTableCellUpdated(rowIndex, columnIndex + 1);
			        break;
				case 3:
					datum.x = (Integer) value;
			        fireTableCellUpdated(rowIndex, columnIndex);
					break;
				case 4:
					datum.y = (Integer) value;
			        fireTableCellUpdated(rowIndex, columnIndex);
					break;
				}
		    }
	    }

	    public void setUpNameColumn(JTable table, TableColumn nameColumn, Set<String> names)
	    {
	    	//Set up the editor for the name cells.
	    	JComboBox<String> comboBox = new JComboBox<String>();

	    	Iterator<String> it = names.iterator();
			while (it.hasNext())
			{
				comboBox.addItem(it.next());
			}

			comboBox.addItem("Unknown");

			nameColumn.setCellEditor(new DefaultCellEditor(comboBox));

	    	//Set up tool tips for the name cells.
	    	DefaultTableCellRenderer renderer = new DefaultTableCellRenderer();
	    	//renderer.setToolTipText("Click to change object name");
	    	nameColumn.setCellRenderer(renderer);
	    }

		class ObjectTablePanel extends JPanel
		{
	        JTable 				table		= null;
	        JScrollPane 		scrollPane	= null;
	        ObjectTableModel	model		= null;

	        public ObjectTablePanel(ObjectMap objectMap)
			{
		        super(new GridLayout(1,0));

		        model = new ObjectTableModel(objectMap);
		        table = new JTable(model);
		        scrollPane = new JScrollPane(table);
		        table.getColumnModel().getColumn(0).setPreferredWidth(25);
		        table.setDefaultRenderer(Integer.class, new ColorRenderer());
		        table.setAutoCreateRowSorter(true);

		        Set<String> names = getEditorFrame().getObjectColorNames();

		        setUpNameColumn(table, table.getColumnModel().getColumn(1), names);

		        add(scrollPane);
		    }

	        void dataChanged()
	        {
	        	model.fireTableDataChanged();
	        }
		}
	}

	public void exit()
	{
		MutableString stringResult = new MutableString();
        Vector<ObjectMap> objectMaps = getEditorFrame().getObjectMaps();
		int minCount = Math.min(objectMaps.size(), tabbedPane.getTabCount());
		if (objectMaps.size() != tabbedPane.getTabCount())
		{
			getEditorFrame().documentIsModified = true;
		}
		for (int i = 0; i < minCount; i++)
        {
            ObjectMap objectMap = objectMaps.elementAt(i);
            ObjectMapPanel panel = (ObjectMapPanel) tabbedPane.getComponentAt(i);
            if (isDifferent(panel.nameTextField.getText(), objectMap.getName(), stringResult))
            {
                objectMap.setName(stringResult.getValue());
                getEditorFrame().documentIsModified = true;
            }
            if (isDifferent(panel.objectMapTextField.getText(), objectMap.getObjectMap(), stringResult))
            {
            	try
            	{
            		objectMap.setObjectMap(stringResult.getValue());
            	}
            	catch (IOException e)
            	{
            	}
                getEditorFrame().documentIsModified = true;
            }

            Vector<ObjectMapPanel.Data> data = panel.getData();
            Vector<ObjShapeObject>	objects = objectMap.getObjects();
    		int minDataCount = Math.min(data.size(), objects.size());

    		if (data.size() != objects.size())
    		{
    			getEditorFrame().documentIsModified = true;
    		}
            for (int j = 0; j < minDataCount; j++)
            {
            	ObjectMapPanel.Data datum = data.get(j);
            	ObjShapeObject object = objects.get(j);

            	if (!datum.color.equals(object.getRGB()))
            	{
            		object.setRGB(datum.color);
        			getEditorFrame().documentIsModified = true;
            	}

            	if (!datum.x.equals(object.getImageX()))
            	{
            		object.setImageX(datum.x);
        			getEditorFrame().documentIsModified = true;
            	}

            	if (!datum.y.equals(object.getImageY()))
            	{
            		object.setImageY(datum.y);
        			getEditorFrame().documentIsModified = true;
            	}
            }
    		if (data.size() < objects.size())
    		{
    			// need to trim objects
    			while (objects.size() > data.size())
    			{
    				objects.remove(objects.size() - 1);
    			}
    		}
    		else if (objects.size() < data.size())
    		{
    			// need to add to objects
    			while (objects.size() < data.size())
    			{
                	ObjectMapPanel.Data datum = data.get(objects.size());

    				objects.add(new ObjShapeObject(datum.color, datum.x, datum.y));
    			}
    		}
		}
		if (objectMaps.size() > tabbedPane.getTabCount())
		{
			// need to trim objectMaps
			while (objectMaps.size() > tabbedPane.getTabCount())
			{
				objectMaps.remove(objectMaps.size() - 1);
			}
		}
		else if (objectMaps.size() < tabbedPane.getTabCount())
		{
			// need to add to objectMaps
			while (objectMaps.size() < tabbedPane.getTabCount())
			{
	            ObjectMapPanel panel = (ObjectMapPanel) tabbedPane.getComponentAt(objectMaps.size());
				ObjectMap objectMap = new ObjectMap();
				objectMap.setName(panel.nameTextField.getText());
				try
				{
					objectMap.setObjectMap(panel.objectMapTextField.getText());
            	}
            	catch (IOException e)
            	{
            	}
				objectMaps.add(objectMap);
			}
		}
	}
}
