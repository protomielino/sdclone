package gui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

public class PreferencesDialog extends JDialog
{
	public static boolean		APPROVE					= false;
	private EditorFrame			editorFrame;
	private JPanel				jPanel					= null;
	private JLabel				dataDirectoryLabel		= null;
	private JTextField			dataDirectoryTextField	= null;
	private JButton				dataDirectoryButton		= null;
	private JButton				okButton				= null;
	private JButton				cancelButton			= null;

	public PreferencesDialog(EditorFrame editorFrame)
	{
		super();
		this.editorFrame = editorFrame;
		initialize();
	}
	
	private void initialize()
	{
		this.setSize(600, 285);
		this.setContentPane(getJPanel());
		this.setModal(true);
		this.setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
		this.setResizable(false);
		this.setTitle("Preferences Dialog");
	}
	
	private JPanel getJPanel()
	{
		if (jPanel == null)
		{
			dataDirectoryLabel = new JLabel();
			dataDirectoryLabel.setBounds(10, 10, 160, 23);
			dataDirectoryLabel.setText("Speed Dreams Data Directory");
			jPanel = new JPanel();
			jPanel.setLayout(null);
			jPanel.add(dataDirectoryLabel, null);
			jPanel.add(getDataDirectoryTextField(), null);
			jPanel.add(getDataDirectoryButton(), null);
			jPanel.add(getOkButton(), null);
			jPanel.add(getCancelButton(), null);
		}
		return jPanel;
	}

	private JTextField getDataDirectoryTextField()
	{
		if (dataDirectoryTextField == null)
		{
			dataDirectoryTextField = new JTextField();
			dataDirectoryTextField.setBounds(170, 10, 320, 23);
			dataDirectoryTextField.setText(editorFrame.getDataDirectory());
		}
		return dataDirectoryTextField;
	}

	private JButton getDataDirectoryButton()
	{
		if (dataDirectoryButton == null)
		{
			dataDirectoryButton = new JButton();
			dataDirectoryButton.setBounds(495, 10, 80, 25);
			dataDirectoryButton.setText("Browse");
			dataDirectoryButton.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					selectDataDirectory();
				}
			});
		}
		return dataDirectoryButton;
	}
	
	protected void selectDataDirectory()
	{
		JFileChooser fc = new JFileChooser();
		fc.setSelectedFiles(null);
		fc.setSelectedFile(null);
		fc.rescanCurrentDirectory();
		fc.setApproveButtonMnemonic(0);
		fc.setDialogTitle("Speed Dreams Data Directory selection");
		fc.setVisible(true);
		fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
		String dataDir = editorFrame.getDataDirectory();
		if (dataDir == null)
			dataDir = System.getProperty("user.dir");
		fc.setCurrentDirectory(new File(dataDir));
		int result = fc.showDialog(this, "Ok");
		if (result == JFileChooser.APPROVE_OPTION)
		{
			getDataDirectoryTextField().setText(fc.getSelectedFile().toString());
		}
	}

	private JButton getOkButton()
	{
		if (okButton == null)
		{
			okButton = new JButton();
			okButton.setBounds(160, 210, 78, 25);
			okButton.setText("Ok");
			okButton.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					exit();
				}
			});
		}
		return okButton;
	}

	private JButton getCancelButton()
	{
		if (cancelButton == null)
		{
			cancelButton = new JButton();
			cancelButton.setBounds(350, 210, 78, 25);
			cancelButton.setText("Cancel");
			cancelButton.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					cancel();
				}
			});
		}
		return cancelButton;
	}

	protected void exit()
	{
		APPROVE = true;
		cancel();
	}

	protected void cancel()
	{
		this.dispose();
	}
	
	public String getDataDirectory()
	{
		return getDataDirectoryTextField().getText();
	}
}
