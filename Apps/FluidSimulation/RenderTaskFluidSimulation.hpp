#pragma once

#include <MesumGraphics/RenderTask.hpp>
#include <MesumGraphics/Renderer.hpp>
#include <MesumCore/Kernel/Math.hpp>
#include <MesumCore/Kernel/Image.hpp>

#ifdef M_DX12_RENDERER
#include <MesumGraphics/DX12Renderer/DX12Context.hpp>
#include <MesumGraphics/DX12Renderer/RendererDX12Impl.hpp>
#endif  // M_DX12_RENDERER
#ifdef M_VULKAN_RENDERER
#include <MesumGraphics/VulkanRenderer/VulkanContext.hpp>
#endif  // M_VULKAN_RENDERER

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct DescriptorHeapFluidSimulation
{
    void                        init();
    D3D12_GPU_DESCRIPTOR_HANDLE create_srvAndGetHandle(
        m::dx12::ComPtr<ID3D12Resource>& a_pResource,
        D3D12_SHADER_RESOURCE_VIEW_DESC& a_descSrv);
    D3D12_GPU_DESCRIPTOR_HANDLE create_uavAndGetHandle(
        m::dx12::ComPtr<ID3D12Resource>&  a_pResource,
        D3D12_UNORDERED_ACCESS_VIEW_DESC& a_descUav);

    m::mUInt m_currentHandle = 0;

    m::dx12::ComPtr<ID3D12DescriptorHeap> m_pHeap            = nullptr;
    m::mUInt                              m_incrementSizeSrv = 0;

    static const m::mUInt scm_maxSizeDescriptorHeap = 30;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct TaskDataFluidSimulation : public m::render::TaskData
{
    struct ControlParameters
    {
        m::mBool        isRunning                      = false;
        m::mBool        displaySpeed                   = false;
        m::math::mIVec2 vectorRepresentationResolution = {80, 80};
    };

    m::render::mIRenderTarget*                pOutputRT    = nullptr;
    m::resource::mTypedImage<m::math::mVec4>* pInitialData = nullptr;

    ControlParameters* pParameters = nullptr;

    mIfDx12Enabled(m::render::Task* getNew_dx12Implementation(
        m::render::TaskData* a_data) override);
    mIfVulkanEnabled(m::render::Task* getNew_vulkanImplementation(
        m::render::TaskData* a_data) override);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct TaskFluidSimulation : public m::render::Task
{
    explicit TaskFluidSimulation(TaskDataFluidSimulation* a_data);
    void prepare() override {}

    TaskDataFluidSimulation m_taskData;
};

template <typename t_Type>
inline t_Type round_up(t_Type a_value, t_Type a_divider)
{
    return (a_value + a_divider - 1) / a_divider;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef M_DX12_RENDERER
struct Dx12TaskFluidSimulation : public TaskFluidSimulation
{
    explicit Dx12TaskFluidSimulation(TaskDataFluidSimulation* a_data);
    void prepare() override;
    void execute() const override;

   private:
    void init_samplers();
    void init_velocityTextures();
    void init_dataTextures(
        m::resource::mTypedImage<m::math::mVec4> const& a_rImage);
    void init_constantBuffer();

    void setup_velocityAdvectionPass();
    void setup_advectionPass();
    void setup_simulationPass();
    void setup_jacobiPass();
    void setup_projectionPass();
    void setup_arrowGenerationPass();
    void setup_fluidRenderingPass();
    void setup_arrowRenderingPass();

   private:
    m::mUInt m_sizeComputeGroup = 16;

    m::mUInt m_simulationWidth{};
    m::mUInt m_simulationHeight{};

    m::mUInt m_iOriginal = 0;
    m::mUInt m_iComputed = 1;

    // Velocity advection
    m::dx12::ComPtr<ID3D12PipelineState> m_psoVelocityAdvection  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoVelocityStaggering = nullptr;
    m::dx12::ComPtr<ID3D12RootSignature> m_rsVelocityAdvection   = nullptr;

    // Advection
    m::dx12::ComPtr<ID3D12RootSignature> m_rsAdvection  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoAdvection = nullptr;

    // Simulation
    m::dx12::ComPtr<ID3D12RootSignature> m_rsSimulation  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoSimulation = nullptr;

    // Solver
    m::dx12::ComPtr<ID3D12RootSignature> m_rsJacobi  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoJacobi = nullptr;

    static const m::mUInt    scm_nbJacobiTexture   = 2;
    static const m::mUInt    scm_nbJacobiIteration = 60;  // Must be pair svp
    static const DXGI_FORMAT scm_formatPressure    = DXGI_FORMAT_R32_FLOAT;
    std::vector<m::dx12::ComPtr<ID3D12Resource>> m_pTextureResourceJacobi{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescHdlJacobiInput[scm_nbJacobiTexture]{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescHdlJacobiOutput[scm_nbJacobiTexture]{};

    // Project
    m::dx12::ComPtr<ID3D12RootSignature> m_rsProject  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoProject = nullptr;

    // Arrow generation
    m::dx12::ComPtr<ID3D12RootSignature> m_rsArrowGeneration  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoArrowGeneration = nullptr;

    static const m::mUInt           scm_arrowFieldMaxResolutionX = 320;
    static const m::mUInt           scm_arrowFieldMaxResolutionY = 320;
    static const m::mUInt           sm_nbVertexPerArrows         = 4;
    static const m::mSize           sm_sizeVertexArrow  = 2 * 4 * sizeof(float);
    static const m::mUInt           sm_nbIndexPerArrows = 7;
    static const m::mSize           sm_sizeIndexArrow   = sizeof(m::mU32);
    m::dx12::ComPtr<ID3D12Resource> m_pVertexBufferArrows = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE     m_GPUDescHdlOutBuffer{};
    m::dx12::ComPtr<ID3D12Resource> m_pIndexBufferArrows = nullptr;

    // Fluid rendering
    m::dx12::ComPtr<ID3D12RootSignature> m_rsFluidRendering  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoFluidRendering = nullptr;

    // Arrow rendering
    m::dx12::ComPtr<ID3D12RootSignature> m_rsArrowRendering  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoArrowRendering = nullptr;

    // ---------- Global rendering data
    std::vector<D3D12_STATIC_SAMPLER_DESC> m_samplersDescs{};

    DescriptorHeapFluidSimulation m_descriptorHeap;

    static const m::mU64 scm_minimalStructSize     = 256;
    static const m::mUInt scm_maxSizeConstantBuffer = 2 * scm_minimalStructSize;
    m::dx12::ComPtr<ID3D12Resource> m_pConstantBuffers     = nullptr;
    void*                           m_pConstantBuffersData = nullptr;

    // ---------- Simulation data
    std::vector<m::dx12::ComPtr<ID3D12Resource>> m_pUploadResourcesToDelete{};
    // Data
    static const m::mUInt    scm_nbDataTextures = 2;
    static const DXGI_FORMAT scm_formatDataTexture =
        DXGI_FORMAT_R32G32B32A32_FLOAT;
    std::vector<m::dx12::ComPtr<ID3D12Resource>> m_pTextureResources{};
    D3D12_GPU_DESCRIPTOR_HANDLE
    m_GPUDescHdlTextureCompute[scm_nbDataTextures]{};
    D3D12_GPU_DESCRIPTOR_HANDLE
    m_GPUDescHdlTextureDisplay[scm_nbDataTextures]{};

    // Velocities
    static const m::mUInt    scm_nbVelocityTexture = 2;
    static const DXGI_FORMAT scm_formatVelocityTexture =
        DXGI_FORMAT_R32G32_FLOAT;
    std::vector<m::dx12::ComPtr<ID3D12Resource>> m_pTextureResourceVelocity{};
    D3D12_GPU_DESCRIPTOR_HANDLE
    m_GPUDescHdlVelocityInput[scm_nbVelocityTexture]{};
    D3D12_GPU_DESCRIPTOR_HANDLE
    m_GPUDescHdlVelocityOutput[scm_nbVelocityTexture]{};
};
#endif  // M_DX12_RENDERER