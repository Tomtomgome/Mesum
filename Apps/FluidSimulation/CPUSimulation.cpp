
#ifdef CPU_SIDE_SIMULATION

#include <MesumCore/Kernel/Kernel.hpp>
#include <MesumCore/Kernel/MathTypes.hpp>
#include <Kernel/Profile.hpp>

const m::logging::mChannelID m_FluidSimulation_ID = mLog_getId();

static const int s_nbRow = 20 * 16;
static const int s_nbCol = 20 * 16;

using namespace m;

static const mDouble s_cellSize = 1.0;
static const mDouble s_density  = 1.0;

static const mInt    s_maxIteration       = 10000;
static const mDouble s_solutionTolerance  = 0.00000000000001;
static const mDouble s_micTunningConstant = 0.97;

static const mDouble s_gravity  = -9.8;
static const mDouble s_wind     = -2.5;
static const mDouble s_ambientT = 270;

// buyancy parameters
static mDouble s_alpha             = -2.5;
static mDouble s_beta              = 8.2;
static mDouble s_vorticityStrength = 0.4;

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
    mDouble S;
};

Particle operator+(Particle const& a_a, Particle const& a_b)
{
    Particle output;
    output.uv = a_a.uv + a_b.uv;
    output.uh = a_a.uh + a_b.uh;
    output.T  = a_a.T + a_b.T;
    output.S  = a_a.S + a_b.S;

    return output;
}

Particle operator-(Particle const& a_a, Particle const& a_b)
{
    Particle output;
    output.uv = a_a.uv - a_b.uv;
    output.uh = a_a.uh - a_b.uh;
    output.T  = a_a.T - a_b.T;
    output.S  = a_a.S - a_b.S;

    return output;
}

Particle operator*(mFloat a_f, Particle const& a_a)
{
    Particle output;
    output.uv = a_f * a_a.uv;
    output.uh = a_f * a_a.uh;
    output.T  = a_f * a_a.T;
    output.S  = a_f * a_a.S;

    return output;
}

void restrict(Particle& a_p, Particle const& a_delta)
{
    a_p.uv = (a_p.uv * a_delta.uv) > 0.0 ? a_p.uv : 0.0;
    a_p.uh = (a_p.uh * a_delta.uh) > 0.0 ? a_p.uh : 0.0;
    a_p.T  = (a_p.T * a_delta.T) > 0.0 ? a_p.T : 0.0;
    a_p.S  = (a_p.S * a_delta.S) > 0.0 ? a_p.S : 0.0;
}

Particle interpolate_cubic(Particle const& a_iMinusOne, Particle const& a_i,
                           Particle const& a_iPlusOne,
                           Particle const& a_iPlusTwo, mDouble const a_alpha)
{
    Particle di        = 0.5 * (a_iPlusOne - a_iMinusOne);
    Particle diPlusOne = 0.5 * (a_iPlusTwo - a_i);

    Particle delta = (a_iPlusOne - a_i);

    restrict(di, delta);
    restrict(diPlusOne, delta);

    Particle output =
        a_i + a_alpha * di +
        a_alpha * a_alpha * (3.0 * delta - 2.0 * di - diPlusOne) +
        a_alpha * a_alpha * a_alpha * (-2.0 * delta + di + diPlusOne);

    return output;
}

Particle interpolate(Particle const& a_a, Particle const& a_b,
                     mDouble const a_alpha)
{
    Particle output;
    output.uv = (1.0f - a_alpha) * a_a.uv + a_b.uv * a_alpha;
    output.uh = (1.0f - a_alpha) * a_a.uh + a_b.uh * a_alpha;
    output.T  = (1.0f - a_alpha) * a_a.T + a_b.T * a_alpha;
    output.S  = (1.0f - a_alpha) * a_a.S + a_b.S * a_alpha;

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
            a_input.Q[index].T  = s_ambientT;
            a_input.Q[index].S  = 0.0;
            a_input.Q[index].uv = 0.0;
            a_input.Q[index].uh = 0.0;

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

    a_input.Q[convert_toIndex(s_nbCol / 2, 1)].T = 350;
    a_input.Q[convert_toIndex(s_nbCol / 2, 1)].S = 200;
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

    math::mIVec2 a_flooredPositionPlusOne{
        std::min(s_nbCol - 1, a_flooredPosition.x + 1),
        std::min(s_nbRow - 1, a_flooredPosition.y + 1)};

    Particle tl = a_input.Q[convert_toIndex(a_flooredPosition.x,
                                            a_flooredPositionPlusOne.y)];
    Particle tr = a_input.Q[convert_toIndex(a_flooredPositionPlusOne.x,
                                            a_flooredPositionPlusOne.y)];
    Particle br =
        a_input
            .Q[convert_toIndex(a_flooredPosition.x + 1, a_flooredPosition.y)];
    Particle bl =
        a_input.Q[convert_toIndex(a_flooredPosition.x, a_flooredPosition.y)];

    Particle top =
        interpolate(tl, tr, saturate(a_position.x - a_flooredPosition.x));
    Particle bottom =
        interpolate(bl, br, saturate(a_position.x - a_flooredPosition.x));

    // return interpolate(bottom, top, a_position.y - a_flooredPosition.y);

    // Cubic interpolation
    math::mIVec2 a_flooredPositionPlusTwo{
        std::min(s_nbCol - 1, a_flooredPosition.x + 2),
        std::min(s_nbRow - 1, a_flooredPosition.y + 2)};
    math::mIVec2 a_flooredPositionMinusOne{
        std::max(0, a_flooredPosition.x - 1),
        std::max(0, a_flooredPosition.y - 1)};

    Particle im1jm1 = a_input.Q[convert_toIndex(a_flooredPositionMinusOne.x,
                                                a_flooredPositionMinusOne.y)];
    Particle ijm1   = a_input.Q[convert_toIndex(a_flooredPosition.x,
                                                a_flooredPositionMinusOne.y)];
    Particle ip1jm1 = a_input.Q[convert_toIndex(a_flooredPositionPlusOne.x,
                                                a_flooredPositionMinusOne.y)];
    Particle ip2jm1 = a_input.Q[convert_toIndex(a_flooredPositionPlusTwo.x,
                                                a_flooredPositionMinusOne.y)];

    Particle im1j = a_input.Q[convert_toIndex(a_flooredPositionMinusOne.x,
                                              a_flooredPosition.y)];
    Particle ij =
        a_input.Q[convert_toIndex(a_flooredPosition.x, a_flooredPosition.y)];
    Particle ip1j = a_input.Q[convert_toIndex(a_flooredPositionPlusOne.x,
                                              a_flooredPosition.y)];
    Particle ip2j = a_input.Q[convert_toIndex(a_flooredPositionPlusTwo.x,
                                              a_flooredPosition.y)];

    Particle im1jp1 = a_input.Q[convert_toIndex(a_flooredPositionMinusOne.x,
                                                a_flooredPositionPlusOne.y)];
    Particle ijp1   = a_input.Q[convert_toIndex(a_flooredPosition.x,
                                                a_flooredPositionPlusOne.y)];
    Particle ip1jp1 = a_input.Q[convert_toIndex(a_flooredPositionPlusOne.x,
                                                a_flooredPositionPlusOne.y)];
    Particle ip2jp1 = a_input.Q[convert_toIndex(a_flooredPositionPlusTwo.x,
                                                a_flooredPositionPlusOne.y)];

    Particle im1jp2 = a_input.Q[convert_toIndex(a_flooredPositionMinusOne.x,
                                                a_flooredPositionPlusTwo.y)];
    Particle ijp2   = a_input.Q[convert_toIndex(a_flooredPosition.x,
                                                a_flooredPositionPlusTwo.y)];
    Particle ip1jp2 = a_input.Q[convert_toIndex(a_flooredPositionPlusOne.x,
                                                a_flooredPositionPlusTwo.y)];
    Particle ip2jp2 = a_input.Q[convert_toIndex(a_flooredPositionPlusTwo.x,
                                                a_flooredPositionPlusTwo.y)];

    Particle jm1 =
        interpolate_cubic(im1jm1, ijm1, ip1jm1, ip2jm1,
                          saturate(a_position.x - a_flooredPosition.x));
    Particle j = interpolate_cubic(
        im1j, ij, ip1j, ip2j, saturate(a_position.x - a_flooredPosition.x));
    Particle jp1 =
        interpolate_cubic(im1jp1, ijp1, ip1jp1, ip2jp1,
                          saturate(a_position.x - a_flooredPosition.x));
    Particle jp2 =
        interpolate_cubic(im1jp2, ijp2, ip1jp2, ip2jp2,
                          saturate(a_position.x - a_flooredPosition.x));

    return interpolate_cubic(jm1, j, jp1, jp2,
                             saturate(a_position.y - a_flooredPosition.y));
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

void compute_vorticity(Universe const& a_input, GridVector& a_outVorticities)
{
    for (mInt j = 0; j < s_nbRow; ++j)
    {
        for (mInt i = 0; i < s_nbCol; ++i)
        {
            math::mDVec2 speed_dx =
                get_speedAt(a_input, {std::min(s_nbCol - 1, i + 1), j}) -
                get_speedAt(a_input, {std::max(0, i - 1), j});
            mDouble      dv_dx = speed_dx.y / (2 * s_cellSize);
            math::mDVec2 speed_dy =
                get_speedAt(a_input, {i, std::min(s_nbRow - 1, j + 1)}) -
                get_speedAt(a_input, {i, std::max(0, j - 1)});
            mDouble du_dy = speed_dy.x / (2 * s_cellSize);

            mInt index = convert_toIndex(i, j);

            a_outVorticities[index] = dv_dx - du_dy;
        }
    }
}

void compute_vorticityForce(GridVector const&           a_input,
                            GridVectorSP<math::mDVec2>& a_outGradient)
{
    for (mInt j = 0; j < s_nbRow; ++j)
    {
        for (mInt i = 0; i < s_nbCol; ++i)
        {
            mInt index = convert_toIndex(i, j);

            mInt iplusone  = std::min(s_nbCol - 1, std::max(0, i + 1));
            mInt iminusone = std::min(s_nbCol - 1, std::max(0, i - 1));
            mInt jplusone  = std::min(s_nbRow - 1, std::max(0, j + 1));
            mInt jminusone = std::min(s_nbRow - 1, std::max(0, j - 1));

            mDouble input_iplusone =
                std::abs(a_input[convert_toIndex(iplusone, j)]);
            mDouble input_iminusone =
                std::abs(a_input[convert_toIndex(iminusone, j)]);
            mDouble input_jplusone =
                std::abs(a_input[convert_toIndex(i, jplusone)]);
            mDouble input_jminusone =
                std::abs(a_input[convert_toIndex(i, jminusone)]);

            a_outGradient[index].x =
                (input_iplusone - input_iminusone) / (2 * s_cellSize);
            a_outGradient[index].y =
                (input_jplusone - input_jminusone) / (2 * s_cellSize);

            a_outGradient[index] = math::normalized_safe(a_outGradient[index]);
            a_outGradient[index] =
                math::mDVec2{a_outGradient[index].y * a_input[index],
                             a_outGradient[index].x * a_input[index]};
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
    // return is_outOfBoundJ(a_j);
    // return false;
    return is_outOfBound(a_i, a_j);
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
    mLog_infoTo(m_FluidSimulation_ID, "SIMULATION STEP ---------------------");
    //---- Advect
    mLog_infoTo(m_FluidSimulation_ID, "Advect");
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
    mLog_infoTo(m_FluidSimulation_ID, "--- Timing : ",
                g_AdvectionProfiler.get_average<mDouble, std::micro>());

    //---- Apply forces
    mLog_infoTo(m_FluidSimulation_ID, "Apply gravity");
    {
        profile::mRAIITiming timming(g_UpdateProfiler);

        GridVector vorticities;
        compute_vorticity(a_output, vorticities);
        GridVectorSP<math::mDVec2> vorticityForces;
        compute_vorticityForce(vorticities, vorticityForces);

        for (mInt row = 0; row < s_nbRow; ++row)
        {
            for (mInt col = 0; col < s_nbCol; ++col)
            {
                mInt index = row * s_nbCol + col;
                mInt indexXPlusOne =
                    row * s_nbCol + std::min(s_nbCol - 1, (col + 1));
                mInt indexYPlusOne =
                    std::min(s_nbRow - 1, (row + 1)) * s_nbCol + col;
                a_output.Q[index].uv += a_deltaTime * s_gravity;
                // a_output.Q[index].uh += a_deltaTime * s_wind;
                //  Boyancy test
                mDouble saturation =
                    (a_output.Q[index].S + a_output.Q[indexYPlusOne].S) / 2.0f;
                mDouble temperature =
                    (a_output.Q[index].T + a_output.Q[indexYPlusOne].T) / 2.0f;
                a_output.Q[index].uv +=
                    a_deltaTime * (s_alpha * saturation +
                                   s_beta * (temperature - s_ambientT));
                // Vorticity test
                math::mDVec2 vorticityForceX =
                    0.5 *
                    (vorticityForces[index] + vorticityForces[indexXPlusOne]);
                math::mDVec2 vorticityForceY =
                    0.5 *
                    (vorticityForces[index] + vorticityForces[indexYPlusOne]);
                a_output.Q[index].uv += a_deltaTime * s_vorticityStrength *
                                        s_cellSize * vorticityForceY.y;
                a_output.Q[index].uh += a_deltaTime * s_vorticityStrength *
                                        s_cellSize * vorticityForceX.x;
            }
        }
    }
    mLog_infoTo(m_FluidSimulation_ID, "--- Timing : ",
                g_UpdateProfiler.get_average<mDouble, std::micro>());

    //---- Project
    mLog_infoTo(m_FluidSimulation_ID, "Project");
    {
        std::stringstream debugString;
        debugString << std::setprecision(3) << std::fixed;
        profile::mRAIITiming timming(g_ProjectionProfiler);
        // Compute divergences
        mLog_infoTo(m_FluidSimulation_ID, "- Compute Divergences");
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
        mLog_infoTo(m_FluidSimulation_ID, "- Set A");
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
        mLog_infoTo(m_FluidSimulation_ID, "- Construct MIC");
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
        mLog_infoTo(m_FluidSimulation_ID, "- Solve");
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
        mLog_infoTo(m_FluidSimulation_ID, "- Update");
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
    mLog_infoTo(m_FluidSimulation_ID, "--- Timing : ",
                g_ProjectionProfiler.get_average<mDouble, std::micro>());

    mDisable_logChannels(m_FluidSimulation_ID);
}

#endif