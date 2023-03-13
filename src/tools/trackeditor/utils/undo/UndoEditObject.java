package utils.undo;

import utils.circuit.ObjShapeObject;
import utils.circuit.ObjectMap;

public class UndoEditObject  implements UndoInterface
{
	private ObjectMap		objectMap;
	private ObjShapeObject 	original;
	private ObjShapeObject 	clone;
	private int 			index;

	public UndoEditObject(ObjectMap objectMap, ObjShapeObject object)
	{
		this.objectMap = objectMap;
		clone = (ObjShapeObject) object.clone();
		this.original = object;
	}

	public void undo()
	{
		index = objectMap.getObjectIndex(original);
		objectMap.setObjectAt(index, clone);
	}

	public void redo()
	{
		index = objectMap.getObjectIndex(clone);
		objectMap.setObjectAt(index, original);
	}
}
