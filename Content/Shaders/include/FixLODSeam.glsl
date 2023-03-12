bool FixLODSeam(inout vec3 pos, float scale, RenderPatch current_patch)
{
	uvec2 vertex_index = uvec2(gl_VertexID / PatchGridSideNum, gl_VertexID % PatchGridSideNum);
	bool on_edge = false;
	if (vertex_index.x == 0 || vertex_index.x == PatchGridSideNum-1 ||
		vertex_index.y == 0 || vertex_index.y == PatchGridSideNum-1)
		on_edge = true;
	if (vertex_index.x == 0 && current_patch.DeltaLod_x_neg > 0)
	{
		uint multiple = 1 << current_patch.DeltaLod_x_neg;	// 旁边那一个patch是这个patch的几倍大
		uint modIndex = vertex_index.y % multiple;	
		if (modIndex != 0)	// 如果是0的话是不需要修改顶点的，因为位于更粗糙patch的顶点上
		{
			pos.y += (multiple - modIndex) * PatchOriginalGridMeterSize * scale;
		}
	}
	if (vertex_index.x == PatchGridSideNum-1 && current_patch.DeltaLod_x_pos > 0)
	{
		uint multiple = 1 << current_patch.DeltaLod_x_pos;	// 旁边那一个patch是这个patch的几倍大
		uint modIndex = vertex_index.y % multiple;	
		if (modIndex != 0)	// 如果是0的话是不需要修改顶点的，因为位于更粗糙patch的顶点上
		{
			pos.y += (multiple - modIndex) * PatchOriginalGridMeterSize * scale;
		}
	}
	if (vertex_index.y == 0 && current_patch.DeltaLod_y_neg > 0)
	{
		uint multiple = 1 << current_patch.DeltaLod_y_neg;	// 旁边那一个patch是这个patch的几倍大
		uint modIndex = vertex_index.x % multiple;	
		if (modIndex != 0)	// 如果是0的话是不需要修改顶点的，因为位于更粗糙patch的顶点上
		{
			pos.x += (multiple - modIndex) * PatchOriginalGridMeterSize * scale;
		}
	}
	if (vertex_index.y == PatchGridSideNum-1 && current_patch.DeltaLod_y_pos > 0)
	{
		uint multiple = 1 << current_patch.DeltaLod_y_pos;	// 旁边那一个patch是这个patch的几倍大
		uint modIndex = vertex_index.x % multiple;	
		if (modIndex != 0)	// 如果是0的话是不需要修改顶点的，因为位于更粗糙patch的顶点上
		{
			pos.x += (multiple - modIndex) * PatchOriginalGridMeterSize * scale;
		}
	}
	return on_edge;
}