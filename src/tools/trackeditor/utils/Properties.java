/*
 *   Properties.java
 *   Created on 27 ??? 2005
 *
 *    The Properties.java is part of TrackEditor-0.3.1.
 *
 *    TrackEditor-0.3.1 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    TrackEditor-0.3.1 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with TrackEditor-0.3.1; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
package utils;

import java.awt.event.ActionListener;
import java.util.Vector;

/**
 * @author babis
 *
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public  class Properties
{
	private static Properties	instance				= new Properties();
	private Vector<ActionListener>	propertiesListeners	= new Vector<ActionListener>();
	public final String			title					= "sd2-trackeditor";
	public final String			version					= "1.5.2";
	private String				path;

	private double				imageScale				= 1;
	private String				image					= "";
	private EditorPoint			imgOffset				= new EditorPoint();
	private double				initx;
	private double				inity;

	private double				currentX				= 0;
	private double				currentY				= 0;
	private double				currentZ				= 0;
	private double				currentA				= 0;
	private double				currentBanking			= 0;

	private double				showArrows				= 0;
	private double				trackStartDist			= 0;
	private int					curveNameCount			= 0;
	private int					straightNameCount		= 0;

	/**
	 *
	 */
	public Properties()
	{
//		System.out.println("Properties constructor");
	}

	public static Properties getInstance()
	{
		return instance;
	}

	/**
	 * @param instance The instance to set.
	 */
	public static void setInstance(Properties instance)
	{
		Properties.instance = instance;
	}
	/**
	 * @return Returns the path.
	 */
	public String getPath()
	{
		return path;
	}
	/**
	 * @param path
	 *            The path to set.
	 */
	public void setPath(String path)
	{
		this.path = path;
	}

	/**
	 * @return Returns the image.
	 */
	public String getImage()
	{
		return image;
	}
	/**
	 * @param image
	 *            The image to set.
	 */
	public void setImage(String image)
	{
		this.image = image;
	}

	/**
	 * @return Returns the imageScale.
	 */
	public double getImageScale()
	{
		return imageScale;
	}
	/**
	 * @param imageScale
	 *            The imageScale to set.
	 */
	public void setImageScale(double imageScale)
	{
		this.imageScale = imageScale;
	}

	/**
	 * @return Returns the initx.
	 */
	public double getInitx()
	{
		return initx;
	}
	/**
	 * @param initx
	 *            The initx to set.
	 */
	public void setInitx(double initx)
	{
		this.initx = initx;
	}

	/**
	 * @return Returns the inity.
	 */
	public double getInity()
	{
		return inity;
	}
	/**
	 * @param inity
	 *            The inity to set.
	 */
	public void setInity(double inity)
	{
		this.inity = inity;
	}

	public synchronized void removePropertiesListener(ActionListener l)
	{

	}

	public synchronized void addPropertiesListener(ActionListener l)
	{
		Vector<ActionListener> v = propertiesListeners == null ? new Vector<ActionListener>(2) : (Vector<ActionListener>) propertiesListeners.clone();
		if (!v.contains(l))
		{
			v.addElement(l);
			propertiesListeners = v;
		}
	}

	public void valueChanged()
	{
		if (propertiesListeners != null)
		{
			Vector<ActionListener> listeners = propertiesListeners;
			int count = listeners.size();
			for (int i = 0; i < count; i++)
			{
				listeners.elementAt(i).actionPerformed(null);
			}
		}
	}
	/**
	 * @return Returns the currentA.
	 */
	public double getCurrentA()
	{
		return currentA;
	}
	/**
	 * @param currentA The currentA to set.
	 */
	public void setCurrentA(double currentA)
	{
		this.currentA = currentA;
	}
	/**
	 * @return Returns the currentBanking.
	 */
	public double getCurrentBanking()
	{
		return currentBanking;
	}
	/**
	 * @param currentBanking The currentBanking to set.
	 */
	public void setCurrentBanking(double currentBanking)
	{
		this.currentBanking = currentBanking;
	}
	/**
	 * @return Returns the currentX.
	 */
	public double getCurrentX()
	{
		return currentX;
	}
	/**
	 * @param currentX The currentX to set.
	 */
	public void setCurrentX(double currentX)
	{
		this.currentX = currentX;
	}
	/**
	 * @return Returns the currentY.
	 */
	public double getCurrentY()
	{
		return currentY;
	}
	/**
	 * @param currentY The currentY to set.
	 */
	public void setCurrentY(double currentY)
	{
		this.currentY = currentY;
	}
	/**
	 * @return Returns the currentZ.
	 */
	public double getCurrentZ()
	{
		return currentZ;
	}
	/**
	 * @param currentZ The currentZ to set.
	 */
	public void setCurrentZ(double currentZ)
	{
		this.currentZ = currentZ;
	}
	/**
	 * @return Returns the showArrows.
	 */
	public double getShowArrows()
	{
		return showArrows;
	}
	/**
	 * @param showArrows The showArrows to set.
	 */
	public void setShowArrows(double showArrows)
	{
		this.showArrows = showArrows;
	}
	/**
	 * @return Returns the trackStartDist.
	 */
	public double getTrackStartDist()
	{
		return trackStartDist;
	}
	/**
	 * @param trackStartDist The trackStartDist to set.
	 */
	public void setTrackStartDist(double trackStartDist)
	{
		this.trackStartDist = trackStartDist;
	}

    /**
     * @return Returns the imgOffset.
     */
    public EditorPoint getImgOffset()
    {
        return imgOffset;
    }
    /**
     * @param imgOffset The imgOffset to set.
     */
    public void setImgOffset(EditorPoint imgOffset)
    {
        this.imgOffset = imgOffset;
    }
    /**
     * @return Returns the curveNameCount.
     */
    public int getCurveNameCount()
    {
        return curveNameCount;
    }
    /**
     * @param curveNameCount The curveNameCount to set.
     */
    public void setCurveNameCount(int curveNameCount)
    {
        this.curveNameCount = curveNameCount;
    }
    /**
     * @return Returns the straightNameCount.
     */
    public int getStraightNameCount()
    {
        return straightNameCount;
    }
    /**
     * @param straightNameCount The straightNameCount to set.
     */
    public void setStraightNameCount(int straightNameCount)
    {
        this.straightNameCount = straightNameCount;
    }
}
