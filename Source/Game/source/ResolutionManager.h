#pragma once

#include <tge/math/Vector.h>

class ResolutionManager
{
public:
	static void Init();
	static void Update();//call on resolution change

	static Tga::Vector2f GetScreenPosition(float normalizedX, float normalizedY);//Convert normalized coordinates (0-1) to screen coordinates

	static Tga::Vector2f ScaleSize(float width, float height);
	static float ScaleValue(float value);

	static Tga::Vector2f GetReferenceResolution() { return myReferenceResolution; }//e.g., 1920x1080
	static Tga::Vector2f GetCurrentResolution() { return myCurrentResolution; }
	static float GetUIScale() { return myUIScale; }//Get scale factor to maintain aspect ratio
	static float GetAspectRatio() { return myCurrentResolution.x / myCurrentResolution.y; }

private:
	static Tga::Vector2f myReferenceResolution;
	static Tga::Vector2f myCurrentResolution;
	static float myUIScale;
};