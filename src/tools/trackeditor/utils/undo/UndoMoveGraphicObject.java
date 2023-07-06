package utils.undo;

import java.util.Vector;

import utils.circuit.GraphicObject;
import utils.circuit.ObjShapeObject;
import utils.circuit.ObjectMap;

public class UndoMoveGraphicObject implements UndoInterface
{
	private Vector<GraphicObject> graphicObjects;
	
	public UndoMoveGraphicObject(Vector<GraphicObject> graphicObjects, GraphicObject graphicObject, ObjectMap objectMap, ObjShapeObject object)
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
