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

using QueryID = m::mUInt;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RAIIGPUTiming
{
   public:
    RAIIGPUTiming() = delete;
    RAIIGPUTiming(m::dx12::ComPtr<ID3D12GraphicsCommandList2>& a_commandList,
                  m::dx12::ComPtr<ID3D12QueryHeap> const&      a_queryHeap,
                  QueryID                                      a_idQuery)
        : m_commandList(a_commandList),
          m_queryHeap(a_queryHeap),
          m_idQuery(a_idQuery)
    {
        m_commandList->EndQuery(m_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP,
                                2 * m_idQuery);
    }
    ~RAIIGPUTiming()
    {
        m_commandList->EndQuery(m_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP,
                                2 * m_idQuery + 1);
    }

   private:
    m::dx12::ComPtr<ID3D12GraphicsCommandList2> m_commandList{};
    m::dx12::ComPtr<ID3D12QueryHeap> const      m_queryHeap{};
    m::mUInt                                    m_idQuery{};
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct TimerTree
{
    std::string             name{};
    m::mDouble              duration{};
    std::vector<TimerTree*> children{};
};

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
        enum DebugDisplays
        {
            none = 0,
            pressure,
            divergence,

            _count
        };

        m::mBool        isRunning                      = false;
        m::math::mIVec2 screenSize                     = {640, 640};
        m::mBool        displaySpeed                   = false;
        m::mBool        displayFluid                   = true;
        m::math::mIVec2 vectorRepresentationResolution = {80, 80};
        m::mInt         nbJacobiIterations             = {150};
        DebugDisplays   debugDisplay                   = DebugDisplays::none;
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
    std::vector<TimerTree>  m_timers{};
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
    void setup_divergencePass();
    void setup_jacobiPass();
    void setup_projectionPass();
    void setup_arrowGenerationPass();
    void setup_fluidRenderingPass();
    void setup_debugDataRenderingPass();
    void setup_arrowRenderingPass();

    QueryID get_queryID();
    void    begin_gpuTimmer(QueryID a_idQuery) const;
    void    end_gpuTimmer(QueryID a_idQuery) const;

   private:
    m::mUInt m_frameCount   = 0;
    m::mUInt m_currentFrame = 0;

    m::mUInt m_sizeComputeGroup = 16;

    m::mUInt m_simulationWidth{};
    m::mUInt m_simulationHeight{};

    m::mUInt m_iOriginal = 0;
    m::mUInt m_iComputed = 1;

    QueryID m_idFullFrameQuery{};
    QueryID m_idFullSimulationQuery{};
    QueryID m_idFullRenderingQuery{};

    // --Velocity advection
    m::dx12::ComPtr<ID3D12PipelineState> m_psoVelocityAdvection  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoVelocityStaggering = nullptr;
    m::dx12::ComPtr<ID3D12RootSignature> m_rsVelocityAdvection   = nullptr;
    QueryID                              m_idVelocityAdvectionQuery{};

    // --Advection
    m::dx12::ComPtr<ID3D12RootSignature> m_rsAdvection  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoAdvection = nullptr;
    QueryID                              m_idAdvectionQuery{};

    // --Simulation
    m::dx12::ComPtr<ID3D12RootSignature> m_rsSimulation  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoSimulation = nullptr;
    QueryID                              m_idSimulationQuery{};

    // --Solver
    // Divergecne
    m::dx12::ComPtr<ID3D12RootSignature> m_rsDivergence  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoDivergence = nullptr;
    QueryID                              m_idDivergenceQuery{};

    static const DXGI_FORMAT scm_formatDivergence = DXGI_FORMAT_R32_FLOAT;
    m::dx12::ComPtr<ID3D12Resource> m_pTextureResourceDivergence{};
    D3D12_GPU_DESCRIPTOR_HANDLE     m_GPUDescHdlDivergenceInput{};
    D3D12_GPU_DESCRIPTOR_HANDLE     m_GPUDescHdlDivergenceOutput{};

    // Jacobi
    m::dx12::ComPtr<ID3D12RootSignature> m_rsJacobi  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoJacobi = nullptr;
    QueryID                              m_idSolverQuery{};

    static const m::mUInt    scm_nbJacobiTexture = 2;
    static const DXGI_FORMAT scm_formatPressure  = DXGI_FORMAT_R32_FLOAT;
    std::vector<m::dx12::ComPtr<ID3D12Resource>> m_pTextureResourceJacobi{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescHdlJacobiInput[scm_nbJacobiTexture]{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescHdlJacobiOutput[scm_nbJacobiTexture]{};

    // --Project
    m::dx12::ComPtr<ID3D12RootSignature> m_rsProject  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoProject = nullptr;
    QueryID                              m_idProjectionQuery{};

    // --Arrow generation
    m::dx12::ComPtr<ID3D12RootSignature> m_rsArrowGeneration  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoArrowGeneration = nullptr;
    QueryID                              m_idArrowGenerationQuery{};

    static const m::mUInt           scm_arrowFieldMaxResolutionX = 320;
    static const m::mUInt           scm_arrowFieldMaxResolutionY = 320;
    static const m::mUInt           sm_nbVertexPerArrows         = 4;
    static const m::mSize           sm_sizeVertexArrow  = 2 * 4 * sizeof(float);
    static const m::mUInt           sm_nbIndexPerArrows = 7;
    static const m::mSize           sm_sizeIndexArrow   = sizeof(m::mU32);
    m::dx12::ComPtr<ID3D12Resource> m_pVertexBufferArrows = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE     m_GPUDescHdlOutBuffer{};
    m::dx12::ComPtr<ID3D12Resource> m_pIndexBufferArrows = nullptr;

    // --Fluid rendering
    m::dx12::ComPtr<ID3D12RootSignature> m_rsFluidRendering  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoFluidRendering = nullptr;
    QueryID                              m_idFluidRenderingQuery{};
    m::dx12::ComPtr<ID3D12PipelineState> m_psoDataRendering = nullptr;

    // --Arrow rendering
    m::dx12::ComPtr<ID3D12RootSignature> m_rsArrowRendering  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoArrowRendering = nullptr;
    QueryID                              m_idArrowRenderingQuery{};

    // ---------- Global rendering data
    static const m::mUInt msc_numFrames = 2;
    // Queries
    static const m::mUInt            msc_maxQueries   = 50;
    static constexpr m::mDouble      scm_ratioAverage = 0.0625;
    m::mDouble                       m_ratioGPUTimestampToMs{};
    m::dx12::ComPtr<ID3D12QueryHeap> m_heapQuery = nullptr;
    m::dx12::ComPtr<ID3D12Resource>  m_pQueryResultsBuffers[msc_numFrames]{};
    void*                            m_pQueryResultData{};
    QueryID                          m_idCurrentQuery{0};
    std::vector<TimerTree>           m_currentTimers{};

    // Samplers
    std::vector<D3D12_STATIC_SAMPLER_DESC> m_samplersDescs{};

    // Descriptor heap
    DescriptorHeapFluidSimulation m_descriptorHeap;

    // Constant buffer
    static const m::mU64  scm_minimalStructSize     = 256;
    static const m::mUInt scm_maxSizeConstantBuffer = 2 * scm_minimalStructSize;
    m::dx12::ComPtr<ID3D12Resource> m_pConstantBuffers[msc_numFrames]{};
    void*                           m_pConstantBuffersData[msc_numFrames]{};

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