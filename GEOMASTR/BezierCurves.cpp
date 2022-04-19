#include <windows.h>

//https://stackoverflow.com/questions/785097/how-do-i-implement-a-bézier-curve-in-c/21642962#21642962
//http://blog.sklambert.com/finding-the-control-points-of-a-bezier-curve/

struct vec2 {
	float x, y;
	vec2(float x, float y) : x(x), y(y) {}
};

vec2 operator + (vec2 a, vec2 b) {
	return vec2(a.x + b.x, a.y + b.y);
}

vec2 operator - (vec2 a, vec2 b) {
	return vec2(a.x - b.x, a.y - b.y);
}

vec2 operator * (float s, vec2 a) {
	return vec2(s * a.x, s * a.y);
}

typedef struct { double x, y; } DPOINT;

vec2 getBezierPoint(vec2* points, int numPoints, float t) {
	vec2* tmp = (vec2*)malloc(numPoints * sizeof(vec2));
	memcpy(tmp, points, numPoints * sizeof(vec2));
	int i = numPoints - 1;
	while (i > 0) {
		for (int k = 0; k < i; k++)
			tmp[k] = tmp[k] + t * (tmp[k + 1] - tmp[k]);
		i--;
	}
	vec2 answer = tmp[0];
	free (tmp);
	return answer;
}

extern "C" DPOINT GetBezierPoint(DPOINT * points, int nPoints, float t)
{
	vec2* vpoints = (vec2*)malloc(nPoints * sizeof(vec2));
	for (int i = 0; i < nPoints;i++)
	{
		vpoints[i].x = points[i].x;
		vpoints[i].y = points[i].y;
	}
	vec2 rtn = getBezierPoint(vpoints, nPoints, t);
	free (vpoints);
	DPOINT drtn;
	drtn.x = rtn.x;
	drtn.y = rtn.y;
	return drtn;
}