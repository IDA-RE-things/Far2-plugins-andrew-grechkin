package org.andrewgrechkin;

import java.io.File;
import java.util.HashSet;
import java.util.Set;

import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.TableModel;

public class MainWindowTable1Model implements TableModel {
	public MainWindowTable1Model() {
	}

	@Override
	public void addTableModelListener(TableModelListener listener) {
		listeners.add(listener);
	}

	@Override
	public Class<?> getColumnClass(int columnIndex) {
		switch (columnIndex) {
		case 0:
			return String.class;
		case 1:
			return Integer.class;
		case 2:
			return String.class;
		case 3:
			return Integer.class;
		case 4:
			return Boolean.class;
		}
		return String.class;
	}

	@Override
	public int getColumnCount() {
		return 5;
	}

	@Override
	public String getColumnName(int columnIndex) {
		switch (columnIndex) {
		case 0:
			return "Имя";
		case 1:
			return "Размер";
		case 2:
			return "Путь";
		case 3:
			return "Хэш";
		case 4:
			return "Директория";
		}
		return "";
	}

	@Override
	public int getRowCount() {
		if (files != null) {
			return files.length;
		}
		return 0;
	}

	@Override
	public Object getValueAt(int rowIndex, int columnIndex) {
		File file = files[rowIndex];
		switch (columnIndex) {
		case 0:
			return file.getName();
		case 1:
			return file.length();
		case 2:
			return file.getPath();
		case 3:
			return file.hashCode();
		case 4:
			return file.isDirectory();
		}
		return "";
	}

	@Override
	public boolean isCellEditable(int rowIndex, int columnIndex) {
		return false;
	}

	@Override
	public void removeTableModelListener(TableModelListener listener) {
		listeners.remove(listener);
	}

	public void setNewRoot(File root) {
		files = root.listFiles();

		TableModelEvent event = new TableModelEvent(this);
		for (TableModelListener it : listeners) {
			it.tableChanged(event);
		}
	}

	@Override
	public void setValueAt(Object value, int rowIndex, int columnIndex) {
	}

	private File[] files;
	private Set<TableModelListener> listeners = new HashSet<TableModelListener>();
}
