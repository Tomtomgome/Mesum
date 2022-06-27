#pragma once

#include <MesumGraphics/RenderTask.hpp>
#include <MesumGraphics/Renderer.hpp>
#include <MesumCore/Kernel/Math.hpp>

#ifdef M_DX12_RENDERER
#include <MesumGraphics/DX12Renderer/DX12Context.hpp>
#include <MesumGraphics/DX12Renderer/RendererDX12Impl.hpp>
#endif  // M_DX12_RENDERER
#ifdef M_VULKAN_RENDERER
#include <MesumGraphics/VulkanRenderer/VulkanContext.hpp>
#endif  // M_VULKAN_RENDERER

static const int s_nbRow = 10;
static const int s_nbCol = 10;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct TaskDataFluidSimulation : public m::render::TaskData
{
    m::render::ISurface::HdlPtr m_hdlOutput{};
    std::vector<m::math::mVec4>*     m_pPixelData = nullptr;

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
mIfDx12Enabled(
    struct Dx12TaskFluidSimulation
    : public TaskFluidSimulation
    {
        explicit Dx12TaskFluidSimulation(TaskDataFluidSimulation * a_data);
        void prepare() override;
        void execute() const override;

       private:
        m::mUInt m_i = 0;

        m::dx12::ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
        m::dx12::ComPtr<ID3D12PipelineState> m_pso           = nullptr;

        std::vector<m::dx12::ComPtr<ID3D12Resource>> m_pTextureResources{};
        std::vector<m::dx12::ComPtr<ID3D12Resource>> m_pUploadResources{};

        D3D12_GPU_DESCRIPTOR_HANDLE
        m_GPUDescHdlTexture[m::dx12::DX12Surface::scm_numFrames]{};
        D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescHdlSampler{};

        m::dx12::ComPtr<ID3D12DescriptorHeap> m_pSrvHeap         = nullptr;
        m::mUInt                              m_incrementSizeSrv = 0;
        static const m::mUInt                 sm_sizeSrvHeap =
            m::dx12::DX12Surface::scm_numFrames;
        m::dx12::ComPtr<ID3D12DescriptorHeap> m_pSamplerHeap         = nullptr;
        m::mUInt                              m_incrementSizeSampler = 0;
        static const m::mUInt                 sm_sizeSamplerHeap     = 1;
    };)