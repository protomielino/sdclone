package utils.undo;

import java.util.Vector;

import utils.circuit.GraphicObject;

public class UndoMoveAllGraphicObjects implements UndoInterface
{
	private Vector<GraphicObject> graphicObjects;
	
	public UndoMoveAllGraphicObjects(Vector<GraphicObject> graphicObjects, Vector<MovedObject>	movedObjects)
	{
		this.graphicObjects = graphicObjects;
	}

	public void undo()
	{
	}

	public void redo()
	{
	}
}
