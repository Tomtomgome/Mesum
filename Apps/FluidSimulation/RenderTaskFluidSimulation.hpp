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

static const int s_nbRow = 300;
static const int s_nbCol = 300;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct TaskDataFluidSimulation : public m::render::TaskData
{
    struct ControlParameters
    {
        m::mBool isRunning    = false;
        m::mBool displaySpeed = false;
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
    void setup_advectionPass();
    void setup_simulationPass();
    void setup_jacobiPass(m::mInt a_width, m::mInt a_height);
    void setup_projectionPass();
    void setup_arrowGenerationPass();
    void setup_fluidRenderingPass();
    void setup_arrowRenderingPass();

   private:
    m::mUInt m_iOriginal = 0;
    m::mUInt m_iComputed = 1;

    // Advection
    m::dx12::ComPtr<ID3D12RootSignature> m_rsAdvection  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoAdvection = nullptr;

    // Simulation
    m::dx12::ComPtr<ID3D12RootSignature> m_rsSimulation  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoSimulation = nullptr;

    // Solver
    m::dx12::ComPtr<ID3D12RootSignature> m_rsJacobi  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoJacobi = nullptr;

    static const m::mUInt scm_nbJacobiTexture   = 2;
    static const m::mUInt scm_nbJacobiIteration = 60;  // Must be pair svp
    std::vector<m::dx12::ComPtr<ID3D12Resource>> m_pTextureResourceJacobi{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescHdlJacobiInput[scm_nbJacobiTexture]{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescHdlJacobiOutput[scm_nbJacobiTexture]{};

    // Project
    m::dx12::ComPtr<ID3D12RootSignature> m_rsProject  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoProject = nullptr;

    // Arrow generation
    m::dx12::ComPtr<ID3D12RootSignature> m_rsArrowGeneration  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoArrowGeneration = nullptr;

    static const m::mUInt           sm_nbVertexPerArrows = 4;
    static const m::mSize           sm_sizeVertexArrow  = 2 * 4 * sizeof(float);
    static const m::mUInt           sm_nbIndexPerArrows = 7;
    static const m::mSize           sm_sizeIndexArrow   = sizeof(m::mU16);
    m::dx12::ComPtr<ID3D12Resource> m_pVertexBufferArrows = nullptr;
    m::dx12::ComPtr<ID3D12Resource> m_pIndexBufferArrows  = nullptr;

    // Fluid rendering
    m::dx12::ComPtr<ID3D12RootSignature> m_rsFluidRendering  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoFluidRendering = nullptr;

    // Arrow rendering
    m::dx12::ComPtr<ID3D12RootSignature> m_rsArrowRendering  = nullptr;
    m::dx12::ComPtr<ID3D12PipelineState> m_psoArrowRendering = nullptr;

    std::vector<m::dx12::ComPtr<ID3D12Resource>> m_pTextureResources{};
    std::vector<m::dx12::ComPtr<ID3D12Resource>> m_pUploadResources{};

    static const m::mUInt scm_maxTextures = 2;

    D3D12_GPU_DESCRIPTOR_HANDLE
    m_GPUDescHdlTextureCompute[scm_maxTextures]{};
    D3D12_GPU_DESCRIPTOR_HANDLE
    m_GPUDescHdlTextureDisplay[scm_maxTextures]{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescHdlSampler{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescHdlOutBuffer{};

    m::dx12::ComPtr<ID3D12DescriptorHeap> m_pSrvHeap         = nullptr;
    m::mUInt                              m_incrementSizeSrv = 0;
    static const m::mUInt                 sm_sizeSrvHeap     = scm_maxTextures;

    static const m::mUInt sm_sizeHeapOutBuffer = 1;

    std::vector<D3D12_STATIC_SAMPLER_DESC> m_samplersDescs{};
};
#endif  // M_DX12_RENDERER