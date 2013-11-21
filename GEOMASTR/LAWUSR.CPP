
/*
***Preliminary***
Pseudo-code to save the user data...

(Loading code does the opposite, however, you need to check the waypoints stored with the route and not create 
duplicate ones if they were already loaded with the waypoint list. The route when edited, should really contain
a list of indexes into the waypoint table. We store the full waypoint with route so that you don't have to save/load 
the waypoints if you dont' want to - just set the number of waypoints to 0...)

We use C++ serialization to store data and pull it out of the user data files. 

Please let me know if I have omitted any needed information, or if you find anything to be incorrect.

(All pseudo-structures I am listing should be serialized in the order listed...)

enums shift out as longs...
*/

// Latitude and longitude for tMCoord are in the lowrance mercator meter format in WGS84.
// Here is the VB code stippet to translate to standard degrees format...
/*

Public Const MetersPerMile As Double = 1609.344
Public Const MilesPerNauticalMile = 1.15078
Public Const SemiMinor As Double = 6356752.3142
Public Const DegreesToRadians As Double = 0.017453292
Public Const pi As Double = 3.141592654

Public Function LatMMToDeg(ByVal y As Double)
    LatMMToDeg = (2# * Atn(Exp(CDbl(y) / SemiMinor)) - pi / 2#) / DegreesToRadians
End Function

Public Function LonMMToDeg(ByVal x As Double)
    LonMMToDeg = CDbl(x) / (DegreesToRadians * SemiMinor)
End Function

Public Function LatDegToMM(Latitude As Double) As Long
    LatDegToMM = CLng(SemiMinor * Log(Tan((Latitude * DegreesToRadians + pi / 2) / 2#)))
End Function
Public Function LonDegToMM(Longitude As Double) As Long
    LonDegToMM = CLng(Longitude * SemiMinor * DegreesToRadians)
End Function
*/

struct tMCoord 
{
	long Latitude;
	long Longitude;
};

struct tText
{
	long TextLength;
	char String[TextLength];
};

// Julian time here is number of seconds since Jan. 1, 2000
typedef long tJulian;

enum tWaypointType
{
	WAYPOINT_TYPE_USER,
	WAYPOINT_TYPE_TEMPORARY,
	WAYPOINT_TYPE_POINT_OF_INTEREST,

	NUM_WAYPOINT_TYPES
};

enum tSymbolID
{
	...
};

struct tWaypoint 
{
	tMCoord Position;
	long Altitude;
	tText Name;

	// A comment associated with the waypoint - may be shown in the arrival alarm when the waypoint is 
	// reached while navigating a route (not yet used by head unit software)
	tText Comment;

	tJulian Time;
	tSymbolID SymbolID;
	tWaypointType Type;	
};


struct tIcon
{
	tMCoord Position;
	tSymbolID SymbolID;
};

struct tRoute
{
	tText RouteName;
	short NumLegs;
	bool RouteReversed;

	tWaypoint Legs[NumLegs];
};

struct tTrail
{
	tText TrailName;
	bool TrailVisible;
	short NumTrailPoints;
	short MaxTrailSize;

//	tTrailSection TrailSections[...];
};

struct tTrailPoint
{
	tMCoord Point;

	// If true, this trail point is connected to the previous point. If false, then there was a position
	// loss in between this point and the one previous to it, so they should not be connected with a 
	// straight line.
	bool Continuous;
};

struct tTrailSection
{
	short NumSectionPoints;
	tTrailPoint Points[NumSectionPoints];
};

// Retreives the points from the given trail and stores them in the given trail section. 
void GetTrailSection(short TrailNumber, short StartPoint, short PointsToRequest, tTrailSection &TrailSection);

void SaveUserData()
{
	long i;
	short MajorVersion;
	short MinorVersion;
	short NumWaypoints;
	short NumRoutes;
	short NumIcons;
	short NumTrails;
	short ObjectNum;

	tWaypoint Waypoint;
	tText ObjectName;


	File << MajorVersion << MinorVersion;

	// Save all waypoints
	File << NumWaypoints;
	for(i = 0; i < NumWaypoints; i++)
	{
		ObjectNum = WaypointNumbers[i];
		Waypoint = Waypoints[NumWaypoints];
		File << ObjectNum << Waypoint;
	}

	// Save all routes
	short NumLegs;
	bool RouteReversed;
	File << NumRoutes;
	for(i = 0; i < NumRoutes; i++)
	{
		ObjectName = Routes[i].RouteName;
		NumLegs = Routes[i].NumLegs;
		RouteReversed = Routes[i].RouteReversed;
		
		File << ObjectName << NumLegs << RouteReversed;
		for(LegNum = 0; LegNum < NumLegs; LegNum++)
		{
			Waypoint = Routes[i].Waypoint[LegNum];
			FileWrite << Waypoint;
		}
	}

	// Save all icons
	tIcon Icon;
	FileWrite << NumIcons;
	for(i = 0; i < NumIcons; i++)
	{	
		Icon = Icons[i];
		FileWrite << Icon;
	}

	// Save all trails
	FileWrite << NumTrails;
	bool Visible;
	short NumTrailPoints, MaxTrailSize, StartPoint, PointsToRequest;
	short StartPoint;
	tTrailSection TrailSection;
	for(i = 0; i < NumTrails; i++)
	{		
		ObjectName = Trails[i].TrailName;
		Visible = Trails[i].Visible;
		NumTrailPoints = Trails[i].NumTrailPoints;
		MaxTrailSize = Trails[i].MaxTrailSize;
		
		File << ObjectName << Visible << NumTrailPoints << MaxTrailSize;
		StartPoint = 0;
		while(StartPoint < NumTrailPoints)
		{
			PointsToRequest = NUM_TRAIL_POINTS_PER_SECTION;
			
			if(StartPoint + PointsToRequest > NumTrailPoints)
				PointsToRequest = NumTrailPoints - StartPoint;
			
			GetTrailSection(i, StartPoint, PointsToRequest, TrailSection);
			
			File << TrailSection;

			StartPoint += TrailSection.NumTrailPoints;
		}
	}
}