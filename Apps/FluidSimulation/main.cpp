#include <MesumCore/Kernel/Kernel.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
#include <MesumGraphics/ApiAbstraction.hpp>
#include <Kernel/Profile.hpp>

#include "RenderTaskFluidSimulation.hpp"
#include "RendererUtils.hpp"
#include "RenderTasksBasicSwapchain.hpp"

#include <iomanip>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

using namespace m;

static const mDouble s_cellSize = 1.0f;
static const mDouble s_density  = 1.0f;

static const mInt    s_maxIteration       = 100;
static const mDouble s_solutionTolerance  = 0.00000000000001;
static const mDouble s_micTunningConstant = 0.97f;

static const mDouble s_gravity = -9.8;
static const mDouble s_wind    = -2.5;

static const mInt g_gridSize = s_nbRow * s_nbCol;
using GridVector             = math::mVec<mDouble, g_gridSize>;
template <typename t_Type>
using GridVectorSP = math::mVec<t_Type, g_gridSize>;

mBool check_visibility(math::mIVec2 a_target, math::mIVec2 a_pos,
                       mDouble a_viewDistance)
{
    a_target -= a_pos;
    return sqlength(a_target) < a_viewDistance * a_viewDistance;
}

struct EntryOfA
{
    mDouble Adiag;
    mDouble Aplusi;
    mDouble Aplusj;
};

struct Particle
{
    mDouble uv;
    mDouble uh;
    mDouble T;
};

Particle interpolate(Particle const& a_a, Particle const& a_b,
                     mDouble const a_alpha)
{
    Particle output;
    output.uv = (1.0f - a_alpha) * a_a.uv + a_b.uv * a_alpha;
    output.uh = (1.0f - a_alpha) * a_a.uh + a_b.uh * a_alpha;
    output.T  = (1.0f - a_alpha) * a_a.T + a_b.T * a_alpha;

    return output;
}

mDouble saturate(mDouble const a_input, mDouble const a_min = 0.0f,
                 mDouble const a_max = 1.0f)
{
    return std::min(a_max, std::max(a_min, a_input));
}

mInt convert_toIndex(mInt a_col, mInt a_row)
{
    return a_row * s_nbCol + a_col;
}

mInt convert_toIndex(math::mIVec2 a_position)
{
    return convert_toIndex(a_position.x, a_position.y);
}

struct Universe
{
    Particle Q[s_nbRow * s_nbCol];
};

void init_universe(Universe& a_input)
{
    for (mInt row = 0; row < s_nbRow; ++row)
    {
        for (mInt col = 0; col < s_nbCol; ++col)
        {
            mInt index          = row * s_nbCol + col;
            a_input.Q[index].T  = 0.0f;
            a_input.Q[index].uv = 0.0f;
            a_input.Q[index].uh = 0.0f;

            /*if (row != s_nbRow - 1 && row != 0)
            {
                a_input.Q[index].uv = 8.0f;
            }
            else
            {
                a_input.Q[index].uv = {};
            }

            if (col != s_nbCol - 1 && col != 0)
            {
                a_input.Q[index].uh = 3.0f;
            }
            else
            {
                a_input.Q[index].uh = {};
            }*/
        }
    }

    a_input.Q[convert_toIndex(s_nbCol / 2, s_nbRow / 2)].T = 100;
}

math::mDVec2 get_speedAt(Universe const& a_input, math::mIVec2 a_position)
{
    mDouble tSpeed = (a_position.y == s_nbRow - 1)
                         ? 0.0f
                         : a_input.Q[convert_toIndex(a_position)].uv;
    mDouble rSpeed = (a_position.x == s_nbCol - 1)
                         ? 0.0f
                         : a_input.Q[convert_toIndex(a_position)].uh;
    mDouble lSpeed =
        (a_position.x == 0)
            ? 0.0f
            : a_input.Q[convert_toIndex(a_position.x - 1, a_position.y)].uh;
    mDouble bSpeed =
        (a_position.y == 0)
            ? 0.0f
            : a_input.Q[convert_toIndex(a_position.x, a_position.y - 1)].uv;

    return {(rSpeed + lSpeed) / 2.0, (bSpeed + tSpeed) / 2.0};
}

Particle get_valueAt(Universe const& a_input, math::mDVec2 a_position)
{
    a_position.x = std::min(s_nbCol - 1.0, std::max(0.0, a_position.x));
    a_position.y = std::min(s_nbRow - 1.0, std::max(0.0, a_position.y));
    math::mIVec2 a_flooredPosition = {mInt(std::floor(a_position.x)),
                                      mInt(std::floor(a_position.y))};
    a_flooredPosition.x =
        std::min(s_nbCol - 1, std::max(0, a_flooredPosition.x));
    a_flooredPosition.y =
        std::min(s_nbRow - 1, std::max(0, a_flooredPosition.y));

    Particle tl =
        a_input
            .Q[convert_toIndex(a_flooredPosition.x, a_flooredPosition.y + 1)];
    Particle tr = a_input.Q[convert_toIndex(a_flooredPosition.x + 1,
                                            a_flooredPosition.y + 1)];
    Particle br =
        a_input
            .Q[convert_toIndex(a_flooredPosition.x + 1, a_flooredPosition.y)];
    Particle bl =
        a_input.Q[convert_toIndex(a_flooredPosition.x, a_flooredPosition.y)];

    Particle top =
        interpolate(tl, tr, saturate(a_position.x - a_flooredPosition.x));
    Particle bottom =
        interpolate(bl, br, saturate(a_position.x - a_flooredPosition.x));

    return interpolate(bottom, top, a_position.y - a_flooredPosition.y);
}

void compute_divergence(Universe const& a_input, GridVector& a_outDivergences,
                        mFloat a_deltaTime)
{
    mDouble globalFactor =
        1.0;  //(s_density * s_cellSize * s_cellSize) / a_deltaTime;
    for (mInt j = 0; j < s_nbRow; ++j)
    {
        for (mInt i = 0; i < s_nbCol; ++i)
        {
            mDouble viplus12 =
                (j == s_nbRow - 1) ? 0.0f : a_input.Q[convert_toIndex(i, j)].uv;
            mDouble uiplus12 =
                (i == s_nbCol - 1) ? 0.0f : a_input.Q[convert_toIndex(i, j)].uh;
            mDouble uiminus12 =
                (i == 0) ? 0.0f : a_input.Q[convert_toIndex(i - 1, j)].uh;
            mDouble viminus12 =
                (j == 0) ? 0.0f : a_input.Q[convert_toIndex(i, j - 1)].uv;

            mInt index = convert_toIndex(i, j);

            a_outDivergences[index] =
                globalFactor * ((uiplus12 - uiminus12) / s_cellSize +
                                (viplus12 - viminus12) / s_cellSize);
        }
    }
}

bool is_outOfBoundI(mInt a_i)
{
    return (a_i == s_nbCol) || (a_i == -1);
}

bool is_outOfBoundJ(mInt a_j)
{
    return (a_j == s_nbRow) || (a_j == -1);
}

bool is_outOfBound(mInt a_i, mInt a_j)
{
    return is_outOfBoundI(a_i) || is_outOfBoundJ(a_j);
}

bool is_solid(Universe const& a_input, mInt a_i, mInt a_j)
{
    return is_outOfBoundJ(a_j);  // is_outOfBound(a_i, a_j);
}

mInt get_numberOfSolidNeighbors(Universe const& a_input, mInt a_i, mInt a_j)
{
    mInt nbSolid = 0;
    nbSolid += is_solid(a_input, a_i + 1, a_j);
    nbSolid += is_solid(a_input, a_i - 1, a_j);
    nbSolid += is_solid(a_input, a_i, a_j + 1);
    nbSolid += is_solid(a_input, a_i, a_j - 1);
    return nbSolid;
}

void compute_entriesOfA(Universe const& a_input, mDouble a_deltaTime,
                        GridVectorSP<EntryOfA>& a_A)
{
    mDouble globalFactor = a_deltaTime / (s_density * s_cellSize * s_cellSize);
    for (mInt j = 0; j < s_nbRow; ++j)
    {
        for (mInt i = 0; i < s_nbCol; ++i)
        {
            mInt index = convert_toIndex(i, j);
            a_A[index].Adiag =
                globalFactor * (4 - get_numberOfSolidNeighbors(a_input, i, j));
            a_A[index].Aplusi =
                is_solid(a_input, i + 1, j) ? 0.0f : -globalFactor;
            a_A[index].Aplusj =
                is_solid(a_input, i, j + 1) ? 0.0f : -globalFactor;
        }
    }
}

void compute_MIC0Entries(GridVectorSP<EntryOfA> const& a_A, GridVector& MIC0)
{
    for (mInt row = 0; row < s_nbRow; ++row)
    {
        for (mInt col = 0; col < s_nbCol; ++col)
        {
            mInt  index               = convert_toIndex(col, row);
            mBool isOutOfBoundIMinus1 = is_outOfBoundI(col - 1);
            mBool isOutOfBoundJinus1  = is_outOfBoundJ(row - 1);

            mDouble ici;
            mDouble icj;
            mDouble mici;
            mDouble micj;
            if (isOutOfBoundIMinus1)
            {
                ici  = 0.0f;
                mici = 0.0f;
            }
            else
            {
                mInt indexMinusI = convert_toIndex(col - 1, row);
                ici              = a_A[indexMinusI].Aplusi * MIC0[indexMinusI];
                mici = a_A[indexMinusI].Aplusi * (a_A[indexMinusI].Aplusj) *
                       MIC0[indexMinusI] * MIC0[indexMinusI];
            }
            if (isOutOfBoundJinus1)
            {
                icj  = 0.0f;
                micj = 0.0f;
            }
            else
            {
                mInt indexMinusJ = convert_toIndex(col, row - 1);
                icj              = a_A[indexMinusJ].Aplusj * MIC0[indexMinusJ];
                micj = a_A[indexMinusJ].Aplusj * (a_A[indexMinusJ].Aplusi) *
                       MIC0[indexMinusJ] * MIC0[indexMinusJ];
            }

            mDouble e = a_A[index].Adiag - (ici * ici) - (icj * icj) -
                        s_micTunningConstant * (mici + micj);

            mAssert(e >= 0);
            MIC0[index] = 1 / std::sqrt(std::max(e, 0.000000000000000000001));
        }
    }
}

void apply_preconditionner(GridVector const&             a_vecPreconditionner,
                           GridVectorSP<EntryOfA> const& a_A,
                           GridVector const&             a_vecInput,
                           GridVector&                   a_vecOutput)
{
    GridVector a_tmpOutput;
    for (mInt j = 0; j < s_nbRow; ++j)
    {
        for (mInt i = 0; i < s_nbCol; ++i)
        {
            mInt index       = convert_toIndex(i, j);
            mInt indexMinusI = convert_toIndex(i - 1, j);
            mInt indexMinusJ = convert_toIndex(i, j - 1);

            mDouble fi;
            mDouble fj;
            if (is_outOfBoundI(i - 1))
            {
                fi = 0;
            }
            else
            {
                fi = a_A[indexMinusI].Aplusi *
                     a_vecPreconditionner[indexMinusI] *
                     a_tmpOutput[indexMinusI];
            }

            if (is_outOfBoundJ(j - 1))
            {
                fj = 0;
            }
            else
            {
                fj = a_A[indexMinusJ].Aplusj *
                     a_vecPreconditionner[indexMinusJ] *
                     a_tmpOutput[indexMinusJ];
            }

            mDouble tmp        = a_vecInput[index] - fi - fj;
            a_tmpOutput[index] = tmp * a_vecPreconditionner[index];
        }
    }

    for (mInt j = s_nbRow - 1; j >= 0; --j)
    {
        for (mInt i = s_nbCol - 1; i >= 0; --i)
        {
            mInt index      = convert_toIndex(i, j);
            mInt indexPlusI = convert_toIndex(i + 1, j);
            mInt indexPlusJ = convert_toIndex(i, j + 1);

            mDouble fi;
            mDouble fj;
            if (is_outOfBoundI(i + 1))
            {
                fi = 0;
            }
            else
            {
                fi = a_A[index].Aplusi * a_vecPreconditionner[index] *
                     a_vecOutput[indexPlusI];
            }

            if (is_outOfBoundJ(j + 1))
            {
                fj = 0;
            }
            else
            {
                fj = a_A[index].Aplusj * a_vecPreconditionner[index] *
                     a_vecOutput[indexPlusJ];
            }

            mDouble tmp        = a_tmpOutput[index] - fi - fj;
            a_vecOutput[index] = tmp * a_vecPreconditionner[index];
        }
    }
}

void apply_A(GridVectorSP<EntryOfA> const& a_A, GridVector const& a_vecInput,
             GridVector& a_vecOutput)
{
    for (mInt j = 0; j < s_nbRow; ++j)
    {
        for (mInt i = 0; i < s_nbCol; ++i)
        {
            mInt index       = convert_toIndex(i, j);
            mInt indexPlusI  = convert_toIndex(i + 1, j);
            mInt indexMinusI = convert_toIndex(i - 1, j);
            mInt indexPlusJ  = convert_toIndex(i, j + 1);
            mInt indexMinusJ = convert_toIndex(i, j - 1);

            mDouble fPlusI  = 0.0f;
            mDouble fMinusI = 0.0f;
            mDouble fPlusJ  = 0.0f;
            mDouble fMinusJ = 0.0f;
            if (!is_outOfBoundI(i + 1))
            {
                fPlusI = a_vecInput[indexPlusI] * a_A[index].Aplusi;
            }
            if (!is_outOfBoundI(i - 1))
            {
                fMinusI = a_vecInput[indexMinusI] * a_A[indexMinusI].Aplusi;
            }
            if (!is_outOfBoundJ(j + 1))
            {
                fPlusJ = a_vecInput[indexPlusJ] * a_A[index].Aplusj;
            }
            if (!is_outOfBoundJ(j - 1))
            {
                fMinusJ = a_vecInput[indexMinusJ] * a_A[indexMinusJ].Aplusj;
            }
            a_vecOutput[index] = a_vecInput[index] * a_A[index].Adiag + fPlusI +
                                 fPlusJ + fMinusI + fMinusJ;
        }
    }
}

mDouble get_maxValue(GridVector& a_vector)
{
    mDouble maxValue = 0;
    for (mInt i = 0; i < g_gridSize; ++i)
    {
        if (a_vector[i] > maxValue)
        {
            maxValue = a_vector[i];
        }
    }
    return maxValue;
}

mBool compute_preconditionedConjugaiteGradient(
    GridVector const& a_vecPreconditionner, GridVectorSP<EntryOfA> const& a_A,
    GridVector& a_pressures, GridVector const& a_divergences)
{
    GridVector vecResidual;
    GridVector vecAuxiliary;
    GridVector vecSearch;
    mDouble    teta;
    for (mInt i = 0; i < s_nbRow * s_nbCol; ++i)
    {
        a_pressures[i] = 0.0f;
        vecResidual[i] = a_divergences[i];
    }
    apply_preconditionner(a_vecPreconditionner, a_A, vecResidual, vecAuxiliary);
    vecSearch = vecAuxiliary;
    teta      = math::dot(vecAuxiliary, vecResidual);

    for (mInt iterID = 0; iterID < s_maxIteration; ++iterID)
    {
        apply_A(a_A, vecSearch, vecAuxiliary);

        mDouble dotValue = dot(vecAuxiliary, vecSearch);
        mDouble alpha    = teta / dotValue;
        a_pressures += alpha * vecSearch;
        vecResidual -= alpha * vecAuxiliary;

        if (get_maxValue(vecResidual) <= s_solutionTolerance)
        {
            return true;
        }

        apply_preconditionner(a_vecPreconditionner, a_A, vecResidual,
                              vecAuxiliary);
        mDouble newTeta = math::dot(vecAuxiliary, vecResidual);
        mDouble beta    = newTeta / teta;
        vecSearch       = vecAuxiliary + beta * vecSearch;
        teta            = newTeta;
    }

    return false;
}

void update_velocities(GridVector& a_pressures, Universe& a_output,
                       mDouble a_deltaTime)
{
    for (mInt j = 0; j < s_nbRow; ++j)
    {
        for (mInt i = 0; i < s_nbCol; ++i)
        {
            mInt index      = convert_toIndex(i, j);
            mInt indexPlusI = convert_toIndex(i + 1, j);
            mInt indexPlusJ = convert_toIndex(i, j + 1);

            mDouble pressure = a_pressures[index];
            mDouble pressurePlusI;
            mDouble pressurePlusJ;

            if (is_outOfBoundI(i + 1))
            {
                pressurePlusI = a_pressures[index] + (s_density * s_cellSize *
                                                      (a_output.Q[index].uh)) /
                                                         a_deltaTime;
            }
            else
            {
                pressurePlusI = a_pressures[indexPlusI];
            }

            if (is_outOfBoundJ(j + 1))
            {
                pressurePlusJ = a_pressures[index] + (s_density * s_cellSize *
                                                      (a_output.Q[index].uv)) /
                                                         a_deltaTime;
            }
            else
            {
                pressurePlusJ = a_pressures[indexPlusJ];
            }

            a_output.Q[index].uh = a_output.Q[index].uh -
                                   a_deltaTime * (pressurePlusI - pressure) /
                                       (s_density * s_cellSize);
            a_output.Q[index].uv = a_output.Q[index].uv -
                                   a_deltaTime * (pressurePlusJ - pressure) /
                                       (s_density * s_cellSize);
        }
    }
}

profile::mProfilerMultiSampling<1> g_AdvectionProfiler;
profile::mProfilerMultiSampling<1> g_UpdateProfiler;
profile::mProfilerMultiSampling<1> g_ProjectionProfiler;

void Simulate(Universe const& a_input, Universe& a_output, mDouble a_deltaTime)
{
    mLog_info("SIMULATION STEP ---------------------");
    //---- Advect
    mLog_info("Advect");
    {
        profile::mRAIITiming timming(g_AdvectionProfiler);
        for (mInt row = 0; row < s_nbRow; ++row)
        {
            for (mInt col = 0; col < s_nbCol; ++col)
            {
                mInt         index     = row * s_nbCol + col;
                math::mDVec2 cellPoint = {mDouble(col), mDouble(row)};
                // Find particle starting point
                math::mDVec2 speed = get_speedAt(a_input, {col, row});
                math::mDVec2 startingPosition = cellPoint - a_deltaTime * speed;

                a_output.Q[index] = get_valueAt(a_input, startingPosition);
            }
        }
    }
    mLog_info("--- Timing : ",
              g_AdvectionProfiler.get_average<mDouble, std::micro>());

    //---- Apply gravity
    mLog_info("Apply gravity");
    {
        profile::mRAIITiming timming(g_UpdateProfiler);
        for (mInt row = 0; row < s_nbRow; ++row)
        {
            for (mInt col = 0; col < s_nbCol; ++col)
            {
                mInt index = row * s_nbCol + col;
                a_output.Q[index].uv += a_deltaTime * s_gravity;
                a_output.Q[index].uh += a_deltaTime * s_wind;
            }
        }
    }
    mLog_info("--- Timing : ",
              g_UpdateProfiler.get_average<mDouble, std::micro>());

    //---- Project
    mLog_info("Project");
    {
        std::stringstream debugString;
        debugString << std::setprecision(3) << std::fixed;
        profile::mRAIITiming timming(g_ProjectionProfiler);
        // Compute divergences
        mLog_info("- Compute Divergences");
        GridVector divergences;
        compute_divergence(a_output, divergences, a_deltaTime);
        divergences *= -1;
        //        debugString.seekp(std::ios_base::beg);
        //        for (mInt row = s_nbRow - 1; row >= 0; --row)
        //        {
        //            for (mInt col = 0; col < s_nbCol; ++col)
        //            {
        //                mInt    index = row * s_nbCol + col;
        //                mDouble value = divergences[index];
        //                debugString << ((value < 0) ? "" : " ") <<
        //                divergences[index]
        //                            << " ";
        //            }
        //            debugString << std::endl;
        //        }
        //        mLog_info("\n", debugString.str());
        // Set entries of A
        mLog_info("- Set A");
        GridVectorSP<EntryOfA> A;
        compute_entriesOfA(a_output, a_deltaTime, A);

        //        debugString.seekp(std::ios_base::beg);
        //        for (mInt row = s_nbRow - 1; row >= 0; --row)
        //        {
        //            for (mInt col = 0; col < s_nbCol; ++col)
        //            {
        //                mInt    index = row * s_nbCol + col;
        //                mDouble value = A[index].Adiag;
        //                debugString << ((value < 0) ? "" : " ") << value << "
        //                ";
        //            }
        //            debugString << std::endl;
        //        }
        //        mLog_info("\n", debugString.str());
        //        debugString.seekp(std::ios_base::beg);
        //        for (mInt row = s_nbRow - 1; row >= 0; --row)
        //        {
        //            for (mInt col = 0; col < s_nbCol; ++col)
        //            {
        //                mInt    index = row * s_nbCol + col;
        //                mDouble value = A[index].Aplusi;
        //                debugString << ((value < 0) ? "" : " ") << value << "
        //                ";
        //            }
        //            debugString << std::endl;
        //        }
        //        mLog_info("\n", debugString.str());
        //        debugString.seekp(std::ios_base::beg);
        //        for (mInt row = s_nbRow - 1; row >= 0; --row)
        //        {
        //            for (mInt col = 0; col < s_nbCol; ++col)
        //            {
        //                mInt    index = row * s_nbCol + col;
        //                mDouble value = A[index].Aplusj;
        //                debugString << ((value < 0) ? "" : " ") << value << "
        //                ";
        //            }
        //            debugString << std::endl;
        //        }
        //        mLog_info("\n", debugString.str());

        // Construct MIC(0)
        mLog_info("- Construct MIC");
        GridVector MIC0;
        compute_MIC0Entries(A, MIC0);

        //        debugString.seekp(std::ios_base::beg);
        //        for (mInt row = s_nbRow - 1; row >= 0; --row)
        //        {
        //            for (mInt col = 0; col < s_nbCol; ++col)
        //            {
        //                mInt    index = row * s_nbCol + col;
        //                mDouble value = MIC0[index];
        //                debugString << ((value < 0) ? "" : " ") << value << "
        //                ";
        //            }
        //            debugString << std::endl;
        //        }
        //        mLog_info("\n", debugString.str());
        // Solve Ap = d with MICCG(0)
        mLog_info("- Solve");
        GridVector pressures;
        compute_preconditionedConjugaiteGradient(MIC0, A, pressures,
                                                 divergences);

        //        debugString.seekp(std::ios_base::beg);
        //        for (mInt row = s_nbRow - 1; row >= 0; --row)
        //        {
        //            for (mInt col = 0; col < s_nbCol; ++col)
        //            {
        //                mInt    index = row * s_nbCol + col;
        //                mDouble value = pressures[index];
        //                debugString << ((value < 0) ? "" : " ") << value << "
        //                ";
        //            }
        //            debugString << std::endl;
        //        }
        //        mLog_info("\n", debugString.str());
        // Change velocities according to pressures
        mLog_info("- Update");
        update_velocities(pressures, a_output, a_deltaTime);
        /*
        debugString.seekp(std::ios_base::beg);
        for (mInt row = 0; row < s_nbRow * s_nbCol; ++row)
        {
            mInt jr = row / s_nbCol;
            mInt ir = row % s_nbCol;
            for (mInt col = 0; col < s_nbCol * s_nbCol; ++col)
            {
                mInt jc = col / s_nbCol;
                mInt ic = col % s_nbCol;

                mDouble value;
                if (row == col)
                {
                    value = A[row].Adiag;
                    debugString << ((value < 0) ? "" : " ") << value << " ";
                }
                else if ((jc == (jr + 1)) && (ic == ir))
                {
                    value = A[row].Aplusj;
                    debugString << ((value < 0) ? "" : " ") << value << " ";
                }
                else if ((ic == (ir + 1)) && (jc == jr))
                {
                    value = A[row].Aplusi;
                    debugString << ((value < 0) ? "" : " ") << value << " ";
                }
                else if ((jc == (jr - 1)) && (ic == ir))
                {
                    mInt index = jc * s_nbCol + ic;
                    value      = A[index].Aplusj;
                    debugString << ((value < 0) ? "" : " ") << value << " ";
                }
                else if ((ic == (ir - 1)) && (jc == jr))
                {
                    mInt index = jc * s_nbCol + ic;
                    value      = A[index].Aplusi;
                    debugString << ((value < 0) ? "" : " ") << value << " ";
                }
                else
                {
                    debugString << " 0 ";  // << 0.0f << " ";
                }
            }
            debugString << std::endl;
        }
        mLog_info("\n", debugString.str());*/

        compute_divergence(a_output, divergences, a_deltaTime);
        //        debugString.seekp(std::ios_base::beg);
        //        for (mInt row = s_nbRow - 1; row >= 0; --row)
        //        {
        //            for (mInt col = 0; col < s_nbCol; ++col)
        //            {
        //                mInt    index = row * s_nbCol + col;
        //                mDouble value = divergences[index];
        //                debugString << ((value < 0) ? "" : " ") <<
        //                divergences[index]
        //                            << " ";
        //            }
        //            debugString << std::endl;
        //        }
        //        mLog_info("Final divergence : \n", debugString.str());
    }
    mLog_info("--- Timing : ",
              g_ProjectionProfiler.get_average<mDouble, std::micro>());
}

class FluidSimulationApp : public m::crossPlatform::IWindowedApplication
{
    void init(m::mCmdLine const& a_cmdLine, void* a_appData) override
    {
        m::crossPlatform::IWindowedApplication::init(a_cmdLine, a_appData);

        m::mCmdLine const& cmdLine = a_cmdLine;
        m::mUInt           width   = 600;
        m::mUInt           height  = 600;

        m_pDx12Api = new m::dx12::mApi();
        m_pDx12Api->init();
        auto& rDx12Api = *m_pDx12Api;

        m_mainWindow = add_newWindow("Fluid Simulation", width, height, false);

        m_tasksetExecutor.init();

        static const mUInt           s_nbBackBuffer = 3;
        m::render::mISynchTool::Desc desc{s_nbBackBuffer};

        auto& dx12SynchTool = rDx12Api.create_synchTool();
        m_pDx12SynchTool    = &dx12SynchTool;
        dx12SynchTool.init(desc);

        auto& dx12Swapchain = rDx12Api.create_swapchain();
        m_pDx12Swapchain    = &dx12Swapchain;
        m::render::init_swapchainWithWindow(*m_pDx12Api, m_tasksetExecutor,
                                            dx12Swapchain, dx12SynchTool,
                                            *m_mainWindow, s_nbBackBuffer);

        m::dearImGui::init(*m_mainWindow);

        m::render::Taskset& taskset_renderPipeline =
            rDx12Api.create_renderTaskset();

        m::render::mTaskDataSwapchainWaitForRT taskData_swapchainWaitForRT{};
        taskData_swapchainWaitForRT.pSwapchain = m_pDx12Swapchain;
        taskData_swapchainWaitForRT.pSynchTool = m_pDx12SynchTool;
        auto& acquireTask = static_cast<m::render::mTaskSwapchainWaitForRT&>(
            taskData_swapchainWaitForRT.add_toTaskSet(taskset_renderPipeline));

        TaskDataFluidSimulation taskdata_fluidSimulation;
        taskdata_fluidSimulation.pOutputRT  = acquireTask.pOutputRT;
        taskdata_fluidSimulation.pPixelData = &m_pixelData;
        taskdata_fluidSimulation.add_toTaskSet(taskset_renderPipeline);

        m::render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.nbFrames  = s_nbBackBuffer;
        taskData_drawDearImGui.pOutputRT = acquireTask.pOutputRT;
        taskData_drawDearImGui.add_toTaskSet(taskset_renderPipeline);

        m::render::mTaskDataSwapchainPresent taskData_swapchainPresent{};
        taskData_swapchainPresent.pSwapchain = m_pDx12Swapchain;
        taskData_swapchainPresent.pSynchTool = m_pDx12SynchTool;
        taskData_swapchainPresent.add_toTaskSet(taskset_renderPipeline);

        m_tasksetExecutor.confy_permanentTaskset(m::unref_safe(m_pDx12Api),
                                                 taskset_renderPipeline);
        m_mainWindow->attach_toDestroy(m::mCallback<void>(
            [this, &rDx12Api, &taskset_renderPipeline]()
            {
                m_tasksetExecutor.remove_permanentTaskset(
                    rDx12Api, taskset_renderPipeline);
            }));

        m_mainWindow->link_inputManager(&m_inputManager);

        m_inputManager.attach_toKeyEvent(
            m::input::mKeyAction::keyPressed(m::input::keyF11),
            m::input::mKeyActionCallback(
                m_mainWindow, &m::windows::mIWindow::toggle_fullScreen));

        set_minimalStepDuration(std::chrono::milliseconds(16));

        m_pixelData.resize(s_nbRow * s_nbCol);
        init_universe(m_universes[0]);
    }

    void destroy() override
    {
        m::crossPlatform::IWindowedApplication::destroy();

        m_pDx12SynchTool->destroy();
        m_pDx12Api->destroy_synchTool(*m_pDx12SynchTool);

        m_pDx12Swapchain->destroy();
        m_pDx12Api->destroy_swapchain(*m_pDx12Swapchain);

        m_pDx12Api->destroy();
        delete m_pDx12Api;

        m::dearImGui::destroy();
    }

    m::mBool step(
        std::chrono::steady_clock::duration const& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        static mFloat simulationSpeed = 1.0f;
        static mInt   i               = 0;
        mInt          previous        = i;

        static mBool displayTmp    = true;
        static mBool displaySpeed  = true;
        static mBool runtimeUpdate = false;

        for (mInt pix = 0; pix < s_nbRow * s_nbCol; ++pix)
        {
            m_pixelData[pix].r =
                displayTmp ? m_universes[previous].Q[pix].T / 10 : 0;

            m_pixelData[pix].g =
                displaySpeed ? m_universes[previous].Q[pix].uh * 0.5 + 0.5 : 0;
            m_pixelData[pix].b =
                displaySpeed ? m_universes[previous].Q[pix].uv * 0.5 + 0.5 : 0;
        }

        if (runtimeUpdate)
        {
            i = (i + 1) % 2;
            Simulate(m_universes[previous], m_universes[i],
                     simulationSpeed * 0.016f);
        }

        start_dearImGuiNewFrame(*m_pDx12Api);

        ImGui::NewFrame();

        m::mBool showDemo = true;
        ImGui::Begin("Simulation Parameters");
        ImGui::DragFloat("SimulationSpeed", &simulationSpeed, 0.01f, 0.01f,
                         2.0f);
        if (ImGui::Button("Reset"))
        {
            init_universe(m_universes[i]);
        }

        if (ImGui::Button("Simulation Step"))
        {
            i = (i + 1) % 2;
            Simulate(m_universes[previous], m_universes[i],
                     simulationSpeed * 0.016f);
            // std::chrono::duration<mDouble>(a_deltaTime).count());
        }

        ImGui::Checkbox("Run Time Update", &runtimeUpdate);

        ImGui::Checkbox("Display Tmp", &displayTmp);
        ImGui::Checkbox("Display Speed", &displaySpeed);
        ImGui::End();

        ImGui::Render();

        m_tasksetExecutor.run();

        return true;
    }

    m::render::mIApi*       m_pDx12Api;
    m::render::mISwapchain* m_pDx12Swapchain;
    m::render::mISynchTool* m_pDx12SynchTool;

    m::render::mTasksetExecutor m_tasksetExecutor;

    m::input::mCallbackInputManager m_inputManager;
    m::windows::mIWindow*           m_mainWindow;

    std::vector<m::math::mVec4> m_pixelData;
    Universe                    m_universes[2];

    const m::logging::mChannelID m_FluidSimulation_ID = mLog_getId();
};

M_EXECUTE_WINDOWED_APP(FluidSimulationApp)
