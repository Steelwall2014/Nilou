#pragma once
#include "Platform.h"
#include "Common/EnumClassFlags.h"
#include "PixelFormat.h"

namespace nilou {
	
	enum { MAX_TEXCOORDS = 1, MAX_STATIC_TEXCOORDS = 1 };

	constexpr int MAX_SIMULTANEOUS_RENDERTARGETS = 8;
	
	constexpr int MAX_VERTEX_ELEMENTS = 17;
	
    enum class EDataAccessFlag : uint8
	{
		DA_ReadOnly,
		DA_WriteOnly,
		DA_ReadWrite
	};

	enum EResourceLockMode
	{
		RLM_ReadOnly,
		RLM_WriteOnly,
		// RLM_WriteOnly_NoOverwrite,
		RLM_Num
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

	enum class EBufferUsageFlags
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

		/** The buffer is a placeholder for streaming, and does not contain an underlying GPU resource. */
		NullResource = 1 << 20,

		/** Buffer can be used as uniform buffer on platforms that do support uniform buffer objects. */
		UniformBuffer = 1 << 21,

		/**
		* EXPERIMENTAL: Allow the buffer to be created as a reserved (AKA tiled/sparse/virtual) resource internally, without physical memory backing.
		* May not be used with Dynamic and other buffer flags that prevent the resource from being allocated in local GPU memory.
		*/
		ReservedResource = 1 << 22,

		// Helper bit-masks
		AnyDynamic = (Dynamic | Volatile),
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
	enum EVertexElementType : uint8
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
	
	enum ESamplerFilter
	{
		SF_Point,
		SF_Bilinear,
		SF_Trilinear,
		SF_AnisotropicPoint,
		SF_AnisotropicLinear,

		ESamplerFilter_Num,
		ESamplerFilter_NumBits = 3,
	};
	static_assert(ESamplerFilter_Num <= (1 << ESamplerFilter_NumBits), "ESamplerFilter_Num will not fit on ESamplerFilter_NumBits");

	enum ESamplerAddressMode
	{
		AM_Wrap,
		AM_Clamp,
		AM_Mirror,
		/** Not supported on all platforms */
		AM_Border,

		ESamplerAddressMode_Num,
		ESamplerAddressMode_NumBits = 2,
	};
	static_assert(ESamplerAddressMode_Num <= (1 << ESamplerAddressMode_NumBits), "ESamplerAddressMode_Num will not fit on ESamplerAddressMode_NumBits");

	enum ESamplerCompareFunction
	{
		SCF_Never,
		SCF_Less
	};
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
		PM_PointList = 0,
		PM_LineList,
		PM_TriangleList,
		PM_TriangleStrip,
	};
	enum EFramebufferAttachment : uint8
	{
		FA_Color_Attachment0 = 0,
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
		//FA_Depth_Attachment,
		//FA_Stencil_Attachment,
		FA_Depth_Stencil_Attachment
	};

	enum ETextureFilters : uint8
	{
		TF_Linear = 0,
		TF_Nearest,
		TF_Nearest_Mipmap_Nearest,
		TF_Linear_Mipmap_Nearest,
		TF_Nearest_Mipmap_Linear,
		TF_Linear_Mipmap_Linear,
	};
	enum ETextureWrapModes : uint8
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

	enum class ETextureDimension
	{
		Texture2D = 0,
		Texture2DArray,
		Texture3D,
		TextureCube,

		TextureDimensionsNum,
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
		RRT_PipelineLayout,
		RRT_PipelineState,
		RRT_DescriptorSetLayout,
		RRT_DescriptorSet,
		RRT_DescriptorPool,
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
		RRT_TextureView,
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

		RRT_Num
	};

	enum EPipelineStage
	{
		PS_Vertex = 0,
		PS_Pixel,
		// PS_Geometry,
		PS_Compute,

		PipelineStageNum
	};

	enum class ETextureCreateFlags : uint64
	{
		None                              = 0,

		// Texture can be used as a render target
		RenderTargetable                  = 1ull << 0,
		// Texture can be used as a resolve target
		ResolveTargetable                 = 1ull << 1,
		// Texture can be used as a depth-stencil target.
		DepthStencilTargetable            = 1ull << 2,
		// Texture can be used as a shader resource.
		ShaderResource                    = 1ull << 3,
		// Texture is encoded in sRGB gamma space
		SRGB                              = 1ull << 4,
		// Texture data is writable by the CPU
		CPUWritable                       = 1ull << 5,
		// Texture will be created with an un-tiled format
		NoTiling                          = 1ull << 6,
		// Texture will be used for video decode
		VideoDecode                       = 1ull << 7,
		// Texture that may be updated every frame
		Dynamic                           = 1ull << 8,
		// Texture will be used as a render pass attachment that will be read from
		InputAttachmentRead               = 1ull << 9,
		/** Texture represents a foveation attachment */
		Foveation                         = 1ull << 10,
		// Prefer 3D internal surface tiling mode for volume textures when possible
		Tiling3D                          = 1ull << 11,
		// This texture has no GPU or CPU backing. It only exists in tile memory on TBDR GPUs (i.e., mobile).
		Memoryless                        = 1ull << 12,
		// Create the texture with the flag that allows mip generation later, only applicable to D3D11
		GenerateMipCapable                = 1ull << 13,
		// The texture can be partially allocated in fastvram
		FastVRAMPartialAlloc              = 1ull << 14,
		// Do not create associated shader resource view, only applicable to D3D11 and D3D12
		DisableSRVCreation                = 1ull << 15,
		// Do not allow Delta Color Compression (DCC) to be used with this texture
		DisableDCC                        = 1ull << 16,
		// UnorderedAccessView (DX11 only)
		// Warning: Causes additional synchronization between draw calls when using a render target allocated with this flag, use sparingly
		// See: GCNPerformanceTweets.pdf Tip 37
		UAV                               = 1ull << 17,
		// Render target texture that will be displayed on screen (back buffer)
		Presentable                       = 1ull << 18,
		// Texture data is accessible by the CPU
		CPUReadback                       = 1ull << 19,
		// Texture was processed offline (via a texture conversion process for the current platform)
		OfflineProcessed                  = 1ull << 20,
		// Texture needs to go in fast VRAM if available (HINT only)
		FastVRAM                          = 1ull << 21,
		// by default the texture is not showing up in the list - this is to reduce clutter, using the FULL option this can be ignored
		HideInVisualizeTexture            = 1ull << 22,
		// Texture should be created in virtual memory, with no physical memory allocation made
		// You must make further calls to RHIVirtualTextureSetFirstMipInMemory to allocate physical memory
		// and RHIVirtualTextureSetFirstMipVisible to map the first mip visible to the GPU
		Virtual                           = 1ull << 23,
		// Creates a RenderTargetView for each array slice of the texture
		// Warning: if this was specified when the resource was created, you can't use SV_RenderTargetArrayIndex to route to other slices!
		TargetArraySlicesIndependently    = 1ull << 24,
		// Texture that may be shared with DX9 or other devices
		Shared                            = 1ull << 25,
		// RenderTarget will not use full-texture fast clear functionality.
		NoFastClear                       = 1ull << 26,
		// Texture is a depth stencil resolve target
		DepthStencilResolveTarget         = 1ull << 27,
		// Flag used to indicted this texture is a streamable 2D texture, and should be counted towards the texture streaming pool budget.
		Streamable                        = 1ull << 28,
		// Render target will not FinalizeFastClear; Caches and meta data will be flushed, but clearing will be skipped (avoids potentially trashing metadata)
		NoFastClearFinalize               = 1ull << 29,
		/** Texture needs to support atomic operations */
		Atomic64Compatible                = 1ull << 30,
		// Workaround for 128^3 volume textures getting bloated 4x due to tiling mode on some platforms.
		ReduceMemoryWithTilingMode        = 1ull << 31,
		/** Texture needs to support atomic operations */
		AtomicCompatible                  = 1ull << 33,
		/** Texture should be allocated for external access. Vulkan only */
		External                		  = 1ull << 34,
		/** Don't automatically transfer across GPUs in multi-GPU scenarios.  For example, if you are transferring it yourself manually. */
		MultiGPUGraphIgnore				  = 1ull << 35,

	};
	ENUM_CLASS_FLAGS(ETextureCreateFlags);

// Compatibility defines
#define TexCreate_None                           ETextureCreateFlags::None
#define TexCreate_RenderTargetable               ETextureCreateFlags::RenderTargetable
#define TexCreate_ResolveTargetable              ETextureCreateFlags::ResolveTargetable
#define TexCreate_DepthStencilTargetable         ETextureCreateFlags::DepthStencilTargetable
#define TexCreate_ShaderResource                 ETextureCreateFlags::ShaderResource
#define TexCreate_SRGB                           ETextureCreateFlags::SRGB
#define TexCreate_CPUWritable                    ETextureCreateFlags::CPUWritable
#define TexCreate_NoTiling                       ETextureCreateFlags::NoTiling
#define TexCreate_VideoDecode                    ETextureCreateFlags::VideoDecode
#define TexCreate_Dynamic                        ETextureCreateFlags::Dynamic
#define TexCreate_InputAttachmentRead            ETextureCreateFlags::InputAttachmentRead
#define TexCreate_Foveation                      ETextureCreateFlags::Foveation
#define TexCreate_3DTiling                       ETextureCreateFlags::Tiling3D
#define TexCreate_Memoryless                     ETextureCreateFlags::Memoryless
#define TexCreate_GenerateMipCapable             ETextureCreateFlags::GenerateMipCapable
#define TexCreate_FastVRAMPartialAlloc           ETextureCreateFlags::FastVRAMPartialAlloc
#define TexCreate_DisableSRVCreation             ETextureCreateFlags::DisableSRVCreation
#define TexCreate_DisableDCC                     ETextureCreateFlags::DisableDCC
#define TexCreate_UAV                            ETextureCreateFlags::UAV
#define TexCreate_Presentable                    ETextureCreateFlags::Presentable
#define TexCreate_CPUReadback                    ETextureCreateFlags::CPUReadback
#define TexCreate_OfflineProcessed               ETextureCreateFlags::OfflineProcessed
#define TexCreate_FastVRAM                       ETextureCreateFlags::FastVRAM
#define TexCreate_HideInVisualizeTexture         ETextureCreateFlags::HideInVisualizeTexture
#define TexCreate_Virtual                        ETextureCreateFlags::Virtual
#define TexCreate_TargetArraySlicesIndependently ETextureCreateFlags::TargetArraySlicesIndependently
#define TexCreate_Shared                         ETextureCreateFlags::Shared
#define TexCreate_NoFastClear                    ETextureCreateFlags::NoFastClear
#define TexCreate_DepthStencilResolveTarget      ETextureCreateFlags::DepthStencilResolveTarget
#define TexCreate_Streamable                     ETextureCreateFlags::Streamable
#define TexCreate_NoFastClearFinalize            ETextureCreateFlags::NoFastClearFinalize
#define TexCreate_ReduceMemoryWithTilingMode     ETextureCreateFlags::ReduceMemoryWithTilingMode
#define TexCreate_Transient                      ETextureCreateFlags::Transient
#define TexCreate_AtomicCompatible               ETextureCreateFlags::AtomicCompatible
#define TexCreate_External               		 ETextureCreateFlags::External
#define TexCreate_MultiGPUGraphIgnore            ETextureCreateFlags::MultiGPUGraphIgnore

// Keep the same with VkAccessFlagBits
enum class ERHIAccess : uint32
{
	None = 0,
	IndirectCommandRead = 0x00000001,
	IndexRead = 0x00000002,
	VertexAttributeRead = 0x00000004,
	UniformRead = 0x00000008,
	// InputAttachmentRead = 0x00000010,	// Used with subpass, not supported currently
	ShaderResourceRead = 0x00000020,
	ShaderResourceWrite = 0x00000040,
	ColorAttachmentRead = 0x00000080,
	ColorAttachmentWrite = 0x00000100,
	DepthStencilAttachmentRead = 0x00000200,
	DepthStencilAttachmentWrite = 0x00000400,
	TransferRead = 0x00000800,
	TransferWrite = 0x00001000,
	HostRead = 0x00002000,
	HostWrite = 0x00004000,
	Max = 0x7FFFFFFF,
};
#if 0	// Steelwall2014: ERHIAccess is too complicated in Unreal Engine, so simplify it...
enum class ERHIAccess : uint32
{
	// Used when the previous state of a resource is not known,
	// which implies we have to flush all GPU caches etc.
	Unknown = 0,

	// Read states
	CPURead             	= 1 <<  0,
	Present             	= 1 <<  1,
	IndirectArgs        	= 1 <<  2,
	VertexOrIndexBuffer 	= 1 <<  3,
	SRVCompute          	= 1 <<  4,
	SRVGraphics         	= 1 <<  5,
	CopySrc             	= 1 <<  6,
	ResolveSrc          	= 1 <<  7,
	DSVRead					= 1 <<  8,

	// Read-write states
	UAVCompute          	= 1 <<  9,
	UAVGraphics         	= 1 << 10,
	RTV                 	= 1 << 11,
	CopyDest            	= 1 << 12,
	ResolveDst          	= 1 << 13,
	DSVWrite            	= 1 << 14,

	// Ray tracing acceleration structure states.
	// Buffer that contains an AS must always be in either of these states.
	// BVHRead -- required for AS inputs to build/update/copy/trace commands.
	// BVHWrite -- required for AS outputs of build/update/copy commands.
	BVHRead                  = 1 << 15,
	BVHWrite                 = 1 << 16,

	// Invalid released state (transient resources)
	Discard					= 1 << 17,

	// Shading Rate Source
	ShadingRateSource	= 1 << 18,

	Last = ShadingRateSource,
	None = Unknown,
	Mask = (Last << 1) - 1,

	// A mask of the two possible SRV states
	SRVMask = SRVCompute | SRVGraphics,

	// A mask of the two possible UAV states
	UAVMask = UAVCompute | UAVGraphics,

	// A mask of all bits representing read-only states which cannot be combined with other write states.
	ReadOnlyExclusiveMask = CPURead | Present | IndirectArgs | VertexOrIndexBuffer | SRVGraphics | SRVCompute | CopySrc | ResolveSrc | BVHRead | ShadingRateSource,

	// A mask of all bits representing read-only states on the compute pipe which cannot be combined with other write states.
	ReadOnlyExclusiveComputeMask = CPURead | IndirectArgs | SRVCompute | CopySrc | BVHRead,

	// A mask of all bits representing read-only states which may be combined with other write states.
	ReadOnlyMask = ReadOnlyExclusiveMask | DSVRead | ShadingRateSource,

	// A mask of all bits representing readable states which may also include writable states.
	ReadableMask = ReadOnlyMask | UAVMask,

	// A mask of all bits representing write-only states which cannot be combined with other read states.
	WriteOnlyExclusiveMask = RTV | CopyDest | ResolveDst,

	// A mask of all bits representing write-only states which may be combined with other read states.
	WriteOnlyMask = WriteOnlyExclusiveMask | DSVWrite,

	// A mask of all bits representing writable states which may also include readable states.
	WritableMask = WriteOnlyMask | UAVMask | BVHWrite
};
#endif
ENUM_CLASS_FLAGS(ERHIAccess)

enum class ERHIPipeline : uint8
{
	Graphics = 0,
	AsyncCompute = 1,
	Copy = 2,

	Num = 3,
};

inline constexpr uint32 GetRHIPipelineCount()
{
	return uint32(ERHIPipeline::Num);
}

// Keep the same with VkPipelineStageFlags
enum class EPipelineStageFlags : uint32
{
	None = 0,
	TopOfPipe = 0x00000001,
	DrawIndirect = 0x00000002,
	VertexInput = 0x00000004,
	VertexShader = 0x00000008,
	TessellationControlShader = 0x00000010,
	TessellationEvaluationShader = 0x00000020,
	GeometryShader = 0x00000040,
	FragmentShader = 0x00000080,
	EarlyFragmentTests = 0x00000100,
	LateFragmentTests = 0x00000200,
	ColorAttachmentOutput = 0x00000400,
	ComputeShader = 0x00000800,
	Transfer = 0x00001000,
	BottomOfPipe = 0x00002000,
	Host = 0x00004000,
	AllGraphics = 0x00008000,
	AllCommands = 0x00010000,
	Max = 0x7FFFFFFF,
};
ENUM_CLASS_FLAGS(EPipelineStageFlags)

// Keep the same with VkImageLayout
enum class ETextureLayout : uint32
{
	Undefined = 0,
	General = 1,
	ColorAttachmentOptimal = 2,
	DepthStencilAttachmentOptimal = 3,
	DepthStencilReadOnlyOptimal = 4,
	ShaderReadOnlyOptimal = 5,
	TransferSrcOptimal = 6,
	TransferDstOptimal = 7,
	Preinitialized = 8,
	Max = 0x7FFFFFFF,
};

}