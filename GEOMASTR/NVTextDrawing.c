//  NVTextDrawing.m
//  CCodeLibrary

#include "graphint.h"

static int ii = 0;
struct CGPoint {
	CGFloat x;
	CGFloat y;
};
typedef struct CGPoint CGPoint;

/* Sizes. */

struct CGSize {
	CGFloat width;
	CGFloat height;
};
typedef struct CGSize CGSize;

/* Vectors. */

#define CGVECTOR_DEFINED 1

struct CGVector {
	CGFloat dx;
	CGFloat dy;
};
typedef struct CGVector CGVector;

/* Rectangles. */

typedef struct {
	CGPoint origin;
	CGSize size;
}CGRect;
//typedef struct CGRect CGRect;
/*** Definitions of inline functions. ***/

CGPoint
CGPointMake(CGFloat x, CGFloat y)
{
	CGPoint p; p.x = x; p.y = y; return p;
}

CGSize
CGSizeMake(CGFloat width, CGFloat height)
{
	CGSize size; size.width = width; size.height = height; return size;
}

CGVector
CGVectorMake(CGFloat dx, CGFloat dy)
{
	CGVector vector; vector.dx = dx; vector.dy = dy; return vector;
}

CGRect
CGRectMake(CGFloat x, CGFloat y, CGFloat width, CGFloat height)
{
	CGRect rect;
	rect.origin.x = x; rect.origin.y = y;
	rect.size.width = width; rect.size.height = height;
	return rect;
}

RECT CGRectToRect(CGRect cgrect)
{
	RECT rect;

	rect.left = cgrect.origin.x - cgrect.size.width / 2;
	rect.right = rect.left + cgrect.size.width;
	rect.top = cgrect.origin.y + cgrect.size.height / 2;
	rect.bottom = rect.top - cgrect.size.height;
	return rect;
}

BOOL
__CGPointEqualToPoint(CGPoint point1, CGPoint point2)
{
	return point1.x == point2.x && point1.y == point2.y;
}
#define CGPointEqualToPoint __CGPointEqualToPoint

BOOL
__CGSizeEqualToSize(CGSize size1, CGSize size2)
{
	return size1.width == size2.width && size1.height == size2.height;
}

#define CGSizeEqualToSize __CGSizeEqualToSize

#define CGRectZero CGRectMake(0, 0, 0, 0)

/* Rectangles. */

typedef struct CGRect {
	CGPoint origin;
	CGSize size;
};
//typedef struct CGRect CGRect;


void drawTextInRotatedRect(char* text, CGContextRef context, CGFloat mpX, CGFloat mpY, CGFloat textWidth,
	CGFloat textHeight, CGFloat rotateRadians, CGFloat fontSize, UIColor *textColor);

void CGContextSaveGState(CGContextRef context)
{
	SaveDC(context);
}
void CGContextRestoreGState(CGContextRef context)
{
	RestoreDC(context, -1);
}

void CGContextTranslateCTM(CGContextRef context, CGFloat mpX, CGFloat mpY)
{
}

void CGContextRotateCTM(CGContextRef context, CGFloat rotateRadians)
{
/*	CurView->hTranScreenToVP = STRAN2(1608, XFROM, YFROM, XTO, YTO, 2, &RSQMIN, 1, 0);
	CurView->xForm = SetXFORMFromTRANS(CurView->hTranScreenToVP);
	CloseTRANS2(&CurView->hTranScreenToVP);
*/
}
void CGContextSetGrayFillColor(CGContextRef context, int i, int j)
{

}
CGContextFillRect(CGContextRef context, CGRect areaRect)
{

}


MNMXCORD PolyBounds(LPDPOINT pPoints, int numPoints)
{
	MNMXCORD bounds;
	GetPolyBoundsD2(pPoints, (int)numPoints, &bounds, TYPE_AREA);
	return bounds;
}

int numLinesInText(const char *txt)
{
	int numLines = 1;

	while (*txt)
	{
		if (*txt++ == '\n')
			numLines++;
	}
	return numLines;
}

void drawTextwithinPolygon(char *text
	/*inContext : */, CGContextRef context
	/*withinPolygon :*/, LPDPOINT pPoints
	/*withNumPoints :*/, NSInteger numPoints
	/*andNumLoops :*/, NSInteger numLoops
	/*loopLen :*/, int* polyPartLen
	/*withinBounds :*/, LPMNMXCORD pBounds
	/*fontSize :*/, CGFloat fontSize
	/*minFontSize :*/, CGFloat minFontSize
	/*textColor :*/, UIColor * textColor)
{
	NSInteger nLines = 0;
	CGFloat textArea = 0.0;
	CGFloat aveCharWidth = 0.0, aveCharHeight = 0.0;

	//PointInArea *pia = [[PointInArea alloc] init];
	//pia.maxDimension = 256;
	HANDLE hPolyPartLen = 0;
	HANDLE hPIA = PointInAreaAcceleratorSetup(numPoints, pPoints, numLoops, hPolyPartLen, 0);

	CGFloat areaSize = ComputeAreaAreaD(pPoints, numPoints, 0);
	MNMXCORD areaBounds = PolyBounds(pPoints, (int)numPoints);
	if (pBounds)
	{
		areaBounds.xmn = max(areaBounds.xmn, pBounds->xmn);
		areaBounds.ymn = max(areaBounds.ymn, pBounds->ymn);
		areaBounds.xmx = min(areaBounds.xmx, pBounds->xmx);
		areaBounds.ymx = min(areaBounds.ymx, pBounds->ymx);
	}
	DPOINT areaMid = MinMaxMidPointD(&areaBounds);
	CGFloat areaWidth = BoundsWidth(&areaBounds);
	CGFloat areaHeight = BoundsHeight(&areaBounds);

	/*	[pia loadArea : (int)numPoints
	numLoops : (int)numLoops
	polyPartLen : polyPartLen
	points : pPoints];*/

	int numTextLines = numLinesInText(text);
	/*
	CTFontRef textPointFontRef = CTFontCreateWithName((CFStringRef)NULL,
	fontSize, NULL);

	NSMutableParagraphStyle *style = NSParagraphStyle.defaultParagraphStyle.mutableCopy;
	style.alignment = NSTextAlignmentCenter;

	NSDictionary *textPointAttributes = [NSDictionary dictionaryWithObjectsAndKeys :
	(__bridge id)textPointFontRef, kCTFontAttributeName,
	style, kCTParagraphStyleAttributeName,
	nil];

	CFRelease(textPointFontRef);

	NSAttributedString *textPointString = [[NSAttributedString alloc] initWithString:text
	attributes : textPointAttributes];*/

	// LPDPOINT textPoints = (LPDPOINT)malloc((sizeof(DPOINT) * (textPointString.length + 32)) + 4);//enough memory for 16 lines
	DPOINT textPoints[256] = { 0 };

	int nTextPoints = (int)getTextContainerPoints(textPoints, &nLines, &textArea, &aveCharWidth, &aveCharHeight, text);

	MNMXCORD textBounds = PolyBounds(textPoints, nTextPoints);
	CGFloat textWidth = BoundsWidth(&textBounds);
	CGFloat textHeight = BoundsHeight(&textBounds);
	DPOINT textMid = MinMaxMidPointD(&textBounds);

	for (int i = 0; i < nTextPoints; i++)
	{
		textPoints[i].x -= textMid.x;
		textPoints[i].y -= textMid.y;
	}

	CGFloat rotation = 0.0;

	/*	if ([NVTextDrawing getRotation : &rotation
	ofLongestStraightLine : (int)numPoints
	points : pPoints
	maxOffset : 1.0
	minLength : textWidth])
	{

	}*/

	if (polyPartLen)
		free(polyPartLen);

	free(pPoints);


	//LPDPOINT testPoints = (LPDPOINT)malloc(nTextPoints * sizeof(DPOINT) + 4);
	DPOINT testPoints[256];

	if (ii)
	{ //debug stuff
		/*		int ir=0, ic;
		CGFloat xoffset = 0.0, yoffset = 0.0;
		CGFloat xinc = 1;
		CGFloat yinc = 1;
		CGRect areaRect = CGRectMake(areaBounds.xmn, areaBounds.ymn, areaWidth, areaHeight);
		while (yoffset < curbounds.ymx)
		{
		xoffset = curbounds.xmn;
		ic = 0;
		while (xoffset < curbounds.xmx)
		{
		DPOINT pt = {xoffset,yoffset};
		if ([pia DPointInArea:&pt])
		{
		CGRect rect = CGRectMake(xoffset, yoffset, 2, 2);
		CGContextSetGrayFillColor(context, 0.5, 0.5);

		CGContextFillRect(context, rect);
		}
		xoffset += xinc;
		}
		ir++;
		yoffset += yinc;
		}
		CGContextSetGrayFillColor(context, 1, 1);
		CGContextFillRect(context, areaRect);*/
	}
	if (areaSize && textArea)
	{
		CGFloat desiredTextFactor = 1.0;
		CGFloat textFactor = (areaSize / textArea);
		CGFloat textFactor2 = 1;
		CGFloat minTextFactor = (desiredTextFactor * ((CGFloat)minFontSize / (CGFloat)fontSize) * ((CGFloat)minFontSize / (CGFloat)fontSize));
		CGFloat shrink = 0.95;
		NSInteger numFit = 0;
#define MAXFIT 256
		CGFloat fitX[MAXFIT] = { 0 }, fitY[MAXFIT] = { 0 };
		CGFloat xoffset = 0.0, yoffset = 0.0;
		BOOL firstFit = YES;
	Top:
		while (!numFit && (textFactor > minTextFactor))
		{
			CGFloat xinc = (aveCharWidth * textFactor) / 2;
			CGFloat yinc = (aveCharHeight * textFactor) / 2;
			yoffset = areaMid.y - areaHeight + textHeight * textFactor / 2;

			while (yoffset < areaBounds.ymx)
			{
				xoffset = areaMid.x - areaWidth / 2 + textWidth * textFactor / 2;

				while (xoffset < areaBounds.xmx)
				{
					for (int i = 0; i < nTextPoints; i++)
					{
						testPoints[i].x = textPoints[i].x * textFactor * textFactor2 + xoffset;
						testPoints[i].y = yoffset - textPoints[i].y * textFactor * textFactor2;
						if (!DPointInBounds(&testPoints[i], pBounds))
							goto NextX;
						//if (![pia DPointInArea : &testPoints[i]])
							goto NextX;
					}
					if (ii)
					{
						for (int i = 0; i < nTextPoints; i++)
						{
							static CGFloat pctBlack = 0;
							CGRect rect = CGRectMake(testPoints[i].x, testPoints[i].y, 16, 16);
							CGContextSetGrayFillColor(context, pctBlack, 1);
							CGContextFillRect(context, rect);
						}
					}
					if (numFit < MAXFIT)
					{
						if (firstFit)
						{
							firstFit = NO;
							textFactor2 = 0.95;
							goto Top;
						}
						fitX[numFit] = xoffset;
						fitY[numFit++] = yoffset;
					}

				NextX:
					xoffset += xinc;
				}

				yoffset += yinc;
			}

			textFactor *= shrink;
		}

		if (numFit)
		{
			CGFloat fontFactor = min(textFactor*textFactor2, desiredTextFactor);
			fontSize *= fontFactor;
			fontSize *= 0.9;

			if (fontSize >= minFontSize)
			{
				NSInteger iFit = numFit / 2;
				if (!(numFit % 2))
				{
					fitX[iFit] += fitX[iFit - 1];
					fitX[iFit] /= 2;
					fitY[iFit] += fitY[iFit - 1];
					fitY[iFit] /= 2;
				}
				drawTextInRotatedRect(text, context,
					fitX[iFit], fitY[iFit], (textWidth * fontFactor), (textHeight * fontFactor), rotation, fontSize, textColor);
			}
		}
	}

}

void drawTextInRotatedRect(char* text, CGContextRef context, CGFloat mpX, CGFloat mpY, CGFloat textWidth,
	CGFloat textHeight, CGFloat rotateRadians, CGFloat fontSize, UIColor *textColor)
{
	CGRect cgrect = CGRectMake(0, 0, 0, 0);
	RECT rect;
	//NSAttributedString *attString = nil;

	cgrect.size.height = (-textHeight * 1.1);
	cgrect.size.width = (textWidth * 1.1);
	cgrect.origin.x = (-cgrect.size.width / 2.0);
	cgrect.origin.y = (textHeight / 2.0);
	rect = CGRectToRect(cgrect);
	CGContextSaveGState(context);
	CGContextTranslateCTM(context, mpX, mpY);
	CGContextRotateCTM(context, rotateRadians);

	/*if (dbug)
	{
	CGRect rectmp = CGRectMake(fitX[iFit]-8, fitY[iFit],16, 2);
	CGContextSetGrayFillColor(context, 0.5, 0.5);
	CGContextFillRect(context, rect);
	CGContextSetGrayFillColor(context, 0, 0.8);
	CGContextFillRect(context, rectmp);
	}*/

/*	CTFontRef fontRef = CTFontCreateWithName((CFStringRef)NULL,
		fontSize, NULL);

	NSMutableParagraphStyle *style = NSParagraphStyle.defaultParagraphStyle.mutableCopy;
	style.alignment = NSTextAlignmentCenter;

	NSDictionary *attributes = [NSDictionary dictionaryWithObjectsAndKeys :
	(__bridge id)fontRef, kCTFontAttributeName,
		style, kCTParagraphStyleAttributeName,
		textColor.CGColor, kCTForegroundColorAttributeName,
		nil];


	CFRelease(fontRef);

	CGPathRef path = CGPathCreateWithRect(rect, NULL);

	attString = [[NSAttributedString alloc] initWithString:text
	attributes : attributes];

	CTFramesetterRef framesetter = CTFramesetterCreateWithAttributedString((CFAttributedStringRef)attString);
	CTFrameRef frame = CTFramesetterCreateFrame(framesetter, CFRangeMake(0, attString.length), path, NULL);
	CTFrameDraw(frame, context);

	CFRelease(frame);
	CFRelease(framesetter);
	CGPathRelease(path);
	CGContextRestoreGState(context);
	*/
	UINT format = DT_CENTER|DT_TOP;
	DRAWTEXTPARAMS parms;
	LPDRAWTEXTPARAMS pParms = 0;
	DrawTextEx(context, text, strlen(text), &rect, format, pParms);
}

int getTextContainerPoints(LPDPOINT textPoints,
	/*numLines : (*/NSInteger *pnLines,
	/*totArea : (*/CGFloat *totArea,
	/*averageCharacterWidth : (*/CGFloat *aveCharWidth,
	/*averageCharacterHeight : (*/CGFloat *aveCharHeight,
	/*attributedString : (*/NSAttributedString *attributedString)
{
	NSUInteger location = 0, nPoints = 0;
	/*	NSArray *textLines = [attributedString.string componentsSeparatedByString : @"\n"];
	*pnLines = textLines.count;
	NSTextStorage *textStorage = [[NSTextStorage alloc] initWithAttributedString:attributedString];
	NSLayoutManager *layoutManager = [[NSLayoutManager alloc] init];
	[textStorage addLayoutManager : layoutManager];
	NSTextContainer *textContainer = [[NSTextContainer alloc] initWithSize:CGSizeMake(2048.0, 2048.0)];
	textContainer.lineFragmentPadding = 0.0;
	[layoutManager addTextContainer : textContainer];

	NSRange glyphRange = { 0, 1 };
	CGFloat totWidth = 0, totHeight = 0;
	NSInteger totChar = 0;
	*totArea = 0;

	for (int i = 0; i < textLines.count; i++)
	{
		NSString *textLineString = textLines[i];
		NSUInteger length = textLineString.length;
		NSRange range = { location, length };

		[layoutManager characterRangeForGlyphRange : range
		actualGlyphRange : &glyphRange];

		CGRect glyphRect = [layoutManager boundingRectForGlyphRange : glyphRange
		inTextContainer : textContainer];

		DPOINT minXY = { glyphRect.origin.x, glyphRect.origin.y };
		DPOINT maxXY = { glyphRect.size.width + minXY.x, glyphRect.size.height + minXY.y };

		textPoints[nPoints++] = minXY;
		textPoints[nPoints++] = maxXY;

		location += (length + 1);
		(*totArea) += (maxXY.x - minXY.x) * (maxXY.y - minXY.y);
		totWidth += maxXY.x - minXY.x;
		totHeight += maxXY.y - minXY.y;
		totChar += length;
	}

	for (int i = 0; i < attributedString.length; i++)
	{
		if ([attributedString.string characterAtIndex : i] != '\n' &&
			[attributedString.string characterAtIndex : i] != '\r')
		{
			NSRange range = { i, 1 };

			[layoutManager characterRangeForGlyphRange : range
			actualGlyphRange : &glyphRange];

			CGRect glyphRect = [layoutManager boundingRectForGlyphRange : glyphRange
			inTextContainer : textContainer];

			DPOINT center = { (glyphRect.origin.x + (glyphRect.size.width / 2.0)),
				(glyphRect.origin.y + (glyphRect.size.height / 2.0)) };

			textPoints[nPoints++] = center;
		}
	}

	if (totChar)
		*aveCharWidth = (totWidth / totChar);

	if (textLines.count)
		*aveCharHeight = (totHeight / textLines.count);
*/
	return nPoints;
}
/*
+(BOOL)getRotation:(CGFloat *)rotation
ofLongestStraightLine : (int)nPoints
					points : (DPOINT *)pPoints
						 maxOffset : (CGFloat)offset
								 minLength : (CGFloat)minLength
{
	BOOL rtn = NO;
	CGFloat longestLine = 0.0, currentRot = 0.0;
	nPoints--;

	for (int i = 0; i < nPoints; i++)
	{
		CGFloat lineLength = GetPolyLengthD(&pPoints[i], 2);

		if (lineLength > longestLine)
		{
			currentRot = getazd(&pPoints[i], &pPoints[i + 1]);
			longestLine = lineLength;
		}
	}

	if (nPoints > 4)
	{
		LPDPOINT wrappedPoints = (LPDPOINT)malloc(sizeof(DPOINT) * (nPoints * 2));

		for (int i = 0; i < nPoints; i++)
		{
			wrappedPoints[i] = pPoints[i];
			wrappedPoints[nPoints + i] = pPoints[i];
		}

		double a = 0.0, b = 0.0, maxDiff = 0.0;
		long locOfMaxDiff = 0;

		for (int i = 0; i < nPoints; i++)
		{
			long numPoints = 2;

			do
			{
				numPoints++;
				LINFIT(&wrappedPoints[i], numPoints, &a, &b, &maxDiff, &locOfMaxDiff);
			}

			while ((maxDiff < offset) && (numPoints < nPoints));

			CGFloat lineLength = GetPolyLengthD(&wrappedPoints[i], numPoints);

			if ((lineLength > longestLine) && (maxDiff < offset))
			{
				currentRot = b;
				longestLine = lineLength;
			}
		}

		free(wrappedPoints);
	}

	if ((currentRot > HALFPI) && (currentRot < (3.0 * HALFPI)))
		currentRot = LTWOPI(currentRot + M_PI);

	if (longestLine > minLength)
	{
		*rotation = currentRot;
		rtn = YES;
	}

	return rtn;
}
*/
