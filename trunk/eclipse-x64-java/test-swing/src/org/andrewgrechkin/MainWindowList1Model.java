package org.andrewgrechkin;

import java.io.File;

import javax.swing.ListModel;
import javax.swing.event.ListDataListener;

public class MainWindowList1Model implements ListModel<File> {
	public MainWindowList1Model() {
		roots = File.listRoots();
	}

	@Override
	public void addListDataListener(ListDataListener l) {
	}

	@Override
	public File getElementAt(int i) {
		return roots[i];
	}

	@Override
	public int getSize() {
		return roots.length;
	}

	@Override
	public void removeListDataListener(ListDataListener l) {
	}

	private File[] roots;
}
