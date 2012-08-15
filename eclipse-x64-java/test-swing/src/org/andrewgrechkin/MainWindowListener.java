package org.andrewgrechkin;

import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.JOptionPane;

public class MainWindowListener extends WindowAdapter {
	@Override
	public void windowClosing(WindowEvent event) {
		Object[] options = { "Да", "Нет" };
		int n = JOptionPane.showOptionDialog(event.getWindow(),
				"Закрыть окно?", "Подтверждение", JOptionPane.YES_NO_OPTION,
				JOptionPane.QUESTION_MESSAGE, null, options, options[0]);
		if (n == 0) {
			event.getWindow().setVisible(false);
			event.getWindow().dispose();
			// System.exit(0);
		}
	}
}
