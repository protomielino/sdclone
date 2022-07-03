/*
 *   UndoDeleteSegment.java
 *   Created on 29 ??? 2005
 *
 *    The UndoDeleteSegment.java is part of TrackEditor-0.6.0.
 *
 *    TrackEditor-0.6.0 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    TrackEditor-0.6.0 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with TrackEditor-0.6.0; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
package utils.undo;

import java.util.Vector;

import gui.EditorFrame;
import utils.Editor;
import utils.circuit.Segment;

/**
 * @author Charalampos Alexopoulos
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class UndoDeleteSegment implements UndoInterface
{
	private EditorFrame	editorFrame;
	private Segment 	undo;
	private Segment 	redo;
	private int 		pos;

	/**
	 * 
	 */
	public UndoDeleteSegment(EditorFrame editorFrame, Segment segment)
	{
		this.editorFrame = editorFrame;
		this.undo = segment;
		this.redo = null;
		Vector<Segment> data = editorFrame.getTrackData().getSegments();
		pos = data.indexOf(undo);
	}

	/* (non-Javadoc)
	 * @see utils.undo.UndoInterface#undo()
	 */
	public void undo()
	{
		Vector<Segment> data = editorFrame.getTrackData().getSegments();	
		Segment prevSeg = undo.getPreviousShape();
		Segment nextSeg = undo.getNextShape();
		if (undo.getType() == "str")
		{
			int count = Editor.getProperties().getStraightNameCount() + 1;
			Editor.getProperties().setStraightNameCount(count);
		}
		else
		{
			int count = Editor.getProperties().getCurveNameCount() + 1;
			Editor.getProperties().setCurveNameCount(count);			
		}		
		undo.addToPrevious(prevSeg);
		undo.addToNext(nextSeg);				
		data.insertElementAt(undo,pos);
		redo = undo;
		undo = null;
	}

	/* (non-Javadoc)
	 * @see utils.undo.UndoInterface#redo()
	 */
	public void redo()
	{
		Vector<Segment> data = editorFrame.getTrackData().getSegments();
		pos = data.indexOf(redo);
		if (redo.getType() == "str")
		{
			int count = Editor.getProperties().getStraightNameCount() - 1;
			Editor.getProperties().setStraightNameCount(count);
		}
		else
		{
			int count = Editor.getProperties().getCurveNameCount() - 1;
			Editor.getProperties().setCurveNameCount(count);			
		}		
		if (redo.getPreviousShape() != null)
			redo.getPreviousShape().setNextShape(redo.getNextShape());
		if (redo.getNextShape() != null)
			redo.getNextShape().setPreviousShape(redo.getPreviousShape());
		data.remove(redo);
		undo = redo;
		redo = null;
	}
}
