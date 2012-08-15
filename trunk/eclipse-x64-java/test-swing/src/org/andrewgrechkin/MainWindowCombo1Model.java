package org.andrewgrechkin;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import javax.swing.ComboBoxModel;
import javax.swing.event.ListDataListener;

public class MainWindowCombo1Model implements ComboBoxModel<String> {
	public MainWindowCombo1Model(List<String> list) {
		data = list;
	}

	@Override
	public void addListDataListener(ListDataListener listener) {
		listeners.add(listener);
	}

	@Override
	public String getElementAt(int i) {
		return data.get(i);
	}

	@Override
	public Object getSelectedItem() {
		return data.get(selected);
	}

	@Override
	public int getSize() {
		return data.size();
	}

	@Override
	public void removeListDataListener(ListDataListener listener) {
		listeners.remove(listener);
	}

	@Override
	public void setSelectedItem(Object o) {
		selected = data.indexOf(o);
	}

	private List<String> data = new ArrayList<String>();
	private Set<ListDataListener> listeners = new HashSet<ListDataListener>();
	private int selected = 0;
}
