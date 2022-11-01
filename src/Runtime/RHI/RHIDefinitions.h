#pragma once
#include "Platform.h"

// #define _GM nilou::GDynamicRHI
// #define DEPTH_BUFFER_BIT 0x00000100
// #define STENCIL_BUFFER_BIT 0x00000400
// #define COLOR_BUFFER_BIT 0x00004000

namespace nilou {
	
	enum { MAX_TEXCOORDS = 1, MAX_STATIC_TEXCOORDS = 1 };

	constexpr int MAX_SIMULTANEOUS_RENDERTARGETS = 8;
	
    enum class EDataAccessFlag : uint8
	{
		DA_ReadOnly,
		DA_WriteOnly,
		DA_ReadWrite
	};

	// class EBufferUsage
	// {
	// public:
	// 	static uint32 const None                    = 0;

	// 	/** The buffer will be written to once. */
	// 	static uint32 const Static                  = 1 << 0;

	// 	/** The buffer will be written to occasionally, GPU read only, CPU write only.  The data lifetime is until the next update, or the buffer is destroyed. */
	// 	static uint32 const Dynamic                 = 1 << 1;

	// 	/** The buffer's data will have a lifetime of one frame.  It MUST be written to each frame, or a new one created each frame. */
	// 	static uint32 const Volatile                = 1 << 2;

	// 	/** Allows an unordered access view to be created for the buffer. */
	// 	static uint32 const UnorderedAccess         = 1 << 3;

	// 	/** Create a byte address buffer, which is basically a structured buffer with a uint32 type. */
	// 	static uint32 const ByteAddressBuffer       = 1 << 4;

	// 	/** Buffer that the GPU will use as a source for a copy. */
	// 	static uint32 const SourceCopy              = 1 << 5;

	// 	/** Create a buffer that can be bound as a stream output target. */
	// 	static uint32 const StreamOutput            = 1 << 6;

	// 	/** Create a buffer which contains the arguments used by DispatchIndirect or DrawIndirect. */
	// 	static uint32 const DrawIndirect            = 1 << 7;

	// 	/** 
	// 	* Create a buffer that can be bound as a shader resource. 
	// 	* This is only needed for buffer types which wouldn't ordinarily be used as a shader resource, like a vertex buffer.
	// 	*/
	// 	static uint32 const ShaderResource          = 1 << 8;

	// 	/** Request that this buffer is directly CPU accessible. */
	// 	static uint32 const KeepCPUAccessible       = 1 << 9;

	// 	/** Buffer should go in fast vram (hint only). Requires BUF_Transient */
	// 	static uint32 const FastVRAM                = 1 << 10;

	// 	// /** Buffer should be allocated from transient memory. */
	// 	// Transient UE_DEPRECATED(5.0, "EBufferUsageFlags::Transient flag is no longer used.") = None,

	// 	/** Create a buffer that can be shared with an external RHI or process. */
	// 	static uint32 const Shared                  = 1 << 12;

	// 	/**
	// 	* Buffer contains opaque ray tracing acceleration structure data.
	// 	* Resources with this flag can't be bound directly to any shader stage and only can be used with ray tracing APIs.
	// 	* This flag is mutually exclusive with all other buffer flags except BUF_Static.
	// 	*/
	// 	static uint32 const AccelerationStructure   = 1 << 13;

	// 	static uint32 const VertexBuffer            = 1 << 14;
	// 	static uint32 const IndexBuffer             = 1 << 15;
	// 	static uint32 const StructuredBuffer        = 1 << 16;

	// 	/** Buffer memory is allocated independently for multiple GPUs, rather than shared via driver aliasing */
	// 	static uint32 const MultiGPUAllocate		= 1 << 17;

	// 	/**
	// 	* Tells the render graph to not bother transferring across GPUs in multi-GPU scenarios.  Useful for cases where
	// 	* a buffer is read back to the CPU (such as streaming request buffers), or written to each frame by CPU (such
	// 	* as indirect arg buffers), and the other GPU doesn't actually care about the data.
	// 	*/
	// 	static uint32 const MultiGPUGraphIgnore		= 1 << 18;
		
	// 	/** Allows buffer to be used as a scratch buffer for building ray tracing acceleration structure,
	// 	* which implies unordered access. Only changes the buffer alignment and can be combined with other flags.
	// 	**/
	// 	static uint32 const RayTracingScratch = (1 << 19) | UnorderedAccess;

	// 	// Helper bit-masks
	// 	static uint32 const AnyDynamic = (Dynamic | Volatile);
	// };

	enum EBufferUsageFlags
	{
		None                    = 0,

		/** The buffer will be written to once. */
		Static                  = 1 << 0,

		/** The buffer will be written to occasionally, GPU read only, CPU write only.  The data lifetime is until the next update, or the buffer is destroyed. */
		Dynamic                 = 1 << 1,

		/** The buffer's data will have a lifetime of one frame.  It MUST be written to each frame, or a new one created each frame. */
		Volatile                = 1 << 2,

		/** Allows an unordered access view to be created for the buffer. */
		UnorderedAccess         = 1 << 3,

		/** Create a byte address buffer, which is basically a structured buffer with a uint32 type. */
		ByteAddressBuffer       = 1 << 4,

		/** Buffer that the GPU will use as a source for a copy. */
		SourceCopy              = 1 << 5,

		/** Create a buffer that can be bound as a stream output target. */
		StreamOutput            = 1 << 6,

		/** Create a buffer which contains the arguments used by DispatchIndirect or DrawIndirect. */
		DrawIndirect            = 1 << 7,
		DispatchIndirect        = 1 << 11,
		/** 
		* Create a buffer that can be bound as a shader resource. 
		* This is only needed for buffer types which wouldn't ordinarily be used as a shader resource, like a vertex buffer.
		*/
		ShaderResource          = 1 << 8,

		/** Request that this buffer is directly CPU accessible. */
		KeepCPUAccessible       = 1 << 9,

		/** Buffer should go in fast vram (hint only). Requires BUF_Transient */
		FastVRAM                = 1 << 10,

		// /** Buffer should be allocated from transient memory. */
		// Transient UE_DEPRECATED(5.0, "EBufferUsageFlags::Transient flag is no longer used.") = None,

		/** Create a buffer that can be shared with an external RHI or process. */
		Shared                  = 1 << 12,

		/**
		* Buffer contains opaque ray tracing acceleration structure data.
		* Resources with this flag can't be bound directly to any shader stage and only can be used with ray tracing APIs.
		* This flag is mutually exclusive with all other buffer flags except BUF_Static.
		*/
		AccelerationStructure   = 1 << 13,

		VertexBuffer            = 1 << 14,
		IndexBuffer             = 1 << 15,
		StructuredBuffer        = 1 << 16,

		/** Buffer memory is allocated independently for multiple GPUs, rather than shared via driver aliasing */
		MultiGPUAllocate		= 1 << 17,

		/**
		* Tells the render graph to not bother transferring across GPUs in multi-GPU scenarios.  Useful for cases where
		* a buffer is read back to the CPU (such as streaming request buffers), or written to each frame by CPU (such
		* as indirect arg buffers), and the other GPU doesn't actually care about the data.
		*/
		MultiGPUGraphIgnore		= 1 << 18,
		
		/** Allows buffer to be used as a scratch buffer for building ray tracing acceleration structure,
		* which implies unordered access. Only changes the buffer alignment and can be combined with other flags.
		**/
		RayTracingScratch = (1 << 19) | UnorderedAccess,

		// Helper bit-masks
		AnyDynamic = (Dynamic | Volatile),

		AtomicCounter = 1 << 20,
	};
	constexpr EBufferUsageFlags operator|(EBufferUsageFlags a, EBufferUsageFlags b)
	{
		return static_cast<EBufferUsageFlags>(static_cast<int>(a) | static_cast<int>(b));
	}

	enum EUniformBufferUsage
	{
		// the uniform buffer is temporary, used for a single draw call then discarded
		UniformBuffer_SingleDraw = 0,
		// the uniform buffer is used for multiple draw calls but only for the current frame
		UniformBuffer_SingleFrame,
		// the uniform buffer is used for multiple draw calls, possibly across multiple frames
		UniformBuffer_MultiFrame,
	};
	// enum class EVertexElementType : uint8
	// {
	// 	VET_None,
	// 	VET_Float1,
	// 	VET_Float2,
	// 	VET_Float3,
	// 	VET_Float4,
	// 	VET_UByte4,
	// 	VET_UByte4N,
	// 	VET_Short2,
	// 	VET_Short4,
	// 	VET_Short2N,		// 16 bit word normalized to (value/32767.0,value/32767.0,0,0,1)
	// 	VET_Short4N,		// 4 X 16 bit word, normalized
	// 	VET_UShort2,
	// 	VET_UShort4,
	// 	VET_UShort2N,		// 16 bit word normalized to (value/65535.0,value/65535.0,0,0,1)
	// 	VET_UShort4N,		// 4 X 16 bit word unsigned, normalized  
	// 	VET_UInt
	// };

	/*
	class EVertexElementTypeFlags
	{
	public:
		static uint32 const VET_None = 0;
		static uint32 const VET_Byte = 1 << 0;
		static uint32 const VET_UByte = 1 << 1;
		static uint32 const VET_Short = 1 << 2;
		static uint32 const VET_UShort = 1 << 3;
		static uint32 const VET_Int = 1 << 4;
		static uint32 const VET_UInt = 1 << 5;
		static uint32 const VET_Float = 1 << 6;
		static uint32 const VET_1 = 1 << 16;
		static uint32 const VET_2 = 1 << 17;
		static uint32 const VET_3 = 1 << 18;
		static uint32 const VET_4 = 1 << 19;
		static uint32 const VET_9 = 1 << 20;
		static uint32 const VET_16 = 1 << 21;
	};
	*/
	enum class EVertexElementTypeFlags : uint32
	{
		VET_None = 0,
		VET_1 = 1 << 1,
		VET_2 = 1 << 2,
		VET_3 = 1 << 3,
		VET_4 = 1 << 4,
		VET_9 = 1 << 9,
		VET_16 = 1 << 16,
		VET_Byte = 1 << 17,
		VET_UByte = 1 << 18,
		VET_Short = 1 << 19,
		VET_UShort = 1 << 20,
		VET_Int = 1 << 21,
		VET_UInt = 1 << 22,
		VET_Float = 1 << 23,
	};
	constexpr EVertexElementTypeFlags operator|(EVertexElementTypeFlags a, EVertexElementTypeFlags b)
	{
		return static_cast<EVertexElementTypeFlags>(static_cast<int>(a) | static_cast<int>(b));
	}
	enum class EVertexElementType : uint32
	{
		VET_None,
		VET_Float1,
		VET_Float2,
		VET_Float3,
		VET_Float4,
		VET_Half2,			// 16 bit float using 1 bit sign, 5 bit exponent, 10 bit mantissa 
		VET_Half4,
		VET_UByte4,
		VET_UByte4N,
		VET_Short2,
		VET_Short4,
		VET_Short2N,		// 16 bit word normalized to (value/32767.0,value/32767.0,0,0,1)
		VET_Short4N,		// 4 X 16 bit word, normalized 
		VET_UShort2,
		VET_UShort4,
		VET_UShort2N,		// 16 bit word normalized to (value/65535.0,value/65535.0,0,0,1)
		VET_UShort4N,		// 4 X 16 bit word unsigned, normalized 
		VET_UInt,
		VET_MAX,
	};

	enum class EIndexType : uint32
	{
		IT_UByte,
		IT_UShort,
		IT_UInt,
	};

	// enum class ERasterizerPolyMode_Face : uint8
	// {
	// 	PMF_Front = 0,
	// 	PMF_Back,
	// 	PMF_Front_and_Back
	// };
	enum ERasterizerFillMode
	{
		FM_Point,
		FM_Wireframe,
		FM_Solid,

		ERasterizerFillMode_Num,
	};
	enum EBlendOperation
	{
		BO_Add,
		BO_Subtract,
		BO_Min,
		BO_Max,
		BO_ReverseSubtract,

		EBlendOperation_Num,
		EBlendOperation_NumBits = 3,
	};
	static_assert(EBlendOperation_Num <= (1 << EBlendOperation_NumBits), "EBlendOperation_Num will not fit on EBlendOperation_NumBits");

	enum EBlendFactor
	{
		BF_Zero,
		BF_One,
		BF_SourceColor,
		BF_InverseSourceColor,
		BF_SourceAlpha,
		BF_InverseSourceAlpha,
		BF_DestAlpha,
		BF_InverseDestAlpha,
		BF_DestColor,
		BF_InverseDestColor,
		BF_ConstantBlendFactor,
		BF_InverseConstantBlendFactor,
		BF_Source1Color,
		BF_InverseSource1Color,
		BF_Source1Alpha,
		BF_InverseSource1Alpha,

		EBlendFactor_Num,
		EBlendFactor_NumBits = 4,
	};
	static_assert(EBlendFactor_Num <= (1 << EBlendFactor_NumBits), "EBlendFactor_Num will not fit on EBlendFactor_NumBits");
	
	enum EColorWriteMask
	{
		CW_RED   = 0x01,
		CW_GREEN = 0x02,
		CW_BLUE  = 0x04,
		CW_ALPHA = 0x08,

		CW_NONE  = 0,
		CW_RGB   = CW_RED | CW_GREEN | CW_BLUE,
		CW_RGBA  = CW_RED | CW_GREEN | CW_BLUE | CW_ALPHA,
		CW_RG    = CW_RED | CW_GREEN,
		CW_BA    = CW_BLUE | CW_ALPHA,

		EColorWriteMask_NumBits = 4,
	};

	enum ERasterizerCullMode
	{
		CM_None,
		CM_CW,
		CM_CCW,

		ERasterizerCullMode_Num,
	};
	enum class ERasterizerFrontFace : uint8
	{
		FF_CCW = 0,
		FF_CW
	};
	enum class EPrimitiveMode : uint8
	{
		PM_Points = 0,
		PM_Lines,
		PM_Line_Loop,
		PM_Line_Strip,
		PM_Triangles,
		PM_Triangle_Strip,
		PM_Triangle_Fan
	};
	enum class EFramebufferAttachment : uint8
	{
		FA_Color_Attachment0,
		FA_Color_Attachment1,
		FA_Color_Attachment2,
		FA_Color_Attachment3,
		FA_Color_Attachment4,
		FA_Color_Attachment5,
		FA_Color_Attachment6,
		FA_Color_Attachment7,
		// FA_Color_Attachment8,
		// FA_Color_Attachment9,
		// FA_Color_Attachment10,
		// FA_Color_Attachment11,
		// FA_Color_Attachment12,
		// FA_Color_Attachment13,
		// FA_Color_Attachment14,
		// FA_Color_Attachment15,
		// FA_Color_Attachment16,
		// FA_Color_Attachment17,
		// FA_Color_Attachment18,
		// FA_Color_Attachment19,
		// FA_Color_Attachment20,
		// FA_Color_Attachment21,
		// FA_Color_Attachment22,
		// FA_Color_Attachment23,
		// FA_Color_Attachment24,
		// FA_Color_Attachment25,
		// FA_Color_Attachment26,
		// FA_Color_Attachment27,
		// FA_Color_Attachment28,
		// FA_Color_Attachment29,
		// FA_Color_Attachment30,
		// FA_Color_Attachment31,
		FA_Depth_Attachment,
		FA_Stencil_Attachment,
		FA_Depth_Stencil_Attachment
	};

    
	enum class EPixelFormat : uint8
	{
		PF_UNKNOWN = 0,
		PF_R8,
		PF_R8G8,
		PF_R8G8B8,
		PF_R8G8B8_sRGB,
		PF_B8G8R8,
		PF_B8G8R8_sRGB,
		PF_R8G8B8A8,
		PF_R8G8B8A8_sRGB,
		PF_B8G8R8A8,
		PF_B8G8R8A8_sRGB,

		PF_D24S8,
		PF_D32F,

		PF_DXT1,
		PF_DXT1_sRGB,
		PF_DXT5,
		PF_DXT5_sRGB,

		PF_R16F,
		PF_R16G16F,
		PF_R16G16B16F,
		PF_R16G16B16A16F,
		PF_R32F,
		PF_R32G32F,
		PF_R32G32B32F,
		PF_R32G32B32A32F
	};

	enum class ETextureFilters : uint8
	{
		TF_Linear = 0,
		TF_Nearest,
		TF_Nearest_Mipmap_Nearest,
		TF_Linear_Mipmap_Nearest,
		TF_Nearest_Mipmap_Linear,
		TF_Linear_Mipmap_Linear,
	};
	enum class ETextureWrapModes : uint8
	{
		TW_Repeat = 0,
		TW_Clamp,
		TW_Mirrored_Repeat
	};
	enum ECompareFunction
	{
		CF_Less,
		CF_LessEqual,
		CF_Greater,
		CF_GreaterEqual,
		CF_Equal,
		CF_NotEqual,
		CF_Never,
		CF_Always,
		
		ECompareFunction_Num,
	};

	enum EStencilOp
	{
		SO_Keep,
		SO_Zero,
		SO_Replace,
		SO_SaturatedIncrement,
		SO_SaturatedDecrement,
		SO_Invert,
		SO_Increment,
		SO_Decrement,

		EStencilOp_Num,
	};

	enum class ETextureType
	{
		TT_Texture2D,
		TT_Texture2DArray,
		TT_Texture3D,
		TT_TextureCube,
	};

	enum ERHIResourceType
	{
		RRT_None,

		RRT_SamplerState,
		RRT_RasterizerState,
		RRT_DepthStencilState,
		RRT_BlendState,
		RRT_VertexDeclaration,
		RRT_VertexShader,
		RRT_MeshShader,
		RRT_AmplificationShader,
		RRT_PixelShader,
		RRT_GeometryShader,
		RRT_RayTracingShader,
		RRT_ComputeShader,
		RRT_GraphicsPipelineState,
		RRT_ComputePipelineState,
		RRT_RayTracingPipelineState,
		RRT_BoundShaderState,
		RRT_Framebuffer,
		RRT_UniformBufferLayout,
		RRT_UniformBuffer,
		RRT_Buffer,
		RRT_Texture,
		RRT_Texture2D,
		RRT_Texture2DArray,
		RRT_Texture3D,
		RRT_TextureCube,
		RRT_TextureReference,
		RRT_TimestampCalibrationQuery,
		RRT_GPUFence,
		RRT_RenderQuery,
		RRT_RenderQueryPool,
		RRT_ComputeFence,
		RRT_Viewport,
		RRT_UnorderedAccessView,
		RRT_ShaderResourceView,
		RRT_RayTracingAccelerationStructure,
		RRT_StagingBuffer,
		RRT_CustomPresent,
		RRT_ShaderLibrary,
		RRT_PipelineBinaryLibrary,

		// DEPRECATED only used in opengl, will be replaced by RRT_GraphicsPipelineState or something
		RRT_LinkedProgram,	

		// DEPRECATED only used in opengl, will be replaced by RRT_VertexDeclaration or something
		RRT_VertexArrayObject,	

		RRT_Num
	};

	enum EPipelineStage
	{
		PS_Vertex,
		PS_Pixel,
		PS_Geometry,
		PS_Compute,

		PipelineStageNum
	};
}