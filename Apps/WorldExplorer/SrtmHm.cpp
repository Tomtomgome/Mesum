#include <numbers>
#include <DirectXMath.h>
#include "SrtmHm.hpp"
#include "Kernel/File.hpp"
#include "Mesh.hpp"
#include "Kernel/Spherical.hpp"

mInt const g_uSrtmPoint   = 3601;  // 5
mInt const g_uSrtmSegment = 3600;  // 4

// oStaticAssert(g_uSrtmSegment == (g_uSrtmPoint - 1));

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mUInt oArray2Index(mUInt a_tIndexInRow, mUInt a_tRowIndex, mUInt a_tRowLength)
{
    return (a_tIndexInRow + a_tRowIndex * a_tRowLength);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mUInt oCeilToUInt(mFloat a_fVal)
{
    return mUInt(ceilf(a_fVal));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mFloat oDegToRad(mFloat a_fDeg)
{
    return (a_fDeg * 0.01745329251994f);
}

//----------------------------------------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------------------------------------
math::mVec3 CrossProd(math::mVec3 a, math::mVec3 b)
{
    return math::mVec3(
        {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x});
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mInt GetLinearIndex(mInt a_uIndexInRow, mInt a_uRowIndex)
{
    mAssert(a_uIndexInRow < g_uSrtmPoint);
    mAssert(a_uRowIndex < g_uSrtmPoint);

    return a_uIndexInRow + a_uRowIndex * g_uSrtmPoint;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSrtmHm::ReadAllHgtFiles()
{
    std::vector<std::string> srtmFiles;
    srtmFiles.emplace_back("N45E006.hgt");
    /*srtmFiles.emplace_back("N00E000.hgt");
    srtmFiles.emplace_back("N00E001.hgt");
    srtmFiles.emplace_back("N00W001.hgt");
    srtmFiles.emplace_back("N00W002.hgt");
    srtmFiles.emplace_back("N01E000.hgt");
    srtmFiles.emplace_back("N01E001.hgt");
    srtmFiles.emplace_back("N01W001.hgt");
    srtmFiles.emplace_back("N01W002.hgt");
    srtmFiles.emplace_back("S01E000.hgt");
    srtmFiles.emplace_back("S01E001.hgt");
    srtmFiles.emplace_back("S01W001.hgt");
    srtmFiles.emplace_back("S01W002.hgt");
    srtmFiles.emplace_back("S02E000.hgt");
    srtmFiles.emplace_back("S02E001.hgt");
    srtmFiles.emplace_back("S02W001.hgt");
    srtmFiles.emplace_back("S02W002.hgt");*/

    m_mHgtData.clear();

    for (auto& srtmFile : srtmFiles)
    {
        // Computes the file coordinates
        math::mIVec2 const i2FileCoordinates =
            GetFileCoordinatesFromFileName(srtmFile);

        // Adds a new entry in the HgtData & Fills the HgtData
        ReadHgtFile("../../../Apps/WorldExplorer/data/" + srtmFile,
                    m_mHgtData[i2FileCoordinates]);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSrtmHm::ReadHgtFile(std::string        a_rsFilePath,
                          std::vector<mU16>& a_rv2HgtData)
{
    // https://stackoverflow.com/questions/16297540/how-to-read-hgt-files-in-c

    mAssert(a_rv2HgtData.empty());
    a_rv2HgtData.resize(g_uSrtmPoint * g_uSrtmPoint, 0);

    std::vector<char> binary;
    files::copy_fileToBinary(a_rsFilePath, binary);

    mAssert(binary.size() ==
            (a_rv2HgtData.size() * 2));  // * 2 because each value uses 2 byte

    /*
            Data in the file are stored from west to east and then north to
       south, ie row by row: 0	1	2	3	4
                    5	6	7	8	9
                    10	11	12	13	14
                    15	16	17	18	19
                    20	21	22	23	24

            But we want store these data in memory from south to nord, from west
       to east, ie column by column: 4	9	14	19	24 3	8
       13	18	23 2	7	12	17	22
                    1	6	11	16	21
                    0	5	10	15	20

            In that way, we can simply iterate later from x by x (latitude) and
       then from y by y (longitude)
    */

    mInt readPos = 0;
    for (int y = 0; y < g_uSrtmPoint; y++)
    {
        for (int x = 0; x < g_uSrtmPoint; x++)
        {
            mU8 uBuffer[2];
            uBuffer[0] = binary[readPos];
            readPos++;
            uBuffer[1] = binary[readPos];
            readPos++;

            a_rv2HgtData[GetLinearIndex(g_uSrtmSegment - y, x)] =
                (uBuffer[0] << 8) |
                uBuffer[1];  // Data in the file are stored in big endian
                             // format, we need it in little endian
        }
    }

    mAssert(readPos == binary.size());
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
math::mIVec2 mSrtmHm::GetFileCoordinatesFromFileName(
    std::string const& a_fileName)
{
    mAssert(a_fileName.size() == 11);  // Must be NXXEXXX.hgt

    /* Latitude */

    std::string sLatitude           = a_fileName.substr(0, 3);
    mChar       cLatitudeSide       = sLatitude[0];
    mInt        iLatitudeAngleValue = std::stoi(sLatitude.substr(1, 2));

    // N is positive angle value, S is negative angle value
    mAssert(cLatitudeSide == 'N' || cLatitudeSide == 'S');
    if (cLatitudeSide == 'S')
    {
        iLatitudeAngleValue = (-iLatitudeAngleValue);
    }

    /* Longitude */

    std::string sLongitude           = a_fileName.substr(3, 4);
    mChar       cLongitudeSide       = sLongitude[0];
    mInt        iLongitudeAngleValue = std::stoi(sLongitude.substr(1, 3));

    // E is positive angle value, W is negative angle value
    mAssert(cLongitudeSide == 'E' || cLongitudeSide == 'W');
    if (cLongitudeSide == 'W')
    {
        iLongitudeAngleValue = -iLongitudeAngleValue;
    }

    return math::mIVec2({iLatitudeAngleValue, iLongitudeAngleValue});
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mI16 mSrtmHm::GetAltitudeFromHgtPosition(
    mHgtPosition const& a_rHgtPosition) const
{
    // Finds the related Hgt file in the map
    auto pTile = m_mHgtData.find(a_rHgtPosition.GetFileCoordinates());

    if (pTile == m_mHgtData.end())  // Maybe the Hgt file isn't in the map
    {
        return 0;
    }

    // Using the tile index, finds the altitude we want
    return pTile->second[GetLinearIndex(a_rHgtPosition.GetTileIndex().x,
                                        a_rHgtPosition.GetTileIndex().y)];
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
math::mVec3 mSrtmHm::AngleToPosition(math::mVec2 a_rv2Angle,
                                     math::mVec2 a_rv2Center,
                                     mI16        a_iAltitude) const
{
    // Computes the position on the sphere corresponding to the angle (but
    // center it)
    math::mSpherical oSpherical(
        oDegToRad(a_rv2Angle.x - a_rv2Center.x) * m_fCurveMultiplier,
        oDegToRad(a_rv2Angle.y - a_rv2Center.y) * m_fCurveMultiplier,
        ((m_fEarthRadius + a_iAltitude) * m_fWorldScale));

    math::mVec3 v3Position;
    oSpherical.compute_cartesianCoordinates(v3Position);

    // qRotCompensate allows to have the 0 0 angle to the top of the sphere, not
    // on the oCVec3::UnitZ

    DirectX::XMMATRIX qRotCompensate = DirectX::XMMatrixRotationAxis(
        DirectX::XMVectorSet(1, 0, 0, 0), -std::numbers::pi_v<mFloat> * 0.5f);

    DirectX::XMVECTOR result = DirectX::XMVector3Transform(
        DirectX::XMVectorSet(v3Position.x, v3Position.y, v3Position.z, 1.0f),
        qRotCompensate);
    DirectX::XMFLOAT4 v2F{};  // the float where we copy the v2 vector members
    XMStoreFloat4(&v2F, result);  // the function used to copy

    v3Position = math::mVec3({v2F.x, v2F.y, v2F.z});

    // Finally sticks the ground to 0, no matter of the radius of the earth
    v3Position.y -= (m_fEarthRadius * m_fWorldScale);

    return v3Position;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
m::render::mMesh mSrtmHm::_UpdateMesh()
{
    ReadAllHgtFiles();
    SelfCheck();

    /*

            The zone that we want to draw starts from the bottom left and goes
       to the top right We draw column by column, from bottom to top, from left
       to right

            |------------|
            |4-.         |
            |3-.		 |
            |2-7         |
            |1-6		 |
            |0-5-.       |
            |------------|

            It's done like that because the real earth coordinates are from
       bottom to top, from left to right:
            - south hemisphere are in negative X values,
            - north hemisphere are in positive X values

            - America are in negative Y values,
            - Russia are in positive Y values

            It's confusing, but X and Y aren't the X and Y of a 2D planisphere:
            - X is latitude, because it's the angle value around the X axis
            - Y is longitude, because it's the angle value around the Y axis
    */

    math::mVec2 v2ZoneAngleBottomLeft, v2ZoneAngleSize, v2ZoneAngleCenter;
    ComputeZoneAngle(v2ZoneAngleBottomLeft, v2ZoneAngleSize, v2ZoneAngleCenter);

    // Computes the start hgt position we need to draw our zone
    mHgtPosition oStartHgtPosition;
    oStartHgtPosition.SetFromAngle(v2ZoneAngleBottomLeft);

    // To get the vertex count of the mesh, we multiply g_uSrtmSegment to have
    // the number of tile, + 1 because we need the final vertex
    math::mUIVec2 u2VertexCount(
        {oCeilToUInt((v2ZoneAngleSize.x * g_uSrtmSegment) + 1),
         oCeilToUInt((v2ZoneAngleSize.y * g_uSrtmSegment) + 1)});

    /* Drawing */

    m::render::mMesh mesh;
    mesh.vertices.resize(u2VertexCount.x * u2VertexCount.y);

    // Because X is latitude, rows are vertical!
    mUInt rowsCount = u2VertexCount.y;
    mUInt rowLength = u2VertexCount.x;

    for (int y = 0; y < rowsCount; y++)
    {
        for (int x = 0; x < rowLength; x++)
        {
            // For each points, we move the start hgt position
            mHgtPosition oMovedHgtPosition(oStartHgtPosition);
            oMovedHgtPosition.Move(x, y);

            // We get the altitude at this point
            mI16 iAltitude = GetAltitudeFromHgtPosition(oMovedHgtPosition);

            // We compute the angle at this point
            math::mVec2 v2Angle(
                {v2ZoneAngleBottomLeft.x +
                     (mFloat(x) /
                      g_uSrtmSegment),  // (1 / g_uSrtmSegment) = angle by tile,
                                        // so (tile * (1 / g_uSrtmSegment)) =
                                        // (tile / g_uSrtmSegment))
                 v2ZoneAngleBottomLeft.y + (mFloat(y) / g_uSrtmSegment)});

            // And finally, we compute the position of the vertex
            mesh.vertices[oArray2Index(x, y, rowLength)].position =
                AngleToPosition(v2Angle, v2ZoneAngleCenter, iAltitude);

            // Triangulates (except the first line of points)
            if (x > 0 && y > 0)
            {
                /*

                [x-1, y-1] --- [x, y-1]
                        |		/	|
                        |	   /	|
                        |     /		|
                 [x-1, y] ----- [x, y]

                */

                // Triangle array ?

                mUInt a = (oArray2Index(x - 1, y - 1, rowLength));
                mUInt b = (oArray2Index(x - 1, y, rowLength));
                mUInt c = (oArray2Index(x, y - 1, rowLength));

                mUInt d = (oArray2Index(x, y, rowLength));
                mUInt e = (oArray2Index(x, y - 1, rowLength));
                mUInt f = (oArray2Index(x - 1, y, rowLength));

                render::mBasicVertex& av3 = mesh.vertices[a];
                render::mBasicVertex& bv3 = mesh.vertices[b];
                render::mBasicVertex& cv3 = mesh.vertices[c];

                render::mBasicVertex& dv3 = mesh.vertices[d];
                render::mBasicVertex& ev3 = mesh.vertices[e];
                render::mBasicVertex& fv3 = mesh.vertices[f];

                math::mVec3 normalABC = normalized(CrossProd(
                    bv3.position - av3.position, cv3.position - av3.position));
                math::mVec3 normalDEF = normalized(CrossProd(
                    ev3.position - dv3.position, fv3.position - dv3.position));

                // Ecrase les precedent vertex... mais tant pis
                av3.normal = normalABC;
                bv3.normal = normalABC;
                cv3.normal = normalABC;

                dv3.normal = normalDEF;
                ev3.normal = normalDEF;
                fv3.normal = normalDEF;

                mesh.triangles.push_back(a);
                mesh.triangles.push_back(b);
                mesh.triangles.push_back(c);

                mesh.triangles.push_back(d);
                mesh.triangles.push_back(e);
                mesh.triangles.push_back(f);
            }
        }
    }

    return mesh;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSrtmHm::ComputeZoneAngle(math::mVec2& a_rv2ZoneAngleBottomLeft,
                               math::mVec2& a_rv2ZoneAngleSize,
                               math::mVec2& a_rv2ZoneAngleCenter) const
{
    a_rv2ZoneAngleBottomLeft =
        math::mVec2({std::min(m_v2ZoneTopLeft.x, m_v2ZoneBottomRight.x),
                     std::min(m_v2ZoneTopLeft.y, m_v2ZoneBottomRight.y)});

    a_rv2ZoneAngleSize =
        math::mVec2({std::abs(m_v2ZoneTopLeft.x - m_v2ZoneBottomRight.x),
                     std::abs(m_v2ZoneTopLeft.y - m_v2ZoneBottomRight.y)});

    a_rv2ZoneAngleCenter =
        math::mVec2(a_rv2ZoneAngleBottomLeft + a_rv2ZoneAngleSize * 0.5f);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSrtmHm::SelfCheck() {}