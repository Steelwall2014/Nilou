vec4 ApplyHandedCoordinateSystem(vec4 ClipPosition)
{
#if USING_OPENGL
	ClipPosition.x *= -1;
	return ClipPosition;
#else
	return ClipPosition;
#endif
}