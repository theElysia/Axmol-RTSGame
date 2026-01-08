#include "GameMap/PathFinderStrategy.h"
#include <queue>
#include "axmol.h"

using Point = PathFinderStrategy::Point;

bool PathFinder_AStar::init(const std::vector<bool>& grid, int x, int y)
{
    return false;
}

void PathFinder_AStar::setDest(const Point& dst) {}
std::optional<std::vector<Point>> PathFinder_AStar::findPath(const Point& src)
{
    return std::nullopt;
}

bool PathFinder_FlowField::init(const std::vector<bool>& grid, int x, int y)
{
    grid_   = grid;
    wx_     = x;
    wy_     = y;
    wtotal_ = x * y;
    grid_index_.clear();
    grid_flow_.clear();
    grid_index_.resize(wtotal_, 0);
    grid_flow_.resize(wtotal_, DirectionType::UNDEF);
    // neighbor_ = {Point(0, -1), Point(-1, 0), Point(0, 1), Point(1, 0)};
    neighbor_ = {Point(0, -1), Point(-1, 0), Point(0, 1), Point(1, 0),
                 Point(0, -1), Point(-1, 0), Point(0, 1), Point(1, 0)};  // 重复以减少取模运算
    return true;
}

// 把全图用bfs遍历,dst=1,idx递增
void PathFinder_FlowField::setDest(const Point& dst)
{
    AXLOGD("start setDest");
    if (dst_ == dst || !withinBoundary(dst))
        return;
    dst_ = dst;
    resetField();
    std::queue<Point> q;
    q.push(dst);
    int tdst = transPoint(dst);
    if (grid_index_[tdst] == -1)
        return;
    grid_index_[tdst] = 1;

    AXLOGD("start setDest2");
    while (!q.empty())
    {
        Point pos = q.front();
        int idx   = grid_index_[transPoint(pos)] + 1;
        q.pop();
        for (int i = 0; i < direct_num; i++)
        {
            Point npos = pos + neighbor_[i];
            if (!withinBoundary(npos))
                continue;
            int tpos = transPoint(npos);
            if (grid_index_[tpos] == 0)
            {
                grid_index_[tpos] = idx;
                q.push(npos);
            }
        }
    }
    AXLOGD("start setDest3");
}

// 找路并标记当前格方向，带有平滑操作
std::optional<std::vector<Point>> PathFinder_FlowField::findPath(const Point& src)
{
    AXLOGD("start findPath1");
    Point pos = src;
    int tpos  = transPoint(pos);
    if (!withinBoundary(pos) || grid_index_[tpos] <= 0)
        return std::nullopt;

    int last_direction = 0;  // 用于平滑

    int idx = grid_index_[tpos];

    std::vector<Point> ret;
    ret.reserve(idx);
    ret.push_back(pos);

    AXLOGD("start findPath2");
    while (idx != 1)
    {
        // 已经寻迹过了
        if (grid_flow_[tpos] != DirectionType::UNDEF)
        {
            flowAlongTrace(pos, ret);
            break;
        }

        idx--;
        for (int i = 0; i < direct_num; i++)
        {
            // int nd   = (last_direction + i) % direct_num;
            int nd     = last_direction + i;
            Point npos = pos + neighbor_[nd];
            if (!withinBoundary(npos))
                continue;
            int tnpos = transPoint(npos);
            if (grid_index_[tnpos] == idx)
            {
                last_direction   = nd % direct_num;
                grid_flow_[tpos] = static_cast<DirectionType>(last_direction);
                ret.push_back(npos);
                pos  = npos;
                tpos = tnpos;
                break;
            }
        }
    }

    AXLOGD("start findPath4");
    return ret;
}

// 沿着以前的流迹快速找路
void PathFinder_FlowField::flowAlongTrace(const Point& src, std::vector<Point>& path)
{
    AXLOGD("start findPath3");
    auto pos = src;
    for (int i = grid_index_[transPoint(pos)] - 1; i > 0; i--)
    {
        int d = static_cast<int>(grid_flow_[transPoint(pos)]);
        pos   = pos + neighbor_[d];
        path.push_back(pos);
    }
}

void PathFinder_FlowField::resetField()
{
    AXLOGD("start reset");
    for (int i = 0; i < wtotal_; i++)
    {
        grid_index_[i] = grid_[i] ? 0 : -1;
        grid_flow_[i]  = DirectionType::UNDEF;
    }
    AXLOGD("finish reset");
}