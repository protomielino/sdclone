package miscel;

import java.awt.Component;
import java.awt.Container;

import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JPanel;

public class NoNewFolderFileChooser extends JFileChooser
{
	public NoNewFolderFileChooser()
	{
		super();
		disableNewFolder(this);
	}

	private void disableNewFolder(Container container)
	{
		int cnt = container.getComponentCount();
		for (int i = 0; i < cnt; i++)
		{
			Component comp = container.getComponent(i);
			if (comp instanceof JButton)
			{
				JButton btn = (JButton)comp;
				if (btn.getToolTipText().indexOf("New Folder") > -1)
				{
					btn.setVisible(false);
					return;
				}
			}
			else if (comp instanceof JPanel)
			{
				disableNewFolder((Container) comp);
			}
		}
	}
}
