#ifndef M_RenderTask3DRender
#define M_RenderTask3DRender
#pragma once

#include <MesumGraphics/RenderTask.hpp>
#include <MesumGraphics/Renderer.hpp>

#include <RenderTasks/RenderTask2DRender.hpp>  // temp
#include <RenderTasks/BasicVertex.hpp>

#include <MesumCore/Kernel/MathTypes.hpp>

#ifdef M_DX12_RENDERER
#include <MesumGraphics/DX12Renderer/DX12Context.hpp>
#endif  // M_DX12_RENDERER

namespace m::render
{
using uploadBuffers3D =
    Dx12BufferBase<mBasicVertex, mU16>[dx12::DX12Surface::scm_numFrames];

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct TaskData3dRender : public TaskData
{
    DataMeshBuffer<mBasicVertex, mU16>* m_pMeshBuffer;
    mIRenderTarget*                     pOutputRT = nullptr;
    math::mMat4x4*                      m_matrix;

    mIfDx12Enabled(Task* getNew_dx12Implementation(TaskData* a_data) override);
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct Task3dRender : public Task
{
    explicit Task3dRender(TaskData3dRender* a_data);

    TaskData3dRender m_taskData;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
mIfDx12Enabled(struct Dx12Task3dRender
               : public Task3dRender
               {
                   explicit Dx12Task3dRender(TaskData3dRender * a_data);

                   void prepare() override;

                   void execute() const override;

                  private:
                   mUInt           m_i = 0;
                   uploadBuffers3D m_buffers;

                   dx12::ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
                   dx12::ComPtr<ID3D12PipelineState> m_pso           = nullptr;
               };)

}  // namespace m::render

#endif  // M_RenderTask3DRender