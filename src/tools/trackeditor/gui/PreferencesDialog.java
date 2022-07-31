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
	private JLabel				binDirectoryLabel		= null;
	private JTextField			binDirectoryTextField	= null;
	private JButton				binDirectoryButton		= null;
	private JLabel				libDirectoryLabel		= null;
	private JTextField			libDirectoryTextField	= null;
	private JButton				libDirectoryButton		= null;
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
			binDirectoryLabel = new JLabel();
			binDirectoryLabel.setBounds(10, 43, 160, 23);
			binDirectoryLabel.setText("Speed Dreams Bin Directory");
			libDirectoryLabel = new JLabel();
			libDirectoryLabel.setBounds(10, 76, 160, 23);
			libDirectoryLabel.setText("Speed Dreams Lib Directory");
			jPanel = new JPanel();
			jPanel.setLayout(null);
			jPanel.add(dataDirectoryLabel, null);
			jPanel.add(binDirectoryLabel, null);
			jPanel.add(libDirectoryLabel, null);
			jPanel.add(getDataDirectoryTextField(), null);
			jPanel.add(getDataDirectoryButton(), null);
			jPanel.add(getBinDirectoryTextField(), null);
			jPanel.add(getBinDirectoryButton(), null);
			jPanel.add(getLibDirectoryTextField(), null);
			jPanel.add(getLibDirectoryButton(), null);
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

	private JTextField getBinDirectoryTextField()
	{
		if (binDirectoryTextField == null)
		{
			binDirectoryTextField = new JTextField();
			binDirectoryTextField.setBounds(170, 43, 320, 23);
			binDirectoryTextField.setText(editorFrame.getBinDirectory());
		}
		return binDirectoryTextField;
	}

	private JButton getBinDirectoryButton()
	{
		if (binDirectoryButton == null)
		{
			binDirectoryButton = new JButton();
			binDirectoryButton.setBounds(495, 43, 80, 25);
			binDirectoryButton.setText("Browse");
			binDirectoryButton.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					selectBinDirectory();
				}
			});
		}
		return binDirectoryButton;
	}
	
	protected void selectBinDirectory()
	{
		JFileChooser fc = new JFileChooser();
		fc.setSelectedFiles(null);
		fc.setSelectedFile(null);
		fc.rescanCurrentDirectory();
		fc.setApproveButtonMnemonic(0);
		fc.setDialogTitle("Speed Dreams Bin Directory selection");
		fc.setVisible(true);
		fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
		String binDir = editorFrame.getBinDirectory();
		if (binDir == null)
			binDir = System.getProperty("user.dir");
		fc.setCurrentDirectory(new File(binDir));
		int result = fc.showDialog(this, "Ok");
		if (result == JFileChooser.APPROVE_OPTION)
		{
			getBinDirectoryTextField().setText(fc.getSelectedFile().toString());
		}
	}

	private JTextField getLibDirectoryTextField()
	{
		if (libDirectoryTextField == null)
		{
			libDirectoryTextField = new JTextField();
			libDirectoryTextField.setBounds(170, 76, 320, 23);
			libDirectoryTextField.setText(editorFrame.getLibDirectory());
		}
		return libDirectoryTextField;
	}

	private JButton getLibDirectoryButton()
	{
		if (libDirectoryButton == null)
		{
			libDirectoryButton = new JButton();
			libDirectoryButton.setBounds(495, 76, 80, 25);
			libDirectoryButton.setText("Browse");
			libDirectoryButton.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					selectLibDirectory();
				}
			});
		}
		return libDirectoryButton;
	}
	
	protected void selectLibDirectory()
	{
		JFileChooser fc = new JFileChooser();
		fc.setSelectedFiles(null);
		fc.setSelectedFile(null);
		fc.rescanCurrentDirectory();
		fc.setApproveButtonMnemonic(0);
		fc.setDialogTitle("Speed Dreams Lib Directory selection");
		fc.setVisible(true);
		fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
		String libDir = editorFrame.getLibDirectory();
		if (libDir == null)
			libDir = System.getProperty("user.dir");
		fc.setCurrentDirectory(new File(libDir));
		int result = fc.showDialog(this, "Ok");
		if (result == JFileChooser.APPROVE_OPTION)
		{
			getLibDirectoryTextField().setText(fc.getSelectedFile().toString());
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
	
	public String getBinDirectory()
	{
		return getBinDirectoryTextField().getText();
	}
	
	public String getLibDirectory()
	{
		return getLibDirectoryTextField().getText();
	}
}
