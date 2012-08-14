package org.andrewgrechkin;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.io.File;

import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JScrollPane;
import javax.swing.JTable;

public class MainWindow implements Runnable {

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		try {
			// UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
			// UIManager.setLookAndFeel(new MotifLookAndFeel());
			// UIManager.setLookAndFeel("javax.swing.plaf.metal.MetalLookAndFeel");
		} catch (Exception e) {
			// Exception handle
		}
		EventQueue.invokeLater(new MainWindow());
	}

	/**
	 * Create the application.
	 */
	public MainWindow() {
		// try {
		// UIManager.setLookAndFeel("javax.swing.plaf.metal.MetalLookAndFeel");
		// SwingUtilities.updateComponentTreeUI(this);
		// }
		// catch (Exception e){
		// System.out.println("������ ��� �������� Metal-Look-And-Feel");
		// }
		initialize();
	}

	@Override
	public void run() {
		try {
			frame.setVisible(true);
			// JFileChooser fileopen = new JFileChooser();
			// int ret = fileopen.showDialog(null, "������� ����");
			// if (ret == JFileChooser.APPROVE_OPTION) {
			// File file = fileopen.getSelectedFile();
			// }
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * Initialize the contents of the frame.
	 */
	private void initialize() {
		// File folder = new File("C:\\tools");
		MainWindowTable1Model tableModel = new MainWindowTable1Model();
		// tableModel.setFiles(folder.listFiles());
		// tableModel.setFiles(File.listRoots());

		frame = new JFrame();
		frame.setBounds(100, 100, 619, 549);
		// frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
		frame.addWindowListener(new MainWindowListener());
		frame.getContentPane().setLayout(new BorderLayout(0, 0));

		JLabel lblNewLabel = new JLabel("New label");
		frame.getContentPane().add(lblNewLabel, BorderLayout.NORTH);

		list_1 = new JList<File>();
		list_1.setModel(new MainWindowList1Model());
		list_1.setPreferredSize(new Dimension(200, 0));
		list_1.addListSelectionListener(new MainWindowList1Listener(tableModel));
		frame.getContentPane().add(list_1, BorderLayout.WEST);

		comboBox = new JComboBox<String>();
		frame.getContentPane().add(comboBox, BorderLayout.SOUTH);
		comboBox.addItem("First");
		comboBox.addItem("Second");

		table = new JTable();
		table.setModel(tableModel);
		// frame.getContentPane().add(table, BorderLayout.CENTER);

		JScrollPane jscrlp = new JScrollPane(table);
		frame.getContentPane().add(jscrlp, BorderLayout.CENTER);
		table.setPreferredScrollableViewportSize(new Dimension(250, 100));
		// frame.pack();
	}

	private JComboBox<String> comboBox;
	private JFrame frame;
	private JList<File> list_1;
	private JTable table;
}
