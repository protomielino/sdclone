package utils;

import java.util.Vector;

import utils.circuit.Segment;

public class SegmentVector extends Vector<Segment>
{
    public synchronized boolean add(Segment segment)
    {
    	Segment last = null;

    	if (size() > 0)
    	{
    		last = lastElement();
   
    		last.nextShape = segment;
    	}
  
    	segment.previousShape = last;
   
    	return super.add(segment);
    }

    public synchronized void insertElementAt(Segment segment, int index)
    {
    	Segment current = null;
    	Segment previous = null;

    	if (index < size())
    	{
    		current = elementAt(index);
    		previous = current.previousShape;
    	}
    	else if (size() != 0)
    	{
    		previous = lastElement();
    	}

    	super.insertElementAt(segment,  index);
    	
    	if (previous != null)
    	{
    		previous.nextShape = segment;
    		segment.previousShape = previous;
    	}

    	segment.nextShape = current;

    	if (current != null)
    	{
    		current.previousShape = segment;
    	}
    }

    public synchronized Segment remove(int index)
    {
    	Segment current = elementAt(index);
    	Segment next = current.nextShape;
    	Segment previous = current.previousShape;

    	if (next != null)
    	{
    		next.previousShape = previous;
    	}

    	if (previous != null)
    	{
    		previous.nextShape = next;
    	}

    	current.nextShape = null;
    	current.previousShape = null;

    	return super.remove(index);
    }

    public synchronized void removeElementAt(int index)
    {
    	Segment current = elementAt(index);
    	Segment next = current.nextShape;
    	Segment previous = current.previousShape;

    	if (next != null)
    	{
    		next.previousShape = previous;
    	}

    	if (previous != null)
    	{
    		previous.nextShape = next;
    	}

    	current.nextShape = null;
    	current.previousShape = null;

    	super.removeElementAt(index);
    }

	public void dump(String indent)
    {
		System.out.println(indent + "Track Segments");

		for (int i = 0; i < size(); i++)
		{
			System.out.println(indent + "  segment[" + i + "]");
			get(i).dump(indent + "    ");
		}
    }
}
