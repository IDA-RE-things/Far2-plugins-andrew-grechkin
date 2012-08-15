package org.andrewgrechkin;

import java.io.File;

import javax.swing.JList;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

public class MainWindowList1Listener implements ListSelectionListener {
	public MainWindowList1Listener(MainWindowTable1Model model) {
		m_model = model;
	}

	@Override
	public void valueChanged(ListSelectionEvent event) {
		if (event.getSource() instanceof JList<?>) {
			JList<?> list = (JList<?>) event.getSource();
			m_model.setNewRoot((File) list.getSelectedValue());
		}
	}

	private MainWindowTable1Model m_model;
}
