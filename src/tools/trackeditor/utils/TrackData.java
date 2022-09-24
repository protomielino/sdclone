/*
 *   TrackData.java
 *   Created on Aug 26, 2004
 *
 *    The TrackData.java is part of TrackEditor-0.6.2.
 *
 *    TrackEditor-0.6.2 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    TrackEditor-0.6.2 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with TrackEditor-0.6.2TrackEditor; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
package utils;

import java.util.Vector;

import utils.circuit.Camera;
import utils.circuit.Graphic;
import utils.circuit.Header;
import utils.circuit.LocalInfo;
import utils.circuit.MainTrack;
import utils.circuit.Sector;
import utils.circuit.StartingGrid;
import utils.circuit.Surface;
import utils.circuit.TrackLight;
import utils.circuit.TrackObject;


/**
 * @author Charalampos Alexopoulos
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public final class TrackData
{
	private Header				header					= new Header();
    private LocalInfo			localInfo				= new LocalInfo();
    private StartingGrid		startingGrid			= new StartingGrid();
    private Graphic				graphic					= new Graphic();
    private Vector<Surface> 	surfaces				= new Vector<Surface>();
    private Vector<Camera> 		cameras					= new Vector<Camera>();
    private Vector<TrackObject> trackObjects			= new Vector<TrackObject>();
    private Vector<TrackLight> 	trackLights				= new Vector<TrackLight>();
    private MainTrack			mainTrack				= new MainTrack();
    private SegmentVector		segments				= null;
    private Vector<Sector>		sectors					= new Vector<Sector>();
   
	/**
	 * @return Returns the header.
	 */
	public Header getHeader()
	{
		return header;
	}
	/**
	 * @param header
	 *            The header to set.
	 */
	public void setHeader(Header header)
	{
		this.header = header;
	}

    /**
     * @return Returns the localInfo.
     */
    public LocalInfo getLocalInfo()
    {
        return localInfo;
    }
    /**
     * @param localInfo The localInfo to set.
     */
    public void setLocalInfo(LocalInfo localInfo)
    {
        this.localInfo = localInfo;
    }

    /**
     * @return Returns the startingGrid.
     */
    public StartingGrid getStartingGrid()
    {
        return startingGrid;
    }
    /**
     * @param startingGrid The startingGrid to set.
     */
    public void setStartingGrid(StartingGrid startingGrid)
    {
        this.startingGrid = startingGrid;
    }

    /**
     * @return Returns the surfaces.
     */
    public Vector<Surface> getSurfaces()
    {
        return surfaces;
    }
    /**
     * @param trackData The surfaces to set.
     */
    public void setSurfaces(Vector<Surface> data)
    {
        surfaces = data;
    }

    /**
     * @return Returns the cameras.
     */
    public Vector<Camera> getCameras()
    {
        return cameras;
    }
    /**
     * @param cameras The cameras to set.
     */
    public void setCameras(Vector<Camera> data)
    {
        cameras = data;
    }

    /**
     * @return Returns the trackObjects.
     */
    public Vector<TrackObject> getObjects()
    {
        return trackObjects;
    }
    /**
     * @param objects The trackObjects to set.
     */
    public void setObjects(Vector<TrackObject> data)
    {
        trackObjects = data;
    }

    /**
     * @return Returns the trackLights.
     */
    public Vector<TrackLight> getTrackLights()
    {
        return trackLights;
    }
    /**
     * @param trackLights The trackLights to set.
     */
    public void setTrackLights(Vector<TrackLight> data)
    {
        trackLights = data;
    }

    /**
     * @return Returns the graphic.
     */
    public Graphic getGraphic()
    {
		return graphic;
	}
    /**
     * @param graphic The graphics to set.
     */
	public void setGraphic(Graphic data)
	{
		graphic = data;
	}

    /**
     * @return Returns the mainTrack.
     */
    public MainTrack getMainTrack()
    {
		return mainTrack;
	}
    /**
     * @param mainTrack The mainTrack to set.
     */
	public void setMainTrack(MainTrack data)
	{
		mainTrack = data;
	}

    /**
     * @return Returns the trackData.
     */
    public SegmentVector getSegments()
    {
        return segments;
    }
    /**
     * @param trackData The trackData to set.
     */
    public void setSegments(SegmentVector segments)
    {
        this.segments = segments;
    }
    
	public Vector<TrackObject> getTrackObjects()
	{
		return trackObjects;
	}
	public void setTrackObjects(Vector<TrackObject> trackObjects)
	{
		this.trackObjects = trackObjects;
	}
	
	public Vector<Sector> getSectors()
	{
		return sectors;
	}
	public void setSectors(Vector<Sector> sectors)
	{
		this.sectors = sectors;
	}
}
