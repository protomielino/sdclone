package gui.properties;

import java.awt.Color;
import java.awt.Component;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.geom.Point2D;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.Set;
import java.util.Vector;

import javax.swing.DefaultCellEditor;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.SwingUtilities;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableColumn;

import gui.EditorFrame;
import utils.circuit.GraphicObject;

public class GraphicObjectProperties extends PropertyPanel
{
	private GraphicObjectTablePanel		graphicObjectTablePanel	= null;
	private Vector<GraphicObject>		graphicObjects			= null;
	private Vector<GraphicObjectData>	data					= new Vector<GraphicObjectData>();

	public GraphicObjectProperties(EditorFrame editorFrame)
	{
		super(editorFrame);
		initialize();
	}

	private void initialize()
	{
		setLayout(null);
		graphicObjects = getEditorFrame().getGraphicObjects();
		add(getGraphicObjectTablePanel(), null);
		getEditorFrame().setGraphicObjectProperties(this);
	}

	public Vector<GraphicObjectData> getData()
	{
		return data;
	}
	
	public void addData(GraphicObjectData graphicObjectData)
	{
		graphicObjectTablePanel.model.addRow(graphicObjectData);
	}

	private GraphicObjectTablePanel getGraphicObjectTablePanel()
	{
		if (graphicObjectTablePanel == null)
		{
			graphicObjectTablePanel = new GraphicObjectTablePanel(graphicObjects);
			graphicObjectTablePanel.setBounds(10, 10, 485, 314);
		}
		return graphicObjectTablePanel;
	}

	public class ColorRenderer extends DefaultTableCellRenderer
	{
		public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected,
				boolean hasFocus, int row, int column)
		{
			Component c = super.getTableCellRendererComponent(table, value, isSelected, hasFocus, row, column);
			JLabel label = (JLabel)c;

			if (column == 3)
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
				if (column == 1)
				{
					// TODO this needs to change the other cells if there was a conflict that is now fixed
					label.setBackground(Color.WHITE);
					String newName = (String) value;
					for (int i = 0; i < data.size(); i++)
					{
						if (i != row)
						{
							if (data.get(i).name.equals(newName))
							{
								label.setBackground(Color.RED);
								break;
							}
						}
					}					
				}
				else					
					label.setBackground(Color.WHITE);
				label.setForeground(Color.BLACK);
			}

			return label;
		}
	}

	public void getDataFromGraphicObjects(Vector<GraphicObject> graphObjects)
	{
		data.clear();

		for (GraphicObject object : graphObjects)
		{
			String name = object.getName();

			if (name == null)
			{
				name = new String("Unknown");
			}

			data.add(new GraphicObjectData(name, object.getColor(), object.getX(), object.getY(), object.getOrientation()));
		}
	}

	class GraphicObjectTableModel extends AbstractTableModel
	{
		private final String[] 		columnNames = { null, "Name", "Object", "Color", "Track X", "Track Y", "Orientation" };
		private final Class<?>[] 	columnClass = new Class[]
				{
						Integer.class, String.class, String.class, Integer.class, Double.class, Double.class, Double.class
				};
		private Vector<GraphicObject>	graphicObjects = null;

		GraphicObjectTableModel(Vector<GraphicObject>	graphicObjects)
		{
			this.graphicObjects = graphicObjects;
			getDataFromGraphicObjects(graphicObjects);
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
			if (columnIndex == 0 || columnIndex == 3)
			{
				return false;
			}
			return true;
		}

		public Object getValueAt(int rowIndex, int columnIndex)
		{
			GraphicObjectData datum = data.get(rowIndex);

			switch (columnIndex)
			{
			case 0:
				return rowIndex + 1;
			case 1:
				return datum.name;
			case 2:
				return getEditorFrame().getObjectColorName(datum.color);
			case 3:
				return String.format("0x%06X", datum.color);
			case 4:
				return datum.trackX;
			case 5:
				return datum.trackY;
			case 6:
				if (datum.orientation != null && !Double.isNaN(datum.orientation))
				{
					return datum.orientation;
				}
				return null;
			}
			return null;
		}

		public void setValueAt(Object value, int rowIndex, int columnIndex)
		{
			GraphicObjectData datum = data.get(rowIndex);

			switch (columnIndex)
			{
			case 1:
				datum.name = (String) value;
				fireTableCellUpdated(rowIndex, columnIndex);
				break;
			case 2:
			{
				datum.name = (String) value;
				fireTableCellUpdated(rowIndex, columnIndex);

				if (value.equals("Unknown"))
				{
					datum.color = graphicObjects.get(rowIndex).getColor();
				}
				else
				{
					datum.color = getEditorFrame().getObjectColor(datum.name);
				}
				fireTableCellUpdated(rowIndex, columnIndex + 1);
				break;
			}
			case 4:
				datum.trackX = (Double) value;
				fireTableCellUpdated(rowIndex, columnIndex);
				break;
			case 5:
				datum.trackY = (Double) value;
				fireTableCellUpdated(rowIndex, columnIndex);
				break;
			case 6:
				datum.orientation = (Double) value;
				fireTableCellUpdated(rowIndex, columnIndex);
				break;
			}
		}

		public void removeRowAt(int row)
		{
			data.removeElementAt(row);
			fireTableRowsDeleted(row - 1, data.size() - 1);
		}
		
		public void addRow(GraphicObjectData graphicObjectData)
		{
			data.add(graphicObjectData);
			fireTableRowsInserted(data.size() - 1, data.size() - 1);
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

	class GraphicObjectTablePanel extends JPanel
	{
		JTable 					table		= null;
		JScrollPane 			scrollPane	= null;
		GraphicObjectTableModel	model		= null;

		public GraphicObjectTablePanel(Vector<GraphicObject> graphicObjects)
		{
			super(new GridLayout(1,0));

			model = new GraphicObjectTableModel(graphicObjects);
			table = new JTable(model);
			scrollPane = new JScrollPane(table);
			table.getColumnModel().getColumn(0).setPreferredWidth(35);
			table.getColumnModel().getColumn(2).setPreferredWidth(120);
			table.setDefaultRenderer(Integer.class, new ColorRenderer());
			table.setDefaultRenderer(String.class, new ColorRenderer());
			table.setAutoCreateRowSorter(true);
			table.putClientProperty("terminateEditOnFocusLost", Boolean.TRUE);

			Set<String> names = getEditorFrame().getObjectColorNames();

			setUpNameColumn(table, table.getColumnModel().getColumn(2), names);

			table.addMouseListener(new MouseAdapter()
			{
				@Override
				public void mouseClicked(MouseEvent me)
				{
					if (SwingUtilities.isRightMouseButton(me) == true)
					{
						int row = table.rowAtPoint(me.getPoint());
						if (row != -1)
						{
							table.setRowSelectionInterval(row, row);
							if (me.getComponent() instanceof JTable )
							{
								JPopupMenu popup = createPopupMenu(graphicObjectTablePanel);
								popup.show(me.getComponent(), me.getX(), me.getY());
							}
						}
					}
				}
			});

			add(scrollPane);
		}

		void dataChanged()
		{
			model.fireTableDataChanged();
		}
	}

	public JPopupMenu createPopupMenu(GraphicObjectTablePanel panel)
	{
		JPopupMenu popupMenu = new JPopupMenu();
		JMenuItem deleteItem = new JMenuItem("Delete Object");
		JMenuItem deleteAllColorItem = new JMenuItem("Delete All Objects With This Color");

		deleteItem.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				int row = panel.table.getSelectedRow();
				if (row != -1)
				{
					if (JOptionPane.showConfirmDialog(null, "Delete this object?", "Delete Object", JOptionPane.YES_NO_OPTION) == JOptionPane.YES_OPTION)
					{
						panel.model.removeRowAt(panel.table.convertRowIndexToModel(row));
					}
				}
			}
		});
		deleteAllColorItem.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				int row = panel.table.getSelectedRow();
				if (row != -1)
				{
					if (JOptionPane.showConfirmDialog(null, "Delete all objects with this color?", "Delete Objects With Color", JOptionPane.YES_NO_OPTION) == JOptionPane.YES_OPTION)
					{
						int color = data.elementAt(panel.table.convertRowIndexToModel(row)).color;
						Vector<Integer> toDelete = new Vector<Integer>();
						for (int i = 0; i < data.size(); i++)
						{
							if (color == data.elementAt(i).color)
							{
								toDelete.add(i);
							}
						}
						Collections.sort(toDelete, new Comparator<Integer>()
						{
							@Override
							public int compare(Integer o1, Integer o2)
							{
								// Changing the order of the elements
								return o2 - o1;
							}
						});
						for (int i = 0; i < toDelete.size(); i++)
						{
							panel.model.removeRowAt(toDelete.elementAt(i));
						}
					}
				}
			}
		});

		popupMenu.add(deleteItem);
		popupMenu.add(deleteAllColorItem);

		return popupMenu;
	}

	public void exit()
	{
		int minDataCount = Math.min(data.size(), graphicObjects.size());

		if (data.size() != graphicObjects.size())
		{
			getEditorFrame().documentIsModified = true;
		}
		for (int j = 0; j < minDataCount; j++)
		{
			GraphicObjectData datum = data.get(j);
			GraphicObject object = graphicObjects.get(j);

			if (!datum.name.equals(object.getName()))
			{
				object.setName(datum.name);
				getEditorFrame().documentIsModified = true;
			}

			if (!datum.color.equals(object.getColor()))
			{
				object.setColor(datum.color);
				getEditorFrame().documentIsModified = true;
			}

			if (!datum.trackX.equals(object.getX()))
			{
				object.setX(datum.trackX);
				getEditorFrame().documentIsModified = true;
			}

			if (!datum.trackY.equals(object.getY()))
			{
				object.setY(datum.trackY);
				getEditorFrame().documentIsModified = true;
			}

			if (datum.orientation == null)
			{
				if (!Double.isNaN(object.getOrientation())) 
				{
					object.setOrientation(Double.NaN);
					getEditorFrame().documentIsModified = true;
				}
			}
			else if (!datum.orientation.equals(object.getOrientation()))
			{
				object.setOrientation(datum.orientation);
				getEditorFrame().documentIsModified = true;
			}
		}

		if (data.size() < graphicObjects.size())
		{
			// need to trim objects
			while (graphicObjects.size() > data.size())
			{
				graphicObjects.remove(graphicObjects.size() - 1);
			}
		}
		else if (graphicObjects.size() < data.size())
		{
			// need to add to objects
			while (graphicObjects.size() < data.size())
			{
				GraphicObjectData datum = data.get(graphicObjects.size());

				graphicObjects.add(new GraphicObject(datum.name, datum.color, new Point2D.Double(datum.trackX, datum.trackY)));

				if (datum.orientation != null && !Double.isNaN(datum.orientation))
				{
					graphicObjects.lastElement().setOrientation(datum.orientation);
				}
			}
		}
	}
}
